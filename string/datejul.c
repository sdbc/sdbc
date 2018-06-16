/****************************** description *****************************/
/*Copyright (C), 2006-2009, EASYWAY. Co., Ltd.							*/
/*function:时间日期函数													*/
/*author:yulihua、huangpeng												*/
/*modify date:2009-4-21													*/
/************************************************************************/
#include <datejul.h>
#include <ctype.h>

static int CheckDate(short ymd[3]);
static int IsLeapYear(int year);
static int LeapNum(int yn);

static int ___not_dtime(char ch)
{
char *p;

	p = strchr("YyMmDdHhNnSsFf", ch);
	return (p == 0);
}

static char *___add_diff(char **buf,register char *cp1)
{
register	char *p;

	if(!cp1)
		return 0;
	p = *buf;
	while(*cp1 && ___not_dtime(*cp1))
		*p++ = *cp1++;
	*p = 0;
	*buf=p;

	return cp1;
}


/************************************************************************/
/* function:IsLeapYear 判断是否是闰年                                   */
/************************************************************************/
static int IsLeapYear(int year)
{
  if(!(year % 400)) 
	  return 1;
  else if(!(year % 100)) 
	  return 0;
  else if(!(year % 4)) 
	  return 1;
  else 
	  return 0;
}

static int LeapNum(int yn)
{
	return (yn / 4 - yn / 100 + (yn + 1900) / 400 - 4 - IsLeapYear(yn + 1900));
}

/************************************************************************/
/*function:CheckDate 检查日期(1900.1.1－3532.11.29)(596409)             */
/************************************************************************/
static const int month_days[12]={31,28,31,30,31,30,31,31,30,31,30,31};

static int CheckDate(register short ymd[3])
{
	int days;

    if((ymd[0] < 1900) || (ymd[0] >3532)) 
		return FORMATERR-101;
    if((ymd[1] < 1) || (ymd[1] >12)) 
		return FORMATERR-102;
    days = month_days[ymd[1]-1];
    if(ymd[1] == 2) 
		days += IsLeapYear(ymd[0]);
    if(ymd[2] > days) 
		ymd[2] = days;
    if(ymd[2] < 1) 
		return FORMATERR-103;
    
	return 0;
}

/************************************************************************/
/*function:rymdjul 将年月日数组转换为准儒略历数,1899.12.31至指定日的天数*/
/************************************************************************/
INT4 rymdjul(short ymd[3])
{
	int ccount;
	register INT4 TotalDays = 0;

    if(ymd[0]==1899 && ymd[1]==12 && ymd[2]==31) return 0;
    if(0!=(ccount = CheckDate(ymd))) 
		return ccount;
    TotalDays = (ymd[0] - 1900) * 365 + LeapNum(ymd[0]-1900);
    for (ccount = 1; ccount < ymd[1]; ccount++) 
		TotalDays += month_days[ccount-1];
    if(ymd[1] > 2) 
		TotalDays += IsLeapYear(ymd[0]);

    return (TotalDays + ymd[2]);
}

/************************************************************************/
/*function:rjulymd 将准儒略历数(距离1899.12.31的天数)转换为年月日数组	*/
/************************************************************************/
INT4 rjulymd(INT4  date,short ymd[3])
{
	int modday, i;

	ymd[0] = (short)(date / 365);
	i = LeapNum(ymd[0]);
	modday = date % 365 - i;
	if(modday < 0) {
		ymd[0]--;
		modday += 365 + IsLeapYear(ymd[0] + 1900);
	}
	ymd[0] += 1900;
	if(modday <= 0) {
		ymd[0]--;
		ymd[1] = 12;
		ymd[2] = 31 + modday;     
		if(ymd[2] <= 0) 
			return FORMATERR-101;
		return 0;
	}
 	ymd[1] = 1;
	for(i = 0; i < 12; i++) {
		int day;
		day = modday;
		modday -= month_days[i];
		if(i == 1) 
			modday -= IsLeapYear(ymd[0]);
		if(modday <= 0) 
		{
			ymd[2] = day;
			return 0;
		}
		ymd[1]++;
	}
	return 0;
}

