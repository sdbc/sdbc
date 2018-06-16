#include <string.h>

char *sc_basename(char *path)
{
char *p;
	if(!path||!*path) return path;
	p=strrchr((const char *)path,'/');
	if(p) return ++p;
	return path;
}
