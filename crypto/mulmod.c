#include "bignum.h"
u_int *_m_m_(int n,u_int *a,u_int *b,u_int *m,u_int *ret)
{
u_int tmp[MAXNUMBER*2+1];
	mulm(n,a,b,tmp);
	divm(n,tmp,m,(u_int *)0,ret);
	return ret;
}
