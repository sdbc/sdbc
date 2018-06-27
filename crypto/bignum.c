#include <bignum.h>

//#define DEBUG
void mula(u_int a,u_int b,u_int ip[])
{
unsigned long long tmp;
	if(a<2) {
		if(a==1) ip[0]=b;
		else ip[0]=0;
		ip[1]=0;
		return;
	}
	tmp=(unsigned long long)a * (unsigned long long)b;
	ip[0] = (u_int)tmp;
	ip[1] = (u_int)(tmp >> BITS);
} 

u_int diva(u_int *pa,u_int b,u_int *pq)
{
unsigned long long tp,d,q;
	tp=((long long)pa[1])<<32;
	tp += 0XFFFFFFFF & pa[0];
	d=b;
	q=tp/d;
	*pq=q&0xFFFFFFFF;
	q=tp%d;
	return (u_int)q;
}
u_int adda(u_int *pd,u_int *ps,u_int cy)
{
int c=0;
	if(ps) {
		*pd += *ps;
		if(*pd < *ps) c=1;
	}
	*pd += cy;
	if(cy && !*pd) c=1;
	return c;
}
u_int suba(u_int *pd,u_int *ps,u_int cy)
{
u_int c=0;
	if(ps) {
		if(*pd < *ps) c=1;
		*pd -= *ps;
	}
	if(*pd < cy) c=1;
	*pd -= cy;
	return c;
}
u_int addm(u_int n,u_int *pd,u_int *ps)
{
int cy,i;
	cy=0;
	for(i=0;i<n;i++) {
		cy=adda(pd+i,ps+i,cy);
	}
	return cy;
}

u_int *mul1(n,pa,b,pm)
u_int n,b;
u_int *pa,*pm;
{
u_int itmp[2];
u_int i;
	n_zero(n+1,pm);
	if(b<=1) {
		if(b==1) numcpy(n,pm,pa);
		return pm;
	}
	for(i=0;i<n;i++) {
		mula(pa[i],b,itmp);
		addm(2,pm+i,itmp);
	}
	return pm;
}

u_int *mulm(n,pa,pb,pm)
u_int n;
u_int *pa,*pb,*pm;
{
u_int i;
u_int tmp[MAXNUMBER+1];
	n_zero(2*n,pm);
	for(i=0;i<n;i++) 
		addm(n+1,pm+i,mul1(n,pa,pb[i],tmp));
	return pm;
}

u_int subm(u_int n,u_int *pd,u_int *ps)
{
u_int cy;
u_int i;
	cy=0;
	for(i=0;i<n;i++) {
		cy=suba(pd+i,ps+i,cy);
	}
	return cy;
}


u_int *incm(u_int n,u_int *pd)
{
int cy;
u_int i;
	cy=1;
	for(i=0;i<n;i++) {
		cy=adda(pd+i,(u_int *)0,cy);
		if(!cy) break;
	}
	return pd;
}

u_int * decm(n,pd)
u_int n;
u_int *pd;
{
int cy;
u_int i;
	cy=1;
	for(i=0;i<n;i++) {
		cy=suba(pd+i,(u_int *)0,cy);
		if(!cy) break;
	}
	return pd;
}

int numcmp(n,pb,ps)
u_int n,*pb,*ps;
{
u_int k;
	do {
		n--;
		k=pb[n]-ps[n];
		if(!k) continue;
		else if(k>pb[n]) return -1;
		else return 1;
	} while (n);
	return 0;
}

u_int div1(u_int n,u_int *pa,u_int b,u_int *pq)
{
u_int rem[2];
	if(b<2) {
		if(b==1) numcpy(n,pq,pa);
		else n_zero(n,pq);
		return 0;
	}
	rem[0]=rem[1]=0;
	do {
		n--;
		rem[0]=pa[n];
		rem[1]=diva(rem,b,pq+n);
	} while(n);
	return rem[1];
}
u_int divm(u_int n,u_int *div,u_int *by,u_int *quot,u_int *rem)
{
u_int tmp[MAXNUMBER+1];
u_int tmp1[MAXNUMBER+1];
int i,j,k,cy;
u_int qtmp,testb;
u_int testd[2];
	i=n<<1;
	j=n;
	testb=by[n-1]; //除数最高位
	if(!testb) return -1;
	numcpy(n,tmp,div+n); //tmp=被除数的高半
	do {
		i--;
		j--;
		for(k=n;k>0;k--) tmp[k]=tmp[k-1];
		tmp[0]=*(div+i-n);
		numcpy(2,testd,tmp+n-1);//testd=中间结果的被除数最高2位
			
		if(testd[HIGH]==testb) { //qtmp=试商
			qtmp=-1;
		} else {
			diva(testd,testb,&qtmp);
		}

		if(qtmp) {
		 mul1(n,by,qtmp,tmp1);
		 if((k=numcmp(n+1,tmp,tmp1)) < 0 ) { // tmp < tmp1
		    while((cy=numcmp(n+1,tmp,tmp1)) < 0) {
			qtmp--;
			cy=subm(n,tmp1,by);
			if(cy) tmp1[n]--;
#ifdef DEBUG
printf("\n -- qtmp=%X\n",qtmp);
fflush(stdout);
#endif
		    }
		    cy=subm(n+1,tmp,tmp1);
		 } else {
		   subm(n+1,tmp,tmp1);
		   while(tmp[n] || (cy=numcmp(n,tmp,by))>=0) {
			cy=subm(n,tmp,by);
			if(cy) tmp[n]--;
			qtmp++;
#ifdef DEBUG
printf("\n ++ qtmp=%X\n",qtmp);
fflush(stdout);
#endif
		   }
		 }
		}
		if(quot) quot[j]=qtmp;
	} while (j);
	numcpy(n,rem,tmp);
	return 0;
}

u_int n_iszero(u_int n,u_int *pa)
{
u_int i;
	for(i=0;i<n;i++) if(pa[i]) break;
	if(i<n && pa[i]) return 0;
	else return 1;
}

u_int rshift(u_int n,u_int *up,int i)
{
int j;
u_int cy;
	cy=up[0]<<(BITS-i);
	cy>>=(BITS-i);
	for(j=0;j<n;j++) {
		if(j<n-1) up[j]=(up[j]>>i) | (up[j+1] << (BITS-i));
		else up[j]=(up[j]>>i) ;
	}
	return cy;
}
u_int lshift(u_int n,u_int *up,int i)
{
u_int j,k;
u_int cy;
	cy=up[n-1]>>(BITS-i);
	for(j=0;j<n;j++) {
		k=n-j-1;
		if(k>0) up[k]=(up[k]<<i) | (up[k-1] >> (BITS-i));
		else up[k] <<= i;
	}
	return cy;
}
u_int *n_zero(u_int n,u_int *pa)
{
u_int *p;
	p=pa;
	while(n){
		*p++ = 0;
		n--;
	}
	return pa;
}

void numcpy(u_int n,u_int *to,u_int *from)
{
	while(n--) *to++ = *from++;
}

u_int *n_ff(u_int n,u_int *pa)
{
u_int *p;
	p=pa;
	while(n--) *p++ = -1;
	return pa;
}

u_int *n_one(u_int n,u_int *pa)
{
u_int *p;
	p=pa;
	n--;
	*p++=1;
	while(n--) *p++ = 0;
	return pa;
}

u_int *n_not(u_int n,u_int *pa)
{
u_int *p;
	p=pa;
	while(n--) {
		*p = ~*p;
		p++ ;
	}
	return pa;
}
