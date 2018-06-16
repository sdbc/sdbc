/****************************************************
 * For SDBC 4.0
 * 这是SDBC的数据包装和拆包程序
 * 本程序完成了串行数据对C语言结构的映射。
 ***************************************************/

/* net_pack.c for ORACLE */

#include <pack.h>
#include <ctype.h>

const char YEAR_TO_DAY[]="YYYY-MM-DD";
const char YEAR_TO_MIN[]="YYYY-MM-DD HH24:MI";
const char YEAR_TO_SEC[]="YYYY-MM-DD HH24:MI:SS";
const char YEAR_TO_USEC[]="YYYY-MM-DD HH24:MI:SS.FF6";

T_PkgType tpl_tpl[] = {
        {CH_INT4,sizeof(int),"type",0,-1},
        {CH_INT4,sizeof(int),"len"},
        {CH_CLOB,-1,"name"},
        {CH_CLOB,-1,"format"},
        {-1,0,NULL,NULL}
};

/*char *malloc(int);*/
#define TIMENULL INTNULL
#define ESC_CHAR '\\'

#ifdef WIN32
#define FMT64 "%I64d"
#else
#       if __WORDSIZE == 64
#               define FMT64 "%ld"
#       else
#               define FMT64 "%lld"
#       endif
#endif

/* for ORACLE */
T_PkgType SqlVarType[]={
        {CH_CHAR,49,"sqlname",0,-1},
        {CH_INT4,sizeof(int),"sqltype"},
        {CH_INT4,sizeof(int),"sqllen"},
        {CH_CHAR,49,"sqlformat"},
        {-1,0,0,0}
};

T_PkgType SqlDaType[]={
        {CH_INT4,4,"cursor_no",0,-1},
        {CH_INT4,4,"cols"},
//SqlVarType,等将来支持结构列  
        {-1,0,0,0}
};

	
int strcpy_esc(char *dest,char *src,int len,char CURDLM)
{
char *p;
int i;
	p=src;
	if(len<=0) len=strlen(src)+1;
	if(!CURDLM) {
		p=stptok(src,dest,len,0);
		return p-src;
	}
	for(i=0;*p&&i<len-1;i++) {
		if(*p != ESC_CHAR) {
			*dest++ = *p++;
			continue;
		}
		if(p>src && GBK_flag &&  firstcc((unsigned char *)src,(unsigned char *)p-1)) {
			*dest++ = *p++;
			continue;
		}
		p++;
		switch(*p) {
		case 0:
			continue;
		case 'n':
			*dest++ = '\n';
			p++;
			continue;
		case 'G':
			*dest++ = CURDLM;
			p++;
			continue;
		default:
			*dest++ = *p++;
			continue;
		}
	}
	*dest=0;
	return i;
}

static int b2h(char *hex,char *bin,int len)
{
register char *hp=hex,*bp=bin;
int i;
	for(i=0;i<len;i++) 
		hp+=sprintf(hp,"%02X",255&*bp++);	
	return hp-hex;
}
/* EBCDIC码不能这么用   
   */
static char h_b(char *p)
{
char c;
	c=*p;
	c=toupper(c);
	c-='0';
	if(c<0) return 0;
	if(c>9) c -= 'A' - '9' - 1;
	return c&0Xf;
}

static char h2b(char *p)
{
char c=0;
	if(!p||!*p) return 0;
	c=h_b(p);
	c<<=4;
	p++;
	if(*p) c+=h_b(p);
	return c;
}

int byte_cpy(char *dest,char *src,int destlen)
{
register char *p,*p1;
int i;
	p=src;
	p1=dest;
	for(i=0;i<destlen;i++) {
		if(!*p) break;
		*p1++ = h2b(p);
		if(*(++p)) p++;
	}
	
	return p-src;
}

