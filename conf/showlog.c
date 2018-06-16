/****************************************************
 * SDBC的日志管理。
 * 日志文件名是循环使用的，可以按周循环或按日循环。
 * 也可以每日分时段写文件
 * 日志分级显示，可以通过环境变量控制日志级别。
 * 本函数是多进程多线程安全的。
 ***************************************************/

#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <errno.h>
#include <strproc.h>
#include <BB_tree.h>

//FILE *logfile=0;
char LOGFILE[512]="";
char *Showid=NULL;

static T_Tree *thread_showid=NULL;
static volatile int mthr_flg=0;
static int level=-1;

static pthread_mutex_t log_mutex=PTHREAD_MUTEX_INITIALIZER;
static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

struct mt_showid {
	pthread_t tid;
	char *showid;
};

static int mts_cmp(void *s,void *d,int len)
{
struct mt_showid *m1,*m2;
	m1=(struct mt_showid *)s;
	m2=(struct mt_showid *)d;
	if(pthread_equal(m1->tid,m2->tid)) return 0;
	return(m2->tid - m1->tid);
}

static int mthr_dup(T_Tree *tp,void *cont,int len)
{
struct mt_showid *m1,*m=(struct mt_showid *)cont;
	m1=(struct mt_showid *)tp->Content;
//如果tid已存在，showid更新  
	m1->showid=m->showid;
	return 0;
}
//定制线程的Showid  
void mthr_showid_add(pthread_t tid,char *showid)
{
struct mt_showid mid;
	mid.tid=tid;
	mid.showid=showid;
	pthread_rwlock_wrlock(&rwlock);
	thread_showid=BB_Tree_Add(thread_showid,&mid,sizeof(mid),mts_cmp,mthr_dup);
	mthr_flg=1;
	pthread_rwlock_unlock(&rwlock);
}

//取消线程的Showid

int mthr_showid_del(pthread_t tid)
{
struct mt_showid mid;
int i=0;
	mid.tid=tid;
	mid.showid=0;
	pthread_rwlock_wrlock(&rwlock);
	thread_showid=BB_Tree_Del(thread_showid,&mid,sizeof(mid),mts_cmp,0,&i);
	if(!thread_showid)  mthr_flg=0;
	pthread_rwlock_unlock(&rwlock);
	return i;
}
//设置日志级别,允许程序动态设置日志级别
//返回原先的级别
int setShowLevel(int new_level)
{
int old=level;
	level=new_level;
	return old;
}

//生成分时间段日志名 
//8-20点6段，0-8点，20-24点各一段 
// LOGSEG=8-20?6
static char * psfx(char *buf,int bh,int eh,int n,int now)
{
int j,bm,em;
int len;
char *p;

	if(n<1) return NULL;
	bm=bh*60;
	em=eh*60;
	len=em-bm;
	if(n>len) n=len;
	if(len==0 || now<bm || now>=em) return NULL;
	j=bm+(now-bm)*n/len*len/n;
	p=buf;
	p+=sprintf(p,"%02d",j/60);
	if((eh-bh) % n) {
		sprintf(p,"%02d",j%60);
	}
	return buf;
}

static char *getsfx(int now)
{
int bh,eh,m,n;
int ret;
int n1,n2;
char *p,buf[5];


	p=getenv("LOGSEG");
	if(!p || !*p) return "log";
	
	ret=sscanf(p,"%d?%d:%d",&bh,&m,&n);
	if(ret<1) return "log";
	else if(ret == 1 ) {
		ret=sscanf(p,"%d-%d?%d:%d",&bh,&eh,&m,&n);
		if(ret==2) {
			m=1;
			ret=5;
		} else if(ret==3) ret=5;
	}
	switch(ret) {
	case 1:
		return psfx(buf,0,24,bh,now);
		break;
	case 2:
		if(bh>23) bh=23;
		if(now/60<bh) return psfx(buf,0,bh,1,now);
		else return psfx(buf,bh,24,m,now);
		break;
	case 3:
		eh=24;
	case 4:
		if(eh>24) eh=24;
		if(bh>eh) bh=eh;
		n1=24-eh;
		n2=n*n1/bh;
		if(eh<24 && n2<1) n2=1;
		n1=n-n2;
		if(n1<1) n1=1;
		ret=now/60;
		if(ret<bh) return psfx(buf,0,bh,n1,now);
		else if(ret <eh ||eh >=24) return psfx(buf,bh,eh,m,now);
		else return psfx(buf,eh,24,n2,now);
		break;
	case 5:
		if(eh>24) eh=24;
		if(bh>(eh-1)) bh=eh-1;
		ret=now/60;
		if(ret<bh) return psfx(buf,0,bh,1,now);
		else if(ret<eh || eh==24) return psfx(buf,bh,eh,m,now);
		else  return psfx(buf,eh,24,1,now);
		break;
	}
	return "log";
}

