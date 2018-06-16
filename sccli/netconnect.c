
#include <ctype.h>
#include <malloc.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sccli.h>
/*****************************************************************
 * for sdbc 4.0
 * SDBC 的客户端连接和释放函数。
 * 连接后收取服务器送来的密钥。
 *****************************************************************/


/*******************************************************************/
/*Network connect                                                  */
/*Host: connect.Host                                               */
/*Service:connect.Service                                          */
/*Return: 0 Success !0:failt                                       */
/*2001.2  FanJiuShun                                               */
/*2001.8  update by YuLiHua ,Add CryptFlag and Prikey              */
/*2002.3.7   update    by FanJiuShun                               */
/*2007.8  update by YuLiHua , upgrad to SDBC 4.0                   */
/*2010.3  update by YuLiHua , upgrad to SDBC 5.0                   */
/*2012.3  update by YuLiHua , upgrad to SDBC 6.0                   */
/*******************************************************************/
extern int get_clikey(T_Connect *conn);
extern u_int family[];

int Net_Connect( T_Connect *conn,void *userdata,u_int *new_family)
{
int i=0,n,err,repeat;
int socket_no;
char *cp,addr[16];

    initconnect(conn);
    if(new_family) conn->family=new_family;
    else conn->family=family; 
    conn->Var=userdata;
    cp=getenv("CONN_REPEAT");
    if(cp && *cp) repeat=atoi(cp);
    else repeat=0;
    cp=getenv("TCPTIMEOUT");
    if(cp && isdigit(*cp)) {
	conn->timeout=atoi(cp);
    }
    n=0;
    do {
	conn->Socket=socket_no=tcpopen(conn->Host,conn->Service);
	if(socket_no>=0) break;
	i=errno;
	if(i!=ECONNREFUSED&&i!=EHOSTUNREACH) return i>0?-i:i==0?-1:i;
	if(n++ >= repeat) break;
	usleep(5000000);
    } while(1);
    if(socket_no<0) return socket_no;
    
    LocalAddr(socket_no,addr);
    i=get_clikey(conn); //取得客户密钥 
    if(i>=0){
        conn->Socket=socket_no;
    	ShowLog(2,"%s:netconnect %s/%s,OK ",__FUNCTION__,conn->Host,conn->Service);
	return 0;
    } else {
	ShowLog(1,"%s i=%d first recv errno=%d",__FUNCTION__,i,errno);
	err=errno;
	close(socket_no); /* in win closesocket();*/
	errno=err;
	return err;
    }
    return 0;
}

/*******************************************************************/
/*Network Disconnect                                               */
/*Return: 0 Success !0:fault                                       */
/*2003.3  YuLiHua                                                  */
/*******************************************************************/
void disconnect(T_Connect *conn)
{
T_NetHead NetHead;
	if(!conn) return;
	if(conn->Socket>=0) {
		NetHead.ERRNO1=0;
		NetHead.ERRNO2=0;
		NetHead.PROTO_NUM=-1;
		NetHead.PKG_LEN=0;
		SendPack(conn,&NetHead);
	}
	freeconnect(conn);
}