static int get_digit(char **cp,int n)
{
int i,num;
	num=0;
	for(i=0;i<n;i++) {
		if(!isdigit(**cp)) break;
		num *= 10;
		num += **cp - '0';
		(*cp)++;
	}
	return num;
}
/************************************************************************/
/* function:rstrfmttotime 将字符串的年月日时分秒转换成准儒略历数	*/
/*description:fmt(YYYY-MM-DD HH24:MI:SS或YYYY-MM-DD HH:NN:SS.FF6等格式)	*/
/*可以任意颠倒次序，可以大写或小写，可以没有分隔符。分隔符可以是汉字	*/
/*返回：准儒略历数(以天为单位)，hms中有时分秒 usec 里是微秒		*/
/************************************************************************/
INT4 rstrfmttotime(char *str,const char *fmt,short hms[3],int *usec)
{
char *cp, *cp1;
short ymd[3];
char buffer[23];
char *scp;
int i,j,f24,ten;
long f;

	cp = cp1 = (char *)fmt;
	scp = str;
	hms[0] = hms [1] = hms[2] = 0;
	ymd[0] = 1900;
	ymd [1] =1;
	ymd[2] = 1;
	while(*cp && (*cp != ' ')) {
	int scp_siz=0;
	char *dcp;
		f24 = 0;
		if(usec) *usec=0;
		switch(*cp) {
		case 'y':
		case 'Y':
			while(*cp == *cp1)
				cp1++;
			scp_siz=cp1-cp;
			strncpy(buffer, scp, scp_siz);
			buffer[scp_siz] = 0;
			ymd[0] = (short)atoi(buffer);
			if(ymd[0] < 70) 
				ymd[0] += 2000;
			else if(ymd[0] < 200) 
				ymd[0] += 1900;
			else if(ymd[0] < 700) 
				ymd[0] += 2000;
			else if(ymd[0] < 1000) 
				ymd[0] += 1000;
			scp_siz=0;
			break;
		case 'm':
		case 'M':
			if((cp[1] == 'I') || (cp[1] == 'i')) {
				cp1 += 2;
				goto minuts;
			}
			while(*cp == *cp1) cp1++;
			scp_siz=cp1-cp;
			strncpy(buffer, scp, scp_siz);
			buffer[scp_siz] = 0;
			ymd[1] = (short)strtol(buffer,&dcp,10);
			scp_siz -= (int)(dcp - buffer);
			break;
		case 'd':
		case 'D':
			while(*cp == *cp1)
				cp1++;
			scp_siz=cp1-cp;
			strncpy(buffer, scp, scp_siz);
			buffer[scp_siz] = 0;
			ymd[2] = (short)strtol(buffer,&dcp,10);
			scp_siz -= (int)(dcp - buffer);
			break;
		case 'H':
		case 'h':
			while(*cp == *cp1)
				cp1++;
			scp_siz=cp1-cp;
			if(!strncmp(cp1, "24", 2)) 
				cp1 += 2, f24 = 2;
			strncpy(buffer, scp, scp_siz);
			buffer[scp_siz] = 0;
			hms[0] = (short)strtol(buffer,&dcp,10);
			scp_siz -= (int)(dcp - buffer);
			break;
		case 'N':
		case 'n':
			while(*cp == *cp1)
				cp1++;
minuts:
			scp_siz=cp1-cp;
			strncpy(buffer, scp, scp_siz);
			buffer[scp_siz] = 0;
			hms[1] = (short)strtol(buffer,&dcp,10);
			scp_siz -= (int)(dcp - buffer);
			break;
		case 'S':
		case 's':
			while(*cp == *cp1)
				cp1++;
			scp_siz=cp1-cp;
			strncpy(buffer, scp, scp_siz);
			buffer[scp_siz] = 0;
			hms[2] = (short)strtol(buffer,&dcp,10);
			scp_siz -= (int)(dcp - buffer);
			break;
		case 'F':
		case 'f':
			scp_siz=0;
			while(*cp == *cp1) cp1++;
			f=strtol(cp1,&cp1,10)%7;
			if(!f) break;
			if(!usec) break;
			*usec=get_digit(&scp,f);
//printf("%s:usec=%d\n",__FUNCTION__,*usec);
			j=6-f;
			ten=1;
			if(j>0) {
				for(i=0;i<j;i++) ten*=10;	
				*usec *= ten;
			}
			break;

		default:
			continue;
		}
		if(!*cp1) break;
		while(*cp1 && ___not_dtime(*cp1)) 
			cp1++;
		scp += (int)(cp1-cp) - f24 - scp_siz;
		cp = cp1;
	}
	return rymdjul(ymd);
}

