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
	int poolno; //Ê¹ÓÃµÄÊı¾İ¿âÁ¬½Ó³ØºÅ  
	int TCB_no; //Ïß³Ì³Ø·şÎñÆ÷ÖĞµÄ»á»°ºÅ
	int o_timeout;//ÔÚ×´Ì¬·şÎñÖĞ±£´æÔ­À´µÄTIMEOUTÖµ
} T_SRV_Var;

#ifdef __cplusplus
extern "C" {
#endif
/* ·şÎñÆ÷¶ËÓĞÊÂ¼şÒªÍ¨Öª¿Í»§¶Ë£¬SendPackÇ°,PROTO_NUM=PutEvent(conn,PROTO_NUM);
   ÊÂÇ°£¬conn->Event_procÒªÖÃÈëÊÂ¼ş´¦Àí³ÌĞò£¬·µ»ØÖµÎªÊÂ¼şºÅ,1-65535¡£
   ×¢Òâ£ºÊÂ¼ş´¦Àí³ÌĞò²»¿ÉÒÔ¶¯ÓÃÍøÂç×ÊÔ´£¬µ«¿ÉÒÔ¶¯ÓÃÊı¾İ¿â×ÊÔ´ */
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
 * Ò»¸ö»ùÓÚ¶àÏß³ÌµÄÁ¬½Ó³Ø·şÎñÆ÷¿ò¼Ü NetMain_r £¬Àı×ÓÔÚ£º
 * ../utility/thread/
 * TPOOL_srv:Thread Pool Server
 * Ò»¸ö»ùÓÚepollµÄÏß³Ì³Ø·şÎñÆ÷¿ò¼Ü TPOOL_srv£¬ÓÃÓÚ¸ßÍÌÍÂÁ¿µÄOLTP´¦Àí £¬Àı×ÓÔÚ£º
 * ../utility/tpool/
 * conn_init:ĞÂÏß³Ì¿ªÊ¼ºó£¬ÇëÄãÔ¤´¦Àí 
 * quit:·şÎñÆ÷ÍË³öº¯Êı
 * poolchk:¶¨ÆÚ¼ì²é³Ø½¡¿µµÄº¯Êı 
 * sizeof_gda:ÓÃ»§ÉÏÏÂÎÄÊı¾İµÄ³¤¶È,ÉÏÏÂÎÄ¿Õ¼äÔÚÏß³ÌÖĞ·ÖÅä  
 * ±ØĞëÌá¹©È«¾Ö·şÎñº¯Êı×é:
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
 * Çå³ıÓÃ»§×Ô¶¨Òå»Øµ÷º¯Êı
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return Ô­»Øµ÷º¯Êı
 */
sdbcfunc  unset_callback(int TCB_no);

/**
 * get_callback
 * È¡ÓÃ»§×Ô¶¨Òå»Øµ÷º¯Êış
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return »Øµ÷º¯Êı
 */

sdbcfunc get_callback(int TCB_no);
/**
 * get_event_status
 * È¡TCB×´Ì¬
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return TCB×´Ì¬
 */

/**
 * set_event
 * ÓÃ»§×Ô¶¨ÒåÊÂ¼ş
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @param fd ÊÂ¼şfd Ö»Ö§³Ö¶ÁÊÂ¼ş 
 * @param call_back ·¢ÉúÊÂ¼şµÄ»Øµ÷º¯Êı
 * @param timeout fdµÄ³¬Ê±Ãë,Ö»ÔÊĞíÉèÖÃsocket fd 
 * @return ³É¹¦ 0
 */
int set_event(int TCB_no,int fd,sdbcfunc call_back,int timeout);
/**
 * clr_event
 * Çå³ıÓÃ»§×Ô¶¨ÒåÊÂ¼ş »Øµ÷º¯ÊıÍê³ÉÈÎÎñºóÓ¦Çå³ıÊÂ¼ş¡£
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return ³É¹¦ 0
 */
int clr_event(int TCB_no);
/**
 * get_event_fd
 * È¡ÊÂ¼şfd,ÓÃÓÚÔÚ»Øµ÷º¯ÊıÖĞÈ¡µÃÊÂ¼şfd 
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return ÊÂ¼şfd
 */
int get_event_fd(int TCB_no);
/**
 * get_event_status
 * È¡ÊÂ¼ş×´Ì¬
 * @param TCB_no ¿Í»§ÈÎÎñºÅ
 * @return ÊÂ¼ş×´Ì¬
 */
int get_event_status(int TCB_no);

//clikey.c
/* Ğ­ÉÌÃÜÔ¿ */
int mk_clikey(int socket,ENIGMA2 *tc,u_int *family);

/* È¡·şÎñÆ÷µ0·şÎñº¯Êı±í£¬×¢²á³É¹¦ºóÌæ»»0ºÅĞ­Òé * */
int get_srvname(T_Connect *conn,T_NetHead *NetHead);
//¶¯Ì¬Ó¦ÓÃÄ£¿é
int dmapp(T_Connect *conn,T_NetHead *head);
int dmmgr(T_Connect *conn,T_NetHead *head);


int AIO_read(int fd,char *buff,size_t iosize);
int AIO_write(int fd,char *buff,size_t iosize);

int return_error(T_Connect *conn,T_NetHead *nethead,const char *msg);

#ifdef __cplusplus
}
#endif

#endif
