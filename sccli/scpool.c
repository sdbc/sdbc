/**************************************************
 * SDBC连接池管理
 **************************************************/
#include <ctype.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <scry.h>
#include <sccli.h>
#include <scpool.h>
#include <json_pack.h>

#include <logs.tpl>
#include <logs.stu>
static int log_level=0;

int set_SC_loglevel(int new_loglevel)
{
	int old_level=log_level;
	log_level=new_loglevel;
	return old_level;
}

T_PkgType SCPOOL_tpl[]={
		{CH_INT,sizeof(int),"d_node",0,-1},
		{CH_CHAR,17,"DEVID"},
		{CH_CHAR,256,"LABEL"},
		{CH_CHAR,17,"UID"},
		{CH_CHAR,14,"PWD"},
		{CH_INT,sizeof(int),"NUM"},
		{CH_INT,sizeof(int),"NEXT_d_node"},
		{CH_CHAR,81,"HOST"},
		{CH_CHAR,21,"PORT"},
		{CH_INT,sizeof(int),"MTU"},
		{CH_CHAR,172,"family"},
		{-1,0,0,0}
};

extern T_PkgType SCPOOL_tpl[];
typedef struct {
	int d_node;
	char DEVID[17];
	char LABEL[256];
	char UID[17];
	char PWD[14];
	int NUM;
	int NEXT_d_node;
	char HOST[81];
	char PORT[21];
	int MTU;
	char family[172];
} SCPOOL_stu;

typedef struct {
	pthread_mutex_t mut;
	pthread_cond_t cond;
	int resource_num;
	SCPOOL_stu log;
	svc_table svc_tbl;
	u_int family[32];
	char DBLABEL[81];
	resource *lnk;
	int free_q;
	int weight;  //权重:0-resource_num,-1暂不可用
} path_lnk;

//在一个sc_path里的各服务器都是同质的，具有相同的D_NODE号
//将在同一个sc_path内进行负载均衡
typedef struct {
	pthread_mutex_t weightLock;
	pthread_cond_t weightCond;
	int d_node;
	char *DBLABEL;
	int path_num;
	path_lnk *path;
} sc_path;

static int SCPOOLNUM=0,ctx_flg=0;
static sc_path  *scpool=NULL;
//释放连接池
void scpool_free()
{
	sc_path *scp;

	if(!scpool) return;
	scp=scpool;
	for(int n=0;n<SCPOOLNUM;n++,scp++) {
		if(scp->path) {
			path_lnk *pl=scp->path;
			for(int j=0;j<scp->path_num;j++,pl++) {
				pthread_cond_destroy(&pl->cond);
				pthread_mutex_destroy(&pl->mut);
				if(pl->lnk) {
					resource *rs=pl->lnk;
					for(int i=0;i<pl->resource_num;i++,rs++) {
						if(rs->Conn.Socket > -1) {
							disconnect(&rs->Conn);
						}
					}
					free(pl->lnk);
				}
			}
			free(scp->path);
		}
	}
	free(scpool);
	scpool=NULL;
}