/************************************************************************/
/* function:rtimetostrfmt将日期型转换成字符串的年月日时分秒             */
/*description:fmt(YYYY-MM-DD HH24:MI:SS或YYYY-MM-DD HH:NN:SS.FF6等格式)		*/
/*可以任意颠倒次序，可以大写或小写，可以没有分隔符。分隔符可以是汉字	*/
/************************************************************************/
char *rtimetostrfmt(char *buffer,const char *fmt,short ymd[3],short hms[3],int usec)
{
int i,j,ten;
long f;
register char  *cp;
char *cp1,*p;

	buffer[0] = 0;
	cp1 = cp = (char *)fmt;
	p=buffer;
	while(*cp)
	{
		switch(*cp)
		{
		case 'y':
		case 'Y':
			while(*cp == *cp1)
				cp1++;
			i = 4 - (cp1 - cp);
			p+=sprintf(p, "%04d", ymd[0]);
			if(i > 0) 
				strsubst(p, i, 0);		/*strproc.c*/
			break;
		case 'm':
		case 'M':
			if((cp[1] == 'I') || (cp[1] == 'i')) 
			{
				cp1+=2;
				goto min;
			}
			if(*cp != cp[1]) 
			{
				*p++ = *cp1++;
				*p = 0;
				break;
			}
			cp1 += 2;
			p+=sprintf(p, "%02d", ymd[1]);
			break;
		case 'd':
		case 'D':
			if(*cp != cp[1]) 
			{
				*p++ = *cp1++;
				*p = 0;
				break;
			}
			cp1 += 2;
			p+=sprintf(p, "%02d", ymd[2]);
			break;
		case 'H':
		case 'h':
			if(*cp != cp[1]) 
			{
				*p++ = *cp1++;
				*p = 0;
				break;
			}
			cp1 += 2;
			if(!strncmp(cp1, "24", 2)) 
				cp1 += 2;
			p+=sprintf(p, "%02d", hms[0]);
			break;
		case 'N':
		case 'n':
			if(*cp != cp[1]) 
			{
				*p++ = *cp1++;
				*p++ = *cp1++;
				*p = 0;
				break;
			}
			cp1 += 2;
min:
			p+=sprintf(p, "%02d", hms[1]);
			break;
		case 'S':
		case 's':
			if(*cp != cp[1]) 
			{
				*p++ = *cp1++;
				*p = 0;
				break;
			}
			cp1 += 2;
			p+=sprintf(p, "%02d", hms[2]);
			break;
		case 'F': // TIMESTAMP
		case 'f':
			if(*cp != cp[1]) 
			{
				*p++ = *cp1++;
				*p = 0;
				break;
			}
			cp1 += 2;
			f=strtol(cp1,&cp1,10)%7;
			j=6-f;
			ten=1;
			if(j>0) {
				for(i=0;i<j;i++) ten*=10;	
			}
			p+=sprintf(p,"%0*d",(int)f,usec/ten);
			break;

		default:
			*p++ = *cp++;
			*p = 0;
			cp1++;
			break;
		}
		cp1 = ___add_diff(&p, cp1);
		cp = cp1;
	}
	return(buffer);
}

/************************************************************************/
/*function:rstrfmttojul 将年月日字符串按指定格式转换为准儒略历数        */
/*description:fmt(YYYY-MM-DD等格式)										*/
/*可以任意颠倒次序，可以大写或小写，可以没有分隔符。分隔符可以是汉字	*/
/************************************************************************/
INT4 rstrfmttojul(char *str,const char *fmt)
{
	short shm[3];

	return rstrfmttotime(str, fmt, shm,0);
}

/************************************************************************/
/*function:rjultostrfmt 将准儒略历数按指定格式转换为年月日字符串        */
/*description:fmt(YYYY-MM-DD等格式)										*/
/*可以任意颠倒次序，可以大写或小写，可以没有分隔符。分隔符可以是汉字	*/
/************************************************************************/
char *rjultostrfmt(char *buffer,INT4  dat,const char *fmt)
{
	int i;
	short ymd[3];
	short hms[3];

	i = rjulymd(dat,ymd);
	if(i) 
	{
		sprintf(buffer, "Date err %d", i);
		return 0;
	}
	buffer[0] = 0;
	hms[0] = hms[1] = hms[2] = 0;

	return rtimetostrfmt(buffer, fmt, ymd, hms,i);
}

/************************************************************************/
/*function:SetDefaultDateFormat 设置默认日期格式				        */
/*description:如果没有设置，则默认格式为YYYY.MM.DD						*/
/************************************************************************/
static char DefaultDateFormat[] = "YYYY.MM.DD";
static char  *DateFormat = DefaultDateFormat;

