/***********************************************
 * Ïß³Ì³Ø·þÎñÆ÷ 
 * SDBC 7.1 Ö§³ÖFiberized IO(coroutine,Ð­³Ì)
 ***********************************************/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <bits/local_lim.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <sys/unistd.h>
#include <ctype.h>
#include <scsrv.h>
#include <datejul.h>
#include <ucontext.h>
#include <sys/mman.h>

#if __WORDSIZE == 64
#define MAX_STACK 0X200000
#else
#define MAX_STACK 0X100000
#endif

static int use_stack_size=MAX_STACK;

#ifdef __cplusplus
extern "C"
#endif

void set_showid(void *ctx);

#ifdef __cplusplus
}
#endif

static T_YIELD other_yield=NULL;
extern srvfunc Function[];// appl function list
extern u_int family[];
static void *thread_work(void *param);

static int g_epoll_fd=-1;

// SDBC task control block for epoll event 
typedef struct event_node {
	struct event_node *next;
	int events;
	int fd;
	T_Connect conn;
	T_NetHead head;
	T_SRV_Var sv;
	char	   *ctx;
	sdbcfunc call_back;
	pthread_mutex_t lock;
	ucontext_t uc;
	int AIO_flg;
	INT64 timestamp;
	int timeout;
	volatile int status; //-1 Î´Á¬½Ó£¬0:Î´µÇÂ¼£¬1£ºÒÑµÇÂ¼
} TCB;

static int do_epoll(TCB *task,int op,int flg);

typedef  struct {
	pthread_mutex_t mut;
	pthread_cond_t cond;
	TCB *queue;
	int svc_num;
	char flg;	//ÎÞÊØ»¤Ïß³Ì0£¬ÓÐ1
} Qpool;

//¾ÍÐ÷¶ÓÁÐ
static Qpool rpool={PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,NULL,-1,0};

//Ïß³Ì³Ø½Úµã
typedef struct {
	pthread_t tid;
	int status;
	ucontext_t tc;
	INT64 timestamp;
} resource;
//Ïß³Ì³Ø
static struct {
	pthread_mutex_t mut;
	int num;
	int rdy_num;
	resource *pool;
	pthread_attr_t attr;
} tpool={PTHREAD_MUTEX_INITIALIZER,0,1,NULL};
//ÈÎÎñ³Ø
static  struct {
	pthread_mutex_t mut;
	pthread_cond_t cond;
	int max_client;
	TCB *pool;
	TCB *free_q;
} client_q={PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER,0,NULL,NULL};

T_SRV_Var * get_SRV_Var(int TCBno)
{
	if(TCBno<0 || TCBno>=client_q.max_client) return NULL;
	return &client_q.pool[TCBno].sv;
}

T_NetHead *getNetHead(int TCBno)
{
	if(TCBno<0 || TCBno>=client_q.max_client) return NULL;
	return &client_q.pool[TCBno].head;
}

int TCB_add(TCB **rp,int TCBno)
{
TCB *task;
	if(TCBno<0 || TCBno>=client_q.max_client) return -1;
	task=&client_q.pool[TCBno];
//	task->timestamp=now_usec();
	if(task->next) {
		ShowLog(1,"%s:TCB:%d ÒÑ¾­ÔÚ¶ÓÁÐÖÐ",__FUNCTION__,TCBno);
		return -2;//²»¿ÉÒÔÔÚÆäËû¶ÓÁÐ
	}
	if(!rp) {
		pthread_mutex_lock(&rpool.mut);
		rp=&rpool.queue;
	}
	if(!*rp) {
		*rp=task;
		task->next=task;
	} else {
		task->next=(*rp)->next;//Á¬½Ó¶ÓÍ·
		(*rp)->next=task;//¹Òµ½Á´Î²
		*rp=task;//Ö¸Ïòµ½Á´Î²
	}
	if(*rp==rpool.queue) {
		pthread_mutex_unlock(&rpool.mut);
//ShowLog(1,"%s:TCB:%d,tid=%lx",__FUNCTION__,TCBno,pthread_self());
		pthread_cond_signal(&rpool.cond); //»½ÐÑ¹¤×÷Ïß³Ì	
	}
	return 0;
}

int TCB_get(TCB **rp)
{
TCB *task;
	if(!rp || !*rp) return -1;
	task=(*rp)->next;//ÕÒµ½¶ÓÍ·
	if(!task->next) {
		ShowLog(1,"%s:TCB=%d,´í£¬Î´ÔÚ¶ÓÁÐÖÐ£¡",__FUNCTION__,task->sv.TCB_no);
		return -2;
	}
	if(task->next == task) *rp=NULL;//×îºóÒ»¸öÁË
	else (*rp)->next=task->next;//ÐÂµÄ¶ÓÍ·
	task->next=NULL;
	return task->sv.TCB_no;
}

sdbcfunc  set_callback(int TCBno,sdbcfunc callback,int timeout)
{
sdbcfunc old;
TCB *task;

	if(TCBno<0 ||TCBno>client_q.max_client) return (sdbcfunc)-1;
	task=&client_q.pool[TCBno];
	task->timeout=timeout;
	old=task->call_back;
	task->call_back=callback;
	return old;
}
/**
 * unset_callback
 * Çå³ýÓÃ»§×Ô¶¨Òå»Øµ÷º¯Êýþ
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return Ô­»Øµ÷º¯Êý
 */
