
#include <malloc.h>
#ifdef WIN32
#include <io.h>
#else
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>
#include <sccli.h>

int N_Get_File_Size(fd)
int fd;
{
struct stat stat_s;
if(fd==0)return -1;
if(fstat(fd,&stat_s))return -1;
return stat_s.st_size;
}

int EventCatch(T_Connect *conn,int Evtno)
{
	int RecvLen,ret;
	char *Recvbuf;
	if(Evtno) ShowLog(5,"EventCatch Evtno=%d",Evtno);
	if(!conn->Event_proc||!Evtno) return 0;
	RecvLen=conn->RecvLen;
	Recvbuf=conn->RecvBuffer;
	conn->RecvLen=0;
	conn->RecvBuffer=0;
	ret=conn->Event_proc(conn,Evtno);
	if(conn->RecvBuffer) free(conn->RecvBuffer);
	conn->RecvLen=RecvLen;
	conn->RecvBuffer=Recvbuf;
	return ret;
}

int N_Rexec(T_Connect *connect, char *cmd, int (*input)(char *),
			void (*output)(char *))
{
	T_NetHead nethead;
	int i;
	char buffer[SDBC_BLKSZ];
	INT4 o_node;
	int Event_no=0;
	o_node=LocalAddr(connect->Socket,0);
	nethead.PROTO_NUM=get_srv_no(connect->Var,"Rexec");
	if( nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:没有这个服务",__FUNCTION__);
		return FORMATERR;
	}
	nethead.O_NODE=htonl(o_node);
	nethead.D_NODE=0;
	nethead.ERRNO1=0;
	nethead.ERRNO2=1;
	nethead.PKG_REC_NUM=0;
	if(input)nethead.ERRNO1|=1;
	if(output)nethead.ERRNO1|=2;
	if(nethead.ERRNO1==3) {
		ShowLog(1,"can not have input and output at a time");
		return EINVAL;
	}
	nethead.data=cmd;
	nethead.PKG_LEN=strlen(nethead.data);
	i=SendPack(connect,&nethead);
	if(i) return i;
	if(input){
/*******************************************/
/* 2007.09.06 modify by ylh add a SendPack */

		i=RecvPack(connect,&nethead);
		if(i){
			ShowLog(1,"N_Rexec:未调试过的代码 input Recv i=%d",i);
			return i;
		}
		Event_no=nethead.PROTO_NUM;
		if(nethead.ERRNO1) {
			ShowLog(1,"N_Rexec 未调试过的代码 "
					  "ERRNO1=%d,ERRNO2=%d,Addr=%s,%s",
					nethead.ERRNO1,nethead.ERRNO2,
					StrAddr(ntohl(nethead.O_NODE),
							buffer+SDBC_BLKSZ-100)); // YYY
			EventCatch(connect,Event_no);
			return(nethead.ERRNO1);
		}
/*******************************************/
		while(!input(buffer)) {
			nethead.PROTO_NUM=1;
			nethead.data=buffer;
			nethead.PKG_LEN=strlen(buffer);
			nethead.ERRNO1=0;
			nethead.ERRNO2=PACK_CONTINUE;
			i=SendPack(connect,&nethead);
			if(i) return i;
		}
		nethead.ERRNO1=0;
		nethead.ERRNO2=PACK_NOANSER;
		nethead.PKG_LEN=0;
		i=SendPack(connect,&nethead);
		EventCatch(connect,Event_no);
		return i;
	}
	i=RecvPack(connect,&nethead);
	if(i){
		ShowLog(1,"N_Rexec Recv i=%d",i);
		return i;
	}
	Event_no=nethead.PROTO_NUM;
	if(nethead.ERRNO1) {
		ShowLog(1,"N_Rexec Recv errno1=%d,errno2=%d,o_node=%s",
				nethead.ERRNO1,nethead.ERRNO2,
				StrAddr(ntohl(nethead.O_NODE),buffer+SDBC_BLKSZ-100)); // YYY
		EventCatch(connect,Event_no);
		return(nethead.ERRNO1);
	}
	if(nethead.PKG_LEN&&output)output(nethead.data);
	while(nethead.ERRNO2==PACK_CONTINUE){
		i=RecvPack(connect,&nethead);
		if(i)return i;
		if(nethead.ERRNO1) {
			EventCatch(connect,nethead.PROTO_NUM);
			return nethead.ERRNO1;
		}
		if(nethead.PKG_LEN&&output)output(nethead.data);
	}
	EventCatch(connect,nethead.PROTO_NUM);
	return 0;
}


int N_Put_File(T_Connect *connect,char *local_file,char *remote_file)
{
	return N_Put_File_Msg(connect,local_file,remote_file,0);
}

