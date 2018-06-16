#include <string.h>
#include <pack.h>
#include <BB_tree.h>

static int cmp_str(void * t,void * s,int len)
{
	return strcmp((char *)t,(char *)s);
}
/* 从模板中选取除去except之外的列名 */
int except_col(char *buf,T_PkgType *tp,const char *except)
{
int num=0;
T_Tree *t=0;
T_PkgType *typ;
char *p,*name,*saveptr;
	if(!buf || !tp) return 0;
	if(except && *except) {
		strcpy(buf,except);
		for(p=strtok_r(buf,",|",&saveptr);p;p=strtok_r(0,",|",&saveptr)) {
			p=skipblk(p);
			TRIM(p);
			t=BB_Tree_Add(t,p,strlen(p)+1,cmp_str,0);
		}
	}
	p=buf;
	*p=0;
	for(typ=tp;typ->type>=0;typ++) {
		name=(char *)plain_name(typ->name);
		if(BB_Tree_Find(t,name,0,cmp_str)) continue;
		p=stpcpy(p,name);
		*p++=',';
		*p=0;
		num++;
	}

	BB_Tree_Free(&t,0);
	return num;
}