char *SetDefaultDateFormat(char *format)
{
	char *old;

	old = DateFormat;
	if(format) 
		DateFormat = format;
	else 
		DateFormat = DefaultDateFormat;

	return old;
}

/************************************************************************/
/*function:rjulstr 将准儒略历数按最后设置默认的格式转换为年月日字符串	*/
/*description:缺省默认格式为YYYY.MM.DD									*/
/************************************************************************/
char *rjulstr(char *str,INT4 jul)
{
	return rjultostrfmt(str, jul, DateFormat);
}

/************************************************************************/
/*function:rstrjul 将最后设置默认格式的年月日字符串转换为准儒略历数		*/
/*description:缺省默认格式为YYYY.MM.DD									*/
/************************************************************************/
INT4 rstrjul(char *str)
{
	return rstrfmttojul(str, DateFormat);
}

/************************************************************************/
/*function:rstrminfmt 将年月日时分按格式转换成准儒略历数(以分钟为单位)	*/
/*description:fmt(YYYY-MM-DD HH24:MI或YYYY-MM-DD HH:NN等格式)			*/
/*可以任意颠倒次序，可以大写或小写，可以没有分隔符。分隔符可以是汉字	*/
/************************************************************************/
INT4 rstrminfmt(char *year_to_min,const char *fmt) 
{
	short hms[3];
	INT4  minuts;

	minuts = rstrfmttotime(year_to_min, fmt, hms,0);
	minuts *= 1440;
	minuts += (hms[0] * 60 + hms[1] + timezone / 60);

	return minuts;
}

/************************************************************************/
/*function:rminstrfmt 将准儒略历数(以分钟为单位)按格式转换成年月日时分	*/
/*description:fmt(YYYY-MM-DD HH24:MI或YYYY-MM-DD HH:NN等格式)			*/
/*可以任意颠倒次序，可以大写或小写，可以没有分隔符。分隔符可以是汉字	*/
/************************************************************************/
char *rminstrfmt(char *year_to_min,INT4  minuts,const char *fmt)
{
	short ymd[3], hms[3];
	INT4  day;

	minuts -= (timezone / 60);
	day = minuts / 1440;
	minuts %= 1440;
	hms[0] = (short)(minuts / 60);
	hms[1] = (short)(minuts % 60);
	hms[2] = 0;
	rjulymd(day,ymd);

	return rtimetostrfmt(year_to_min, fmt, ymd, hms,0);
}

/************************************************************************/
/*function:rstrmin 将本地的年月日时分按格式转换成GREENWITH(以分钟为单位)*/
/*description:fmt(默认为YYYY.MM.DD HH24:MI格式)							*/
/************************************************************************/
static char *minuts_fmt = "YYYY.MM.DD HH24:MI";
INT4 rstrmin(char *year_to_min) 
{
	return rstrminfmt(year_to_min, minuts_fmt);
}

/************************************************************************/
/*function:rminstr 将GREENWITH(以分钟为单位)按格式转换成本地的年月日时分*/
/*description:fmt(默认为YYYY.MM.DD HH24:MI格式)							*/
/************************************************************************/
char *rminstr(char *year_to_min,INT4  minuts)
{
	return rminstrfmt(year_to_min, minuts, minuts_fmt);
}

/************************************************************************/
/* function:rstrsecfmt 将字符串的年月日时分秒转换成准儒略历数			*/
/*description:fmt(YYYY-MM-DD HH24:MI:SS或YYYY-MM-DD HH:NN:SS等格式)		*/
/*可以任意颠倒次序，可以大写或小写，可以没有分隔符。分隔符可以是汉字	*/
/*准儒略历数(以秒为单位)												*/
/************************************************************************/
INT64 rstrsecfmt(char *str,const char *fmt)
{
	short hms[3];
	INT4  day;
	INT64 sec;

	day = rstrfmttotime(str, fmt, hms,0);
	sec = (INT64)day * 86400;
	sec += (hms[0] * 60 + hms[1]) * 60 + hms[2] + timezone;

	return sec;
}
/************************************************************************/
/* function:rstrusecfmt 将字符串的年月日时分秒转换成准儒略历微秒数	*/
/*description:fmt(YYYY-MM-DD HH24:MI:SS或YYYY-MM-DD HH:NN:SS等格式)	*/
/*可以任意颠倒次序，可以大写或小写，可以没有分隔符。分隔符可以是汉字	*/
/*返回：准儒略历数(以微秒为单位)						*/
/************************************************************************/
INT64 rstrusecfmt(char *str,const char *fmt)
{
short hms[3];
INT4  day,u;
INT64 usec;

	day = rstrfmttotime(str, fmt, hms, &u);
	usec = (INT64)day * 86400;
	usec += (hms[0] * 60 + hms[1]) * 60 + hms[2] + timezone;

	return usec*1000000 + u;
}

