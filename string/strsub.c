#include <stdlib.h>

/************************************************************************/
/*function:strsub 截取字符串函数                                        */
/*description:'src'中截取从start开始的cnt个字符到'dest'中               */
/************************************************************************/
void strsub(char *dest, const char *src,int start,int cnt)
{
    strncpy(dest, src + start, cnt);
    dest[cnt] = '\0';
}

