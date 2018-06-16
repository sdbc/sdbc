#include <sys/stat.h>
#include <unistd.h>

int isdir(char *path)
{
struct stat buf;
int cc;
	cc=stat(path,&buf);
	if(!cc && (buf.st_mode & S_IFDIR)) return(1);
	return(cc);
}
//²âÊÔÎÄ¼þ¿É¶Á
int isrfile(char *path)
{
struct stat buf;
int cc;
int euid,egid;
	cc=stat(path,&buf);
	if(!cc) {
		if((buf.st_mode & S_IFMT) != S_IFREG) return 0;
		euid=geteuid();
		egid=getegid();
		if(euid==0) { // if root
			if(buf.st_mode & S_IRUSR || buf.st_mode & S_IRGRP ||
			   buf.st_mode & S_IROTH)
				 return 1;
			else return 0;
		}
		if((buf.st_mode & S_IROTH)!=0) return 1;
		if((buf.st_gid == egid) && ((buf.st_mode & S_IRGRP)!=0))
				return 1;
		if((buf.st_uid == euid) && ((buf.st_mode & S_IRUSR)!=0))
				return 1;
		
	}
	return cc;
}
