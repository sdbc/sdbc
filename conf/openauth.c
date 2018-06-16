/*****************************************************
 * For SDBC 4.0
 * 数据库帐号口令的获取函数
 * SDBC封装了数据库的帐号口令。
 * 客户端需要以连接串关联数据库
 * 本程序就是在auth文件里以连接串提取帐号口令。
 *****************************************************/


#include <regex.h>
#include <strproc.h>
#include <unistd.h>

/*************************************************
 * 由用户提供的解密函数地址，
 * 空为不需要解密。
 *************************************************/

static const char *database_auth_item[]={
	"^[	 ]*UID[ 	]*=[ 	]*",
	"^[	 ]*PWD[ 	]*=[ 	]*",
	"^[	 ]*USERNAME[ 	]*=[ 	]*",
	"^[	 ]*PASSWORD[ 	]*=[ 	]*",
	"^[	 ]*SID[ 	]*=[ 	]*",
	"^[	 ]*DBOWN[ 	]*=[ 	]*",
};
#define REG_NUM (sizeof(database_auth_item)/sizeof(char *))

int open_auth(char * authfile,char *name,
	char *DNS,char *UID,char *Pwd,char *DBOWN)
{
FILE *fp;
char buffer[256];
regex_t dns_name;
regex_t dns_end;
regex_t auth[REG_NUM+1];

char *dns_end_str="[ 	]*[<]DBLABEL";
char dns_str[181];
regmatch_t retstr[5];
int i;
int ret;
int found=0;
char *p;
	if(DBOWN) *DBOWN=0;
	if(!authfile||!*authfile) {
		ShowLog(1,"authfile is Empty!");
		return FORMATERR;
	}
	if(!name||!*name) {
		ShowLog(1,"DBLABEL is Empty!");
		return FORMATERR;
	}
	for(i=0;i<REG_NUM;i++) {
	    ret=regcomp(&auth[i],database_auth_item[i],REG_EXTENDED|REG_ICASE);
	    if(ret) {
		regerror(ret,&auth[i],dns_str,sizeof(dns_str));
		ShowLog(1,"SQL_Auth regcomp %s,err=%s",
				database_auth_item[i],dns_str);
		return MEMERR;
	    }
	}
	*DNS=*UID=*Pwd=0;
	sprintf(dns_str,"^[ 	]*[<][ 	]*DBLABEL[ 	]*%s[ 	]*[>].*",name);
	ret=regcomp(&dns_name,dns_str,REG_EXTENDED|REG_ICASE);
	ret=regcomp(&dns_end,dns_end_str,REG_EXTENDED|REG_ICASE);
	fp=fopen(authfile,"r");
	if(!fp) {
		ShowLog(1,"SQL_AUTH open %s err %d",authfile,errno);
		regfree(&dns_name);
		regfree(&dns_end);
		for(i=0;i<REG_NUM;i++) regfree(&auth[i]);
		return errno;
	}
/* wait for another thread compile reg */
	while(!ferror(fp)&&!feof(fp)){
		p=fgets(buffer,sizeof(buffer),fp);
		if(!p) break;
		if(ferror(fp)) break;
		TRIM(buffer);
		p=skipblk(buffer);
		if(!*p||*p=='#') continue;
		if(!found){
		    ret=regexec(&dns_name,p,5,retstr,0);
		    if(ret)continue;
		    found=1;
		    continue;
	        }
	        else {
		    ret=regexec(&dns_end,p,5,retstr,0);
		    if(!ret)break;
		}
        	for(i=0;i<REG_NUM;i++){
			ret=regexec(&auth[i],p,5,retstr,0);
			if(!ret){
//ShowLog(5,"openauth[%d]:%s",i,p);
				switch(i){
				case 0:
				case 2:
					strcpy(UID,buffer+retstr[0].rm_eo);
					if(*UID=='@' && encryptproc) {
						strdel(UID);
						encryptproc(UID);
					}
					break;
				case 1:
				case 3:
					strcpy(Pwd,buffer+retstr[0].rm_eo);
					if(*Pwd=='@' && encryptproc) {
                                                strdel(Pwd);
                                                encryptproc(Pwd);
                                        }

						break;
				case 4:
					strcpy(DNS,buffer+retstr[0].rm_eo);
						break;
				case 5:
					if(DBOWN)
						strcpy(DBOWN,
						  buffer+retstr[0].rm_eo);
				default:
					break;
				}
			}
		}
	}
	fclose(fp);
	regfree(&dns_name);
	regfree(&dns_end);
	for(i=0;i<REG_NUM;i++) regfree(&auth[i]);
	if(*DNS==0||*UID==0) return -3;
	return 0;
}
