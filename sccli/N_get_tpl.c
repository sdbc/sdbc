/************************************************************
 * N_get_tpl:从服务器取得表模板
 * tabnames="表名1,表名2,,"
 * 返回JSON对象:{表名:[模板],表名:[模板],...}
 * **********************************************************/

#include <arpa/inet.h>
#include <sccli.h>
#include <pack.h>
#include <json_pack.h>

JSON_OBJECT N_get_tpl(T_Connect *conn,char *tabnames,int num)
{
T_NetHead Head;
T_CLI_Var *clip;
int ret;
JSON_OBJECT json;

	if(!conn) return NULL;
	clip=(T_CLI_Var *)conn->Var;
	if(!clip) {
		ShowLog(1,"%s:T_CLI_Var is empty!",__FUNCTION__);
		return NULL;
	}

	Head.data=tabnames;
	Head.PKG_LEN=strlen(Head.data);
	Head.PKG_REC_NUM=num;
	Head.ERRNO1=0;
	Head.ERRNO2=conn->status?PACK_STATUS:0;
	Head.O_NODE=clip->ctx_id;
	Head.PROTO_NUM=get_srv_no(clip,"get_tpl");
	if(Head.PROTO_NUM==1) {
		json=json_object_new_object();
		add_string_to_json(json,"error","no such service 'get_tpl'!\n");
		return json;
	}
	ret=SendPack(conn,&Head);
	if(ret) {
		clip->Errno=-1;
		return NULL;
	}
	ret=RecvPack(conn,&Head);
	if(ret) {
	char buf[512];
		json=json_object_new_object();
		sprintf(buf,"RecvPack err=%d,%s",errno,strerror(errno));
		add_string_to_json(json,"error",buf);
		clip->Errno=-1;
		return json;
	}
	json=json_tokener_parse(Head.data);
	return json;
}