static int resource_init(path_lnk *pl,int m)
{
	sc_path *scp;
	int i;
	scp=&scpool[m];
	if(0!=(i=pthread_mutex_init(&pl->mut,NULL))) {
		ShowLog(1,"%s:mutex_init err %s",__FUNCTION__,
				strerror(i));
		return -12;
	}
	if(0!=(i=pthread_cond_init(&pl->cond,NULL))) {
		ShowLog(1,"%s:cond init  err %s",__FUNCTION__,
				strerror(i));
		return -13;
	}
	pl->svc_tbl.srvn=0;
	*pl->DBLABEL=0;
	pl->resource_num=pl->log.NUM>0?pl->log.NUM:1;
	pl->weight=pl->resource_num;
	pl->lnk=(resource *)malloc(pl->resource_num * sizeof(resource));
	if(!pl->lnk) {
		ShowLog(1,"%s:malloc lnk error!",__FUNCTION__);
		pl->resource_num=0;
		return MEMERR;
	}
	pl->free_q=pl->resource_num-1;
	for(i=0;i<pl->resource_num;i++) {
		Init_CLI_Var(&pl->lnk[i].cli);
		pl->lnk[i].cli.Errno=-1;
		pl->lnk[i].cli.NativeError=i;
		pl->lnk[i].cli.ctx_id=0;
		pl->lnk[i].cli.var=NULL;
		pl->lnk[i].pool_no=m;
		pl->lnk[i].path_no=(pl-scp->path);
		pl->lnk[i].timeout_deal=NULL;
		initconnect(&pl->lnk[i].Conn);
		strcpy(pl->lnk[i].Conn.Host,pl->log.HOST);
		strcpy(pl->lnk[i].Conn.Service,pl->log.PORT);
		pl->lnk[i].Conn.pos=(i&0xffff)|(pl->lnk[i].path_no<<16);
//ShowLog(5,"%s:pos=%08X",__FUNCTION__,pl->lnk[i].Conn.pos);
		if(*pl->log.family)
			str_a64n(32,pl->log.family,pl->family);
		if(i<pl->resource_num-1) pl->lnk[i].next=i+1;
		else pl->lnk[i].next=0;
		pl->lnk[i].timestamp=now_usec();
	}
	return 0;
}
//初始化连接池
int scpool_init(int ctx_flag)
{
	int n,i,j,ret;
	char *p,buf[512],dnode_key[15];
	FILE *fd;
	JSON_OBJECT cfg,json,ajson;
	SCPOOL_stu node;

	if(scpool) return 0;
	ctx_flg=ctx_flag;
	p=getenv("SCPOOLCFG");
	if(!p||!*p) {
		ShowLog(1,"%s:缺少环境变量SCPOOLCFG!",__FUNCTION__);
		return -1;
	}
	fd=fopen((const char *)p,"r");
	if(!fd) {
		ShowLog(1,"%s:CFGFILE %s open err=%d,%s",__FUNCTION__,
				p,errno,strerror(errno));
		return -2;
	}
	cfg=json_object_new_object();
	SCPOOLNUM=0;
	while(!ferror(fd)) {
		fgets(buf,sizeof(buf),fd);
		if(feof(fd)) break;
		TRIM(buf);
		if(!*buf || *buf=='#') continue;
		ret=net_dispack(&node,buf,SCPOOL_tpl);
		if(ret<=0) continue;
		sprintf(dnode_key,"DNODE%d",node.d_node);
		json=json_object_new_object();
		struct_to_json(json,&node,SCPOOL_tpl,0);
		ajson=json_object_object_get(cfg,dnode_key);
		if(ajson) {
			json_object_array_add(ajson,json);
		} else {
			ajson=json_object_new_array();
			json_object_array_add(ajson,json);
			json_object_object_add(cfg,dnode_key,ajson);
			SCPOOLNUM++;
		}
	}
	fclose(fd);
	if(!SCPOOLNUM) {
		json_object_put(cfg);
		ShowLog(1,"%s:empty SCPOOL",__FUNCTION__);
		return -3;
	}
	scpool=(sc_path *)malloc(SCPOOLNUM * sizeof(sc_path));
	if(!scpool) {
		json_object_put(cfg);
		SCPOOLNUM=0;
		return MEMERR;
	}

	p=getenv("SCPOOL_LOGLEVEL");
	if(p && isdigit(*p)) log_level=atoi(p);

	ShowLog(5,"cfg=%s,POOLNUM=%d",json_object_to_json_string(cfg),SCPOOLNUM);
	sc_path *scp=scpool;
	for(n=0;n<SCPOOLNUM;n++,scp++) {
		scp->path=NULL;
		if(0!=(i=pthread_mutex_init(&scp->weightLock,NULL))) {
			ShowLog(1,"%s:mutex_init weight err %s",__FUNCTION__,
					strerror(i));
			json_object_put(cfg);
			return -12;
		}
		if(0!=(i=pthread_cond_init(&scp->weightCond,NULL))) {
			ShowLog(1,"%s:cond init weight err %s",__FUNCTION__,
					strerror(i));
			json_object_put(cfg);
			return -13;
		}
	}
	scp=scpool;
	n=0;
	json_object_object_foreach(cfg,key,pjson) { //SCPOOLNUM
		sscanf(key,"DNODE%d",&scp->d_node);
		scp->path_num=json_object_array_length(pjson);
		ShowLog(5,"%s:path_num=%d",key,scp->path_num);
		scp->path=(path_lnk *)malloc(sizeof(path_lnk) * scp->path_num);
		if(!scp->path) {
			ShowLog(1,"%s:malloc scp->path fault!",__FUNCTION__);
			json_object_put(cfg);
			scpool_free();
			return MEMERR;
		}
		path_lnk *pl=scp->path;
		for(j=0;j<scp->path_num;j++,pl++) pl->lnk=NULL;
		scp->DBLABEL=NULL;
		pl=scp->path;
		for(j=0;j<scp->path_num;j++,pl++) {
			json=json_object_array_get_idx(pjson,j);
			json_to_struct(&pl->log,json,SCPOOL_tpl);
			ret=resource_init(pl,n);
			if(ret<0) {
				json_object_put(cfg);
				scpool_free();
				return ret;
			}
			ShowLog(5,"\tlink[%d]=%d",j,pl->resource_num);
		}
		n++;
		scp++;
	}
	json_object_put(cfg);
	return SCPOOLNUM;
}

