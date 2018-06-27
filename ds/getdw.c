#include <errno.h>
#include <strproc.h>
#include <dw.h>

static int dwcomp();
static void dw_free();

char *getdw(int k,DWS *dw_CTX)
{
struct dwnode dwk;
T_Tree *tp;
	dwk.num=k;
	tp=BB_Tree_Find(dw_CTX->node,&dwk,sizeof(dwk),dwcomp);
	if(tp) return ((struct dwnode *)tp->Content)->name;
	else return NULL;
}

static int dwcomp(dw1,dw2,len)
register struct dwnode *dw1,*dw2;
int len;
{
	if(dw1->num > dw2->num) return 2;
	else if (dw1->num < dw2->num) return -2;
	else return 0;
}

int initdw(file,dwp)
char  *file;
DWS *dwp;
{
char bb[1024],*aa;
FILE *fd;
struct dwnode node;
	dwp->node=0;
	fd=fopen(file,"r");
	if(!fd) {
//		ShowLog(1,"%s:err=%d,%s",__FUNCTION__,errno,strerror(errno));
		return -1;
	}
	while(!ferror(fd)) {
		fgets(bb,sizeof(bb),fd);
		if(feof(fd)) break;
		if(*bb=='#') {
			continue;
		}
		node.num=strtoul(bb,&aa,10);
		if(aa==bb) {
			continue;
		}
		aa=skipblk(aa);
		TRIM(aa);
		node.name=strdup(aa);
		dwp->node=BB_Tree_Add(dwp->node,&node,sizeof(node),dwcomp,0);
	}
	fclose(fd);
	return 0;
 }

void freedw(DWS *dwp)
{
	BB_Tree_Free(&dwp->node,dw_free);
}

static void dw_free(struct dwnode *node)
{
	if(node->name) free(node->name);
}

