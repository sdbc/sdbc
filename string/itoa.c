#include <strproc.h>

char * itoStr(register int n, char *s)
{
register char *p;
int sign;

	p=s;
        if ((sign = n) < 0) n = -n;
        do {
                *p++ = n % 10 + '0';
        } while ((n /= 10) > 0);
        if (sign < 0) *p++ = '-';
        *p = '\0';
        strrevers(s);
	return p;
}

char * lltoStr(register INT64 n, char *s)
{
register char *p;
int sign;

	p=s;
	sign=(n<0);
        if (sign ) n = -n;
        do {
                *p++ = n % 10 + '0';
        } while ((n /= 10) > 0);
        if (sign) *p++ = '-';
        *p = '\0';
        strrevers(s);
	return p;
}