static int lnk_no(path_lnk *pl,T_Connect *conn)
{
	int i,e;
	resource *rs=pl->lnk;

	if(!conn) return -1;
	e=conn->pos&0XFFFF;
	if(e<pl->resource_num && conn == &pl->lnk[e].Conn) {
		return e;
	}
	ShowLog(1,"%s:conn not equal pos=%d",__FUNCTION__, e);
	e=pl->resource_num;
	for(i=0;i<e;i++,rs++) {
		if(conn == &rs->Conn) {
			conn->pos=(rs->path_no<<16)|(i&0XFFFF);
			return i;
		}
	}
	return -1;

}
static int get_lnk_no(path_lnk *pl)
{
	int i,*ip,*np;
	resource *rs;

	if(pl->free_q<0) return -1;
	ip=&pl->free_q;
	rs=&pl->lnk[*ip];
	i=rs->next;
	np=&pl->lnk[i].next;
	if(i==*ip) *ip=-1;
	else rs->next=*np;
	*np=-1;
	if(pl->weight>0) pl->weight--;
	return i;
}
//将pl[i]加入free_q
static void add_lnk(path_lnk *pl,int i)
{
	int *np,*ip=&pl->lnk[i].next;
	if(*ip>=0) {
		ShowLog(5,"%s:Tid=%lx,i=%d,next=%d,已经在队列中!",__FUNCTION__,pthread_self(),i,*ip);

		return;//已经在队列中
	}
	np=&pl->free_q;
	if(*np < 0) {
		*np=i;
		*ip=i;
	} else { //插入队头
		resource *rs=&pl->lnk[*np];
		*ip=rs->next;
		rs->next=i;
		if(pl->lnk[i].Conn.Socket<0) {
			*np=i;//坏连接排队尾
		}
	}
	if(pl->weight>=0) pl->weight++;
//ShowLog(5,"%s:tid=%lx,weight=%d,next=%d",__FUNCTION__,pthread_self(),pl->weight,*ip);
}

