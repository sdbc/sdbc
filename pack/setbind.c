#include <pack.h>

int set_bindtype_idx(T_PkgType *tp,int bindtype,const char *choose,int cols,char *idx)
{
char *p,buf[100],*p1;
int i=0;

	if(!tp) return -1;

	if(!choose) {
	T_PkgType *typ;
		for(typ=tp;typ->type>-1;typ++) {
			typ->bindtype = bindtype;
			i++;
		}
	} else {
		p=(char *)choose;
		while(*p) {
		int n;
			p=stptok(p,buf,sizeof(buf),",|");
			if(*p) p++;
			p1=skipblk(buf);
			TRIM(p1);
			n=index_col(idx,cols,p1,tp);
			if(n>-1) {
				tp[n].bindtype = bindtype;
				i++;
			}
		}
	}
	return i;
}
