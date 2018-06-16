#include <sccli.h>

void s_output(char *str,FILE *fd);
//char *getenv(char *);

/* getls: client */
int getls(T_Connect *conn,T_NetHead *NetHead,char *path,FILE *outfile)
{
int ret;
	NetHead->data=path;
	NetHead->PKG_LEN=strlen(NetHead->data)+1;
	NetHead->ERRNO1=0;
	NetHead->ERRNO2=0;
	NetHead->PROTO_NUM=get_srv_no(conn->Var,"filels");
        if(NetHead->PROTO_NUM==1) {
                ShowLog(1,"%s:服务不存在",__FUNCTION__);
                return FORMATERR;
        }

	ret=SendPack(conn,NetHead);
	for(;;) {
		ret=RecvPack(conn,NetHead);
		if(ret|| NetHead->ERRNO1 || NetHead->ERRNO2) {
			if(NetHead->ERRNO2==100) break;
			fprintf(stderr,"getls recv ret=%d,err1=%d,err2=%d\n",
				ret,NetHead->ERRNO1,NetHead->ERRNO2);
			return -1;
		}
		s_output(NetHead->data,outfile);
	}
	return 0;
}
void s_output(char *str,FILE *fd)
{
char buf[1024];
char *p;
	for(p=str;*(p=stptok(p,buf,sizeof(buf)," "));p++) {
		fprintf(fd,"%s\n",buf);
	}
}

int s_getfile(T_Connect *conn,FILE *inputfile)
{
char buf[2048];
int ret,i;
char sflnm[256],dflnm[256];
char *p,*p1;
	*sflnm=0;
	*dflnm=0;
	while(!ferror(inputfile)) {
		fgets(buf,sizeof(buf),inputfile);
		if(feof(inputfile) || ferror(inputfile)) break;
		if(*buf=='#') continue;
		ret=sscanf(buf,"%s%s",sflnm,dflnm);
		p=sc_basename(sflnm);
		if(ret==1) {
			p1=getenv("RECVDIR");
			if(!p1||!*p1) p1="/tmp";
			if(p1[strlen(p1)-1]=='/') sprintf(dflnm,"%s%s",p1,p);
			else  sprintf(dflnm,"%s/%s",p1,p);
		}
		else if(dflnm[(i=strlen(dflnm))-1]=='/') {
			strcat(dflnm,p);
		} else if(isdir(dflnm)>0) {
				sprintf(dflnm+ret,"/%s",p);
		} else ;
		ret=N_Get_File(conn,dflnm,sflnm);
		if(ret && ret!=100) {
			sprintf(buf,"N_Get_File return %d",ret);
			ShowLog(1,buf);
			return ret;
		}
	}
	return 0;
}

int m_getfile(T_Connect *conn,T_NetHead *NetHead,char *path)
{
int ret;
FILE *tmpfd;
	tmpfd=tmpfile();
	if(!tmpfd) {
		fprintf(stderr,"create tmpfile err %d\n",errno);
		return -1;
	}
	ret=getls(conn,NetHead,path,tmpfd);
	if(ret) {
		fprintf(stderr,"getls err %d\n",ret);
		return ret;
	}
	fseek(tmpfd,0,0);
	ret=s_getfile(conn,tmpfd);
	return ret;
}