sdbcfunc unset_callback(int TCB_no)
{
sdbcfunc old;
TCB *task;
        if(TCB_no<0 || client_q.max_client <= TCB_no) return NULL;
	task=&client_q.pool[TCB_no];
	old=task->call_back;
        task->call_back=NULL;
        task->timeout=task->conn.timeout;
	
        return old;
}

/**
 * get_callback
 * È¡ÓÃ»§×Ô¶¨Òå»Øµ÷º¯Êýþ
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return »Øµ÷º¯Êý
 */
sdbcfunc get_callback(int TCB_no)
{
        if(TCB_no<0 || client_q.max_client <= TCB_no) return NULL;
	return client_q.pool[TCB_no].call_back;
}

T_Connect *get_TCB_connect(int TCBno)
{
	if(TCBno<0 ||TCBno>client_q.max_client) return NULL;
	return &client_q.pool[TCBno].conn;
}

void *get_TCB_ctx(int TCBno)
{
	if(TCBno<0 ||TCBno>client_q.max_client) return NULL;
	return client_q.pool[TCBno].ctx;
}
void tpool_free()
{
int i;
	if(client_q.pool) {
		if(client_q.pool[0].ctx) 
			free(client_q.pool[0].ctx);
		for(i=0;i<client_q.max_client;i++) {
			client_q.pool[i].conn.Var=NULL;
			pthread_mutex_destroy(&client_q.pool[i].lock);
			freeconnect(&client_q.pool[i].conn);
		}
		free(client_q.pool);
		client_q.pool=NULL;
		client_q.free_q=NULL;
	}
	client_q.max_client=0;

	rpool.svc_num=-1;
	rpool.flg=0;

	pthread_mutex_destroy(&tpool.mut);
	if(tpool.pool) {
		free(tpool.pool);
		tpool.pool=NULL;
	}
	tpool.num=0;
	pthread_attr_destroy(&tpool.attr);
	if(g_epoll_fd > -1) close(g_epoll_fd);
	g_epoll_fd=-1;
	return ;
}

//½¨Á¢ÐÂÏß³Ì
static int new_wt(int n)
{
int ret;

	if(n<0) return n;
	tpool.pool[n].status=1;
	tpool.pool[n].timestamp=now_usec();
	ret=pthread_create(&tpool.pool[n].tid,&tpool.attr,thread_work,&tpool.pool[n]);
        if(ret) {
		tpool.pool[n].tid=0;
                ShowLog(1,"%s:pthread_create:%s",__FUNCTION__,strerror(ret));
		return ret;
        }
	return 0;
}
extern srvfunc *SRVFUNC;
static int tpool_init(int size_ctx)
{
char *p;
int ret,i,limit;
struct rlimit sLimit;
TCB *task;
int mtu;

	p=getenv("SENDSIZE");
        if(p && isdigit(*p)) {
                mtu=atoi(p);
        } else mtu=0;

	rpool.svc_num=-1;
	SRVFUNC=Function;//used by get_srvname();

	limit=getrlimit(RLIMIT_NOFILE,&sLimit);
	if(limit==0) {
		limit=sLimit.rlim_cur;
	}

	p=getenv("MAXCLT");
	if(!p || !isdigit(*p)) {
		ShowLog(4,"%s:È±ÉÙ»·¾³±äÁ¿MAXCLT,ÉèÖÃÎª2",__FUNCTION__);
		client_q.max_client=2;
	} else {
		client_q.max_client=atoi(p);
		if(limit>0) {
			i=(limit<<3)/10;
			if(client_q.max_client > i) client_q.max_client=i;
		}
	}
	if(NULL==(client_q.pool=(TCB *)malloc((client_q.max_client+1) * sizeof(TCB)))) return -4;
	if(size_ctx>0)
		if(NULL==(client_q.pool[0].ctx=malloc((client_q.max_client+1) * size_ctx))) {
			free(client_q.pool);
			client_q.pool=NULL;
			return -2;
		} else ;
	else client_q.pool[0].ctx=NULL;
	client_q.free_q=NULL;
	
	task=client_q.pool;
	for(i=0;i<=client_q.max_client;i++,task++) {
		initconnect(&task->conn);
		task->next=NULL;
		task->AIO_flg=0;
		task->call_back=NULL;
		task->sv.TCB_no=i;
		task->timeout=0;
		task->conn.timeout=0;
		task->conn.MTU=mtu;
		task->conn.family=family;
		task->events=0;
		task->sv.poolno=-1;
		task->sv.SQL_Connect=NULL;
		task->status=-1;
		pthread_mutex_init(&task->lock, NULL);
		memset(&task->uc,0,sizeof(task->uc));
		getcontext(&task->uc);
		task->uc.uc_stack.ss_flags=0;
		task->uc.uc_stack.ss_size=0;
		task->uc.uc_stack.ss_sp=NULL;
		task->uc.uc_link=NULL;
		if(!client_q.pool[0].ctx) task->ctx=NULL;
		else if(i>0) task->ctx=client_q.pool[0].ctx+i*size_ctx;
		task->sv.var=task->ctx;
		TCB_add(&client_q.free_q,i);
	}

	p=getenv("RDY_NUM");
	if(p && isdigit(*p)) tpool.rdy_num=atoi(p);
	else tpool.rdy_num=1;
	p=getenv("MAXTHREAD");
	if(!p || !isdigit(*p)) {
		tpool.num=tpool.rdy_num+1;
		ShowLog(4,"%s:È±ÉÙ»·¾³±äÁ¿MAXTHREAD,ÉèÖÃÎª%d",__FUNCTION__,tpool.num);
	} else tpool.num=atoi(p);
	ShowLog(0,"%s:MAXCLIENT=%d,threads=%d",__FUNCTION__,client_q.max_client,tpool.num);
	if(NULL==(tpool.pool=(resource *)malloc(tpool.num * sizeof(resource)))) {
		if(client_q.pool) {
			free(client_q.pool);
			client_q.pool=NULL;
		}
		return -3;
	}
	ret= pthread_attr_init(&tpool.attr);
        if(ret) {
                ShowLog(1,"%s:can not init pthread attr %s",__FUNCTION__,strerror(ret));
        } else {
//ÉèÖÃ·ÖÀëÏß³Ì
        	ret=pthread_attr_setdetachstate(&tpool.attr,PTHREAD_CREATE_DETACHED);
        	if(ret) {
               	 ShowLog(1,"%s:can't set pthread attr PTHREAD_CREATE_DETACHED:%s",
		 	__FUNCTION__,strerror(ret));
       		}
//ÉèÖÃÏß³Ì¶ÑÕ»±£»¤Çø 16K
		pthread_attr_setguardsize(&tpool.attr,(size_t)(1024 * 16));
//ÉèÖÃÓÃ»§Õ»¿Õ¼ä
		p=getenv("USERSTACKSZ");
		if(p && isdigit(*p)) {
size_t sz;
char c;
			ret=sscanf(p,"%ld%c",&sz,&c);
			if(ret>1) {
				switch(toupper(c)) {
				case 'K':
					sz *= 1024;
					break;
				case 'M':
					sz*=1024*1024;
					break;
				default:break;
				}
			}
			if(sz>0) {
				sz+=PTHREAD_STACK_MIN;
				ret = pthread_attr_setstacksize(&tpool.attr, sz);	
				if(!ret) use_stack_size=sz;
			}
		}

	}
	for(i=0;i<tpool.num;i++) {
		tpool.pool[i].tid=0;
		tpool.pool[i].status=0;
		memset(&tpool.pool[i].tc,0,sizeof(tpool.pool[i].tc));
		tpool.pool[i].tc.uc_stack.ss_flags=0;
		tpool.pool[i].tc.uc_stack.ss_size=0;
		tpool.pool[i].tc.uc_stack.ss_sp=NULL;
		tpool.pool[i].tc.uc_link=NULL;
	}	
	rpool.queue=NULL;
//ShowLog(5,"%s:maxfd=%d,maxclt=%d",__FUNCTION__,limit,client_q.max_client);
	if( 0 <= (g_epoll_fd=epoll_create(limit>0?limit<(client_q.max_client<<1)?limit:client_q.max_client<<1:client_q.max_client))) {
		for(i=0;i<tpool.num;i++) new_wt(i);
		return 0;
	}
	ShowLog(1,"%s:epoll_create err=%d,%s",
		__FUNCTION__,errno,strerror(errno));
	tpool_free();
	return SYSERR;
}