int get_one_str(char *buf,void *data,T_PkgType *typ,char CURDLM)
{
register char *cp1,*cp2;
int cnt,J,len;
char datebuf[31];
char *sp;
//short iTiny;
T_PkgType Char_Type[2];

	cp1=buf;
	*cp1=0;
	cnt=0;
	cp2 = (char *)data+typ->offset;
	sp=cp2;
	if(isnull(cp2,typ->type)) return cnt;
	switch(typ->type) {
	case CH_BYTE:
		cnt=b2h(cp1,cp2,typ->len);
		break;
	case CH_CLOB:
		Char_Type[0].type=CH_CHAR;
		Char_Type[0].len=-1;
		Char_Type[0].offset=0;
		Char_Type[1].type=-1;
		Char_Type[1].len=0;
		J=get_one_str(buf,*(char **)cp2,Char_Type,CURDLM);
		cnt += J;
		break;
	case CH_DATE:
	case CH_CNUM:
	case CH_CHAR:
		len=(typ->len>0)?typ->len-1:strlen(cp2);
		for(J=0;J<len&&*cp2;J++,cnt++) {
			if(!CURDLM) goto norm;
			switch(*cp2) {
			case ESC_CHAR:
				if(cp2>sp && GBK_flag && firstcc((unsigned char *)sp,(unsigned char *)cp2-1)) goto norm;
				*cp1++=*cp2;
				*cp1++=*cp2++;
				cnt++;
				break;
			case '\n':
				if(cp2>sp && firstcc((unsigned char *)sp,(unsigned char *)cp2-1)) cp1[-1]&=0x7f;
				*cp1++=ESC_CHAR;
				*cp1++='n';
				cp2++;
				cnt++;
				break;
			default:
				if(*cp2==CURDLM) {
					if(cp2>sp && GBK_flag && firstcc((unsigned char *)sp,(unsigned char *)cp2-1))
						goto norm;
					*cp1++=ESC_CHAR;
					*cp1++='G';
					cp2++;
					cnt++;
					break;
				}
norm:
				*cp1++=*cp2++;
				break;
			}
		}
		*cp1=0;
		if(cp2>sp) {
			if(firstcc((unsigned char *)sp,(unsigned char *)cp2-1)) cp1[-1] &= 0x7f;
		}
		break;
	case CH_FLOAT:
		if(!typ->format)
		 cnt=sprintf(cp1,"%g", (double)*(float *)cp2);
		else
		 cnt=sprintf(cp1,typ->format,(double)*(float *)cp2);
		break;
	case CH_DOUBLE:
		if(!typ->format)
		 cnt=sprintf(cp1,"%g", *(double *)cp2);
		else
		 cnt=sprintf(cp1,typ->format,*(double *)cp2);
		break;
	case CH_LDOUBLE:
		if(!typ->format)
		 cnt=sprintf(cp1,"%Lg", *(long double *)cp2);
		else
		 cnt=sprintf(cp1,typ->format,*(long double *)cp2);
		break;
	case CH_TINY:
		if(typ->format) cnt=sprintf(cp1,typ->format,255&*cp2);
		else cnt=itoStr((int)(*cp2),cp1)-cp1;
		break;
	case CH_SHORT:
		if(typ->format) cnt=sprintf(cp1,typ->format,0XFFFF&*(short *)cp2);
		else cnt=itoStr((int)(*(short *)cp2),cp1)-cp1;
		break;
	case CH_INT:
		if(typ->format) cnt=sprintf(cp1,typ->format,*(int *)cp2);
		else cnt=itoStr(*(int *)cp2,cp1)-cp1;
		break;
	case CH_LONG: 
		if(typ->format) cnt=sprintf(cp1,typ->format,*(long *)cp2);
		else cnt=lltoStr((INT64)(*(long *)cp2),cp1)-cp1;
		break;
	case CH_INT64: 
		if(typ->format) cnt=sprintf(cp1,typ->format,*(INT64 *)cp2);
		else cnt=lltoStr(*(INT64 *)cp2,cp1)-cp1;
		break;
	case CH_CJUL:
	case CH_JUL:
		if(typ->format) {
			 rjultostrfmt(datebuf,*(int *)cp2,
				    typ->format);
		} else {
			 rjultostrfmt(datebuf,*(int *)cp2,
				    "YYYYMMDD");
		}
		cnt=sprintf(cp1,"%s",datebuf);
		break;
	case CH_MINUTS:
	case CH_CMINUTS:
		if(typ->format) {
			rminstrfmt(datebuf,*(INT4 *)cp2,typ->format);
		} else rminstr(datebuf,*(INT4 *)cp2);
		cnt=sprintf(cp1,"%s",datebuf);
		break;
	case CH_TIME:
	case CH_CTIME:
		if(typ->format) {
			rsecstrfmt(datebuf,*(INT64 *)cp2,typ->format);
		} else rsecstrfmt(datebuf,*(INT64 *)cp2,"YYYYMMDDHH24MISS");
		cnt=sprintf(cp1,"%s",datebuf);
		break;
	case CH_USEC:
		if(typ->format) {
			rusecstrfmt(datebuf,*(INT64 *)cp2,typ->format);
		} else rusecstrfmt(datebuf,*(INT64 *)cp2,"YYYYMMDDHH24MISS.FF6");
		cnt=sprintf(cp1,"%s",datebuf);
		break;
	default:
		break;
	}
	return cnt;
}
/************************************************************
 * get data from struct(data) to string(buf) "..|..|" 
 ************************************************************/

