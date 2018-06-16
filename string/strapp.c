#include <strproc.h>

/************************************************************************/
/*function:trim 去除字符串前后空格	                                    */
/************************************************************************/
/*char *trim(char *src)
{
	int i = 0;
	char *ptr = src;

	while(src[i] != '\0')
	{
		if(src[i] != ' ')
			break;
		else
			ptr++;
		i++;
	}
	for(i = strlen(src)-1; i >= 0; i--)
	{
		if(src[i] != ' ')
			break;
		else
			src[i] = '\0';
	}

	return ptr;
}
*/
#define ISSPACE(ch) ((ch)==' '||(ch)=='\r'||(ch)=='\n'||(ch)=='\f'||(ch)=='\b'||(ch)=='\t')
char *trim(char *src)
{
	char *p = src;
	char *ptr;
	if(p)
	{
		ptr = p + strlen(src) - 1;
		while(*p && ISSPACE(*p)) 
			p++;
		while(ptr > p && ISSPACE(*ptr)) 
			*ptr-- = '\0';
	}
	return p;
}

/************************************************************************/
/*function:rtrim 去除字符串后的空格		                                */
/************************************************************************/
char *rtrim(char *str)
{
	unsigned char *cp;

	cp = (unsigned char *)(str + strlen(str));
	while((*cp <=(unsigned char)' ') && (cp >= (unsigned char *)str))
	{
		*cp-- = 0;
	}

	return str;
}

/************************************************************************/
/*function:ltrim 去除字符串前的空格		                                */
/************************************************************************/
char *ltrim(char* str)
{
	strrevers(str);
	rtrim(str);		//调用上面的rtrim()函数
	strrevers(str);
	return str;
}