extern int usleep (__useconds_t __useconds);
static int sc_connect(path_lnk *pl,resource *rs)
{
	int ret=-1;
	T_NetHead Head;
	struct utsname ubuf;
	char finger[256],buf[200],*p;
	log_stu logs;
	sc_path *scp=&scpool[rs->pool_no];

	ret=Net_Connect(&rs->Conn,&rs->cli,*pl->log.family?pl->family:NULL);
	if(ret) {
		rs->cli.Errno=errno;
		stptok(strerror(errno),rs->cli.ErrMsg,sizeof(rs->cli.ErrMsg),0);
		return -1;
	}

	p=getenv("TCPTIMEOUT");
	if(p && isdigit(*p)) {
		Head.ERRNO2=60*atoi(p);
		rs->Conn.timeout=Head.ERRNO2>=120?Head.ERRNO2-60:Head.ERRNO2;
	} else rs->Conn.timeout=Head.ERRNO2=0;

//login
	Head.O_NODE=LocalAddr(rs->Conn.Socket,finger);
	fingerprint(finger);
	uname(&ubuf);
	p=buf;
	again_login:
	p+=sprintf(p,"%s|%s|%s,%s|||",pl->log.DEVID,pl->log.LABEL,
			   ubuf.nodename,finger);
	rs->Conn.MTU=pl->log.MTU;
	Head.PROTO_NUM=0;
	Head.D_NODE=pl->log.NEXT_d_node;
	Head.ERRNO1=rs->Conn.MTU;
	if(!ctx_flg) Head.PKG_REC_NUM=-1; //不要求ctx_id
	else Head.PKG_REC_NUM=rs->cli.ctx_id;
	Head.data=buf;
	Head.PKG_LEN=strlen(Head.data);

	ret=SendPack(&rs->Conn,&Head);
	ret=RecvPack(&rs->Conn,&Head);
	if(ret) {
		rs->cli.Errno=errno;
		stptok(strerror(errno),rs->cli.ErrMsg,sizeof(rs->cli.ErrMsg),0);
		disconnect(&rs->Conn);
		ShowLog(1,"%s:network error ret=%d,%d,%s",__FUNCTION__,ret,rs->cli.Errno,rs->cli.ErrMsg);
		rs->cli.Errno=-1;
		return -2;
	}
	if(Head.ERRNO1 || Head.ERRNO2) {
		if(ctx_flg && Head.ERRNO1==-197) { //ctx_id已经超时，失效了。
			rs->cli.ctx_id=0;
			goto again_login;
		}

		ShowLog(1,"%s:login error ERRNO1=%d,ERRNO2=%d,%s",__FUNCTION__,
				Head.ERRNO1,Head.ERRNO2,Head.data);
		disconnect(&rs->Conn);
		stptok(Head.data,rs->cli.ErrMsg,sizeof(rs->cli.ErrMsg),0);
		rs->cli.Errno=-1;
		return -3;
	}
	memset(&logs,0,sizeof(logs));
	net_dispack(&logs,Head.data,log_tpl);
	strcpy(rs->cli.DBOWN,logs.DBOWN);
	strcpy(rs->cli.UID,logs.DBUSER);
	if(!ctx_flg && !*pl->DBLABEL) {
		strcpy(pl->DBLABEL,logs.DBLABEL);
		if(!scp->DBLABEL) scp->DBLABEL=pl->DBLABEL;
	}
//session id for end server
	if(ctx_flg && *logs.DBLABEL) {
//ShowLog(5,"$s[%d]:DBLABEL=%s",__FUNCTION__,__LINE__,
		str_a64n(1,logs.DBLABEL,(u_int *)&rs->cli.ctx_id);
	}

//取服务名
	rs->cli.svc_tbl=&pl->svc_tbl;
	pthread_mutex_lock(&pl->mut);
	if(pl->svc_tbl.srvn == 0) {
		reload:
		pl->svc_tbl.srvn=-1;
		pthread_mutex_unlock(&pl->mut);
		ret=init_svc_no(&rs->Conn);
		if(ret) { //取服务名失败
			ShowLog(1,"%s:HOST=%s/%s,init_svc_no error ",__FUNCTION__,
					rs->Conn.Host,rs->Conn.Service);
			rs->cli.Errno=-1;
			disconnect(&rs->Conn);
			return -4;
		}
	} else {
		while(pl->svc_tbl.srvn<0) usleep(1000);
		if(pl->svc_tbl.srvn>0) pl->svc_tbl.usage++;
		else goto reload;

		pthread_mutex_unlock(&pl->mut);
		rs->Conn.freevar=(void (*)(void *)) free_srv_list;
		*rs->cli.ErrMsg=0;
	}
	rs->cli.Errno=ret;

	return 0;
}