static int setlogfile(time_t today)
{
struct tm tim;
char *cp,*mode="a";
struct stat sbuf;
int ret,fd,fflag;
char *dp,*sfx;
char fn[512];
FILE *efd=0;

	localtime_r(&today,&tim);
	cp=getenv("LOGFILE");
	if(!cp || !*cp) return 0;
	sfx=getsfx(tim.tm_hour*60+tim.tm_min);
 	dp=getenv("LOGDAY");
	if(dp&&toupper(*dp)=='D') 
		sprintf(fn,"%s%02d.%s",cp,tim.tm_mday,sfx);
	else sprintf(fn,"%s%d.%s",cp,tim.tm_wday,sfx);

//printf("%s:%s\n",__FUNCTION__,fn);
	pthread_mutex_lock(&log_mutex);
	if(!*LOGFILE || strcmp(LOGFILE,fn)) {
		strcpy(LOGFILE,fn);
		ret=stat(LOGFILE,&sbuf);
		if(ret<0 || ((today-timezone)/86400-
			(sbuf.st_ctime-timezone)/86400)>0) mode="w"; 
    		efd=freopen(LOGFILE,mode,stderr);
    		if(efd) {
	    		fd=fileno(efd);
	    		fflag=fcntl(fd,F_GETFL,0);
	    		if(fflag>=0)
    			ret=fcntl(fd,F_SETFL,fflag|O_DSYNC);
			pthread_mutex_unlock(&log_mutex);
	    		return 1;
    		} else {
			fprintf(stderr,"open logfile %s:errno=%d,%s\n",
				LOGFILE,errno,strerror(errno));
			efd=freopen("/dev/null","w",stderr);
		}
	}
	fflush(stderr);
	fseek(stderr,0,SEEK_END);
	pthread_mutex_unlock(&log_mutex);
	return 0;
}
/* 返回0没记,返回1 记了 */     
int ShowLog(int debug_level,const char * fmt,...)
{
char *cp;
struct tm *today_t;
time_t tim;
va_list vlist;
int len;
char *showid=Showid;

	if(debug_level==-1){
		pthread_mutex_lock(&log_mutex);
		if(stderr) {
				fflush(stderr);
		}
		*LOGFILE=0;
		if(mthr_flg) {
			pthread_rwlock_wrlock(&rwlock);
			BB_Tree_Free(&thread_showid,0);
			mthr_flg=0;
			pthread_rwlock_unlock(&rwlock);
		}
		pthread_mutex_unlock(&log_mutex);
		return 0;
	}
	if(level<0) {
		cp=getenv("LOGLEVEL");
		if(cp && isdigit(*cp))level=atoi(cp);
		else level=2;
	}
	if(level<debug_level) return 0;
	time(&tim);
	today_t=localtime((time_t *)&tim);
	len=strlen(fmt);
char ufmt[len+((showid)?strlen(showid):1)+50];

	setlogfile(tim);
	if(mthr_flg) {	//多线程日志 
	struct mt_showid mid;
	T_Tree *tp;

		mid.tid=pthread_self();
		mid.showid=0;
		pthread_rwlock_rdlock(&rwlock);
		tp=BB_Tree_Find(thread_showid,&mid,sizeof(mid),mts_cmp);
		pthread_rwlock_unlock(&rwlock);
		if(!tp) showid=Showid;
		else showid=((struct mt_showid *)tp->Content)->showid;
	}
	cp=ufmt+sprintf(ufmt,"%d %s %02d/%02d %02d:%02d\'%02d %s",
			debug_level,
			showid?showid:"",
			today_t->tm_mon+1,
			today_t->tm_mday,
			today_t->tm_hour,
			today_t->tm_min,
			today_t->tm_sec,
			fmt
	);
	if(fmt[len-1]!='\n') strcpy(cp,"\n");
	va_start(vlist,fmt);
	pthread_mutex_lock(&log_mutex);
	vfprintf(stderr,ufmt,vlist);
	fflush(stderr);
	pthread_mutex_unlock(&log_mutex);
	va_end(vlist);
	
	return 1;
}
