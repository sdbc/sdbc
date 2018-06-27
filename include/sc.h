/*******************************************
 * sc.h: Secure Connect Package
 * 2014.12 by YuliHua for SDBC7.1
 * 支持Fiberized.IO
 *******************************************/
#ifndef HEADPACKLENGTH

#include <errno.h>
#include <sys/types.h>
#include <strproc.h>
#include <enigma.h>

#define HEADPACKLENGTH 36

#ifndef INTNULL
#define INTNULL 0X80000000
#endif

#define		PARANUM 9
#define 	PROTO_NUM   para[0]     	/*协议号:
					客户呼叫服务器时是调用号，
					服务器返回时是事件号，1－65535 */
#define		ERRNO1	    para[1]     	/*主错误码	*/
#define		ERRNO2	    para[2]     	/*辅助错误码	*/
#define		PKG_REC_NUM para[3]      	/*数据记录数	*/
#define		PKG_LEN	    para[4]        	/*数据包长度	*/
#define		T_LEN	    para[5]   		/* 传输长度  */
#define		O_NODE      para[6]      	/*原结点地址*/
#define		D_NODE	    para[7]		/*目的结点描述*/
#define		PKG_CRC	    para[8]		/*数据包CRC */

typedef unsigned int u_int;

typedef struct {             	/*协议头	*/
	int	para[PARANUM];
	char	*data;
} T_NetHead;

typedef struct S_Connect {
	int	Socket;			/*socket*/
	char	Host[81];		/*主机名或地址*/
	char	Service[21];		/*信口名或编号*/
	u_int   *family;		/* 身份识别密钥 */
	INT4	SendLen;
	char	*SendBuffer;		/*发送缓冲区*/
	INT4	RecvLen;
	char	*RecvBuffer;		/*接受缓冲区*/
	int	CryptFlg;		/* 加密标志 */
	ENIGMA2 t; 			/* 加密参数区 */
	void 	*Var;  			/* 用户定义数据指针 Free By user,*/
	void	(*freevar)(void *); 	/* offer function FreeVar(void *p);*/
	int	(*only_do)(struct S_Connect *,T_NetHead *);
	unsigned int 	timeout;	/* for second */
	unsigned int	MTU;
	unsigned int 	status;		/* 服务状态,0:无状态 */
/* 事件处理函数的地址 */
	int	(*Event_proc)(struct S_Connect *conn,int id);
	unsigned int pos; //连接池用
} T_Connect;

/* define for T_Connect.CryptFlg*/
#define DO_CRYPT 3
#define CHECK_CRC 4
#define DO_ZIP 8
#define UNDO_ZIP 0X80000000

#define SDBC_BLKSZ 65536
/* NetHead->ERRNO2,转发器标志 */
#define PACK_CONTINUE 0X80000000

/* 客户端通知转发器，该包不需要服务器回答 */
#define PACK_NOANSER ((PACK_CONTINUE)|1)

/* NetHead->ERRNO2,有状态服务标志 */
#define PACK_STATUS 0X80000002
typedef int (*sdbcfunc)(T_Connect *conn,T_NetHead *head);

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*T_YIELD)(int socket,int rwflg,int timeout);
T_YIELD get_yield(void);
T_YIELD set_yield(T_YIELD new_yield);
int quick_send_pkg(T_Connect *connect,T_NetHead *nethead);
int  SendPack(T_Connect *connect,T_NetHead *nethead);
int  RecvPack(T_Connect *connect,T_NetHead *nethead);
int peeraddr(int socket,char in_addr[16]);
char *StrAddr(INT4 addr,char str[16]);
INT4 LongAddr(char *p);
INT4 LocalAddr(int Sock, char szAddr[16]);
int tcpopen(char *host,char *server);
int SendNet(int socket,char *buf,int len,int MTU);
//timout for second
int RecvNet(int s,char *buf,int n,int timeout);
/*********************************************
 * init the T_Connect
 *********************************************/
void initconnect(T_Connect *connect);

/***************************************
 * client end
 * disconnect, free T_Connect.
 ***************************************/
void disconnect(T_Connect *conn);
void freeconnect(T_Connect *conn);

#ifdef __cplusplus
}
#endif

#endif

