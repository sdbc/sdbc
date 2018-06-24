#include <scsrv.h>
#include <json_pack.h>

extern JSON_OBJECT dm_app(T_SRV_Var *srvp,JSON_OBJECT json,JSON_OBJECT err_json);

/******************************************************
 * 动态模块的应用服务入口,svc层
 * head->data="{model:"modelname",param:{....}}"
 *              model是模块名
 *              param是提交给模块的数据
 ******************************************************/
int dmapp(T_Connect *conn,T_NetHead *head)
{
	T_SRV_Var *srvp=(T_SRV_Var *)conn->Var;
	JSON_OBJECT json,err_json,result=NULL;
	char msg[2048];
	int ret,event=head->PROTO_NUM;
	int cont=head->ERRNO2;

	conn->status=(head->ERRNO2==PACK_STATUS)?1:0;

	err_json=json_object_new_array();
	json=json_tokener_parse(head->data);
	if(!json) {
		sprintf(msg,"请求的JSON 格式错,PKG_LEN=%d",head->PKG_LEN);
		json_object_array_add(err_json,jerr(100,msg));
		head->ERRNO1=FORMATERR;
		head->ERRNO2=conn->status>0?PACK_NOANSER:-1;
		return_error(conn,head,json_object_to_json_string(err_json));
		conn->status=0;
		ShowLog(1,"%s:%s",__FUNCTION__,json_object_to_json_string(err_json));
		json_object_put(err_json);
		return 0;
	}

	result=dm_app(srvp,json,err_json); //调用应用模块

	ret=0;
	if(result == (JSON_OBJECT)-1) ret=THREAD_ESCAPE;
	else if(!result) {
		head->data=(char *)json_object_to_json_string(err_json);
		head->ERRNO1=-1;
		head->ERRNO2=conn->status>0?PACK_NOANSER:-1;
	} else {
		json_object_object_add(result,"status",err_json);
		head->data=(char *)json_object_to_json_string(result);
		head->ERRNO1=0;
		head->ERRNO2=conn->status>0?PACK_STATUS:0;
	}

	if(ret==0 && cont != PACK_NOANSER) {
		head->PKG_LEN=strlen(head->data);
		head->PROTO_NUM=PutEvent(conn,event);
		head->O_NODE=ntohl(LocalAddr(conn->Socket,NULL));
		ret=SendPack(conn,head);
	}

	if(result) json_object_put(result);
	else json_object_put(err_json);
	json_object_put(json);

	return ret;
}
//卸载模块,svc层

extern int dmmgr_app(T_SRV_Var *srvp,char *model_name,JSON_OBJECT err_json);

int dmmgr(T_Connect *conn,T_NetHead *head)
{
	T_SRV_Var *srvp=(T_SRV_Var *)conn->Var;
	JSON_OBJECT err_json;
	char msg[2048];
	int ret,event=head->PROTO_NUM;

	err_json=json_object_new_array();
	conn->status=(head->ERRNO2==PACK_STATUS)?1:0;
	if(!head->PKG_LEN) {
		sprintf(msg,"data is empty!");
		json_object_array_add(err_json,jerr(100,msg));
		head->ERRNO1=LENGERR;
		head->ERRNO2=conn->status>0?PACK_NOANSER:-1;
		return_error(conn,head,json_object_to_json_string(err_json));
		conn->status=0;
		ShowLog(1,"%s:%s",__FUNCTION__,json_object_to_json_string(err_json));
		json_object_put(err_json);
		return 0;
	}

	ret=dmmgr_app(srvp,head->data,err_json); //调用应用模块

	if(head->ERRNO2 != PACK_NOANSER) {
		if(ret)
			sprintf(msg,"model %s unloaded errno=%d,%s",head->data,errno,strerror(errno));
		else
			sprintf(msg,"model %s is unloaded",head->data);
		ShowLog(2,"%s:%s",__FUNCTION__,msg);
		json_object_array_add(err_json,jerr(ret,msg));

		head->data=(char *)json_object_to_json_string(err_json);
		head->ERRNO1=0;
		head->ERRNO2=0;
		head->PKG_LEN=strlen(head->data);
		head->PROTO_NUM=PutEvent(conn,event);
		head->O_NODE=ntohl(LocalAddr(conn->Socket,NULL));
		ret=SendPack(conn,head);
	}
	json_object_put(err_json);

	return 0;
}
