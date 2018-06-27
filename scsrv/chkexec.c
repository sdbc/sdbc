#include <scsrv.h>
#include <regex.h>

/*******************************************************************
 * int regcomp(regex_t *preg, const char *regex, int cflags);
 *     int regexec(const   regex_t  *preg,  const  char  *string,
 *                 size_t  nmatch,   regmatch_t   pmatch[],   int
 *                 eflags);
 *     size_t regerror(int  errcode,  const  regex_t  *preg, char
 *                     *errbuf, size_t errbuf_size);
 *     void regfree(regex_t *preg);
 ******************************************************************/

#define MATCHN 3
static const char *black[]={
	"[ 	]rm[ 	]",
	"[ 	]unlink[ 	]",
	"kill[ 	]",
	"cat[ 	]*>",
	"^[ 	]*>",
	"[ 	]/etc/",
	"passwd",
	"password",
	"\\.log"
};
#define BLKN sizeof(black)/sizeof(char*)
static regex_t blkname[BLKN];
static int flg=0;

int chkexec(char *str)
{
int i,cc;
regmatch_t pmatch[MATCHN];
char errbuf[128];
	if(!str) return -9999;
	i=strlen(str);
	if(i<1) return -9999;
	if(str[i-1]=='|') str[i-1]=0;
	if(!flg) {
		flg=1;
		for(i=0;i<BLKN;i++) {
			cc=regcomp(&blkname[i],black[i],REG_EXTENDED);
			if(cc) {
 				regerror(cc,&blkname[i],errbuf,sizeof(errbuf));
				ShowLog(1,"chkexec regcomp black %s:%s",
					black[i],errbuf);
				return -i-1;
			}
		}
		flg=2;
	}
	for(i=0;i<BLKN;i++) {
		cc=regexec(&blkname[i],str,MATCHN,pmatch,0);
		if(!cc) {
			ShowLog(1,"chkexec bad command %s:%d,%s",
						str,i+1,black[i]);
			return i+1;
		} else if(cc!=REG_NOMATCH) {
 			regerror(cc,&blkname[i],errbuf,sizeof(errbuf));
			ShowLog(1,"chkexec bad match %s:%d,%s",
					str,i+1,errbuf);
			return i+1;
		}
	}
	return 0;
}