//取连接
//连接 线程锁不能跨越AIO，应用者应该提供他的已锁定的线程锁
//在AIO开始前先解锁，完成后重新加锁
static resource * get_SC_resource(path_lnk *pl,int flg,pthread_mutex_t *Lock)
{
	int ret,i;
	resource *rs;

	if(!pl->lnk) {
		ShowLog(1,"%s:无效的连接池",__FUNCTION__);
		return NULL;
	}
	if(0!=pthread_mutex_lock(&pl->mut)) return NULL;
	while(0>(i=get_lnk_no(pl))) {
		if(flg) {   //flg !=0,don't wait
			pthread_mutex_unlock(&pl->mut);
			return NULL;
		}
		//	if(log_level) ShowLog(log_level,"%s:tid=%lx pool suspend",__FUNCTION__,pthread_self());
		pthread_cond_wait(&pl->cond,&pl->mut); //没有资源，等待
		//	if(log_level) ShowLog(log_level,"%s:tid=%lx pool weakup",__FUNCTION__,pthread_self());
	}
	pthread_mutex_unlock(&pl->mut);

	rs=&pl->lnk[i];
	rs->next=-1;
	rs->timestamp=now_usec();
	if(rs->Conn.Socket<0 || rs->cli.Errno<0) {
		if(Lock) pthread_mutex_unlock(Lock); //线程锁不可跨AIO
		ret=sc_connect(pl,rs);
		if(Lock) pthread_mutex_lock(Lock);
		if(ret) {
			ShowLog(1,"%s:DNODE[%d][%d][%d] 连接%s/%s错:err=%d,%s",
					__FUNCTION__,pl->log.d_node,rs->path_no,i,pl->log.HOST,pl->log.PORT,
					rs->cli.Errno, rs->cli.ErrMsg);
			rs->cli.Errno=-1;
			pthread_mutex_lock(&pl->mut);
			add_lnk(pl,i);
			pl->weight=-1;
			pthread_mutex_unlock(&pl->mut);
			return (resource *)-1;
		}
//恢复计权
		pthread_mutex_lock(&pl->mut);
		if(pl->weight<0){
			for(pl->weight=0,ret=0;ret<pl->resource_num;ret++) {
				if(pl->lnk[ret].next>=0) pl->weight++;
			}
		}
		pthread_mutex_unlock(&pl->mut);
		sc_path *scp=&scpool[rs->pool_no];
		pthread_cond_signal(&scp->weightCond); //如果有等待连接池的线程就唤醒它
	}

	if(log_level) ShowLog(log_level,"%s:tid=%lx,DNODE[%d][%d][%d]",__FUNCTION__,
						  pthread_self(),pl->log.d_node,rs->Conn.pos>>16,i);
	rs->cli.Errno=0;
	*rs->cli.ErrMsg=0;
	return rs;
}

resource * get_path_resource(int poolno,int path_no,int flg)
{
	sc_path *scp;
	if(!scpool || poolno<0 || poolno>=SCPOOLNUM) return NULL;
	scp=&scpool[poolno];
	if(path_no<0 || path_no>=scp->path_num) return NULL;
	return get_SC_resource(&scp->path[path_no],flg,NULL);
}

#include <time.h>
#ifndef CLOCK_REALTIME
struct timespec
{
	__time_t tv_sec;            /* Seconds.  */
	long int tv_nsec;           /* Nanoseconds.  */
};
/* Get current value of clock CLOCK_ID and store it in TP.  */
extern int clock_gettime (clockid_t __clock_id, struct timespec *__tp) __THROW;

#define CLOCK_REALTIME 0
#endif

