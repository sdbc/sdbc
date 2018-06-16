#include <string.h>
char *TRIM(unsigned char *str)
{
unsigned char * cp;
	if(!str || !*str) return (char *)str;
	cp=str+strlen((char*)str);
	while(*cp<=(unsigned char)' '&&cp>=str){
		*cp--=0;
	}
	return (char *)str;
}