int pkg_pack(char *buf,void *net_struct,T_PkgType *pkg_type,char delimit)
{
char *cp;
register char *cp1;
register T_PkgType *typ;

	cp=buf;
	if(!cp)return -1;
	cp1=cp;
	typ=pkg_type;
	if(typ->offset<0) set_offset(pkg_type);// at netvar.c
	for(;typ->type>-1;typ++) {
		if(typ->bindtype&NOSELECT) continue;
		if(typ->type==CH_STRUCT) {
			cp1+=pkg_pack(cp1,(char *)net_struct+typ->offset,(T_PkgType *)typ->format,delimit);
			continue;
		}
		cp1+=get_one_str(cp1,net_struct,typ,delimit);
		*cp1++=delimit;
		*cp1=0;
	}
	return (cp1-cp);
}

/************************************************************
 * put data from  string(cp) "..|..|" into struct(buf)
 ************************************************************/

int put_str_one(void *buf,char *cp,T_PkgType *typ,char CURDLM)
{
int ret;
register char *cp1;

	cp1=typ->offset+(char *)buf;	
	ret=0;
	switch(typ->type) {
		case CH_CLOB:
			*(char **)cp1=cp;
			if(CURDLM) strcpy_esc(cp,cp,-1,CURDLM); //有问题 
			typ->len=strlen(cp);
			ret=1;
			break;
		case CH_BYTE:
			byte_cpy(cp1,cp,typ->len);
			ret=1;
			break;
		case CH_DATE:
		case CH_CNUM:
		case CH_CHAR:
			*cp1=0;
			strcpy_esc(cp1,cp,typ->len,CURDLM);
			ret=1;
			break;
		case CH_FLOAT:
			*(float *)cp1=strtof(cp,&cp);;
			ret=1;
			break;
		case CH_DOUBLE:
			*(double *)cp1=strtod(cp,&cp);;
			ret=1;
			break;
		case CH_LDOUBLE:
			*(long double *)cp1=strtold(cp,&cp);
			break;

		case CH_JUL:
		case CH_CJUL:
			if(!*cp) *(INT4 *)cp1=TIMENULL;
			else if(typ->format){
			    *(int *)cp1=rstrfmttojul(cp,
					typ->format);
			} else {
			    *(int *)cp1= rstrjul(cp);
			}
			ret=1;
			break;
		case CH_MINUTS:
		case CH_CMINUTS:
			if(!*cp) *(INT4 *)cp1=TIMENULL;
			else if(typ->format){
			    *(INT4 *)cp1=rstrminfmt(cp,typ->format);
			} else *(INT4 *)cp1=rstrmin(cp);
			ret=1;
			break;
		case CH_TIME:
		case CH_CTIME:
			if(!*cp) *(INT64 *)cp1=INT64NULL;
			else if(typ->format){
			    *(INT64 *)cp1=rstrsecfmt(cp,typ->format);
			} else *(INT64 *)cp1=rstrsecfmt(cp,"YYYYMMDDHH24MISS");
			ret=1;
			break;
		case CH_USEC:
			if(!*cp) *(INT64 *)cp1=INT64NULL;
			else if(typ->format){
			    *(INT64 *)cp1=rstrusecfmt(cp,typ->format);
			} else *(INT64 *)cp1=rstrusecfmt(cp,"YYYYMMDDHH24MISS.FF6");
			ret=1;
			break;
		case CH_TINY:
		    {
int tmp;
			if(!*cp) {
				*cp1=TINYNULL;
				break;
			}
			tmp=atoi(cp);
			*cp1=(char)tmp;
			break;
		    }
		case CH_SHORT:
			*(short *)cp1=SHORTNULL;
			ret=sscanf(cp,"%hd",(short *)cp1);
			ret=1;
			break;
		case CH_INT:
			if(!*cp) {
				*(int *)cp1=INTNULL;
				break;
			}
			*(int *)cp1=atoi(cp);
			break;
		case CH_LONG:
			if(!*cp) {
				*(long *)cp1=LONGNULL;
				break;
			}
			*(long *)cp1=strtol(cp,&cp,10);
			break;
		case CH_INT64:
			if(!*cp) {
				*(INT64 *)cp1=INT64NULL;
				break;
			}
			*(INT64 *)cp1=strtoll(cp,&cp,10);
			break;
		default:
			ret=0;
			break;
	}
	return ret;
}

