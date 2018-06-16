#include <malloc.h>
#include <sccli.h>

typedef T_CLI_Var T_User_Var;

int get_d_node()
{
char *p;
	p=getenv("DNODE");
	if(!p || !*p) return 0;
	else return atoi(p);
}
static int GetError(T_Connect *conn,T_NetHead *NetHead)
{
register T_User_Var *up;
	
        up=(T_User_Var *)conn->Var;
       	*up->ErrMsg=0;
        up->NativeError=NetHead->PKG_REC_NUM;
       	up->Errno=NetHead->ERRNO2;
	if(NetHead->PKG_LEN) {
			stptok(NetHead->data,up->ErrMsg,sizeof(up->ErrMsg),NULL);
	}
	EventCatch(conn,NetHead->PROTO_NUM);
	return -abs(NetHead->ERRNO1);
}

int  N_SQL_EndTran(T_Connect *connect,int TranFlag)
{
T_NetHead nethead;
T_CLI_Var *clip=(T_CLI_Var *)connect->Var;
int i;
	nethead.PROTO_NUM=get_srv_no(connect->Var,"SQL_EndTran");
	if(nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:服务不存在",__FUNCTION__);
		return FORMATERR;
	}
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=get_d_node();
	if(TranFlag == TRANBEGIN) connect->status++;
	else connect->status--;
	nethead.ERRNO2=(connect->status>0)?PACK_STATUS:0;
	nethead.ERRNO1=TranFlag;
	nethead.PKG_REC_NUM=0;
	nethead.data=0;
	nethead.PKG_LEN=0;
	i=SendPack(connect,&nethead);
	if(i)return i;
	i=RecvPack(connect,&nethead);
	if(i)return i;
	EventCatch(connect,nethead.PROTO_NUM);
	return GetError(connect,&nethead);
}

int  N_SQL_Select(T_Connect *connect,char *cmd,char **data,int num)
{
T_NetHead nethead;
T_CLI_Var *clip=(T_CLI_Var *)connect->Var;
int i;
	nethead.PROTO_NUM=get_srv_no(connect->Var,"SQL_Select");
	if(nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:服务不存在",__FUNCTION__);
		return FORMATERR;
	}
	nethead.ERRNO1=0;
	nethead.ERRNO2=(connect->status>0)?PACK_STATUS:0;
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=get_d_node();
	if(num) nethead.PKG_REC_NUM=num;
	else nethead.PKG_REC_NUM=0;
	nethead.data=cmd;
	nethead.PKG_LEN=strlen(nethead.data);
	i=SendPack(connect,&nethead);
	if(i<0)return i;
	i=RecvPack(connect,&nethead);
	if(i<0)return i;
	EventCatch(connect,nethead.PROTO_NUM);
	if(nethead.ERRNO1)  return GetError(connect,&nethead);
        *((T_User_Var *)connect->Var)->ErrMsg=0;
        ((T_User_Var *)connect->Var)->Errno=nethead.ERRNO1;
	*data=nethead.data;
/* column num */
	((T_User_Var *)connect->Var)->NativeError=nethead.ERRNO2;
	return nethead.PKG_REC_NUM;
}