int N_Put_File_Msg(T_Connect *connect,
				   char *local_file,
				   char *remote_file,
				   void (*Msg)(int,int))
{
	int i,r_len;
	int fd;
	char buffer[SDBC_BLKSZ];
	T_NetHead nethead;
	unsigned long  FileSize,count=0;
	T_CLI_Var *clip;
	if(!connect||!local_file||!remote_file) return -1;
	clip=(T_CLI_Var *)connect->Var;
	if(!clip) {
		ShowLog(1,"%s:no T_CLI_Var",__FUNCTION__);
		return FORMATERR;
	}

	nethead.PROTO_NUM=get_srv_no(clip,"PutFile");
	if( nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:没有这个服务",__FUNCTION__);
		return FORMATERR;
	}
	fd=open(local_file,O_RDONLY);
	if(fd<0){
		return errno;
	}
	FileSize=N_Get_File_Size(fd);
	nethead.ERRNO2=0;
	nethead.ERRNO1=0;
	nethead.PKG_REC_NUM=0;
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=0;
	nethead.data=remote_file;
	nethead.PKG_LEN=strlen(nethead.data);
//ShowLog(5,"N_Put_File local_file=%s,data=%s",local_file,nethead.data);
	if((i=SendPack(connect,&nethead))||(i=RecvPack(connect,&nethead))) {
		ShowLog(1,"%s:ret=%d,err=%d,%s",__FUNCTION__,i,errno,strerror(errno));
		close(fd);
		clip->Errno=-1;
		return -1;
	}
	if(nethead.ERRNO1) {
		StrAddr(ntohl(nethead.O_NODE),buffer);
		ShowLog(1,"PutFile recv err errno1=%di,errno2=%d,enode=%s",
				nethead.ERRNO1,nethead.ERRNO2,buffer);
		EventCatch(connect,nethead.PROTO_NUM);
		close(fd);
		return(nethead.ERRNO1);
	}
//ShowLog(5,"N_Put_File errno1=%d,errno2=%d continue %s",
//		nethead.ERRNO1,nethead.ERRNO2,nethead.data);
	{
		nethead.PROTO_NUM=1;
		nethead.ERRNO2=PACK_CONTINUE;
		nethead.PKG_REC_NUM=0;
		nethead.ERRNO1=0;
		nethead.O_NODE=clip->ctx_id;
		nethead.D_NODE=0;
		do {
			nethead.PKG_LEN=0;
			r_len=read(fd,buffer,sizeof(buffer));
			if(r_len<=0)break;
			nethead.PKG_LEN=r_len;
			nethead.data=buffer;
			i=SendPack(connect,&nethead);
			if(i){
				ShowLog(1,"PutFile loop send ret=%d,errno=%d",1,errno);
				close(fd);
				clip->Errno=-1;
				return i;
			}
			count+=r_len;
			if(Msg)Msg(FileSize,count);
		} while(r_len==sizeof(buffer));
		nethead.PKG_LEN=0;
		nethead.data=0;
		nethead.ERRNO2=0;
		nethead.ERRNO1=0;
		i=SendPack(connect,&nethead);
		close(fd);
		if(i) {
			clip->Errno=-1;
			return i;
		}
	}
	i=RecvPack(connect,&nethead);
	if(i){
		clip->Errno=-1;
		return i;
	}
	EventCatch(connect,nethead.PROTO_NUM);
	if(nethead.ERRNO1) {
		StrAddr(ntohl(nethead.O_NODE),buffer);
		ShowLog(1,"PutFile Final recv errno1=%d,errno2=%d,enode=%s",
				nethead.ERRNO1,nethead.ERRNO2,buffer);
	}
	return nethead.ERRNO1;
}

int N_Get_File(T_Connect *connect,char *local_file,char *remote_file)
{
	return N_Get_File_Msg(connect,local_file,remote_file,0);

}

