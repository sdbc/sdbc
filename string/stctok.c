#include <strproc.h>

char *cstrchr(const char *from,char *tok)
{
char *p;
	for(p=(char *)from;*p;p++) {
		if(*p != *tok) continue;
		if(!iscc(*(unsigned char *)tok)) return p;
		if(tok[1] == p[1]) return p;
		if(*p) p++;
	}
	return NULL;
}

char *stctok(const char *str,char *ctok,char **savep,int *by)
{
char *p=NULL,*start,*tok;
	if(!ctok||!*ctok) return (char *)str;

	if(str) start=(char *)str;
	else if(savep&&*savep) start=*savep;
	else return NULL;

	if(!*start) return NULL;
	if(by) *by=-1;
	for(tok=start;*tok;tok++) {
		p=cstrchr(ctok,tok);
		if(p) {
			if(by) *by=(int)(p-ctok);
			p=tok;
			break;
		}
		if(iscc(*(unsigned char *)tok)) tok++;
		if(!*tok) break;
	}

	if(p==NULL) {
		if(*start) p=start+strlen(start);
		else if(savep&&*savep) *savep=start;
	}
	if(!savep||!p) return p;
	if(iscc((unsigned char)(*p))) *p++ = 0;
	if(*p) *p++ = 0;
	*savep=p;
	return start;
}
