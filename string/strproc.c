/****************************** description *****************************/
/*Copyright (C), 2006-2009, EASYWAY. Co., Ltd.							*/
/*function:字符串处理函数												*/
/*author:yulihua、huangpeng												*/
/*modify date:2009-4-21													*/
/************************************************************************/
#include <ctype.h>
#include <strproc.h>

#ifndef MIN
#define MIN(a,b) ((a>b)?b:a)
#endif

/*function:iscc 判断是否是汉字*/
int iscc(unsigned char ch)
{
	return (ch >= 0x81 && ch < 0xff);
}
/*function:cc1 取汉字的一个字节*/
static int cc1(unsigned char *bp,unsigned char *bufp)
{
	register unsigned char *p;
	register int i = 0;
	for(p = bufp; iscc(*p); p--)
	{
		i++;
		if(p <= bp) 
			break;
	}
	return (i & 1);
}
/*function:firstcc 汉字的第一个字节*/
int firstcc(unsigned char *bp,unsigned char *bufp)
{
	if(!bufp || !(*bufp) || (bufp < bp) || !iscc(*bufp)) 
		return 0;
	return (cc1(bp, bufp));
}
/*function:secondcc 汉字的第二个字节*/
int secondcc(unsigned char *bp,unsigned char *bufp)
{
	if(!firstcc(bp, bufp-1)) 
		return 0;
	if(*bufp == 0x7f) 
		return 0;
	if((*bufp >= 0x80) && (*bufp <= 0xfe)) 
		return 1; 
	if(GBK_flag && *bufp>=0x40 && *bufp<0X7f) return 1;
	return 0;
}

/************************************************************************/
/*function:stptok 分解字符串为一组标记串	                        */
/*description:根据分隔符'tok'，从'src'分解到'trg'中，如果'tok'不设置，则*/
/*根据'src'与len的最小值，将'src'拷贝到'trg'中。可以解决汉字中出现的与  */
/*'tok'相同的分隔符被分解。						*/
/************************************************************************/
char *stptok(const char *src,char *trg,int len,const char *tok)
{
	register unsigned char *p;
	if(trg) *trg = 0;
	p = (unsigned char *)src;
	if(!p || !(*p)) return (char *)src ;
	if(tok && *tok) {
		while(*p) {
			if(strchr(tok, *p)) {
				if((p > (unsigned char *)src) && GBK_flag && firstcc((unsigned char *)src, p-1)) {
					p++;
					continue;
				}
				break;
			}
			p++;
		}
	} else {
		size_t l=strlen(src);
		p=(unsigned char *)src+MIN(l,(size_t)len);
	}
	if(len==0) return (char *)p;
	if(!trg) return ((size_t)((const char *)p-src))<len?(char *)p:(char *)(src+len);

	while(*src && --len) {
		if((unsigned char *)src == p) 
			break;
		*trg++ = *src++;
	}
	*trg = 0;

	return (char *)src;
}

/************************************************************************
 *function:strsubst 替换字符串函数	        
 *description:用'str'替换'from'的前cnt个字符	
 * 返回指向替换后边的字节。 
 ************************************************************************/
char *strsubst(char *from,int cnt,char *str)
{
	int i;
	register char *cp, *cp1, *cp2;

 	if(!from) 
		return 0;
	i = strlen(from);
	if(cnt < 0) 
		cnt = 0;
	else if(cnt > i) 
		cnt = i;
	else ;
	i = str ? strlen(str) : 0;
	if(i < cnt)				/* delete some char*/
	{  
		cp1 = from + i;
		cp = from + cnt;
		while(*cp) 
			*cp1++ = *cp++;
		*cp1 = 0;
	}
	else if (i > cnt)			/* extend some*/
	{ 
		cp2 = from + cnt;
		cp = from + strlen(from);
		cp1 = cp + i - cnt;
		while(cp >= cp2) 
			*cp1-- = *cp--;
	}
	else ;
	if(str) 
		strncpy(from, str, i);

	return (from + i);
}
/************************************************************************
 *function:strins 插入字符函数                                          
 *description:在'str'前插入字符'ch'	
 * 返回指向替换后边的字节。
 ************************************************************************/
char *strins(char *str,char ch)
{
	char p[2];

	p[0] = ch;
	p[1] = 0;

	return strsubst(str, 0, p);
}


/************************************************************************/
/*function:strupper 将字符串中的小写字母转换成大写                      */
/************************************************************************/
char *strupper(char *str)
{
register	char *p;

	if(!str) 
		return str;
	for(p = str; *p; p++) {
		if((*p & 128) && GBK_flag) {
			p++;
			continue;
		}
		*p=toupper(*p);
//		if(islower(*p)) *p -= 'a'-'A';
	}

	return str;
}

/************************************************************************/
/*function:strlower 将字符串中的大写字母转换成小写                      */
/************************************************************************/
char *strlower(char *str)
{
	char *p;

	if(!str) 
		return str;
	for(p = str; *p; p++) {
		if((*p & 128) && GBK_flag) {
			p++;
			continue;
		}
		*p=tolower(*p);
//		if(isupper(*p)) *p += 'a'-'A';
	}

	return str;
}
