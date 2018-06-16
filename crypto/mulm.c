#include "bignum.h"
u_int *F_mulm(u_int n,u_int *pa,u_int *pb,u_int *pm);
extern void mul64(unsigned long long a,unsigned long long b,unsigned int *pm);
static int mysub(int n,u_int *a,u_int *b,u_int *ret);

u_int *mulm(u_int n,u_int *pa,u_int *pb,u_int *pm)
{
int i,j;
u_int tmp[MAXNUMBER+10],atmp[MAXNUMBER+1],btmp[MAXNUMBER+1];
	if(n & 1) {
		if(n == 1) mula(*pa,*pb,pm);
		else {
			n_zero((n<<1),pm);
			for(i=0;i<n;i++) 
				addm(n+1,pm+i,mul1(n,pa,pb[i],tmp));
		}
	} else if(n==2) {
		unsigned long long *lpa,*lpb;
		lpa=(unsigned long long *)pa;
		lpb=(unsigned long long *)pb;
		mul64(*lpa,*lpb,pm);
	} else {
		n_zero((n<<1),pm);
		i=n>>1;
//		n_zero(n+1,tmp);
		tmp[n]=0;
		mulm(i,pa,pb,pm+i);    // A0 * B0
//		addm(n+1,pm,tmp);
		if(addm(n,pm,pm+i)) incm(i,pm+i+i);
//		n_zero(n+1,tmp);
		mulm(i,pa+i,pb+i,tmp);  // A1 * B1
		addm(n+1,pm+i,tmp);
		addm(n,pm+i+i,tmp);
		if(!(*atmp=numcmp(i,pa+i,pa))) return pm;
		if(!(*btmp=numcmp(i,pb,pb+i))) return pm;
		j=mysub(i,pa+i,pa,atmp);
		j^=mysub(i,pb,pb+i,btmp);
//		n_zero(n+1,tmp);
		mulm(i,atmp,btmp,tmp); // (A1-A0)*(B0-B1)
		if(j) subm(n+1,pm+i,tmp);
		else addm(n+1,pm+i,tmp);
	}
	return pm;
}

static int mysub(int n,u_int *a,u_int *b,u_int *ret)
{
	if((int)*ret > 0) {
		numcpy(n,ret,a);
		subm(n,ret,b);
		return 0;
	} else {
		numcpy(n,ret,b);
		subm(n,ret,a);
		return 1;
	}
}
