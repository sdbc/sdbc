/********************************************
 * 快速查找模板，利用hash算法
 *******************************************/
#include <sccli.h>

T_PkgType slist_tpl[]={
	{CH_INT,sizeof(int),0,0,-1},
	{CH_CHAR,49,0,0},
	{-1,0,0,0}
};

typedef struct {
	int srv_no;
	char srv_name[49];
} srv_list;

typedef struct {
	char *name;
	int colno;
	int link;
} srvhs;

static int hash_name(const char *name,int mod)
{
register unsigned int hashval=0;
	if(!mod) return -1;
	while(*name) hashval += *(unsigned char *)name++; 
	return (int)(hashval%mod);
}
//产生索引 
static char *mk_srv_hash(srv_list slist[],int coln)
{
int i,hashnum;
srvhs *colp;
int *lp;
char *p;
	colp=(srvhs *)malloc(coln * sizeof(srvhs));
	if(!colp) return NULL;

srvhs *top,stack[coln];

	top=stack;
	for(i=0;i<coln;i++) {
		colp[i].colno=-1;
		colp[i].link=-1;
	}
	for(i=0;i<coln;i++) {
		p=slist[i].srv_name;
	    	hashnum=hash_name(p,coln);
//ShowLog(5,"%s:name[%d]=%s,hashnum=%d\n",__FUNCTION__,i,p,hashnum);
	    if(colp[hashnum].colno==-1) {	//没有散列冲突 
		colp[hashnum].name=p;
		colp[hashnum].colno=i;
	    } else {				//有散列冲突，存储冲突链
		top->name=p;
		top->colno=i;
		top->link=hashnum;
		top++;
	    }
	}
	if(top != stack) { //有散列冲突，构建冲突链 
		for(i=0;i<coln;i++) {
			if(colp[i].colno != -1) continue;
			top--;
//找到索引表里的空项 
			colp[i].name=top->name;
			colp[i].colno=top->colno;
			for(lp=&colp[top->link].link;*lp != -1;lp=&colp[*lp].link)
				;
			*lp=i;
		}
	}

	return (char *)colp;
}
//查找服务号，无此服务返回1,=echo() 
int get_srv_no(T_CLI_Var *clip,const char *key)
{
register srvhs *cp;
svc_table *svcp;
	if(!clip || !clip->svc_tbl) return 1;
	svcp=clip->svc_tbl;
	if(svcp->srvn<1) return 1;
srvhs *colp=((srvhs *)(svcp->srv_hash));
	for(cp=colp+hash_name(key,svcp->srvn);
	   strcmp(cp->name,key); cp=colp+cp->link) {
		if(cp->link==-1) return 1;
	} 
	return ((srv_list *)svcp->srvlist)[cp->colno].srv_no;
}
//释放服务名相关数据 
void free_srv_list(T_CLI_Var *clip)
{
	if(!clip || !clip->svc_tbl) return;
	
	if(clip->svc_tbl->usage>0) clip->svc_tbl->usage--;
	else {
		if(clip->svc_tbl->srvlist) {
			free(clip->svc_tbl->srvlist);
			clip->svc_tbl->srvlist=NULL;
		}
		if(clip->svc_tbl->srv_hash) {
			free(clip->svc_tbl->srv_hash);
			clip->svc_tbl->srv_hash=NULL;
		}
		clip->svc_tbl->srvn=0;
		clip->svc_tbl=NULL;
	}
	return;
}

/*******************************************************
 * 取得服务器端服务名列表
 * 服务器端的0号函数必须是登录认证函数。登录认证完成后，
 * 必须把0号函数值换成 get_srvname();
 * 1号函数必须是echo()
 * conn->freevar指向free_srv_list,如果需要其他善后操作
 * 应重置，但最后应补做free_srv_list(T_CLI_Var *);
 *******************************************************/
int init_svc_no(T_Connect *conn)
{
T_CLI_Var *clip;
T_NetHead head;
int ret,i;
char *p;
srv_list *lp;
svc_table *svcp;

	if(!conn || !conn->Var) return -1;
	clip=(T_CLI_Var *)conn->Var;
	if(!clip) return -1;
	svcp=clip->svc_tbl;
	if(!svcp) return -1;
	svcp->srv_hash=0;
	svcp->srvlist=0;

	head.PROTO_NUM=0;
	head.ERRNO1=head.ERRNO2=0;
	head.PKG_REC_NUM=0;
	head.O_NODE= clip->ctx_id;
	head.D_NODE=0;
	head.data=0;
	head.PKG_LEN=0;
	ret=SendPack(conn,&head);
	if(ret<0) {
		svcp->srvn=0;
		svcp->usage=-1;
		return ret;
	}
	ret=RecvPack(conn,&head);
	if(ret<0) {
		svcp->srvn=0;
		svcp->usage=-1;
		return(ret);
	}
	if(head.ERRNO1 && head.ERRNO2) return -2;
	svcp->usage=0;
	svcp->srvn=head.PKG_REC_NUM;
	svcp->srvlist=malloc(sizeof(srv_list) * clip->svc_tbl->srvn);
	if(!svcp->srvlist) {
		svcp->srvn=0;
		svcp->usage=-1;
		clip->svc_tbl=NULL;
		return -1;
	}
	lp=(srv_list *)svcp->srvlist;
	p=head.data;
ShowLog(5,"%s:srvn=%d,data=%s",__FUNCTION__,svcp->srvn,p);
	for(i=0;i<svcp->srvn&&*p;i++) {
		p+=net_dispack(lp,p,slist_tpl);
//ShowLog(5,"src_no=%d,srv_name=%s",lp->srv_no,lp->srv_name);
		lp++;
	}
	svcp->srv_hash=mk_srv_hash((srv_list *)clip->svc_tbl->srvlist,clip->svc_tbl->srvn);
	conn->freevar=(void (*)(void *)) free_srv_list;
	return 0;
}
