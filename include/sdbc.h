/*******************************************************
 * Secure Database Connect
 * SDBC 6.0 for ORACLE 
 * 2012.09.15 by ylh
 *******************************************************/

#ifndef SDBCDEF
#define SDBCDEF

#include <sqli.h>
#include <scsrv.h>
#include <json_pack.h>

#ifdef __cplusplus
extern "C" {
#endif
/* Server database interface functions */
extern int SQL_OpenDatabase(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_Prepare(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_Select(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_Exec(T_Connect *connect ,T_NetHead *NetHead);
extern int SQL_Fetch(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_Close(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_RPC(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_CloseDatabase(T_Connect *connect,T_NetHead *NetHead);
extern int SQL_EndTran(T_Connect *connect,T_NetHead *NetHead);
extern int locktab(T_Connect *connect,T_NetHead *NetHead);
//extern int SQL_Quit(T_SRV_Var *);
extern const char * _get_data_type_str(int dtype);
/************************************************
 * mod_DB.c:数据库连接池高级管理器
 ************************************************/ 
/**
 * DB_connect_MGR:向数据库连接池高级管理器申请数据库连接(mod_DB.c)
 * 返回值:0:成功取得连接。1:连接忙，你的任务排入队列，-1:出错。
 */
int DB_connect_MGR(int TCBno,int poolno,T_SQL_Connect **sqlp,sdbcfunc call_back);
/* 释放mod_DB的队列结构   */
void mod_DB_free(void);
/* 取服务器结构 */
T_SRV_Var * get_SRV_Var(int TCBno);
/******************************************************
 * bind_DB.c
 * 配合mod_DB,用于取得数据库连接后，传递给相应的TCB,
 * 或从相应的TCB中除去之。 可以被其他应用需求重载
 *****************************************************/
int bind_DB(int TCBno,T_SQL_Connect *sql);
int unbind_DB(int TCBno);

int dui(T_Connect *conn,T_NetHead *nethead);
int page_select(T_Connect *conn,T_NetHead *nethead);

//清空服务器中的模板库
int tpl_cancel(T_Connect *conn,T_NetHead *head);
//取数据库表模板
//head->data="表名1,表名2,,,"
//返回JSON对象:{表名:[模板],表名:[模板],...}
int get_tpl(T_Connect *conn,T_NetHead *head);

#ifdef __cplusplus
}
#endif

#endif
