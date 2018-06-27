#include <scsrv.h>

srvfunc *SRVFUNC;

int get_srvname(T_Connect *conn,T_NetHead *NetHead)
{
char *p=0;
int i;
	if(!SRVFUNC) {
		NetHead->ERRNO1=NetHead->ERRNO2=-1;
		NetHead->O_NODE=LocalAddr(conn->Socket,0);
		NetHead->data="SRVFUNC is empty!";
		NetHead->PKG_LEN=strlen(NetHead->data);
		NetHead->PKG_REC_NUM=0;
		return SendPack(conn,NetHead);
	}
	for(i=1;SRVFUNC[i].funcaddr;i++) {
		if(!p) {
			p=malloc(strlen(SRVFUNC[i].srvname)+20);
			sprintf(p,"%d|%s|",i,SRVFUNC[i].srvname);
		} else {
			p=realloc(p,strlen(p)+strlen(SRVFUNC[i].srvname)+20);
			sprintf(p+strlen(p),"%d|%s|",i,SRVFUNC[i].srvname);
		}
	}
	NetHead->ERRNO1=NetHead->ERRNO2=0;
	NetHead->O_NODE=LocalAddr(conn->Socket,0);
	NetHead->data=p;
	NetHead->PKG_LEN=strlen(NetHead->data);
	NetHead->PKG_REC_NUM=i-1;
	i=SendPack(conn,NetHead);
	free(p);
	return 0;
}