//负载均衡，找一个负载最轻的池
//flg>0:忙时等待flg次，每次6秒,flg=0,不等待,flg=-1永远等待
//return：NULL没有空闲的连接,-1:连接已损坏
resource * get_SC_by_weight(int path_no,int flg)
{
	int i,max_weight,w,n,m,repeat=flg;
	path_lnk *pl;
	sc_path *scp;
	struct timespec tims;
	resource *rs;

	if(path_no<0 || path_no>=SCPOOLNUM) return NULL;

	scp=&scpool[path_no];
	pthread_mutex_lock(&scp->weightLock);
	do {
		max_weight=-1,n=-1,m=-1;
		pl=scp->path;
		for(i=0;i<scp->path_num;i++,pl++) {
//找权重最重的那个池
			ShowLog(5,"%s:tid=%lx,DNODE[%d] weight[%d]=%d",__FUNCTION__,pthread_self(),scp->d_node,i,pl->weight);
			pthread_mutex_lock(&pl->mut);
			if(pl->weight>0) {
				w=pl->weight<<9;
				w/=pl->resource_num;
				if(w>max_weight) {
					max_weight=w;
					n=i;
				}
			}
			pthread_mutex_unlock(&pl->mut);
		}
		if(n>=0) {
			pl=&scp->path[n];
			rs=get_SC_resource(pl,1,&scp->weightLock);
			if(rs && rs != (resource *)-1) {
				pthread_mutex_unlock(&scp->weightLock);
				ShowLog(5,"%s:tid=%lx,path[%d] succeed",__FUNCTION__,pthread_self(),n);
				return rs;
			}
			continue;
		}
		if(log_level) ShowLog(log_level,"%s:tid=%lx,pool[%d] n=%d,weight=%d",__FUNCTION__,
							  pthread_self(),path_no,n,max_weight);
		pl=scp->path;
		int bad=1;
		for(m=0;m<scp->path_num;m++,pl++) { //测一遍故障池,看看能否恢复
			if(pl->weight>=0) {
				bad=0;
				continue;
			}
			rs=get_SC_resource(pl,1,&scp->weightLock);
			if(rs && rs != (resource *)-1) {
				pthread_mutex_unlock(&scp->weightLock);
				return rs;
			}
		}
		if(flg==0) {
			pthread_mutex_unlock(&scp->weightLock);
			ShowLog(5,"%s:tid=%lx,POOL %d BUSY!",__FUNCTION__,pthread_self(),path_no);
			return NULL;
		} else if(bad || 0==repeat) {
			pthread_mutex_unlock(&scp->weightLock);
			ShowLog(5,"%s:tid=%lx,POOL %d BAD=%d!",__FUNCTION__,pthread_self(),path_no,bad);
			return bad?(resource *)-1:NULL;
		}
		clock_gettime(CLOCK_REALTIME, &tims);
		tims.tv_sec+=6;//因为归还连接并不锁weightLock，可能丢失事件，等6秒
		if(pthread_cond_timedwait(&scp->weightCond,&scp->weightLock,(const struct timespec *restrict )&tims )) //实在没有了，等
		repeat--;
	} while(1);
}
//flg>0:忙时等待flg次，每次6秒,flg=0,不等待,flg=-1永远等待
//return：NULL没有空闲的连接,-1:连接已损坏。其他:返回连接
T_Connect * get_SC_connect(int path_no,int flg)
{
	resource *rs=get_SC_by_weight(path_no,flg);
	if(!rs || (long)rs==-1) {
		if(flg==-1) return NULL;
		return (T_Connect *)rs;
	}
	return &rs->Conn;
}

//归还连接
void release_SC_connect(T_Connect **Connect,int n)
{
	int i;
	pthread_t tid=pthread_self();
	path_lnk *pl;
	sc_path *scp;
	resource *rs;
	T_CLI_Var *clip;
	if(!Connect || !scpool || n<0 || n>=SCPOOLNUM) {
		ShowLog(1,"%s:poolno=%d,错误的参数",__FUNCTION__,n);
		return;
	}
	if(!*Connect) {
		ShowLog(1,"%s:,Connect is Empty!",__FUNCTION__);
		return;
	}
	(*Connect)->CryptFlg &= ~UNDO_ZIP;
	scp=&scpool[n];
	clip=(T_CLI_Var *)((*Connect)->Var);
	i=(*Connect)->pos >> 16;
	if(i>=scp->path_num) {
		ShowLog(1,"%s:pos=%08X,path_no=%d error",(*Connect)->pos,i);
		return;
	}
	pl=&scp->path[i];
//ShowLog(5,"%s:tid=%lx,pos=%08X,path=%d,weight=%d",__FUNCTION__,pthread_self(),(*Connect)->pos,i,pl->weight);
	pthread_mutex_lock(&pl->mut);
	i=lnk_no(pl,*Connect);
	if(i<0) {
		pthread_mutex_unlock(&pl->mut);
		ShowLog(1,"%s:无效的连接池[%d],pos=%d",__FUNCTION__,n,(*Connect)->pos >> 16);
		return;
	}
	rs=&pl->lnk[i];
	rs->timestamp=now_usec();
	if(rs->cli.Errno==-1) {  //连接失效
		ShowLog(1,"%s:scpool[%d][%d][%d] by fail!",__FUNCTION__,n,(*Connect)->pos >> 16,i);
		disconnect(&rs->Conn);
//		pl->weight=-1;
	}
	add_lnk(pl,i);
	pthread_mutex_unlock(&pl->mut);
	pthread_cond_signal(&pl->cond); //如果有等待连接的线程就唤醒它
	pthread_cond_signal(&scp->weightCond); //如果有等待连接池的线程就唤醒它
	clip->Errno=0;
	*clip->ErrMsg=0;
	if(log_level) ShowLog(log_level,"%s:tid=%lx,DNODE[%d][%d][%d],weight=%d",__FUNCTION__,
						  tid,pl->log.d_node,(*Connect)->pos>>16,i,pl->weight);
	*Connect=NULL;
}