static TCB *rdy_get()
{
TCB *enp;
	if(!rpool.queue) return NULL;
	enp=rpool.queue->next;
	if(enp==NULL) {
		ShowLog(1,"%s:bad ready queue TCB:%d!",__FUNCTION__,rpool.queue->sv.TCB_no);
		enp=rpool.queue;
		rpool.queue=NULL;
		return enp;
	}
	if(enp->next == enp) rpool.queue=NULL;
	else rpool.queue->next=enp->next;
	enp->next=NULL;
	return enp;
}
/**
 * set_event
 * ÓÃ»§×Ô¶¨ÒåÊÂ¼þ
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @param fd ÊÂ¼þfd Ö»Ö§³Ö¶ÁÊÂ¼þ 
 * @param call_back ·¢ÉúÊÂ¼þµÄ»Øµ÷º¯Êý
 * @param timeout fdµÄ³¬Ê±Ãë,Ö»ÔÊÐíÉèÖÃsocket fd
 * @return ³É¹¦ 0
 */
int set_event(int TCB_no,int fd,sdbcfunc call_back,int timeout)
{
TCB *task;

	if(TCB_no<0 || client_q.max_client <= TCB_no) return -1;
	task=&client_q.pool[TCB_no];
	task->fd=fd;
	task->call_back=call_back;
	task->timestamp=now_usec();
	task->timeout=timeout;
//ShowLog(5,"%s:set fd(%d) to epoll for read",__FUNCTION__,task->fd);
	return do_epoll(task,0,0);
}

/**
 * clr_event
 * Çå³ýÓÃ»§×Ô¶¨ÒåÊÂ¼þ
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return ³É¹¦ 0
 */
int clr_event(int TCB_no)
{
TCB *task;
int ret;
	if(TCB_no<0 || client_q.max_client <= TCB_no) return -1;
	task=&client_q.pool[TCB_no];
	if(task->fd == task->conn.Socket) {
		task->call_back=NULL;
		task->status=1;
		return -2;
	}
	task->call_back=NULL;
	ret=do_epoll(task,EPOLL_CTL_DEL,0);
	if(ret) ShowLog(1,"%s:tid=%lx,TCB:%d,do_epoll ret=%d",
		__FUNCTION__,pthread_self(),TCB_no,ret);
	task->fd=task->conn.Socket;
	task->timeout=task->conn.timeout;
	return 0;
}

