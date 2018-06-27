#include <stdio.h>

typedef struct {
	char a;
	char * b;
	char c;
} tc;
typedef struct {
	char a;
	short b;
	char c;
} ts;
typedef struct {
	char a;
	int b;
	char c;
} ti;
typedef struct {
	char a;
	float b;
	char c;
} tf;
typedef struct {
	char a;
	double b;
	char c;
} td;
typedef struct {
	char a;
	long b;
	char c;
} tl;

typedef struct {
	char a;
	long double b;
	char c;
} tld;

main()
{
printf("sizeof tc=%d,ts=%d,ti=%d,tf=%d,td=%d,tl=%d,tld=%d,ldouble=%d\n",
	sizeof(tc),
	sizeof(ts),
	sizeof(ti),
	sizeof(tf),
	sizeof(td),
	sizeof(tl),
	sizeof(tld),
	sizeof(long double));
 return 0;
}