int  N_SQL_Exec(T_Connect *connect,char *cmd)
{
T_CLI_Var *clip=(T_CLI_Var *)connect->Var;
T_NetHead nethead;
int i;
	nethead.PROTO_NUM=get_srv_no(connect->Var,"SQL_Exec");
	if(nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:服务不存在",__FUNCTION__);
		return FORMATERR;
	}
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=get_d_node();
	nethead.ERRNO1=0;
	nethead.ERRNO2=(connect->status>0)?PACK_STATUS:0;
	nethead.PKG_REC_NUM=0;
	nethead.data=cmd;
	nethead.PKG_LEN=strlen(nethead.data);
	i=SendPack(connect,&nethead);
	if(i)return i;
	i=RecvPack(connect,&nethead);
//ShowLog(5,"SQL_Exec:ret=%d,errno1=%d,len=%d:%s",i,nethead.ERRNO1,nethead.PKG_LEN,nethead.data);
	if(i){
		return i;
	}
	EventCatch(connect,nethead.PROTO_NUM);
	return GetError(connect,&nethead);
}
int  N_SQL_Prepare(T_Connect *connect,char *cmd,T_SqlDa *sqlda)
{
T_CLI_Var *clip=(T_CLI_Var *)connect->Var;
T_NetHead nethead;
int i;
char *p;

	nethead.PROTO_NUM=get_srv_no(connect->Var,"SQL_Prepare");
	if(nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:服务不存在",__FUNCTION__);
		return FORMATERR;
	}
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=get_d_node();
	nethead.ERRNO1=0;
	connect->status++;
	nethead.ERRNO2=PACK_STATUS;
	nethead.PKG_REC_NUM=0;
	nethead.data=cmd;
	nethead.PKG_LEN=strlen(nethead.data);
	i=SendPack(connect,&nethead);
	if(i) {
		connect->status--;
		return i;
	}
	i=RecvPack(connect,&nethead);
	if(i) {
		connect->status--;
		return i;
	}
	if((int)nethead.ERRNO1<0) {
		int ret=GetError(connect,&nethead);
		return ret;
	}
	if(sqlda) {
		sqlda->cursor_no=nethead.ERRNO1;
		sqlda->cols=nethead.PKG_REC_NUM;
		if(nethead.PKG_LEN>0) {
			sqlda->sqlvar=(T_SqlVar *)malloc(sizeof(T_SqlVar)*sqlda->cols);
			if(sqlda->sqlvar) {
			    p=nethead.data;
			    for(i=0;i<sqlda->cols;i++) {
				p+=net_dispack(&sqlda->sqlvar[i],p,SqlVarType);
			    }
			}
		} else sqlda->sqlvar=0;
	}
       	((T_CLI_Var *)connect->Var)->Errno=0;
       	((T_CLI_Var *)connect->Var)->NativeError=nethead.PKG_REC_NUM;
        *((T_CLI_Var *)connect->Var)->ErrMsg=0;
	EventCatch(connect,nethead.PROTO_NUM);
	return nethead.ERRNO1;;
}

int  N_SQL_Fetch(T_Connect *connect,int curno,char **data,int recnum)
{
T_CLI_Var *clip=(T_CLI_Var *)connect->Var;
T_NetHead nethead;
int i;
	nethead.PROTO_NUM=get_srv_no(connect->Var,"SQL_Fetch");
	if(nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:服务不存在",__FUNCTION__);
		return FORMATERR;
	}
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=get_d_node();
	nethead.ERRNO2=PACK_STATUS;
	nethead.ERRNO1=curno;
/* recnum:进入时每次Fetch的记录数，0=全部记录 */
	nethead.PKG_REC_NUM=recnum;
	nethead.data=0;
	nethead.PKG_LEN=0;
	i=SendPack(connect,&nethead);
	if(i<0)return i;
	i=RecvPack(connect,&nethead);
	if(i<0)return i;
	EventCatch(connect,nethead.PROTO_NUM);
	if(!nethead.ERRNO1) {
		*data=nethead.data;
        	*((T_User_Var *)connect->Var)->ErrMsg=0;
       		((T_User_Var *)connect->Var)->Errno=
					nethead.ERRNO1;
/* colunm num */
       		((T_User_Var *)connect->Var)->NativeError=
					nethead.ERRNO2;
/* 返回实际得到的记录数 */
		return nethead.PKG_REC_NUM;
	} else return GetError(connect,&nethead);
}

int  N_SQL_Close_RefCursor(T_Connect *connect,int ref_cursor)
{
T_CLI_Var *clip=(T_CLI_Var *)connect->Var;
T_NetHead nethead;
int i;

	nethead.PROTO_NUM=get_srv_no(connect->Var,"SQL_Close");
	if(nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:服务不存在",__FUNCTION__);
		return FORMATERR;
	}
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=0;
	connect->status--;
	nethead.ERRNO1=ref_cursor;
	nethead.PKG_REC_NUM=0;
	nethead.data=NULL;
	nethead.PKG_LEN=0;
	if(ref_cursor<0) {
//终止中间件的状态
		if(connect->status <= 0) {
			connect->status=0;
//请求夭折状态
			nethead.ERRNO2=PACK_NOANSER;
			SendPack(connect,&nethead);
		}
		return 0;
	}
	nethead.ERRNO2=(connect->status>0)?PACK_STATUS:0;
	i=SendPack(connect,&nethead);
	if(i)return i;
	i=RecvPack(connect,&nethead);
	if(i)return i;
	EventCatch(connect,nethead.PROTO_NUM);
	return GetError(connect,&nethead);
}

