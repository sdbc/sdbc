#include <scsrv.h>
#include <arpa/inet.h>

int return_error(T_Connect *conn,T_NetHead *nethead,const char *msg)
{
	nethead->data=(char *)msg;
	nethead->PKG_REC_NUM=0;
	nethead->PKG_LEN=msg?strlen(nethead->data):0;
	nethead->PROTO_NUM=PutEvent(conn,65535 & nethead->PROTO_NUM);
	nethead->O_NODE=ntohl(LocalAddr(conn->Socket,NULL));
	nethead->ERRNO1=SendPack(conn,nethead);
	return 0;
}

