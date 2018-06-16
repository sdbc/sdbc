#include <strproc.h>
#include <bignum.h>

int isprime4(u_int p[4])
{
u_int t[4],a[4],x[4],one[4];
int *ip,n[]={2,3,5,7,11,13,17,0};

	n_zero(4,a);
	n_one(4,one);
	numcpy(4,t,p);
	decm(4,t); //t=p-1;
        for(ip=n;*ip;ip++) {
		a[0]=*ip;
                _e_m_(4,a,t,p,x);
                if(numcmp(4,x,one)) return 0;
        }
        return 1;
}

main()
{
u_int m[4],x[4],y[4],a[4],ax[4],ay[4],axy[4],ayx[4];
int i;
char buf[1024];
//求128bit最大素数 
	for( m[0]=m[1]=m[2]=m[3]=0XFFFFFFFF;
		m[0]>5;m[0]-=2) {
		if(isprime4(m)) break;
	}
	printf("m=");
	for(i=0;i<4;i++)
		printf("%08X ",m[3-i]);
m[0]=0xFFFFFF61;
	n_zero(4,x);
	n_zero(4,y);
	n_zero(4,a);
	x[0]=0XF7AAB423;
	x[1]=0X00000128;
	x[2]=0X90FDA901;
	x[3]=0XF7AAB423;
b_to_d(4,x,buf);
printf("x=%s\n",buf);
	y[0]=5678;
//a=00000128,F7B91FD9,ACD75828,9B957CF2

	a[0]=0X9B957CF2;
	a[1]=0XACD75828;
	a[2]=0XF7B91FD9;
	a[3]=0X00000128;
	_e_m_(4,a,x,m,ax);
b_to_d(4,ax,buf);
printf("ax=%s\n",buf);
	_e_m_(4,a,y,m,ay);
	_e_m_(4,ay,x,m,ayx);
	_e_m_(4,ax,y,m,axy);
	printf("\nax=");
	for(i=0;i<4;i++)
		printf("%08X ",ax[3-i]);
	printf("\nay=");
	for(i=0;i<4;i++)
		printf("%08X ",ay[3-i]);
	printf("\naxy=");
	for(i=0;i<4;i++)
		printf("%08X ",axy[3-i]);
	printf("\nayx=");
	for(i=0;i<4;i++)
		printf("%08X ",ayx[3-i]);
	printf("\n");
	
	return 0;
}
