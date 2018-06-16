/**********************************************
 * @(#) TCP SERVER Tools                      *
 * to suport Fiberized.IO
 **********************************************/
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <strproc.h>

#ifndef MIN
#define MIN(a,b) ((a)<(b))?(a):(b)
#endif

//timeout for second 
int RecvNet(int s,char *buf,int n,int timeout,int TCB_no)
{
int bcount,br,ret,num=0;
struct timeval tmout;

	if(!buf) return 0;
	tmout.tv_sec=timeout;
        tmout.tv_usec=0;
        ret=setsockopt(s,SOL_SOCKET,SO_RCVTIMEO,(char *)&tmout,sizeof(tmout));
	if(ret) {
                ShowLog(5,"%s:setsockopt err=%d,%s",__FUNCTION__,
                                errno,strerror(errno));
        }

	*buf=0;
	if(n<=0) return 0;
	bcount=0;
	br=0;

/* 为防止防火墙切断长期空闲的连接，对方可能定时发来心跳字符0x00,将其丢弃
	do {
		br=read(s,buf,1);
		if(br<0) return -1;
		if(br==0) continue;
	} while(*buf==0);
	buf++;
	br=1;
	bcount++;
*/
	num=0;
	while(bcount<n){
		if((br=read(s,buf,n-bcount))>0){
			bcount+=br;
			buf+=br;
			num=0;
			continue;
		}
		if(errno==EAGAIN) return TIMEOUTERR;
		if(br<0){
		    if(errno!=ECONNRESET) ShowLog(1,"%s:br=%d,err=%d,%s",__FUNCTION__,br,errno,strerror(errno));
		    return SYSERR;
		}
//ShowLog(5,"RecvNet:read br=0,errno=%d,%s",errno,strerror(errno));
		if(!br) {
			if( ++num>100) break;
			usleep(5000);
			continue;
		} else num=0;
	}
	if(bcount<=0) {
		ShowLog(1,"%s:s=%d,bcount=%d,err=%d,%s",__FUNCTION__,s,bcount,errno,strerror(errno));
		return -1;
	}
	return bcount;
}

int SendNet(int socket,char *buf,int n,int MTU,int TCB_no)
{
int bcount,br;

int sz,i;
socklen_t SendSize=0;
//socklen_t len = sizeof(SendSize);

	bcount=0;
	br=0;
	if(MTU>500) SendSize=MTU;
	else SendSize=n;
//	i=setsockopt(socket, SOL_SOCKET, SO_SNDBUF, (void*)&SendSize, len);
	i=0;
	while(bcount<n){
		i++;
		sz=MIN(n-bcount,SendSize);
		if((br=write(socket,buf,sz))>0){
			bcount+=br;
//ShowLog(5,"SedNet:n=%d,sz=%d,br=%d,bcount=%d,err=%d,%s",
//		n,sz,br,bcount,errno,strerror(errno));
			buf+=br;
			continue;
		}
		if(br<0){
		    return -1;
		}
		if(!br){
			return bcount;
		}
	}
	if(bcount<=0)return -1;
	return bcount;
}