void release_SC_resource(resource **rsp)
{
	if(!rsp || !*rsp) return;
	T_Connect *conn=&(*rsp)->Conn;
	release_SC_connect(&conn,(*rsp)->pool_no);
	*rsp=NULL;
}

//连接池监控
void scpool_check()
{
	int n,j,i,num;
	path_lnk *pl;
	sc_path *scp;
	resource *rs;
	INT64 now;
	char buf[40];

	if(!scpool) return;
	now=now_usec();

	scp=scpool;
	for(n=0;n<SCPOOLNUM;n++,pl++,scp++) {
		pl=scp->path;
		for(j=0;j<scp->path_num;j++,pl++) {
			if(!pl->lnk) continue;
			rs=pl->lnk;
			num=pl->resource_num;
			pthread_mutex_lock(&pl->mut);
			for(i=0;i<num;i++,rs++) {
				if(rs->next >= 0) {
					if(rs->Conn.Socket>-1 && (now-rs->timestamp)>299000000) {
//空闲时间太长了
//ShowLog(log_level,"%s:scpool[%d][%d][%d],Socket=%d,to free",__FUNCTION__,n,j,i,rs->Conn.Socket);
						disconnect(&rs->Conn);//
						if(log_level)
							ShowLog(log_level,"%s:Close SCpool[%d][%d][%d],since %s",__FUNCTION__,
									n,j,i,rusecstrfmt(buf,rs->timestamp,YEAR_TO_USEC));
					}
				} else {
					int k=(now-rs->timestamp)/1000000;
					if(rs->timeout_deal && rs->Conn.timeout>0 && k>rs->Conn.timeout) { //超时
						pthread_mutex_unlock(&pl->mut);
						rs->timeout_deal(rs);
						pthread_mutex_lock(&pl->mut);
						if(log_level) ShowLog(log_level,"%s:scpool[%d][%d][%d],since %s,timeout=%d,k=%d",
											  __FUNCTION__,n,j,i,
											  rusecstrfmt(buf,rs->timestamp,YEAR_TO_USEC),
											  rs->Conn.timeout,k);
					}
				}
			}
			pthread_mutex_unlock(&pl->mut);
		}
	}
}

/**
 * 根据d_node取连接池号
 * 失败返回-1
 */
int get_scpool_no(int d_node)
{
	int n;
	if(!scpool) return -1;
	for(n=0;n<SCPOOLNUM;n++) {
		if(scpool[n].d_node==d_node) return n;
	}
	return -1;
}

int get_scpoolnum()
{
	return SCPOOLNUM;
}

int getPathNum(int poolno)
{
	if(!scpool || poolno<0 || poolno>=SCPOOLNUM) return -1;
	return scpool[poolno].path_num;
}

int get_total_conn_num(int poolno)
{
	int i,total=0;

	if(!scpool || poolno<0 || poolno>=SCPOOLNUM) return -1;
	for(i=0;i<scpool[poolno].path_num;i++) {
		total += scpool[poolno].path[i].resource_num;
	}
	return total;
}

char *get_SC_DBLABEL(int poolno)
{
	if(!scpool || ctx_flg) return NULL;
	if(poolno<0 || poolno>=SCPOOLNUM) return NULL;
	if(!scpool[poolno].DBLABEL) {
		T_Connect *conn=get_SC_connect(poolno,0);
		release_SC_connect(&conn,poolno);
	}
	return scpool[poolno].DBLABEL;
}

