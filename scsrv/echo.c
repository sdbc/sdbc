#include <scsrv.h>
#include <datejul.h>

static char * reversln(char *str)
{
char c,*p,*p1;
	p=str;
	p1=str+strlen(str)-1;
	if(*p1=='\n') p1--;
	while(p1>p) {
		c=*p;
		*p=*p1;
		*p1=c;
		p++;
		p1--;
	}
	return str;
}

int Echo(T_Connect *connect,T_NetHead *NetHead)
{
	if(NetHead->ERRNO2==PACK_NOANSER||NetHead->ERRNO2==PACK_CONTINUE) return 0;
	if(NetHead->PKG_LEN>0) {
		ShowLog(5,"Echo:PROTO_NUM=%d,data=%.30s,at %llu",
				NetHead->PROTO_NUM,NetHead->data,now_usec());
		reversln(NetHead->data);
	} 
	NetHead->PROTO_NUM=PutEvent(connect,NetHead->PROTO_NUM);
	NetHead->ERRNO1=0;
	NetHead->ERRNO2=0;
	NetHead->PKG_REC_NUM=0;
	NetHead->O_NODE=LocalAddr(connect->Socket,0);
    	SendPack(connect,NetHead);
	return 0;
}