/**
 * get_event_fd
 * È¡ÊÂ¼þfd
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return ÊÂ¼þfd
 */
int get_event_fd(int TCB_no)
{
	if(TCB_no<0 || client_q.max_client <= TCB_no) return -1;
	return client_q.pool[TCB_no].fd;
}
/**
 * get_event_status
 * È¡ÊÂ¼þ×´Ì¬
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return ÊÂ¼þ×´Ì¬
 */
int get_event_status(int TCB_no)
{
	if(TCB_no<0 || client_q.max_client <= TCB_no) return -1;
	return client_q.pool[TCB_no].events;
}

/**
 * get_event_status
 * È¡TCB×´Ì¬
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return TCB×´Ì¬
 */
int get_TCB_status(int TCB_no)
{
	if(TCB_no<0 || client_q.max_client <= TCB_no) return -1;
	return client_q.pool[TCB_no].status;
}

static void client_del(TCB *task)
{
struct linger so_linger;

	so_linger.l_onoff=1;
        so_linger.l_linger=0;
        setsockopt(task->conn.Socket, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger);
	pthread_mutex_lock(&tpool.mut);
	task->fd=-1;
	task->AIO_flg=0;
	task->status=-1;
	freeconnect(&task->conn);
	task->events=0;
	task->timeout=0;
	pthread_mutex_unlock(&tpool.mut);
	pthread_mutex_lock(&client_q.mut);
	TCB_add(&client_q.free_q,task->sv.TCB_no);
	pthread_mutex_unlock(&client_q.mut);
	pthread_cond_signal(&client_q.cond); //»½ÐÑÖ÷Ïß³Ì	
	ShowLog(3,"%s:tid=%lx,TCB:%d deleted!",__FUNCTION__,pthread_self(),task->sv.TCB_no);
}

//¼ÓÈëÐÂÁ¬½Ó
static int do_epoll(TCB *task,int op,int flg)
{
struct epoll_event epv = {0, {0}};
int  ret;
	if(task->fd<0) return FORMATERR;
	if(task->next) {
		ShowLog(1,"%s:tid=%lx,TCB:%d ÒÑ¾­ÔÚ¶ÓÁÐÖÐ,fd=%d,Sock=%d",__FUNCTION__,
			pthread_self(),task->sv.TCB_no,task->fd,task->conn.Socket);
		return -1;
	}
	epv.events =  flg?EPOLLOUT:EPOLLIN;
	epv.events |= EPOLLONESHOT;
	epv.data.ptr = task;
	task->events=0;
	if(op) 
		ret=epoll_ctl(g_epoll_fd,op,task->fd,&epv);
	else {
		ret=epoll_ctl(g_epoll_fd,EPOLL_CTL_MOD,task->fd,&epv);
		if(ret < 0 && errno==ENOENT) 
			ret=epoll_ctl(g_epoll_fd,EPOLL_CTL_ADD,task->fd,&epv);
	}
	if(ret<0 || op==EPOLL_CTL_DEL) {
		if(ret<0) {
			if( errno != EEXIST) ShowLog(1,"%s:tid=%lx,epoll_ctl fd[%d]=%d,op=%d,ret=%d,err=%d,%s",__FUNCTION__,
				pthread_self(),task->sv.TCB_no, task->fd,op,ret,errno,strerror(errno));
		} else {
			if(task->status>-1) 
			   ShowLog(3,"%s:tid=%lx epoll_ctl fd[%d]=%d,deleted,op=%d",__FUNCTION__,
				pthread_self(),task->sv.TCB_no, task->fd,op);
			task->fd=-1;
		}
	}
	return ret;
}

static TCB * get_TCB(int TCB_no)
{
	if(TCB_no<0 || client_q.max_client <= TCB_no) return NULL;
	return &client_q.pool[TCB_no];
}

