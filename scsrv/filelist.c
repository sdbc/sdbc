#include <scsrv.h>
#include <string.h>

//extern char *getenv(char *);
//extern char *stpcpy(char *d,char *s);
/* file_list: server */
int filels(T_Connect *conn,T_NetHead *NetHead)
{
char *buff,*env,*p;
char *p1;
char tmp[1000],dir[512],tmp1[2000];
FILE *fd;
int ret;
char buffer[SDBC_BLKSZ];
int Event_no=NetHead->PROTO_NUM;
	if(!NetHead->PKG_LEN) {
		ShowLog(1,"getlist what to list?");
     		NetHead->ERRNO1=-1;
errret:
		NetHead->PROTO_NUM=PutEvent(conn,Event_no);
		NetHead->data=0;
		NetHead->PKG_LEN=0;
     		NetHead->ERRNO2=-1;
     		NetHead->PKG_REC_NUM=0;
     		NetHead->O_NODE=LocalAddr(conn->Socket,0);
     		NetHead->D_NODE=0;
		ret=SendPack(conn,NetHead);
		return 0;
	}
	buff=malloc(NetHead->PKG_LEN+200);
	if(!buff) {
		NetHead->ERRNO1=-2;
		goto errret;
	}
	strcpy(buff,NetHead->data);
//ShowLog(5,"errno2=%d %s",NetHead->ERRNO2,buff);
	if(!strncmp(buff,"$/",2)) {
		env=getenv("SENDDIR");
		p=buff+1;
	} else if(!strncmp(buff,"@/",2)) {
		env=getenv("HOME");
		p=buff+1;
	} else if(*buff=='$') {
		p=stptok(buff+1,tmp,sizeof(tmp),"/");
		env=getenv(tmp);
	} else {
		p=buff;
	}

	if(p>buff) {
		if(!env||!*env) env=".";
		if(env[strlen(env)-1]=='/') env[strlen(env)-1]=0;
		strcpy(tmp,env);
//ShowLog(5,"%s:env=%s,buff=%s",__FUNCTION__,env,buff);
		strsubst(buff,p-buff,tmp);
	}
ShowLog(5,"filelist:path=%s",buff);
	sprintf(tmp,"ls %s",buff);
	if(isdir(buff)>0) strcpy(dir,buff);
	else {
		p1=strrchr(buff,'/');
		if(p1) {
			*p1=0;
			strcpy(dir,buff);
		} else strcpy(dir,".");
	}
	if(dir[strlen(dir)-1]=='/') dir[strlen(dir)-1]=0;
	fd=popen(tmp,"r");
	if(!fd) {
		ShowLog(1,"%s:err=%d",tmp,errno);
		free(buff);
		NetHead->ERRNO1=errno;
		goto errret;
	}
//ShowLog(5,"tmp=%s",tmp);
	p=buffer;
	*p=0;
	while((ret=fscanf(fd,"%s",tmp))==1) {
		if(*buffer==0 && tmp[strlen(tmp)-1] == ':') {//√ª’“µΩ
			break;
		}
		p1=sc_basename(tmp);
		ret=sprintf(tmp1,"%s/%s",dir,p1);
		if(isrfile(tmp1)<=0) continue; //not readble reg file
/* send tmp1 to client */
		if((int)(buffer+sizeof(buffer)-2-p)>ret) {
			p=stpcpy(p,tmp1);
		} else {
//ShowLog(5,"%s",buffer);
			NetHead->PROTO_NUM=0;
			NetHead->data=buffer;
     			NetHead->PKG_LEN=strlen(NetHead->data);
     			NetHead->ERRNO2= PACK_CONTINUE;
     			NetHead->O_NODE=LocalAddr(conn->Socket,0);
     			NetHead->D_NODE=0;
			ret=SendPack(conn,NetHead);
			if(ret) break;
			p=buffer;
			p=stpcpy(p,tmp1);
			*p++ = ' ';
			*p=0;
		}
	}
	pclose(fd);
	if(*buffer) {
//ShowLog(5,"final %s",buffer);
		NetHead->PROTO_NUM=0;
		NetHead->data=buffer;
     		NetHead->PKG_LEN=strlen(NetHead->data);
     		NetHead->ERRNO2= PACK_CONTINUE;
     		NetHead->O_NODE=LocalAddr(conn->Socket,0);
		ret=SendPack(conn,NetHead);
		*buffer=0;
	}
	NetHead->PROTO_NUM=PutEvent(conn,Event_no);
	NetHead->data=0;
     	NetHead->PKG_LEN=0;
     	NetHead->ERRNO2=0;  
     	NetHead->PKG_REC_NUM=0;
     	NetHead->O_NODE=LocalAddr(conn->Socket,0);
     	NetHead->D_NODE=0;
	ret=SendPack(conn,NetHead);
	free(buff);
	return 0;
}
