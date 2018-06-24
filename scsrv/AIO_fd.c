/*******************************************
 * 需要linux 2.6.22 以上版本
 * 和libaio 3.107以上版本
 * 如果不具备这个条件，在makefile里用SIO_fd.o
 * 取代本模块
 *******************************************/

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <libaio.h>

#include <scsrv.h>

static int AIO_oper(int fd,char *buff,size_t iosize,int flg)
{
	io_context_t myctx;
	int rc,num;
	uint64_t finished_aio;
	struct iocb _iocb,*io=&_iocb;
	struct io_event event;
	int	efd = eventfd(0, 0);
	T_YIELD yield=get_yield();

	if (efd == -1) {
		return flg?write(fd,buff,iosize):read(fd,buff,iosize);
	}

	memset(&myctx,0,sizeof(myctx));
	io_set_eventfd(io,efd);
	io_queue_init(1, &myctx);
	if(flg) io_prep_pread(io, fd, buff, iosize, 0);
	else	io_prep_pwrite(io, fd, buff, iosize, 0);
	rc = io_submit(myctx, 1, &io);
	if(rc<0) {
		close(efd);
		io_destroy(myctx);
		return flg?write(fd,buff,iosize):read(fd,buff,iosize);
	}
	if(yield) {
		rc = yield(efd,0,0);
		if(rc==0) eventfd_read(efd, &finished_aio);
	}
	close(efd);
	num = io_getevents(myctx, 1, 1, &event, NULL);
	if(num>0) {
		if(event.res2==0) num=event.res;
		else num=-1;
	}
	io_destroy(myctx);
	return num;
}

int AIO_read(int fd,char *buff,size_t iosize)
{
	return AIO_oper(fd,buff,iosize,0);
}

int AIO_write(int fd,char *buff,size_t iosize)
{
	return AIO_oper(fd,buff,iosize,1);
}