//¹¤×÷Ïß³Ì
static void do_work(int TCB_no)
{
int ret;
TCB *task=get_TCB(TCB_no);
T_Connect *conn;
void (*init)(T_Connect *,T_NetHead *);
T_SRV_Var *ctx=&task->sv;
int timeout=0;

	ctx->tid=pthread_self();//±êÖ¾¶àÏß³Ì·þÎñ
	conn=&task->conn;
	conn->Var=ctx;
	timeout=task->timeout;
	task->timeout=0;
	ret=0;
    do {
	if(!task->call_back) { //SDBC±ê×¼ÊÂ¼þ 
//Ð­ÉÌÃÜÔ¿
		if(task->status==-1) {
			init=(void (*)())conn->only_do;
			conn->only_do=0;
			ret=mk_clikey(conn->Socket,&conn->t,conn->family);
			if(ret<0) {
				if(ret!=SYSERR) { 
				char addr[16];
					peeraddr(conn->Socket,addr);
					ShowLog(1,"%s:tid=%lx,TCB:%d,Ð­ÉÌÃÜÔ¿Ê§°Ü!,addr=%s,ret=%d",__FUNCTION__,
							ctx->tid,task->sv.TCB_no,addr,ret);
				}
	//ÊÍ·ÅÁ¬½Ó
				ret=-1;
				break;
			} 
			conn->CryptFlg=ret;
			task->status=0;
			if(init) init(conn,&task->head);
			task->timeout=60;//60ÃëÄÚ±ØÐëµÇÂ¼
			ret=0;
			break;
		}
	
		ret=RecvPack(conn,&task->head);
		if(ret) {
			ShowLog(1,"%s:TCB:%d,½ÓÊÕ´íÎó,tid=%lx,err=%d,%s,event=%08X",__FUNCTION__,
				task->sv.TCB_no,ctx->tid,errno,strerror(errno),task->events);
			ret=-1;
			break;
		}
		task->timestamp=now_usec();
		ShowLog(4,"%s: TCB_no=%d,tid=%lx,PROTO_NUM=%d pkg_len=%d,t_len=%d,O_NODE=%u,USEC=%llu",
			__FUNCTION__,TCB_no,ctx->tid,
	                task->head.PROTO_NUM,task->head.PKG_LEN,task->head.T_LEN,
			task->head.O_NODE,task->timestamp);
	
		if(task->head.PROTO_NUM==65535) {
			ShowLog(3,"%s: disconnect by client",__FUNCTION__);
			ret=-1;
			break;
		} else if(task->head.PROTO_NUM==1){
	                ret=Echo(conn,&task->head);
	        } else if(task->status==0) {
			if(!task->head.PROTO_NUM) {
				ret=Function[0].funcaddr(conn,&task->head);
				if(ret>=0) {
					task->status=ret;
					if(ret==1) task->timeout=conn->timeout;
				}
	                } else {
	                        ShowLog(1,"%s:TCB:%d,Î´µÇÂ¼",__FUNCTION__,task->sv.TCB_no);
	                        ret=-1;
				break;
	                }
		} else if (conn->only_do) {
			ret=conn->only_do(conn,&task->head);
		} else {
			if(task->head.PROTO_NUM==0) {
				ret=get_srvname(conn,&task->head);
				if(task->status>0) set_showid(task->ctx);//Showid Ó¦¸ÃÔÚ»á»°ÉÏÏÂÎÄ½á¹¹Àï 
			} else if(task->head.PROTO_NUM>rpool.svc_num) {
	                         ShowLog(1,"%s:Ã»ÓÐÕâ¸ö·þÎñºÅ %d",__FUNCTION__,task->head.PROTO_NUM);
	                         ret=-1;
	                } else {
	                        ret=Function[task->head.PROTO_NUM].funcaddr(conn,&task->head);
	                        if(ret==-1)
					ShowLog(1,"%s:TCB:%d,disconnect by server",__FUNCTION__,
						task->sv.TCB_no);
			}
		}
		if(ret==0) task->timeout=task->conn.timeout;
	} else { //ÓÃ»§×Ô¶¨ÒåÊÂ¼þ
		task->timestamp=now_usec();
		ShowLog(5,"%s:call_back TCB_no=%d,tid=%lx,USEC=%llu",__FUNCTION__,
			TCB_no,pthread_self(),task->timestamp);
		ret=task->call_back(conn,&task->head);
		if(task->status==0)  //finish_login,return 0 or 1
			task->status=ret;
	        if(ret==-1)
			ShowLog(1,"%s:TCB:%d,disconnect by server",__FUNCTION__,
				task->sv.TCB_no);
	}
    } while(0);
	if(!task->timeout) task->timeout=timeout; //timeout Ã»ÓÐ±»Ó¦ÓÃÉèÖÃ¹ý
	switch(ret) {
	case -1:
		do_epoll(task,EPOLL_CTL_DEL,0);
		task->status=-2;//delete the task by return
	case THREAD_ESCAPE:
		break;
	default:
		if(do_epoll(task,EPOLL_CTL_MOD,0) && errno != EEXIST) {
			ShowLog(1,"%s:cancel by server",__FUNCTION__);
			task->status=-2;
		}
		break;
	}
	if(task->status == -2) {
		client_del(task);
	}
	task->timestamp=now_usec();//taskÓërsµÄ²î¾ÍÊÇÓ¦ÓÃÈÎÎñÖ´ÐÐÊ±¼ä/
	ucontext_t *tc=task->uc.uc_link;
	if(tc) { //·ÀÖ¹×²Õ»
		pthread_t tid=pthread_self();
		resource *rs=tpool.pool;
		for(ret=0;ret<tpool.num;ret++,rs++) {
			if(rs->tid==tid) break;
		}
		if(ret<tpool.num && &rs->tc == tc) setcontext(tc);
	}
	ShowLog(2,"%s:%lx SYNC returned TCB_no=%d",__FUNCTION__,pthread_self(),task->sv.TCB_no);
	return ;
}