int N_Get_File_Msg(T_Connect *connect,
				   char *local_file,
				   char *remote_file,
				   void (*Msg)(int,int))
{
//char buffer[1025];
	int i;
	int fd;
	T_NetHead nethead;
	unsigned long FileSize=0,count=0;
	int Event_no=0;
	T_CLI_Var *clip;
	if(!connect||!local_file||!remote_file) return -1;
	clip=(T_CLI_Var *)connect->Var;
	if(!clip) {
		ShowLog(1,"%s:no T_CLI_Var",__FUNCTION__);
		return FORMATERR;
	}
	ShowLog(5,"文件传输 本地文件名:%s <-------- 远程文件名:%s",local_file,remote_file);
	nethead.PROTO_NUM=get_srv_no(clip,"GetFile");
	if( nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:没有这个服务",__FUNCTION__);
		return FORMATERR;
	}

	fd=open(local_file,O_WRONLY|O_CREAT|O_TRUNC,S_IREAD|S_IWRITE);
	if(fd<0){
		ShowLog(1,"Open %s err %d",local_file,errno);
		return -1;
	}

	nethead.PKG_REC_NUM=0;
	nethead.ERRNO1=0;
	nethead.ERRNO2=0;
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=0;
	nethead.data=remote_file;
	nethead.PKG_LEN=strlen(nethead.data);
	i=SendPack(connect,&nethead);
	if(i) {
		close(fd);
		clip->Errno=-1;
		return i;
	}
	do {
		i=RecvPack(connect,&nethead);
		if(i){
			close(fd);
			clip->Errno=-1;
			return i;
		}
		Event_no=nethead.PROTO_NUM;
		if(nethead.PKG_REC_NUM>0){
			FileSize=nethead.PKG_REC_NUM;
			continue;
		}
		if(nethead.PKG_LEN){
			count+=nethead.PKG_LEN;
			i=write(fd,nethead.data,nethead.PKG_LEN);
			if(i!=(int)nethead.PKG_LEN){
				close(fd);
				EventCatch(connect,Event_no);
				ShowLog(1,"N_GetFile:len=%d,write=%d",nethead.PKG_LEN,i);
				clip->Errno=-1;
				return errno;
			}
			if(Msg) Msg(FileSize,count);
		}
	} while(nethead.ERRNO2==PACK_CONTINUE);
	EventCatch(connect,Event_no);
	close(fd);
	return nethead.ERRNO1;
}

int N_PWD(T_Connect *connect,char *pwd_buf)
{
	T_NetHead nethead;
	T_CLI_Var *clip;
	if(!connect||!pwd_buf) return -1;
	clip=(T_CLI_Var *)connect->Var;
	if(!clip) {
		ShowLog(1,"%s:no T_CLI_Var",__FUNCTION__);
		return FORMATERR;
	}

	nethead.PROTO_NUM=get_srv_no(clip,"Pwd");
	if( nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:没有这个服务",__FUNCTION__);
		return FORMATERR;
	}
	nethead.ERRNO2=0;
	nethead.PKG_LEN=0;
	nethead.PKG_REC_NUM=0;
	nethead.ERRNO1=0;
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=0;
	nethead.data=0;
	if(SendPack(connect,&nethead)||RecvPack(connect,&nethead)) {
		clip->Errno=-1;
		return -1;
	}
	EventCatch(connect,nethead.PROTO_NUM);
	if(nethead.ERRNO1) return nethead.ERRNO1;
	if(nethead.PKG_LEN) strcpy(pwd_buf,nethead.data);
	return 0;
}

int N_ChDir(T_Connect *connect,char *dir)
{
	T_NetHead nethead;
	T_CLI_Var *clip;
	if(!connect||!dir) return -1;
	clip=(T_CLI_Var *)connect->Var;
	if(!clip) {
		ShowLog(1,"%s:no T_CLI_Var",__FUNCTION__);
		return FORMATERR;
	}
	nethead.PROTO_NUM=get_srv_no(clip,"ChDir");
	if( nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:没有这个服务",__FUNCTION__);
		return FORMATERR;
	}
	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=0;
	nethead.ERRNO1=0;
	nethead.ERRNO2=0;
	nethead.PKG_REC_NUM=0;
	nethead.PKG_LEN=0;
	nethead.data=dir;
	nethead.PKG_LEN=strlen(nethead.data);
	if(SendPack(connect,&nethead)||RecvPack(connect,&nethead)) {
		clip->Errno=-1;
		return -1;
	}
	EventCatch(connect,nethead.PROTO_NUM);
	return nethead.ERRNO1;
}

int N_PutEnv(T_Connect *connect,char *env)
{
	T_NetHead nethead;
	T_CLI_Var *clip;
	if(!connect||!env) return -1;
	clip=(T_CLI_Var *)connect->Var;
	if(!clip) {
		ShowLog(1,"%s:no T_CLI_Var",__FUNCTION__);
		return FORMATERR;
	}
	nethead.PROTO_NUM=get_srv_no(clip,"PutEnv");
	if( nethead.PROTO_NUM==1) {
		ShowLog(1,"%s:没有这个服务",__FUNCTION__);
		return FORMATERR;
	}

	nethead.O_NODE=clip->ctx_id;
	nethead.D_NODE=0;
	nethead.ERRNO1=2;
	nethead.ERRNO2=0;
	nethead.PKG_REC_NUM=0;
	nethead.data=env;
	nethead.PKG_LEN=strlen(nethead.data);
	if(SendPack(connect,&nethead)||RecvPack(connect,&nethead)) {
		clip->Errno=-1;
		return -1;
	}
	EventCatch(connect,nethead.PROTO_NUM);
	return nethead.ERRNO1;
}
