#include <stdio.h>
#include "bignum.h"
/* ret = a ** z % n */
u_int *_e_m_(int n,u_int *a,u_int *z,u_int *m,u_int *ret)
{
u_int a1[MAXNUMBER+1];
u_int z1[MAXNUMBER+1];
u_int x[MAXNUMBER+1];
	numcpy(n,a1,a);
	numcpy(n,z1,z);
	n_one(n,x);
	while(!n_iszero(n,z1)) {
		while(!(z1[0]&1)) {
			rshift(n,z1,1);
			/* a1 = (a1 *a1) % m; */
			_m_m_(n,a1,a1,m,a1);
		}
		decm(n,z1);
		/* x = (x*a1) % m; */
		_m_m_(n,x,a1,m,x);
	}
	numcpy(n,ret,x);
	return ret;
}
