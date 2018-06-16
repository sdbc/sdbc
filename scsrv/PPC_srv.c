/************************************************************
 * netmain():SDBC5.X 服务器的单线程调度管理器。依靠inetd启动 
 * 依据包头中的 PROTO_NUM 调用结构数组FUNCTION中的过程。
 * 启动后首先由客户端发起密钥协商  
 ************************************************************/

#include <signal.h>
#include <scsrv.h>
#include <ctype.h>
#include <malloc.h>
extern void exit(int);
extern u_int family[];

void setquit(Quit)
void (*Quit)();
{
    signal(SIGTERM,Quit);
    signal(SIGHUP,Quit);
    signal(SIGINT,Quit);
    signal(SIGQUIT,Quit);
    signal(SIGALRM,Quit);
    signal(SIGPIPE,SIG_IGN);
}
/************************************************************/
/* Process Per Connection  Server                           */
/* 2010.7 by Yulihua                                        */
/*2000.5       FanJiuShun                                   */
/*2001.2  Update By FanJiuShun Add Function Parameter       */
/*2001.8  Update By YuLiHua Add Crypt Flag and UserDate     */
/*        CryptFlag=1:do crypt else don't crypt             */
/*2007.8  Update By YuLiHua Add Crypt Flag and UserDate     */
/*        CryptFlag=2:do check else don't it                */
/*        CryptFlag=4:do zip else don't it                  */
/************************************************************/

extern srvfunc *SRVFUNC;
int PPC_srv(void(*NetInit)(T_Connect *,T_NetHead *),srvfunc *Function,void *userdata)
{
T_NetHead NetHead;
int cc;
char clientkey[81];
T_Connect connect;
int logined=0;
    SRVFUNC=Function;
    initconnect(&connect);
    *connect.Host=0;
    connect.Socket=0;
#ifdef LINUX
    cc=1;
    setsockopt(connect.Socket,SOL_SOCKET,SO_REUSEADDR,&cc,sizeof(cc));
#endif
    connect.family=family;
    connect.Var=userdata;
    connect.CryptFlg=mk_clikey(connect.Socket,&connect.t,connect.family);
    if(connect.CryptFlg<0) { //协商密钥失败
 	freeconnect(&connect);   
	ShowLog(1,"协商密钥失败");
	return -1;
    }
    if(NetInit) NetInit(&connect,&NetHead);
    ShowLog(2,"Server start");
    for(;;){
	cc=RecvPack(&connect,&NetHead);
	if(cc<0) {
		peeraddr(connect.Socket,clientkey);
		if(cc!=FORMATERR)
			ShowLog(0,"Connect refused by client %d,%s",
				cc,strerror(errno));
		else ShowLog(0,"NETERR %s,%d",clientkey,cc);
		freeconnect(&connect);
		return(cc);
	}
	ShowLog(4,"netmain PROTO_NUM:%d T_LEN=:%d,PKG_LEN=%d",
			NetHead.PROTO_NUM,NetHead.T_LEN,NetHead.PKG_LEN);
	if(NetHead.PROTO_NUM==1){
		Function[1].funcaddr(&connect,&NetHead);
		continue;
	}
	if(NetHead.PROTO_NUM==0xFFFF){
		ShowLog(0,"Disconnect by client");
		freeconnect(&connect);
		return(0);
	}
	if(!logined ) {
	    if(!NetHead.PROTO_NUM) {
		logined=Function[0].funcaddr(&connect,&NetHead);
		if(logined==-1) goto _quit_;
		if(!connect.only_do) {
			Function[0].funcaddr=get_srvname;
			Function[0].srvname="get_srvname";
		}
	    } else {
		if(NetHead.PKG_LEN) ShowLog(0,"NotLogin,cmd=%d:%.60s",
					NetHead.PROTO_NUM,NetHead.data);
		else ShowLog(0,"NotLogin,cmd=%d",NetHead.PROTO_NUM);
		NetHead.data="Not Login!";
		NetHead.ERRNO1=NOTLOGIN;
		NetHead.PKG_LEN=strlen(NetHead.data)+1;
		SendPack(&connect,&NetHead);
	    }
	} else if (connect.only_do) {
		cc=connect.only_do(&connect,&NetHead);
		if(cc==-1) { 
			ShowLog(0,"Disconnect by server onlydo PROTO_NUM=%d,cc=%d",
				NetHead.PROTO_NUM,cc);
			freeconnect(&connect);
			return(cc);
		}
	} else {
		cc=Function[NetHead.PROTO_NUM].funcaddr(&connect,&NetHead);
		if(cc==-1) {
_quit_:
			ShowLog(0,"Disconnect by server PROTO_NUM=%d,cc=%d",
				NetHead.PROTO_NUM,cc);
			freeconnect(&connect);
			return(0);
		}
	}
    }
}
