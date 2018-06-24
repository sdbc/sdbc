/*******************************************************
 * Secure Database Connect
 * SDBC 7.0 for ORACLE
 * 2012.09.15 by ylh
 *******************************************************/

#ifndef SCLIDEF
#define SCLIDEF
#include <pack.h>
#include <json_pack.h>
#include <sc.h>

#ifndef TRAINBEGIN

#define TRANBEGIN 0
#define TRANCOMMIT 1
#define TRANROLLBACK 2

/* For ORACLE */
#define SQLNOTFOUND 1403
#define FETCHEND    100
#define DUPKEY   1
#define LOCKED 		 54

#endif
typedef struct {
	int usage;
	int srvn;		/* 服务个数 */
	char *srvlist;          /* 服务名列表 */
	char *srv_hash;		/* 服务名索引 */
} svc_table;

typedef struct {
	char DSN[81];			/*database server name,no used*/
	char UID[81];			/*database user ID*/
	char PWD[81];			/*database user password, no used*/
	char DBOWN[81];			/* database owner */
	int Errno;
	int NativeError;
	char ErrMsg[2048];
	char SqlState[128];
	svc_table *svc_tbl;
	unsigned int ctx_id;            //32bit context id,copy to NetNead->O_NODE
	void *var;
} T_CLI_Var;

/* Client system functions */
#ifdef __cplusplus
extern "C" {
#endif

/* used by client */
extern void Init_CLI_Var(T_CLI_Var *CLI_Var);
int Net_Connect( T_Connect *conn,void *userdata,u_int *new_family);

/* 如果客户端要处理事件,收到服务器应答后，服务器的事件号放在 PROTO_NUM里,1-65535
 * ,应记录PROTO_NUM,在本次会话全部完成后 调用EventCatch(conn,PROTO_NUM),
 * 事前，conn->Event_proc要置入事件捕获程序，事件捕获程序可以动用网络资源。
*/
int EventCatch(T_Connect *conn,int Evtno);
extern int N_Rexec(T_Connect *connect, char *cmd, int (*input)(char *), void (*output)(char *));
extern int N_Put_File(T_Connect *connect,char *local_file,char *remote_file);
extern int N_Put_File_Msg(T_Connect *connect,char *local_file,char *remote_file, void (*Msg)(int,int));
extern int N_Get_File(T_Connect *connect,char *local_file,char *remote_file);
extern int N_Get_File_Msg(T_Connect *connect, char *local_file, char *remote_file, void (*Msg)(int,int));
extern int N_PWD(T_Connect *connect,char *pwd_buf);
extern int N_ChDir(T_Connect *connect,char *dir);
extern int N_PutEnv(T_Connect *connect,char *env);

/* Client database functions */
int N_SQL_Prepare(T_Connect *conn,char *stmt,T_SqlDa *sqlda);
/* Fetch() recnum:进入时每次Fetch的记录数,0=全部记录,返回实际得到的记录数,NativeError:列数 */
extern int N_SQL_Fetch(T_Connect *,int curno,char **result,int recnum);
/* Select() recnum:进入时希望取得的记录数,0=全部记录,返回实际得到的记录数,NativeError:列数 */
int N_SQL_Select(T_Connect *,char *cmd,char **data,int recnum);
extern int N_SQL_Close_RefCursor(T_Connect *connect,int ref_cursor);
int N_SQL_Close(T_Connect *conn,T_SqlDa *sqlda);
extern int N_SQL_Exec(T_Connect *conn,char * cmd);
/* st_lvs:状态级别，有几个游标类型就写几 */
extern int N_SQL_RPC(T_Connect *connect,char *stmt,char **data,int *ncols,int st_lvs);
extern int N_SQL_EndTran(T_Connect *connect,int TranFlag);
extern int N_insert_db(T_Connect *conn,char *tabname,void *data,T_PkgType *tp);
extern int getls(T_Connect *conn,T_NetHead *NetHead,char *path,FILE *outfile);

//查找服务号，无此服务返回1,=echo()
// key=服务名，返回服务号
int get_srv_no(T_CLI_Var *clip,const char *key);
//释放服务名相关数据
void free_srv_list(T_CLI_Var *clip);
/*******************************************************
 * 取得服务器端服务名列表
 * 服务器端的0号函数必须是登录认证函数。登录认证完成后，
 * 必须把0号函数值换成
 *      get_srvname();
 * 1号函数必须是
 *   echo();
 * conn->freevar指向free_srv_list,如果需要其他善后操作
 * 应重置，但最后应补做
 * free_srv_list(T_CLI_Var *);
 * 成功返回0，失败-1；
 *******************************************************/
int init_svc_no(T_Connect *conn);
/************************************************************
 * N_get_tpl:从服务器取得表模板
 * tabnames="表名1,表名2,,"
 * 返回JSON对象:{表名:[模板],表名:[模板],...}
 * **********************************************************/
JSON_OBJECT N_get_tpl(T_Connect *conn,char *tabnames,int num);

#ifdef __cplusplus
}
#endif

#endif