/************************************************************************/
/* function:rsecstrfmt 将准儒略历数转换成字符串的年月日时分秒			*/
/*description:fmt(YYYY-MM-DD HH24:MI:SS或YYYY-MM-DD HH:NN:SS等格式)		*/
/*可以任意颠倒次序，可以大写或小写，可以没有分隔符。分隔符可以是汉字	*/
/*准儒略历数(以秒为单位)												*/
/************************************************************************/
char *rsecstrfmt(char *buf,INT64 sec,const char *fmt)
{
	short hms[3], ymd[3];
	INT4  day;
	int i;

	sec -= timezone;
	day = (INT4 )(sec / 86400);
	i = (int)(sec % 86400);
	rjulymd(day, ymd);
	hms[2] = i % 60;
	i /= 60;
	hms[1] = i % 60;
	hms[0] = i / 60;

	return rtimetostrfmt(buf, fmt, ymd, hms,0);
}

/************************************************************************/
/* function:rusecstrfmt 将准儒略历微秒数转换成字符串的年月日时分秒.微秒	*/
/*description:fmt(YYYY-MM-DD HH24:MI:SS.FF6或YYYY-MM-DD HH:NN:SS.FF6等格式)*/
/*可以任意颠倒次序，可以大写或小写，可以没有分隔符。分隔符可以是汉字	*/
/*返回：准儒略历数(以微秒为单位)					*/
/************************************************************************/
char *rusecstrfmt(char *buf,INT64 usec,const char *fmt)
{
short hms[3], ymd[3];
INT4  day;
int u,i;
INT64 sec;

	u=usec%1000000;
	sec=usec/1000000;
	sec -= timezone;
	day = (INT4 )(sec / 86400);
	i = (int)(sec % 86400);
	rjulymd(day, ymd);
	hms[2] = i % 60;
	i /= 60;
	hms[1] = i % 60;
	hms[0] = i / 60;

	return rtimetostrfmt(buf, fmt, ymd, hms,u);
}


