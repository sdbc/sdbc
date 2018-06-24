/*******************************************************
 * Secure Database Connect
 * SDBC 6.0 for ORACLE
 * 2012.9.19 by ylh
 *******************************************************/

#ifndef SCSRVDEF
#define SCSRVDEF
#include <pack.h>
#include <sc.h>

typedef struct {
	sdbcfunc funcaddr;
	const char *srvname;
} srvfunc;

typedef struct {
	struct S_SQL_Connect *SQL_Connect;
	pthread_t tid;
	void *var;
	int poolno; //使用的数据库连接池号
	int TCB_no; //线程池服务器中的会话号
	int o_timeout;//在状态服务中保存原来的TIMEOUT值
} T_SRV_Var;

#ifdef __cplusplus
extern "C" {
#endif
/* 服务器端有事件要通知客户端，SendPack前,PROTO_NUM=PutEvent(conn,PROTO_NUM);
   事前，conn->Event_proc要置入事件处理程序，返回值为事件号,1-65535。
   注意：事件处理程序不可以动用网络资源，但可以动用数据库资源 */
int PutEvent(T_Connect *conn,int Evtno);

/* Server system interface functions */
extern int Rexec(T_Connect *connect,T_NetHead *NetHead);
extern int GetFile(T_Connect *connect,T_NetHead *NetHead);
extern int GetFile1(T_Connect *connect,T_NetHead *NetHead);
extern int PutFile(T_Connect *connect,T_NetHead *NetHead);
extern int Pwd(T_Connect *connect,T_NetHead *NetHead);
extern int ChDir(T_Connect *connect,T_NetHead *NetHead);
extern int filels(T_Connect *connect,T_NetHead *NetHead);
extern int PutEnv(T_Connect *connect,T_NetHead *NetHead);
extern int Echo(T_Connect *connect,T_NetHead *NetHead);
/*************************************************************
 * Process Per Connection  Server
 * PPC_srv(): server entry poit
 * cryptflg=0:not crypt,can be DO_CTYPT,or DO_CRYPT+CHECK_CRC
 *************************************************************/
int PPC_srv( void (*NetInit)(T_Connect *,T_NetHead *),
			 srvfunc *func, void *userdata);
extern void setquit (void (*Quit)());
/***********************************************************
 * TPC_srv:Thread Per Connection  Server
 * 一个基于多线程的连接池服务器框架 NetMain_r ，例子在：
 * ../utility/thread/
 * TPOOL_srv:Thread Pool Server
 * 一个基于epoll的线程池服务器框架 TPOOL_srv，用于高吞吐量的OLTP处理 ，例子在：
 * ../utility/tpool/
 * conn_init:新线程开始后，请你预处理
 * quit:服务器退出函数
 * poolchk:定期检查池健康的函数
 * sizeof_gda:用户上下文数据的长度,上下文空间在线程中分配
 * 必须提供全局服务函数组:
 * svcfunc Function[];
 ***********************************************************/

void TPC_srv(void (*conn_init)(T_Connect *,T_NetHead *),void (*quit)(int),void (*poolchk)(void),int sizeof_gda);
//tpool.c
int TPOOL_srv(void (*conn_init)(T_Connect *,T_NetHead *),void (*quit)(int),void (*poolchk)(void),int sizeof_gda);
void tpool_free();

struct event_node;
typedef struct event_node *pTCB;

int TCB_add(pTCB *queue,int TCBno);
int TCB_get(pTCB *queue);
sdbcfunc set_callback(int TCBno,sdbcfunc callback,int timeout);

T_Connect *get_TCB_connect(int TCBno);
void *get_TCB_ctx(int TCBno);
int get_TCB_status(int TCB_no);
T_NetHead *getNetHead(int TCBno);
/**
 * unset_callback
 * 清除用户自定义回调函数
 * @param TCB_no 客户任务号
 * @return 原回调函数
 */
sdbcfunc  unset_callback(int TCB_no);

/**
 * get_callback
 * 取用户自定义回调函数�
 * @param TCB_no 客户任务号
 * @return 回调函数
 */

sdbcfunc get_callback(int TCB_no);
/**
 * get_event_status
 * 取TCB状态
 * @param TCB_no 客户任务号
 * @return TCB状态
 */

/**
 * set_event
 * 用户自定义事件
 * @param TCB_no 客户任务号
 * @param fd 事件fd 只支持读事件
 * @param call_back 发生事件的回调函数
 * @param timeout fd的超时秒,只允许设置socket fd
 * @return 成功 0
 */
int set_event(int TCB_no,int fd,sdbcfunc call_back,int timeout);
/**
 * clr_event
 * 清除用户自定义事件 回调函数完成任务后应清除事件。
 * @param TCB_no 客户任务号
 * @return 成功 0
 */
int clr_event(int TCB_no);
/**
 * get_event_fd
 * 取事件fd,用于在回调函数中取得事件fd
 * @param TCB_no 客户任务号
 * @return 事件fd
 */
int get_event_fd(int TCB_no);
/**
 * get_event_status
 * 取事件状态
 * @param TCB_no 客户任务号
 * @return 事件状态
 */
int get_event_status(int TCB_no);

//clikey.c
/* 协商密钥 */
int mk_clikey(int socket,ENIGMA2 *tc,u_int *family);

/* 取服务器�0服务函数表，注册成功后替换0号协议 * */
int get_srvname(T_Connect *conn,T_NetHead *NetHead);
//动态应用模块
int dmapp(T_Connect *conn,T_NetHead *head);
int dmmgr(T_Connect *conn,T_NetHead *head);


int AIO_read(int fd,char *buff,size_t iosize);
int AIO_write(int fd,char *buff,size_t iosize);

int return_error(T_Connect *conn,T_NetHead *nethead,const char *msg);

#ifdef __cplusplus
}
#endif

#endif
