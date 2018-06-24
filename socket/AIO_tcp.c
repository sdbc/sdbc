/**********************************************
 * @(#) SDBC 7.1 TCP RECV/SEND  Tools                      *
 * to suport coroutine
 **********************************************/
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sc.h>

#ifndef MIN
#define MIN(a,b) ((a)<(b))?(a):(b)
#endif

static T_YIELD yield=NULL;

T_YIELD get_yield()
{
	return yield;
}

T_YIELD set_yield(T_YIELD new_yield)
{
	T_YIELD oldyield=yield;
	yield=new_yield;
	return oldyield;
}

//timeout for second
int RecvNet(int socket,char *buf,int n,int timeout)
{
	int bcount=0,br,ret;
	int i,repeat=0;
	int fflag=-1;

	if(socket<0) return SYSERR;
	if(!buf && n<0) return 0;
	if(yield) {
		fflag=fcntl(socket,F_GETFL,0);
		if(fflag!=-1) fcntl(socket,F_SETFL,fflag|O_ASYNC|O_NONBLOCK); //异步操作
	} else if(timeout>0) {
		struct timeval tmout;
		tmout.tv_sec=timeout;
		tmout.tv_usec=0;
		ret=setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,(char *)&tmout,sizeof(tmout));
		if(ret) ShowLog(1,"%s:setsockopt errno=%d,%s",__FUNCTION__,errno,strerror(errno));
	}

	*buf=0;
	br=0;

	while(bcount<n){
		if((br=read(socket,buf,n-bcount))>0){
			bcount+=br;
			buf+=br;
			repeat=0;
			continue;
		}
		if(fflag==-1 && errno==EAGAIN) return TIMEOUTERR;
		if(br<=0 && errno && errno != EAGAIN){
			if(errno!=ECONNRESET)
				ShowLog(1,"%s:br=%d,err=%d,%s",__FUNCTION__,br,errno,strerror(errno));
			break;
		}
		if(bcount < n && fflag!=-1) { //切换任务
			if(repeat++>3) return -errno;
/*
yield:
1.找到当前线程的context。找不到返回-1；(后续任务以同步阻塞完成）
2.把事件提交给epoll，作为resume的条件。
3.swapcontext，挂起这个任务，线程回到epoll_wait，可以为别人服务了。
4.一旦这个任务的事件发生了，立即由epoll_wait激活一个线程，抓取相应的context，使用setcontext，恢复任务现场，返回0.
后续的动作就是继续NONBLOCK的IO。直至完成返回。
*/
			ShowLog(5,"%s:tid=%lx,socket=%d,yield to schedle bcount=%d/%d",__FUNCTION__,pthread_self(),socket,bcount,n);
			i=yield(socket,0,timeout);
			if(i<0) {
				if(timeout>0) {
					struct timeval tmout;
					tmout.tv_sec=timeout;
					tmout.tv_usec=0;
					ret=setsockopt(socket,SOL_SOCKET,SO_RCVTIMEO,(char *)&tmout,sizeof(tmout));
				}
				fcntl(socket,F_SETFL,fflag);
				fflag=-1;
				if(i==TIMEOUTERR) return i;
			}
		}
	}
	if(fflag!=-1) fcntl(socket,F_SETFL,fflag);
	return bcount==0?-1:bcount;
}

int SendNet(int socket,char *buf,int n,int MTU)
{
	int bcount,bw;
	int sz,i=0;
	int fflag=-1;
	size_t SendSize;

	if(socket<0) return SYSERR;
	if(yield) {
		fflag=fcntl(socket,F_GETFL,0);
		if(fflag != -1) fcntl(socket,F_SETFL,fflag|O_NONBLOCK); //异步操作
	}
	bcount=0;
	bw=0;
	if(MTU>500) SendSize=MTU;
	else SendSize=n;
	while(bcount<n){
		sz=MIN(n-bcount,SendSize);
		if((bw=write(socket,buf,sz))>0){
			bcount+=bw;
			buf+=bw;
		}
		if(bw<0&&errno!=EAGAIN) {
			ShowLog(1,"%s:err=%d,%s",__FUNCTION__,errno,strerror(errno));
			break;
		}
		if(bw < sz && fflag != -1) { //切换任务
//ShowLog(5,"%s:tid=%lx,socket=%d,yield bw=%d/%d",__FUNCTION__,pthread_self(),socket,bw,sz);
			i=yield(socket,1,0);
			if(i<0) {
				fcntl(socket,F_SETFL,fflag);
				fflag = -1;
			}
		}
	}
	if(fflag != -1)  fcntl(socket,F_SETFL,fflag);
	return bcount==0?-1:bcount;
}