/************************************************************************/
/*function:cvtdate 相对日期转换函数,返回准儒略历数						*/
/*description:															*/
/*基本形式:str(YYYY.MM.DD), refday:参考日期								*/
/*如果哪一段缺省,以参考日期的相应数字代替								*/
/*DD = 31,将被参考日期月底日取代										*/
/*哪一段以+ -开头，相对参考日期的相应数字运算							*/
/*L代表月底(包括2月的28或29日,以及其它月的30日、31日)					*/
/*例：																	*/
/*  .代表当日，..代表本月当日。											*/
/*  +1代表明天，-1代表昨天, .31或31或.L代表本月月底。					*/
/*  .-1.或-1. 代表上月同日，-1.31代表上月月底。							*/
/*  -1.1.1代表去年1月1日。 -1..代表去年本月当日。-1.2.31代表去年2月底	*/
/*  2009.1.1代表2009年1月1日。.1.1或1.1代表今年1月1日。.1或1代表本月1日 */
/************************************************************************/
INT4 cvtdate(char *str,INT4 refday)
{
	INT4 day = 0;
	int cc;
	char s1[20], s2[20], s3[20], *p;
	static char sepr[] = "./";
	short ymd[3];

	if(!str || !(*str)) 
	{
		return (refday);
	}

	while(*str && (*str <= ' ')) 
		str++;
	if(*str == '\\') 
		str++;
	if(!*str) 
		return (refday);
	if(!strpbrk(str,"+-L ")) 
	{
		day = rstrjul(str);
		if(day > 0) 
			return (day); 
	}
	if(*(p = stptok(str, s1, sizeof(s1), sepr)))		/*./utils/StrProc.c*/
	{
 		p++;
		if(*p == '\\') 
			p++;
		if(*(p = stptok(p, s2, sizeof(s2), sepr))) 
		{
			/* YYYY.MM.DD */
			p++;
			if(*p == '\\') 
				p++;
			strcpy(s3,p);
dat3:
			cc = 0;
			if(!(*s1) && !(*s2) && !(*s3)) 
				return (refday);

			/* DAY */
			if((*s3 == '+') || (*s3=='-')) 
			{
				sscanf(s3, "%d", &cc);
				day = refday + cc;
				cc = 0;
			}
			else if(isdigit(*s3)) 
			{
				sscanf(s3, "%d", &cc);
				day = refday;
			}
			else 
				day = refday;
			rjulymd(day, ymd);
			if(cc) 
			{
				ymd[2] = cc;
				cc = 0;
			}

			/* MON */
			if((*s2 == '+') || (*s2 == '-')) 
			{
				sscanf(s2, "%d", &cc);
				ymd[1] += cc;
				if(ymd[1] <= 0) 
				{
					while(ymd[1] <= 0) 
					{
						ymd[0]--;
						ymd[1] += 12;
					}
				}
				else if(ymd[1] > 12) 
				{	
					while(ymd[1] > 12) 
					{
						ymd[0]++;
						ymd[1] -= 12;
					}
				}
			}
			else if(isdigit(*s2)) 
				sscanf(s2, "%hd", &ymd[1]);
			else ;
			while(ymd[1] <= 0) 
				ymd[1] += 12, ymd[1]--;
			while(ymd[1] > 12) 
				ymd[1] -= 12, ymd[1]++;
			cc = 0; 

			/* YEAR */
			if(ymd[0] < 70) 
				ymd[0] += 2000;
			else if(ymd[0] < 100) 
				ymd[0] += 1900;
			if((*s1 == '+') || (*s1 == '-')) 
			{
				sscanf(s1, "%d", &cc);
				ymd[0] += cc;
			}
			else if(isdigit(*s1)) 
			{
				sscanf(s1, "%hd", &ymd[0]);
				if(ymd[0] < 71) 
					ymd[0] += 2000;
				else if(ymd[0] < 100) 
					ymd[0] += 1900;
			} 
			else ;
			day = rymdjul(ymd);
			if(*s3 == 'L') 
				return (mon_end(day));
			return (day);
		}

		/*  MM.DD */
		strcpy(s3, s2);
		strcpy(s2, s1);
		*s1 = 0;
		goto dat3;
	}

	/* DD */
	if((*str == '+') || (*str == '-')) 
	{
		sscanf(str, "%d", &day);
		return (refday + day);
	}
	else 
	{
		*s1 = 0;
		*s2 = 0;
		strcpy(s3, str);
		if(*s3 == 'L') 
			return (mon_end(refday));
		goto dat3 ;
	}
}

/************************************************************************/
/*function:yday 年时间戳函数											*/
/*description:															*/
/*计算指定的准儒略历数对应的年月日距离该年1月1日(包含该天)的天数		*/
/************************************************************************/
INT4 yday(INT4 day)
{
	short ymd[3];
	INT4  day1;

	if(day < 366) 
		return day;
	rjulymd(day, ymd);
	ymd[1] = 1;
	ymd[2] = 1;
	day1 = rymdjul(ymd);
	return (day - day1 + 1);
}

/************************************************************************/
/*function:jday 季度时间戳函数											*/ 
/*description:															*/
/*计算指定的准儒略历数对应的年月日距离每一季度(1、4、7、10月1日(包含	*/
/*该天))的天数															*/
/************************************************************************/
INT4 jday(INT4 day)
{
	short ymd[3];
	INT4  day1;

	rjulymd(day, ymd);
	ymd[1] = (ymd[1] - 1) / 3 * 3 + 1;
	ymd[2] = 1;
	day1 = rymdjul(ymd);
	return (day - day1 + 1);
}

/************************************************************************/
/*function:mon_end 月底时间戳函数										*/
/*description:															*/
/*计算指定的准儒略历数对应月的月底距离1899.12.31的天数					*/
/************************************************************************/
INT4 mon_end(INT4 day) 
{
	short ymd[3];

	rjulymd(day, ymd);
	ymd[2] = 31;

	return rymdjul(ymd);
}

/************************************************************************/
/*function:mday	每月天数函数											*/
/*description:计算指定的准儒略历数对应月的总天数						*/
/************************************************************************/
INT4 mday(INT4 day) 
{
	short ymd[3];

	rjulymd(mon_end(day), ymd);

	return (ymd[2]);
}

/************************************************************************/
/*function:dday	月时间戳函数											*/
/*description:计算指定的准儒略历数对应月距离该月1日的天数				*/
/************************************************************************/
INT4 dday(INT4 day) 
{
	short ymd[3];

	rjulymd(day, ymd);

	return (ymd[2]);
}