int  N_SQL_Close(T_Connect *connect,T_SqlDa *sqlda)
{
int i;
	i=N_SQL_Close_RefCursor(connect,sqlda->cursor_no);
	if(i) return i;
	if(sqlda->sqlvar) free(sqlda->sqlvar);
	sqlda->cols=0;
	sqlda->sqlvar=0;
	return i;
}

int N_insert_db(T_Connect *conn,char *tabname,void *data,T_PkgType *tp)
{
char stmt[32768];
char *p;
T_CLI_Var *clip;
	clip=(T_CLI_Var *)conn->Var;
	p=stmt;
        p+=sprintf(p,"INSERT INTO ");
        if(*clip->DBOWN)
                p+=sprintf(p,"%s.",clip->DBOWN);
        p+=sprintf(p,"%s (",tabname);
	p=mkset(p,tp);
	p+=sprintf(p,") VALUES(");
	p=mk_values(p,data,tp);
	p+=sprintf(p,")");
        return N_SQL_Exec(conn,stmt);
}
/* only used by SYBASE 
int  N_SQL_Cur_Exec(T_Connect *connect,int cur_no,char *cmd)
{
T_CLI_Var *clip=(T_CLI_Var *)connect->Var;
T_NetHead nethead;
int i;

	nethead.PROTO_NUM=9;
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=get_d_node();
	nethead.ERRNO2=(connect->status>0)?PACK_STATUS:0;
	nethead.ERRNO1=cur_no;
	nethead.PKG_REC_NUM=0;
	nethead.data=cmd;
	nethead.PKG_LEN=strlen(nethead.data);
	i=SendPack(connect,&nethead);
	if(i)return i;
	i=RecvPack(connect,&nethead);
//ShowLog(5,"SQL_Exec:ret=%d,errno1=%d,len=%d:%s",i,nethead.ERRNO1,nethead.PKG_LEN,nethead.data);
	if(i){
		return i;
	}
	return GetError(connect,&nethead);
}
*/
/*****************************************************************
 * ORACLE Stored Procedure Call
 * cmd: RPC_NAME(value,:TYPE(Format) Value,.......)
 * TYPE is:num,int,double,date,cursor,char, default is char;
 * data:address of point:  restlt1|..result n|value1|value2|....value n|
 * return:if != 0:error。
 *	else:
 *      SQL_Connect->NativeError=result_num;
 ****************************************************************/

int  N_SQL_RPC(T_Connect *connect,char *cmd,char **data,int *nrets,int st_lev)
{
T_NetHead nethead;
int i;
T_CLI_Var *clip;
	clip=(T_CLI_Var *)connect->Var;
	nethead.PROTO_NUM=get_srv_no(connect->Var,"SQL_RPC");
	if(nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:服务不存在",__FUNCTION__);
		return FORMATERR;
	}
	connect->status += st_lev;
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=get_d_node();
	nethead.ERRNO1=0;
	nethead.ERRNO2=(connect->status>0)?PACK_STATUS:0;
	nethead.PKG_REC_NUM=0;
	nethead.data=cmd;
	nethead.PKG_LEN=strlen(nethead.data);
	i=SendPack(connect,&nethead);
	//free(buffer);
	if(i)return i;
	i=RecvPack(connect,&nethead);
	if(i)return i;
	EventCatch(connect,nethead.PROTO_NUM);
        *clip->ErrMsg=0;
        clip->Errno=nethead.ERRNO1;
	if(nrets) *nrets=0;*data=0;
	if(!nethead.ERRNO1) {
		if(nrets) *nrets=nethead.PKG_REC_NUM; //ncols
		*data=nethead.data;
		clip->NativeError=nethead.PKG_REC_NUM; //ncols
		return 0;
	} else {
		return GetError(connect,&nethead);
	}
}
