#include <string.h>

char * strrevers(char *str)
{
register char *p,*p1;
char c;

p=str;
p1=str+strlen(str)-1;
	while(p1>p) {
        c=*p;
        *p=*p1;
        *p1=c;
        p++;
        p1--;
    }
	    return str;
}

