#include <SRM.h>

char * mk_where(const char *keys,char *stmt)
{
char pks[104],*p,*p1;
int i; 

	if(!stmt) return 0;
	if(!keys || !*keys) {
		*stmt=0;
		return stmt;
	}
	p1=stmt;
	*p1=0;
	p=(char *)keys;
	for(i=0;*p;i++) {
		p=stptok(skipblk(p),pks,sizeof(pks),",|");
		if(*p) p++;
        	if(i==0) p1=stpcpy(p1,"WHERE $");
		else p1=stpcpy(p1," AND $");
		p1=stpcpy(stpcpy(stpcpy(p1,pks),"=:"),pks);
	}
	return p1;
}