static void *thread_work(void *param)
{
resource *rs=(resource *)param;
int ret,fds;
TCB *task=NULL;
struct epoll_event event;

	ShowLog(2,"%s:thread %lx start!",__FUNCTION__,pthread_self());
	getcontext(&rs->tc);
	if(task)  pthread_mutex_unlock(&task->lock);

	while(1) {
//´Ó¾ÍÐ÷¶ÓÁÐÈ¡Ò»¸öÈÎÎñ
		pthread_mutex_lock(&rpool.mut);
		while(!(task=rdy_get())) {
			if(rpool.flg >= tpool.rdy_num) break;
			rpool.flg++;
			ret=pthread_cond_wait(&rpool.cond,&rpool.mut); //Ã»ÓÐÈÎÎñ£¬µÈ´ý
 			rpool.flg--;
		}
		pthread_mutex_unlock(&rpool.mut);
		if(task) {
			if(!task->AIO_flg && !task->call_back) {
				task->fd=task->conn.Socket;
				ShowLog(5,"%s:tid=%lx,TCB_no=%d from rdy_queue",__FUNCTION__,
					pthread_self(),task->sv.TCB_no);
				if(task->fd>=0) {
					do_epoll(task,0,0);
				}
				continue;
			}
		} else  {
			fds = epoll_wait(g_epoll_fd, &event, 1 , -1);
			if(fds < 0){
       	 			ShowLog(1,"%s:epoll_wait err=%d,%s",__FUNCTION__,errno,strerror(errno));
				usleep(30000000);
				continue;
       	 		}
		 	task = (TCB *)event.data.ptr;
			if(task->events) {
			    ShowLog(1,"%s:tid=%lx,TCB_no=%d,task->events=%08X,conflict!",__FUNCTION__,
			            pthread_self(),task->sv.TCB_no,task->events);//·¢ÏÖ¾ªÈº
			    task=NULL;
			    continue;//¶ªµôËü
			}
			task->events=event.events;
		}
		rs->timestamp=now_usec();
		if(task->status>0) set_showid(task->ctx);//Showid Ó¦¸ÃÔÚ»á»°ÉÏÏÂÎÄ½á¹¹Àï 
		
		if(task->AIO_flg) {//fiber task
		    task->uc.uc_link=&rs->tc;
		    rs->tc.uc_link=(ucontext_t *)task;
ShowLog(5,"%s:tid=%lx,resume to TCB_no=%d",__FUNCTION__,pthread_self(),task->sv.TCB_no);
			pthread_mutex_lock(&task->lock);//·ÀÖ¹ÆäËûÏß³ÌÌáÇ°´³Èë
			setcontext(&task->uc);	//== longjmp()
			continue;//no action,logic only
		}
		if(task->uc.uc_stack.ss_size>0) {//call_backÄ£Ê½£¬ÇÀÈëÁË£¬½øÈëÍ¬²½Ä£Ê½
            rs->tc.uc_link=NULL;
ShowLog(5,"%s:tid %lx ÇÀÈë SYNC",__FUNCTION__,pthread_self());
			do_work(task->sv.TCB_no);
			continue;
		}
		if(!rs->tc.uc_stack.ss_sp) {
ShowLog(5,"%s:%lx create fiber for TCB_no=%d",__FUNCTION__,rs->tid,task->sv.TCB_no);
			task->uc.uc_stack.ss_sp=mmap(0, use_stack_size,
				PROT_READ | PROT_WRITE | PROT_EXEC,
				MAP_PRIVATE | MAP_ANON | MAP_GROWSDOWN, -1, 0);
			if(task->uc.uc_stack.ss_sp==MAP_FAILED) {
				task->uc.uc_stack.ss_sp=NULL;
				do_work(task->sv.TCB_no); //½øÐÐÄãµÄ·þÎñ,²»Ê¹ÓÃAIO
				continue;
			}
		} else {
//ShowLog(5,"%s:%lx reuse fiber for TCB_no=%d",__FUNCTION__,rs->tid,task->sv.TCB_no);
			task->uc.uc_stack.ss_sp=rs->tc.uc_stack.ss_sp;
			rs->tc.uc_stack.ss_sp=NULL;
			rs->tc.uc_stack.ss_size=0;
		}
		task->uc.uc_stack.ss_size=use_stack_size;
		task->uc.uc_link=&rs->tc;
		rs->tc.uc_link=(ucontext_t *)task;
		makecontext(&task->uc,(void (*)())do_work,1,task->sv.TCB_no);

		ret=swapcontext(&rs->tc,&task->uc);
		if(ret<0) {
			ShowLog(1,"%s:swapcontext fault TCB_NO=%d,tid=%lx,errno=%d,%s",
				__FUNCTION__,task->sv.TCB_no,pthread_self(),ret,strerror(abs(ret)));
			rs->tc.uc_link=NULL;
			task->uc.uc_link=NULL;
			if(task->uc.uc_stack.ss_sp)
				munmap(task->uc.uc_stack.ss_sp,task->uc.uc_stack.ss_size);
			task->uc.uc_stack.ss_sp=NULL;
			task->uc.uc_stack.ss_size=0;
			do_work(task->sv.TCB_no);
			continue;
		}
		if(!task) {
			ShowLog(1,"%s:aft swapcontext task is NULL",__FUNCTION__);
			continue;
		}
		if(!task->AIO_flg) {//service complate
			if(!rs->tc.uc_stack.ss_size) {//»ØÊÕfiber stack
//ShowLog(5,"%s:%lx release fiber from TCB_no=%d",__FUNCTION__,rs->tid,task->sv.TCB_no);
				rs->tc.uc_stack.ss_sp=task->uc.uc_stack.ss_sp;
				if(rs->tc.uc_stack.ss_sp)
					 rs->tc.uc_stack.ss_size=use_stack_size;
				else rs->tc.uc_stack.ss_size=0;
			} else {
ShowLog(5,"%s:%lx destroy fiber from TCB_no=%d",__FUNCTION__,rs->tid,task->sv.TCB_no);
				if(task->uc.uc_stack.ss_sp) 
					munmap(task->uc.uc_stack.ss_sp,task->uc.uc_stack.ss_size);
			}
			task->uc.uc_stack.ss_sp=NULL;
			rs->tc.uc_link=NULL;
			task->uc.uc_link=NULL;
			task->uc.uc_stack.ss_size=0;//mark fiber cpmplate
//ShowLog(5,"%s:TCB_no=%d,tid=%lx,timeout=%d,conn.timeout=%d",__FUNCTION__,task->sv.TCB_no,rs->tid,task->timeout,task->conn.timeout);
		} else {
			pthread_mutex_unlock(&task->lock);

ShowLog(5,"%s:tid=%lx,fiber yield from TCB_no=%d",
			__FUNCTION__,pthread_self(),task->sv.TCB_no);
		}
		mthr_showid_del(rs->tid);
	}
	ShowLog(1,"%s:tid=%lx canceled",__FUNCTION__,pthread_self());
	mthr_showid_del(rs->tid);
	rs->timestamp=now_usec();
	rs->status=0;
	rs->tid=0;
	if(rs->tc.uc_stack.ss_sp) {
		munmap(rs->tc.uc_stack.ss_sp,rs->tc.uc_stack.ss_size);
		rs->tc.uc_stack.ss_sp=NULL;
		rs->tc.uc_stack.ss_size=0;
	}
	return NULL;
}
//yield to schedle
static int do_event(int sock,int flg,int timeout)
{
TCB *task;
int save_fd,save_timeo=-1;
int ret;
pthread_t tid=pthread_self();
resource *rs=tpool.pool;

	for(ret=0;ret<tpool.num;ret++,rs++) { //ÕÒÔ­À´µÄÕ»
		if(tid == rs->tid) break;
	}
	if(ret>=tpool.num) return THREAD_ESCAPE;
	task=(TCB *)rs->tc.uc_link;
	if(!task || task->uc.uc_stack.ss_size == 0) return MEMERR;
	save_fd=task->fd;
	if(timeout>0) {
		save_timeo=task->timeout;
		task->timeout=timeout;
	}
	task->AIO_flg=flg+1;
	task->fd=sock;
	task->timestamp=now_usec();
	pthread_mutex_lock(&task->lock);//·ÀÖ¹ÆäËûÏß³ÌÌáÇ°´³Èë
	ret=do_epoll(task,0,flg);
	if(ret<0) {
		pthread_mutex_unlock(&task->lock);
		task->fd=save_fd;
		task->AIO_flg=0;
		if(save_timeo>-1) task->timeout=save_timeo;
		return FORMATERR;
	}
	ret=swapcontext(&task->uc,&rs->tc);
	pthread_mutex_unlock(&task->lock);
	task->fd=save_fd;
	task->AIO_flg=0;
	if(save_timeo>-1) task->timeout=save_timeo;
	if(flg==0 && task->events != EPOLLIN) {
		ShowLog(1,"%s:tid=%lx resumed events=%X",__FUNCTION__,
			pthread_self(),task->events);
		return TIMEOUTERR;
	}
	return ret;
}

