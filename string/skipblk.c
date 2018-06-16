#include <ctype.h>
char * skipblk(const char *);
char * skipblk(const char *str)
{
const char *p;
	p=str;
	while(!(*p&0x80)&&isspace(*p)) p++;
	return (char *)p;
}
