#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <scsrv.h>
extern char *getenv();
extern FILE *logfile;
extern int chkexec(char *str);
extern int substitute_env(char *line);


#include <sys/stat.h>
/*************************************************/
/* 得到文件长度                                  */
/*     语法: int GetFileSize(FILE *fd)		 */
/*           fd:以打开的文件                     */
/*           返回: -1 失败                       */
/*                 >=0 文件长度            */
/*************************************************/
int GetFileSize(fd)
		int fd;
{
	struct stat stat_s;
	if(fd==0)return -1;
	if(fstat(fd,&stat_s)) return -1;
	if(!(stat_s.st_mode & S_IFREG)) return -2;
	return stat_s.st_size;
}

int Rexec(connect,NetHead)
		T_NetHead *NetHead;
		T_Connect *connect;
{
	char *cmd;
	char errbuf[580];
	int i,e_node;
	FILE *fd;
	int io_flag;
	int Event_no;
	e_node=LocalAddr(connect->Socket,0);
	Event_no=NetHead->PROTO_NUM;
	if(!NetHead->PKG_LEN) {
		ShowLog(1,"Rexec:which command?");
		NetHead->ERRNO1=EINVAL;
		goto errret;
	}
//    ShowLog(5,"Rexec:len=%d,%s",NetHead->PKG_LEN,NetHead->data);
/* 检查一下，命令是否允许执行 */
	i=chkexec(NetHead->data);
	if(i) {
		sprintf(errbuf,"rexec %.520s permisson denied %d",
				NetHead->data,i);
		ShowLog(1,"%s",errbuf);
		NetHead->ERRNO1=i;
		NetHead->data=errbuf;
		NetHead->PKG_LEN=strlen(NetHead->data);

		goto errret;
	}
	cmd=strdup(NetHead->data);
	ShowLog(5,"rexec:%s:errno1=%d,errno2=%d",
			cmd,NetHead->ERRNO1,NetHead->ERRNO2);
	io_flag=NetHead->ERRNO1;
	if(!io_flag){
		i=system(cmd);
		ShowLog(5,"System(%s):%04X",cmd,i);
		free(cmd);
		NetHead->ERRNO1=i>>8;
		NetHead->data=0;
		NetHead->PKG_LEN=0;
		errret:
		NetHead->PROTO_NUM=PutEvent(connect,Event_no);
		NetHead->PKG_REC_NUM=0;
		NetHead->D_NODE=0;
		NetHead->O_NODE=e_node;
		NetHead->ERRNO2=0;
		i=SendPack(connect,NetHead);
		return 0;
	} else {
		fd=0;
		if(io_flag==1){
			fd=popen(cmd,"w");
		}
		else if(io_flag==2){
			fd=popen(cmd,"r");
		} else {
			ShowLog(1,"rexec: io_flag %d error",io_flag);
			NetHead->PROTO_NUM=PutEvent(connect,Event_no);
			NetHead->PKG_REC_NUM=0;
			NetHead->ERRNO1=FORMATERR;
			NetHead->ERRNO2=0;
			NetHead->D_NODE=0;
			NetHead->O_NODE=e_node;
			NetHead->PKG_LEN=0;
			NetHead->data=0;
			i=SendPack(connect,NetHead);
			free(cmd);
			return 1;
		}
		if(!fd){
			NetHead->PROTO_NUM=PutEvent(connect,Event_no);
			NetHead->ERRNO1=errno;
			NetHead->ERRNO2=0;
			ShowLog(1,"popen %s error:%d",cmd,errno);
			NetHead->data=0;
			NetHead->PKG_LEN=0;
			NetHead->PKG_REC_NUM=0;
			NetHead->D_NODE=0;
			NetHead->O_NODE=e_node;
			i=SendPack(connect,NetHead);
			free(cmd);
			return 0;
		}
		if((io_flag)==1){ // Only put text to the process
			NetHead->PROTO_NUM=PutEvent(connect,Event_no);
			NetHead->PKG_LEN=0;
			NetHead->data=0;
			NetHead->PKG_REC_NUM=0;
			NetHead->D_NODE=0;
			NetHead->O_NODE=e_node;
			NetHead->ERRNO2=0;
			NetHead->ERRNO1=0;
			i=SendPack(connect,NetHead);
/*******************************************/
			do {
				i=RecvPack(connect,NetHead);
				if(i){
					ShowLog(1,"rexec wfile RecvPack err %d",i);
					pclose(fd);
					free(cmd);
					return 1;
				}
				if(NetHead->PKG_LEN){
					fprintf(fd,"%s",NetHead->data);
					fflush(fd);
				}
			} while (NetHead->ERRNO2 == PACK_CONTINUE);
			i=pclose(fd);
		} else if((io_flag)==2){ //Only get text from the process
			char buffer[SDBC_BLKSZ];
			NetHead->ERRNO2= PACK_CONTINUE;
			NetHead->ERRNO1=0;
			NetHead->PKG_REC_NUM=0;
			NetHead->D_NODE=0;
			NetHead->O_NODE=e_node;
			buffer[0]=0;
			do {
				NetHead->PKG_LEN=0;
				*buffer=0;
				for(i=0;i<(sizeof(buffer)-100);i=strlen(buffer)) {
					fgets(buffer+i,sizeof(buffer)-i,fd);
					if(ferror(fd)||feof(fd))break;
				}
				NetHead->PROTO_NUM=0;
				NetHead->data=buffer;
				NetHead->PKG_LEN=strlen(NetHead->data);
				i=SendPack(connect,NetHead);
				if(i){
					ShowLog(1,"rexec rfile SendPack err=%d",i);
					pclose(fd);
					free(cmd);
					return 1;
				}
			} while(!ferror(fd)&&!feof(fd));
			i=pclose(fd);
			NetHead->PROTO_NUM=PutEvent(connect,Event_no);
			NetHead->PKG_LEN=0;
			NetHead->data=0;
			NetHead->ERRNO1=i>>8;
			NetHead->ERRNO2=0;
			i=SendPack(connect,NetHead);
			if(i) {
				ShowLog(1,"rexec rfile final SendPack err %d",i);
				free(cmd);
				return i;
			}
		}
	}
	ShowLog(3,"Rexec %s success!",cmd);
	free(cmd);
	return 0;
}