static int do_yield(int socket,int rwflg,int timeout)
{
int ret=do_event(socket,rwflg,timeout);
	if(THREAD_ESCAPE == ret && other_yield)
		ret=other_yield(socket,rwflg,timeout);
	return ret;
}

//¼ì²é³¬Ê±µÄÁ¬½Ó
int check_TCB_timeout()
{
int i,cltnum=client_q.max_client;
TCB * task=client_q.pool;
INT64 now=now_usec();
int num=0,t;

        for(i=0;i<cltnum;i++,task++) {
		if(task->timeout<=0 || task->fd<0) continue;
               	t=(int)((now-task->timestamp)/1000000);
		if(t<task->timeout) continue;
                if(task->next) {
			ShowLog(2,"%s:TCB_no=%d,ÒÑ¾­ÔÚ¶ÓÁÐÖÐ",__FUNCTION__,i);
			continue;
		}
                if(task->call_back) {
			char buf[30];
			task->events=0X10;
			TCB_add(NULL,task->sv.TCB_no);
			ShowLog(5,"%s:TCB_no=%d,callback task->timeout=%d,since %s",__FUNCTION__,
				task->sv.TCB_no,task->timeout,
				rusecstrfmt(buf,task->timestamp,"YYYY-MM-DD HH24:MI:SS.FF6"));
                } else {
		  do_epoll(task,EPOLL_CTL_DEL,0);
		  task->events=0X10;
		  if(task->AIO_flg) {
			TCB_add(NULL,task->sv.TCB_no);
			ShowLog(1,"%s:TCB_no=%d.%d,AIO t=%d,task->timeout=%d",
				__FUNCTION__,i,task->sv.TCB_no,t,task->timeout);
		  } else {
			client_del(task);
			if(task->status<1) ShowLog(1,"%s:TCB:%d canceled,t=%d",__FUNCTION__,i,t);
			else ShowLog(1,"%s:TCB:%d deleted,t=%d",__FUNCTION__,i,t);
			num++;
		   }
		}
        }
        return num;
}