int pkg_dispack(void *net_struct,char *buf,T_PkgType *pkg_type,char delimit)
{
char *cp;
register char *cp1;
char dml[2];
T_PkgType *typ;

	*dml=delimit;
	dml[1]=0;
	cp=buf;
	if(!cp||!*cp) return 0;
	if(pkg_type->offset<0) set_offset(pkg_type);
	for(typ=pkg_type;typ->type>-1;typ++){
		if(typ->bindtype&NOSELECT) continue;
		if(typ->type==CH_STRUCT) {
			cp+=pkg_dispack((char *)net_struct+typ->offset,cp,(T_PkgType *)typ->format,delimit);
			continue;
                }
		cp1=cp;
		cp=stptok(cp,0,0,dml);
		if(*cp==delimit) *cp++=0;
		put_str_one(net_struct,cp1,typ,delimit);
		if(!*cp) break;
	}
	return (cp-buf);
}
/*******************************************
 * 找别名
 *******************************************/
const char *plain_name(const char *name)
{
char *p;
	if(!name) return name;
	p=strrchr(name,' ');
	if(!p) return name;
	else return ++p;
}

int pkg_getnum(const char *key,register T_PkgType *type)
{
int i;
	for(i=0;type->type >= 0 ;i++,type++) {
		if(!strcmp(key,plain_name(type->name))) {
			break;
		}
	}
	return i;
}
T_PkgType * pkg_getType(const char *key,register T_PkgType *type)
{
int i;
	for(i=0;type->type >= 0 ;i++,type++) {
		if(!strcmp(key,plain_name(type->name))) {
			break;
		}
	}
	return type;
}

char *getitem_idx(char *buf,void *data,T_PkgType *pkg_type,const char *key,const char *colidx,int colnum)
{
int k;

	if(pkg_type->offset<0) set_offset(pkg_type);
	k=index_col(colidx,colnum,key,pkg_type);
	if(k<0) return 0;
	k=get_one_str(buf,data,pkg_type+k,0);
	return buf+k;
}
int putitem_idx(void *buf,char *cp,T_PkgType *pkg_type,const char *key,const char *colidx,int colnum)
{
int k;

	if(pkg_type->offset<0) set_offset(pkg_type);
	k=index_col(colidx,colnum,key,pkg_type);
	if(k<0) return 0;
	k=put_str_one(buf,cp,pkg_type+k,0);
	return k;
}
int data_init(void *data,T_PkgType *type)
{
int i,j;
char *cp;
	if(!data) return -1;
	if(type->offset<0) i=set_offset(type);
	for(j=0;type[j].type >= 0;j++) {
		cp=(char *)data+type[j].offset;
		switch(type[j].type) {
		case CH_DOUBLE:
			*(double *)cp=0.0;
			break;
		case CH_CLOB:
			*(char **)cp=0;
			break;
		case CH_JUL:
		case CH_CJUL:
		case CH_MINUTS:
		case CH_CMINUTS:
		case CH_INT:
			*(INT4 *)cp=INTNULL;
			break;
		case CH_TIME:
		case CH_CTIME:
		case CH_USEC:
		case CH_INT64:
			*(INT64 *)cp=INT64NULL;
			break;
		case CH_SHORT:
			*(short *)cp=SHORTNULL;
			break;
		case CH_TINY:
			*cp=TINYNULL;
			break;
		case CH_LONG:
			*(long *)cp=LONGNULL;
			break;
		default:
			memset(cp,0,type[j].len);
			break;
		}
	}
	return 0;
}
