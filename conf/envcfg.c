#include <strproc.h>
#include <regex.h>
#include <errno.h>

#define REGNUM 3

static char env_src[]="\\$[{(]{0,1}([A-Za-z_][0-9A-Za-z_]*)[})]{0,1}";
static regex_t env_rp;
static int compflg=0;

static void reg_free(void)
{
	if(compflg) {
		compflg=0;
		regfree(&env_rp);
	}
}

int substitute_env(char *line)
{
char env[256];
char *p=line,*envp,*getenv();
regmatch_t pmatch[REGNUM];
int i;
	if(!line) return 0;
	if(!compflg) {
	    i=regcomp(&env_rp,env_src,REG_EXTENDED);
	    if(i) {
		regerror(i,&env_rp,line,100);
		return -1;
	    }
	    compflg=1;
	}
	while(!(i=regexec(&env_rp,p,REGNUM,pmatch,0))) {
		for(envp=p+pmatch[1].rm_so;envp<p+pmatch[1].rm_eo;envp++) {
			env[i++]=*envp;			
		}
		env[i]=0;
		envp=getenv(env);
		if(!envp) envp=".";
		p=strsubst(p+pmatch[0].rm_so,
			pmatch[0].rm_eo - pmatch[0].rm_so,envp);
	}
	return 0;
}

int envcfg(char *fname)
{
FILE *fd;
int i,err=-1;
char buffer[1024],*cp;
	tzset();
	if(fname && NULL != (fd=fopen(fname,"r"))) {
		err=0;
		while(!ferror(fd)){
			fgets(buffer,sizeof(buffer),fd);
			if(feof(fd)) break;
			TRIM(buffer);
			i=skipblk(buffer)-buffer;
			strsubst(buffer,i,(char *)0);
			if(!*buffer || *buffer=='#') continue;
			i=substitute_env(buffer);
			if(i) {
				fprintf(stderr,"%s",buffer);
				fclose(fd);
				reg_free();
				return i;
			}
#ifdef WIN32
			putenv(buffer);
#else
			cp=strdup(buffer);
			putenv(cp);
#endif
		}
		reg_free();
		fclose(fd);
	} else if(!fd) {
		err=errno;
		fprintf(stderr,"%s:open file %s err=%d,%s",__FUNCTION__,
			err,strerror(err));
	}

	cp=getenv("GBK_FLAG");
        if(cp && *cp=='T') GBK_flag=1;

	return err;
}

int strcfg(char *buffer)
{
int i;
char *cp;
	i=skipblk(buffer)-buffer;
	strsubst(buffer,i,(char *)0);
	if(!*buffer || *buffer=='#') return 1;
	i=substitute_env(buffer);
	if(i) return i;
#if defined WIN32 || defined _WIN64
	_putenv(buffer);
#else
	cp=strdup(buffer);
	putenv(cp);
#endif
	reg_free();
	return 0;
}
