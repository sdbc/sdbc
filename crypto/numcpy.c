#include <stdio.h>
void numcpy(n,to,from)
unsigned int n,*to,*from;
{
	while(n--) *to++ = *from++;
}