int GetFile(connect,NetHead)
		T_NetHead *NetHead;
		T_Connect *connect;
{
	char fname[1024],*buffer;
	int i,size,e_node;
	int fd;
	int seekflag=0;
	int Event_no=NetHead->PROTO_NUM;
	e_node=LocalAddr(connect->Socket,0);
	seekflag=NetHead->PKG_REC_NUM;
	stptok(NetHead->data,fname,sizeof(fname),NULL);
	substitute_env(fname);
	ShowLog(5,"GetFile:%s,errno2=%d",fname,NetHead->ERRNO2);
	fd=open(fname,O_RDONLY);
	if(fd<0){
		ShowLog(1,"Can Not Open File:%s errno:%d:%s",
				fname,errno,strerror(errno));
		NetHead->ERRNO1=errno;
		errret:
		NetHead->PROTO_NUM=PutEvent(connect,Event_no);
		NetHead->ERRNO2=-1;
		NetHead->D_NODE=0;
		NetHead->O_NODE=e_node;
		NetHead->PKG_REC_NUM=0;
		NetHead->PKG_LEN=0;
		NetHead->data=0;
		i=SendPack(connect,NetHead);
		return 0;
	}
	{
		int r_len=0;
		size=GetFileSize(fd);
		NetHead->PKG_LEN=0;
		if(size>0) {
			NetHead->ERRNO2= PACK_CONTINUE;
			NetHead->ERRNO1=0;
			NetHead->O_NODE=e_node;

		} else {
			NetHead->ERRNO2=0;
			NetHead->ERRNO1=EINVAL;
			NetHead->O_NODE=e_node;
		}
		NetHead->PROTO_NUM=PutEvent(connect,Event_no);
		NetHead->PKG_REC_NUM=size;
		NetHead->D_NODE=0;
		i=SendPack(connect,NetHead);
		if(i || size<=0){
			ShowLog(1,"GetFile SendPack err %d,size=%d",i,size);
			close(fd);
			return 0;
		}
		buffer=valloc(SDBC_BLKSZ);
		if(!buffer) {
			NetHead->ERRNO1=MEMERR;
			close(fd);
			goto errret;
		}
		NetHead->PKG_REC_NUM=0;
		NetHead->ERRNO1=0;
		NetHead->O_NODE=e_node;
		NetHead->D_NODE=0;
		if(seekflag)lseek(fd,seekflag,SEEK_SET);
		do {
			*buffer=0;
			r_len=AIO_read(fd,buffer,SDBC_BLKSZ);
			if(r_len<=0)break;
			NetHead->PROTO_NUM=0;
			NetHead->PKG_LEN=r_len;
			NetHead->data=buffer;
			NetHead->ERRNO2= PACK_CONTINUE;
			i=SendPack(connect,NetHead);
			if(i){
				ShowLog(1,"GetFile1 SendPack2 err %d",i);
				close(fd);
				free(buffer);
				return 0;
			}
		} while(r_len==SDBC_BLKSZ);
		close(fd);
		NetHead->PROTO_NUM=PutEvent(connect,Event_no);
		NetHead->ERRNO2=0;
		NetHead->data=0;
		NetHead->PKG_LEN=0;
		i=SendPack(connect,NetHead);
		free(buffer);
	}
	ShowLog(2,"GetFile %s success",fname);
	return 0;
}

