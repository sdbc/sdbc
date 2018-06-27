#include <stdio.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sc.h>

static char IFMT[]="%u.%u.%u.%u";
int peeraddr(int sock,char net_addr[16])
{
int cc;
struct p_name {
	unsigned short family;
	unsigned short port;
	unsigned char addr[14];
}pname;
socklen_t lname;
       lname=14;
       	if((cc=getpeername(sock,(struct sockaddr *)&pname,&lname))==0){
		sprintf(net_addr,IFMT,
		 	(unsigned char)pname.addr[0],
		    	(unsigned char)pname.addr[1],
		    	(unsigned char)pname.addr[2],
		    	(unsigned char)pname.addr[3]);
		lname=*(int *)pname.addr;
		return lname;
       	}
	else {
		sprintf(net_addr,"000000");
		return 0;
	}
}

INT4 LongAddr(char *p)
{
INT4 addr,i;
//#ifdef INET_ADDRSTRLEN
	i=inet_pton(AF_INET,p,&addr);
	if(i==1) return addr;
	else {
		ShowLog(1,"longaddr %s err=%d",p,errno);
		return 0;
	}
/*
#else
	addr=0;
	sscanf(p,IFMT,(unsigned char)&buf[0],(unsigned char)&buf[1],
			(unsigned char)&buf[2],(unsigned char)&buf[3]);
	for(i=0;i<4;i++) {
		addr+=buf[i];
		addr <<= 8;
	}
	return addr;
#endif
*/
}

char *StrAddr( INT4 addr,char str[16])
{
char *p;
	if(str) {
//#ifdef INET_ADDRSTRLEN
		p=(char *)inet_ntop(AF_INET,&addr,str,INET_ADDRSTRLEN);
		if(p) return p;
		return "ADDR ERROR";
/*
#else
		sprintf(str, IFMT, 
			addr>>24,
			(addr>>16) &255, 
			(addr>>8) &255,
			addr &255
		);
#endif
*/
	}
	return str;
}

int LocalAddr(int Sock, char szAddr[16])
{
struct sockaddr_in *psockaddr;
char cInfo[128];
socklen_t lSize;

    psockaddr = (struct sockaddr_in*)cInfo;
    lSize = sizeof(cInfo);

    if(getsockname(Sock, (struct sockaddr *)psockaddr, &lSize) < 0)
		return -1;;

    StrAddr(psockaddr->sin_addr.s_addr,szAddr);
    return psockaddr->sin_addr.s_addr;
}

