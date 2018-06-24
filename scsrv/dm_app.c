#include <scsrv.h>
#include <json_pack.h>
#include <dlfcn.h>
#include <BB_tree.h>

extern int substitute_env(char *line);

static pthread_rwlock_t dmlock = PTHREAD_RWLOCK_INITIALIZER;

typedef struct {
	char cmd_name[128];
	void *handle;
	JSON_OBJECT (*cmd)(T_SRV_Var *srvp,JSON_OBJECT param,JSON_OBJECT jerr);
	void (*destruct)(T_SRV_Var *srvp);
	int lock;
	pthread_mutex_t mut;
	pthread_cond_t cond;
} cmd_node;

static int cmd_cmp(void *s,void *d,int len)
{
	cmd_node *sp,*dp;
	sp=(cmd_node *)s;
	dp=(cmd_node *)d;
	return strcmp(sp->cmd_name,dp->cmd_name);
}

static T_Tree *cmd_tree=NULL;

/******************************************************
 * 动态模块的应用服务入口,app层
 * json="{model:"modelname",param:{....}}"
 *        model是调用的so名字
 *                          param是提交给模块的数据
 ******************************************************/
JSON_OBJECT dm_app(T_SRV_Var *srvp,JSON_OBJECT json,JSON_OBJECT err_json)
{
	JSON_OBJECT model_json,result=NULL;
	char msg[2048];
	cmd_node *nodep,node={{0},NULL,NULL,NULL,1,PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER};
	T_Tree *trp;

	model_json=json_object_object_get(json,"model");
	if(!model_json) {
		sprintf(msg,"请求的JSON 格式错");
		json_object_array_add(err_json,jerr(100,msg));
		return NULL;
	}
	stptok(json_object_get_string(model_json),node.cmd_name,sizeof(node.cmd_name),NULL);

	pthread_rwlock_rdlock(&dmlock);
	trp=BB_Tree_Find(cmd_tree,&node,sizeof(node),cmd_cmp);
	if(!trp) {
		char *envp,so_name[200];

		envp=getenv("SO_USE");
		if(envp) sprintf(so_name,"%s/lib%s.so",envp,node.cmd_name);
		else sprintf(so_name,"lib%s.so",node.cmd_name);

		node.handle = dlopen(so_name, RTLD_NOW);
		if(node.handle) {
			dlerror();
			*(void **) (&node.cmd) = dlsym(node.handle, node.cmd_name);
			if(!node.cmd) {
				sprintf(msg,"cmd %s err %s",node.cmd_name,dlerror());
				ShowLog(1,"%s:%s",__FUNCTION__,msg);
				json_object_array_add(err_json,jerr(102,msg));
			}
			sprintf(so_name,"_%s",node.cmd_name);
			*(void **)&node.destruct = dlsym(node.handle, so_name);
			cmd_tree=BB_Tree_Add(cmd_tree,&node,sizeof(node),cmd_cmp,NULL);
			trp=BB_Tree_Find(cmd_tree,&node,sizeof(node),cmd_cmp);
			nodep=(cmd_node *)trp->Content;
		} else {
			sprintf(msg,"%s:load %s err %s",__FUNCTION__,so_name,dlerror());
			json_object_array_add(err_json,jerr(101,msg));
			ShowLog(1,"%s:%s",__FUNCTION__,msg);
			node.cmd=NULL;
			nodep=&node;
		}

	} else {
		nodep=(cmd_node *)trp->Content;
		pthread_mutex_lock(&nodep->mut);
		nodep->lock++;
		pthread_mutex_unlock(&nodep->mut);
	}

	pthread_rwlock_unlock(&dmlock);

	if(nodep->cmd) {
		result=nodep->cmd(srvp,json_object_object_get(json,"param"),err_json);
		pthread_mutex_lock(&nodep->mut);
		nodep->lock--;
		pthread_mutex_unlock(&nodep->mut);
		if(nodep->lock==0) pthread_cond_signal(&nodep->cond);
	}

	return result;
}
//卸载模块，app层
int dmmgr_app(T_SRV_Var *srvp,char *model_name,JSON_OBJECT err_json)
{
	char *p,*envp,so_use[128],so_lib[200],msg[2048];
	int flg=0,ret;
	cmd_node *nodep=NULL;
	T_Tree *trp;

	envp=getenv("SO_USE");
	if(!envp||!*envp) envp=".";
	sprintf(so_use,"%s/lib%s.so",envp,model_name);
	sprintf(msg,"mv -f %s %s.bak",so_use,so_use);

	pthread_rwlock_wrlock(&dmlock);
	ret=system(msg);
	if(ret >> 8) {
		strcat(msg,":移动原模块失败");
		json_object_array_add(err_json,jerr(105,msg));
		ret=SYSERR;
		goto err1;
	}
	p=msg;
	envp=getenv("SO_LIB");
	if(envp&&*envp) {
		sprintf(so_lib,"%s/lib%s.so",envp,model_name);
	} else {
		sprintf(so_lib,"$HOME/lib/lib%s.so",model_name);
		substitute_env(so_lib);
	}
	sprintf(msg,"cp -f %s %s",so_lib,so_use);

	ret=system(msg);
	if(ret >> 8) { //失败
		strcat(msg,":拷贝失败");
		json_object_array_add(err_json,jerr(105,msg));
		sprintf(msg,"mv -f %s.bak %s",so_use,so_use);
		system(msg);
		ret=SYSERR;
		err1:
		pthread_rwlock_unlock(&dmlock);
		return ret;
	}
	cmd_node node;
	strcpy(node.cmd_name,model_name);

	trp=BB_Tree_Find(cmd_tree,&node,sizeof(node),cmd_cmp);
	if(!trp) {
		sprintf(msg,"no such model[%s]",model_name);
		json_object_array_add(err_json,jerr(106,msg));
		ret=-106;
		goto err1;
	}
	nodep=(cmd_node *)trp->Content;
	pthread_mutex_lock(&nodep->mut);
	while(nodep->lock>0) {
		pthread_cond_wait(&nodep->cond,&nodep->mut);
	}
	pthread_mutex_unlock(&nodep->mut);
	if(nodep->destruct) nodep->destruct(srvp);
	do {
		ret=dlclose(nodep->handle);
	} while(ret>0);
	cmd_tree=BB_Tree_Del(cmd_tree,nodep,sizeof(node),cmd_cmp,NULL,&flg);
	pthread_rwlock_unlock(&dmlock);

	return 0;
}
