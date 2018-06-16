/***********************************************
 * 线程池服务器 
 ***********************************************/

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <sys/resource.h>

#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/unistd.h>
#include <ctype.h>
#include <datejul.h>
#include <sccli.h>
#include <scsrv.h>
#include <scpool.h>

typedef struct wqueue {
	pthread_mutex_t mut;
	pthread_cond_t cond;
	pTCB queue;
	pthread_t tid;
} Qpool;

//资源线程池
typedef struct {
	pthread_mutex_t mut;
	int poolnum;
	Qpool *QP;
	pthread_attr_t attr;
} WTHREAD;
//mod_sc.c
extern void wthread_free(WTHREAD *wp);

//等待队列
static WTHREAD wpool={PTHREAD_MUTEX_INITIALIZER,0,NULL};

void wpool_free()
{
WTHREAD *wp=&wpool;
	if(wp->QP) {
		free(wp->QP);
		wp->QP=NULL;
	}
	wp->poolnum=0;
	pthread_attr_destroy(&wp->attr);
	pthread_mutex_destroy(&wp->mut);
}

static void wpool_init(WTHREAD *wp,int num)
{
int i;
	if(wp->QP) return;
	wp->QP=(Qpool *)malloc(num * sizeof(Qpool));
	if(!wp->QP) {
		ShowLog(1,"%s:QP malloc error!",__FUNCTION__);
		wp->poolnum=0;
		return;
	}
	wp->poolnum=num;
	for(i=0;i<wp->poolnum;i++) {
		wp->QP[i].queue=NULL;
		wp->QP[i].tid=0;
		pthread_mutex_init(&wp->QP[i].mut,NULL);
		pthread_cond_init(&wp->QP[i].cond,NULL);
	}
	
        i=pthread_attr_setdetachstate(&wp->attr,PTHREAD_CREATE_DETACHED);
        if(i) {
                ShowLog(1,"%s:can't set pthread attr PTHREAD_CREATE_DETACHED:%s",
			__FUNCTION__,strerror(i));
        }
//设置线程堆栈保护区 16K
        pthread_attr_setguardsize(&wp->attr,(size_t)(1024 * 16));

}

static void *wait_sc(void *para)
{
int ret,clr_q=0;
int poolno=(int)(long)para;
Qpool *qp=&wpool.QP[poolno];
int TCBno=-1;
T_Connect *conn=NULL;
pthread_t tid=pthread_self();
struct timespec tim;

	ShowLog(3,"%s:scpool[%d],tid=%lx created!",__FUNCTION__,poolno,tid);
	while(1) {
//从等待队列取一个任务
		pthread_mutex_lock(&qp->mut);
		while(0>(TCBno=TCB_get(&qp->queue))) {
                        gettimeofday((struct timeval *)&tim,0);
                        tim.tv_sec+=300; //等待5分钟
                        tim.tv_nsec*=1000;
			clr_q=0;
                        ret=pthread_cond_timedwait(&qp->cond,&qp->mut,&tim); //没有任务，等待
                        if(ret)  {
				if(ETIMEDOUT != ret) 
				    ShowLog(1,"%s:pthread_cond_timedwait ret=%d,err=%d,%s",
					__FUNCTION__,ret,errno,strerror(errno));
				qp->tid=0;
				break;
			}
		};
		pthread_mutex_unlock(&qp->mut);
		if(TCBno<0) break;

		conn=NULL;
		if(!clr_q) {
			conn=get_SC_connect(poolno,5); //等待得到连接
			if(conn == (T_Connect *)-1) {
				clr_q=1; //清空队列
				conn=NULL;
				ShowLog(1,"%s:tid=%lx,get poolno[%d] fault for TCB:%d",
					__FUNCTION__,tid,poolno,TCBno);
			} else ShowLog(5,"%s[%d]:TCB:%d got connect!",__FUNCTION__,poolno,TCBno);
		}
		ret=bind_sc(TCBno,conn);
		if(ret<0) {
			if(conn) release_SC_connect(&conn,poolno);
			ShowLog(1,"%s:tid=%lx,bad bind connect to application!,TCB:%d,ret=%d",
				__FUNCTION__,tid,TCBno,ret);
		}
		ret=TCB_add(NULL,TCBno); //加入到主任务队列
		if(ret==-2) {
			unbind_sc(TCBno);
			release_SC_connect(&conn,poolno);
		}
if(clr_q) ShowLog(5,"%s[%d]:TCB:%d NULL to ready!,ret=%d",__FUNCTION__,poolno,TCBno,ret);
	}
	ShowLog(3,"%s[%d]:tid=%lx cancel!",__FUNCTION__,poolno,tid);
	return NULL;
}

int scpool_MGR(int TCBno,int poolno,T_Connect **connp,int (*call_back)(T_Connect *,T_NetHead *))
{
int ret;
pthread_t *tp;

	if(TCBno<0) {
		ShowLog(1,"%s:bad TCB no!",__FUNCTION__);
		return -1;
	}
	ShowLog(3,"%s:get pool[%d],TCB_no=%d,tid:%lx,USEC=%llu",__FUNCTION__,
		poolno,TCBno,pthread_self(),now_usec());
	*connp=get_SC_connect(poolno,0);
	if(*connp == (T_Connect *)-1) {
		*connp=NULL;
		return -1;
	}
	if(*connp) {
		if((*connp)->only_do == (sdbcfunc)1) {
			(*connp)->only_do=call_back;
			TCB_add(NULL,TCBno); //加入到主任务队列
//ShowLog(5,"%s:tid=%lx,only_do=1,TCB_no=%d",__FUNCTION__,pthread_self(),TCBno);
			return 1;
		}
		return 0;
	}
	set_callback(TCBno,call_back,120);
	if(wpool.poolnum<=0) {
		ret=get_scpoolnum();
		pthread_mutex_lock(&wpool.mut);
		wpool_init(&wpool,ret);
		pthread_mutex_unlock(&wpool.mut);
	}
	if(poolno < 0 || poolno >= wpool.poolnum) {
		ShowLog(1,"%s:TCB_no=%d bad poolno %d,poolnum=%d",__FUNCTION__,
			TCBno,poolno,wpool.poolnum);
		return -1;
	}
	pthread_mutex_lock(&wpool.QP[poolno].mut);
	TCB_add(&wpool.QP[poolno].queue,TCBno);
	tp=&wpool.QP[poolno].tid;
	if(*tp==0) *tp=TCBno+1;
	pthread_mutex_unlock(&wpool.QP[poolno].mut);
//new thread
	if(*tp==(pthread_t)(TCBno+1)) {
		pthread_create(&wpool.QP[poolno].tid,&wpool.attr,wait_sc,(void *)(long)poolno);
	} else pthread_cond_signal(&wpool.QP[poolno].cond); //唤醒工作线程
	return 1;
}

