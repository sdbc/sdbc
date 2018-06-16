
#include <ctype.h>
#include <strproc.h>

int c_isspace(char *line)
{
unsigned char *p=(unsigned char *)line;
	if(iscc(*p)) {
		if((*p)==0xA1 && (p[1]==0xA1)) return 2;
		return 0;
	} else if(isspace(*p)) return 1;
	return 0;
}
char *trim_all_space(char *str)
{
char *p,*savep;
int cc,count;
	savep=NULL;
	count=0;
	for(p=str;*p;) {
		if(0!=(cc=c_isspace(p))) {
			if(!count) savep=p;
			count+=cc;
			p+=cc;
		} else if(count) {
			p=strsubst(savep,count,NULL);
			savep=NULL;
			count=0;
		} else p++;
	}
	if(count) {
		p=strsubst(savep,count,NULL);
	}
	return str;
}