int PutFile(connect,NetHead)
		T_NetHead *NetHead;
		T_Connect *connect;
{
	char errbuf[512],buffer[SDBC_BLKSZ];
	int flg,i,count,e_node;
	int fd;
	int breakflag;
	int Event_no;
	e_node=LocalAddr(connect->Socket,0);
	Event_no=NetHead->PROTO_NUM;
	if(!NetHead->PKG_LEN) {
		ShowLog(1,"PutFile Filename EMPTY!");
		NetHead->ERRNO1=EINVAL;
		goto errret;
	}
	breakflag=NetHead->PKG_REC_NUM;
	stptok(NetHead->data,buffer,sizeof(buffer),NULL);
	if(*buffer=='$') {
		if(buffer[1]=='/') strsubst(buffer+1,0,"RECVDIR");
		else if(buffer[1]=='@') strsubst(buffer+1,1,"HOME");
		else ;
	}
	if(!strncmp(buffer,"@/",2)) strsubst(buffer,1,"$HOME");
	substitute_env(buffer);
	ShowLog(5,"PutFile %s",buffer);
	fd=0;
	if(breakflag){
		fd=open(buffer,O_WRONLY|O_APPEND|O_TRUNC,S_IREAD|S_IWRITE);
		if(!fd)
			fd=open(buffer,O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
	} else
		fd=open(buffer,O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE|S_IRGRP|S_IROTH);
	if(fd<0){
		NetHead->ERRNO1=errno;
		ShowLog(1,"Can't Open File:%s errno:%d",buffer,NetHead->ERRNO1);
		errret:
		NetHead->PROTO_NUM=PutEvent(connect,Event_no);
		NetHead->data=0;
		NetHead->PKG_LEN=0;
		NetHead->PKG_REC_NUM=0;
		NetHead->ERRNO2=PACK_NOANSER;
		NetHead->D_NODE=0;
		NetHead->O_NODE=e_node;
		i=SendPack(connect,NetHead);
		return 0;
	}
	if(breakflag){
		lseek(fd,0,SEEK_END);
		NetHead->PKG_REC_NUM = GetFileSize(fd);
	} else NetHead->PKG_REC_NUM=0;
	NetHead->PROTO_NUM=PutEvent(connect,Event_no);
	NetHead->ERRNO1=0;
	NetHead->ERRNO2=0;
	NetHead->O_NODE=e_node;
	NetHead->D_NODE=0;
	NetHead->data=0;
	NetHead->PKG_LEN= 0;
	i=SendPack(connect,NetHead);

	count=1;
	flg=0;
	do {
		i=RecvPack(connect,NetHead);
		if(i){
			ShowLog(1,"PutFile Recv:%s errno:%d",buffer,i);
			NetHead->ERRNO1=PACK_NOANSER;
			close(fd);
			goto errret;
		}
		if(NetHead->PKG_LEN&&!flg){
			i=AIO_write(fd,NetHead->data,NetHead->PKG_LEN);
			if(i<0) {
				flg=errno;
				NetHead->ERRNO1=errno;
				ShowLog(1,"Write %d File:%s count=%d i=%d,errno:%d,%s",
						count,buffer,count,i,errno,
						strerror_r(errno,errbuf,sizeof(errbuf)));
			}
			count++;
		}
	} while (NetHead->ERRNO2 == PACK_CONTINUE);
	close(fd);
	if(flg) {
		NetHead->ERRNO1=flg;
		goto errret;
	}

	NetHead->PROTO_NUM=PutEvent(connect,Event_no);
	NetHead->O_NODE=e_node;
	NetHead->D_NODE=0;
	NetHead->PKG_LEN=0;
	NetHead->data=0;
	NetHead->ERRNO2=0;
	NetHead->ERRNO1=0;
	i=SendPack(connect,NetHead);
	ShowLog(2,"PutFile %s succes",buffer);
	return 0;
}

int Pwd(connect,NetHead)
		T_NetHead *NetHead;
		T_Connect *connect;
{
	char buffer[SDBC_BLKSZ];
	int i,e_node;
	e_node=LocalAddr(connect->Socket,0);
	getcwd(buffer,sizeof(buffer));
	ShowLog(5,"PWD:%s",buffer);
	NetHead->PROTO_NUM=PutEvent(connect,NetHead->PROTO_NUM);
	NetHead->ERRNO2=0;
	NetHead->PKG_REC_NUM=0;
	NetHead->O_NODE=e_node;
	NetHead->D_NODE=0;
	NetHead->ERRNO1=errno;
	NetHead->data=buffer;
	NetHead->PKG_LEN=strlen(buffer);
	i=SendPack(connect,NetHead);
	if(i){
		return 1;
	}
	return 0;

}

int ChDir(connect,NetHead)
		T_NetHead *NetHead;
		T_Connect *connect;
{
	int i;
	char *cp;
	cp=NetHead->data+NetHead->PKG_LEN-1;
	if(*cp=='|') *cp=0;
	substitute_env(NetHead->data);
	ShowLog(5,"ChDir:%s",NetHead->data);
	errno=0;
	i=chdir(NetHead->data);
	if(i) {
		ShowLog(1,"%s:error=%d,%s",__FUNCTION__,errno,strerror(errno));
	}
	NetHead->ERRNO1=i;
	NetHead->PKG_LEN=0;
	NetHead->ERRNO2=errno;
	NetHead->PROTO_NUM=PutEvent(connect,NetHead->PROTO_NUM);
	NetHead->O_NODE=LocalAddr(connect->Socket,0);
	NetHead->D_NODE=0;
	i=SendPack(connect,NetHead);
	return i;
}

int PutEnv(T_Connect *connect,T_NetHead *NetHead)
{
	char buffer[SDBC_BLKSZ];
	int i,e_node;
	e_node=LocalAddr(connect->Socket,0);
	if(NetHead->PKG_LEN <=0) {
		ShowLog(1,"PutEnv PKG_LEN=0");
		NetHead->PROTO_NUM=PutEvent(connect,NetHead->PROTO_NUM);
		NetHead->PKG_LEN=0;
		NetHead->ERRNO2=-1;
		NetHead->PKG_REC_NUM=0;
		NetHead->O_NODE=e_node;
		NetHead->D_NODE=0;
		SendPack(connect,NetHead);
		return 1;
	}
	if(NetHead->data[NetHead->PKG_LEN-2] == '|')
		NetHead->data[NetHead->PKG_LEN-2] = 0;
	strcpy(buffer,NetHead->data);
	strcfg(buffer);
	ShowLog(5,"PutEnv:%s",buffer);
	errno=0;
	NetHead->PROTO_NUM=PutEvent(connect,NetHead->PROTO_NUM);
	NetHead->ERRNO1=0;
	NetHead->ERRNO2=0;
	NetHead->data=buffer;
	NetHead->PKG_LEN=strlen(NetHead->data);
	NetHead->O_NODE=e_node;
	NetHead->D_NODE=0;
	i=SendPack(connect,NetHead);
	return i;
}