static int get_task_no()
{
int i,ret;
struct timespec abstime;
	abstime.tv_sec=0;
	pthread_mutex_lock(&client_q.mut);
	while(0>(i=TCB_get(&client_q.free_q))) {
		if(abstime.tv_sec==0) ShowLog(1,"%s:³¬¹ý×î´óÁ¬½ÓÊý£¡",__FUNCTION__);
		clock_gettime(CLOCK_REALTIME, &abstime);
		abstime.tv_sec+=5;
		ret=pthread_cond_timedwait(&client_q.cond,&client_q.mut,&abstime);
		if(ret==ETIMEDOUT) {
			pthread_mutex_unlock(&client_q.mut);
			check_TCB_timeout();
			pthread_mutex_lock(&client_q.mut);
		}
	}
	pthread_mutex_unlock(&client_q.mut);
	return i;
}

/***************************************************************
 * Ïß³Ì³ØÄ£ÐÍµÄÈë¿Úº¯Êý£¬Ö÷Ïß³ÌÖ÷Òª¸ºÔð×ÊÔ´¹ÜÀí
 *Ó¦ÓÃ²å¼þµÄ·þÎñº¯ÊýÔÚ
 * extern srvfunc Function[];
 */
int TPOOL_srv(void (*conn_init)(T_Connect *,T_NetHead *),void (*quit)(int),void (*poolchk)(void),int sizeof_gda)
{
int ret,i;
int s;
struct sockaddr_in sin,cin;
struct servent *sp;
char *p;
struct timeval tm;
fd_set efds;
socklen_t leng=1;
int sock=-1;
srvfunc *fp;
TCB *task;
struct linger so_linger;

	signal(SIGPIPE,SIG_IGN);
	signal(SIGHUP,SIG_IGN);
	signal(SIGINT ,SIG_IGN);
	signal(SIGPWR ,quit);
	signal(SIGTERM,quit);

	p=getenv("SERVICE");
	if(!p || !*p) {
		ShowLog(1,"È±ÉÙ»·¾³±äÁ¿ SERVICE ,²»ÖªÊØºòÄÄ¸ö¶Ë¿Ú£¡");
		quit(3);
	}
//²âÊÔ¶Ë¿ÚÊÇ·ñ±»Õ¼ÓÃ 
	sock=tcpopen("localhost",p);
	if(sock>-1) {
		ShowLog(1,"¶Ë¿Ú %s ÒÑ¾­±»Õ¼ÓÃ",p);
		close(sock);
		sock=-1;
		quit(255);
	}

	ret=tpool_init(sizeof_gda);
	if(ret) return(ret);

	for(fp=Function;fp->funcaddr!=0;fp++) rpool.svc_num++;

	other_yield=set_yield(do_yield);

	bzero(&sin,sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = INADDR_ANY;

	if(isdigit(*p)){
		sin.sin_port=htons((u_short)atoi(p));
	} else {
		if((sp=getservbyname(p,"tcp"))==NULL){
        		ShowLog(1,"getsrvbyname %s error",p);
        		quit(3);
		}
		sin.sin_port=(u_short)sp->s_port;
	}

	sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0) {
		ShowLog(1,"open socket error=%d,%s",errno,
			strerror(errno));
		quit(3);
	}

	bind(sock,(struct sockaddr *)&sin,sizeof(sin));
	leng=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&leng,sizeof(leng));
//±ÜÃâ TIME_WAIT
        so_linger.l_onoff=1;
        so_linger.l_linger=0;
        ret=setsockopt(sock, SOL_SOCKET, SO_LINGER, &so_linger, sizeof so_linger);
        if(ret) ShowLog(1,"set SO_LINGER err=%d,%s",errno,strerror(errno));

	listen(sock,client_q.max_client);

	ShowLog(0,"main start tid=%lx sock=%d",pthread_self(),sock);
	
	int repeat=0;
	leng=sizeof(cin);

	while(1) {
		do {
			FD_ZERO(&efds);
			FD_SET(sock, &efds);
//½¡¿µ¼ì²éÖÜÆÚ
			tm.tv_sec=15;
			tm.tv_usec=0;
			ret=select(sock+1,&efds,NULL,&efds,&tm);
			if(ret==-1) {
				ShowLog(1,"select error %s",strerror(errno));
				close(sock);
				quit(3);
			}
			if(ret==0) {
				check_TCB_timeout();
				if(poolchk) poolchk();
			}
		} while(ret<=0);
		i=get_task_no();
		task=&client_q.pool[i];
		s=accept(sock,(struct sockaddr *)&cin,&leng);
		if(s<0) {
			ShowLog(1,"%s:accept err=%d,%s",__FUNCTION__,errno,strerror(errno));
                       	client_del(task);
			switch(errno) {
			case EMFILE:	//fdÓÃÍêÁË,ÆäËûÏß³Ì»¹Òª¼ÌÐø¹¤×÷£¬Ö÷Ïß³ÌÐÝÏ¢Ò»ÏÂ¡£  
			case ENFILE:
				usleep(30000000);
				continue;
			default:break;
			}
			usleep(15000000);
			if(++repeat < 20) continue;
			ShowLog(1,"%s:network fail! err=%s",__FUNCTION__,strerror(errno));
			close(sock);
			quit(5);
		}
		repeat=0;
		task->fd=task->conn.Socket=s;
		task->timestamp=now_usec();
		task->timeout=60;
		task->status=-1;
		task->conn.only_do=(int (*)())conn_init;
		ret=do_epoll(task,EPOLL_CTL_ADD,0);//ÈÎÎñ½»¸øÆäËûÏß³Ì×ö
	}
	
	close(sock);
	tpool_free();
	return (0);
}

