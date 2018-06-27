#include <string.h>
#include <bignum.h>
extern char *strrevers(char *);


#ifdef DEBUG
#include <stdio.h>
main()
{
char buf[256],input[512];
u_int tmpnum[MAXNUMBER+1];
FILE *fd;
int n;
/*
fd=fopen("key.txt","r");
if(!fd) {
	perror("key.txt");
	exit(1);
}
n=0;
fscanf(fd,"----RSA%d",&n);
n/=32;
if(!n) {
	fprintf(stderr,"n is 0!\n");
	fclose(fd);
	exit(2);
}
printf("----RSA%d----\n",n*32);
fgets(input,sizeof(input),fd);
fgets(input,sizeof(input),fd);
loadnum(n,input,tmpnum);
str_n64a(n,tmpnum,buf);
puts(buf);
fgets(input,sizeof(input),fd);
loadnum(n,input,tmpnum);
str_n64a(n,tmpnum,buf);
puts(buf);
*/
fd=fopen("key64.txt","r");
if(!fd) {
	perror("key64.txt");
	exit(1);
}
n=0;
fscanf(fd,"----RSA%d",&n);
n/=32;
if(!n) {
	fprintf(stderr,"n is 0!\n");
	fclose(fd);
	exit(2);
}
printf("----RSA%d----\n",n*32);
fgets(buf,sizeof(buf),fd);
fgets(buf,sizeof(buf),fd);
str_a64n(n,buf,tmpnum);
strhex(n,tmpnum,input);
puts(input);
fgets(buf,sizeof(buf),fd);
str_a64n(n,buf,tmpnum);
strhex(n,tmpnum,input);
puts(input);
memset(input,0,n*BITS/8);
fprintf(stderr,"test byte_to_num Input:\n");
gets(input);
byte2n(n,tmpnum,input);
n2byte(n,tmpnum,buf);
fprintf(stderr,"input=%s\n",input);
fprintf(stderr,"buf=%s\n",buf);
}
#endif



static const char tab64[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_~@";

char n64a(int n)
{
	return(tab64[n&63]);
}

int a64n(char a)
{
char *p;
	p=strchr(tab64,a);
	if(!p) return -1;
	return(p-tab64);
}

char * str_n64a(int n,u_int *num,char *str)
{
u_int tmp,numtmp[MAXNUMBER+1];
char *p;
	if(!num||!str) return 0;
	p=str;
	numcpy((u_int)n,numtmp,num);
	while(!n_iszero((u_int)n,numtmp)) {
		tmp=rshift((u_int)n,numtmp,6);
		*p++=n64a(tmp);
	}
	*p=0;
	strrevers(str);
	return str;
}

char * str_a64n(int n,char *str,u_int *num)
{
char *p;
register int tmp,over;
	if(!num||!str||n<=0) return str;
	n_zero((u_int)n,num);
	for(p=str;*p;p++) {
		tmp=a64n(*p);
		if(tmp<0) break;
		over=lshift((u_int)n,num,6);
		if(over) {
			rshift((u_int)n,num,6);
			over<<=BITS-6;
			num[n-1] |= over;
			break;
		}
		num[0] |= tmp;
	}
	return p;
}
void loadnum(int len,char *str,u_int *num)
{
int i;
char *p,*p1;
	p=str;
	for(i=0;i<len;i++) {
		p1=strchr(p,',');
		sscanf(p,"%X",num+len-1-i);
		if(!*p1) break;
		p=p1+1;
	}
}
char *strhex(int n,u_int *num,char *str)
{
char *p;
int i;
	if(!str) return str;
	p=str;
	for(i=0;i<n;i++) {
		p+=sprintf(p,"%08X,",num[n-i-1]);
	}
	return str;
}

void byte2n(int n,u_int num[],char *bytestr)
{
char *p;
int i,j;
u_int *up;
	if(!bytestr || !num) return;
	p=bytestr;
	for(i=0;i<n;i++) {
		up=&num[n-1-i];
		*up=0;
		for(j=0;j<BITS/8;j++) {
			*up <<= 8;
			*up += *p++ & 255;
		}
	}	
}

void n2byte(int n,u_int *num,char *bytestr)
{
char *p;
int i,j;
u_int up;
	if(!bytestr || !num) return;
	p=bytestr;
	for(i=0;i<n;i++) {
		up=num[n-1-i];
		j=BITS/8;
		while(j--) 
			*p++ = (up >> (j*8)) & 255;
	}
}

u_int * d_to_b(n,cp,dp)
u_int n,*dp;
char *cp;
{
char *p;
u_int num[MAXNUMBER+1];
u_int tmp[MAXNUMBER+1];
	n_zero(n,dp);
	n_zero(n,num);
	for(p=cp;*p;p++) {
		mul1(n,dp,10,tmp);
		numcpy(n,dp,tmp);
		num[0]=(*p - '0') % 10;
		addm(n,dp,num);
	}
	return dp;
}

char *b_to_d(n,pb,d)
u_int n,*pb;
char *d;
{
u_int i,*ip0,*ip,*ip1,rem[MAXNUMBER+1],num;
u_int zero[MAXNUMBER+1];
u_int tmp[MAXNUMBER+1];
char *cp;
	cp=d;
	i=0;
	n_zero(n,zero);
	numcpy(n,tmp,pb);
	ip0=tmp,ip1=rem;
	while(numcmp(n,ip0,zero)) {
		i++;
		num=div1(n,ip0,10,ip1);
		ip=ip0;
		ip0=ip1;
		ip1=ip;
		*cp++=num+'0';
	}
	if(!i) *cp++='0';
	*cp=0;
	return strrevers(d);
}

/*
char * revers(char *str)
{
char c,*p,*p1;
	p=str;
	p1=str+strlen(str)-1;
	while(p1>p) {
		c=*p;
		*p=*p1;
		*p1=c;
		p++;
		p1--;
	}
	return str;
}
*/

char * byte_a64(char* str,char *byte,int len)
{
u_int l;
int i;
char *p,*bp;
	p=str;
	bp=byte;
	for(i=len;i>2;i-=3) {
		/* load 3 byte into l from head of byte */
		l=*bp++ & 255;
		l<<=8;
		l |= *bp++ & 255;
		l<<=8;
		l |= *bp++ & 255;
		/* write a64 to str 4 char */
		*p++ = n64a((l>>18)&63);
		*p++ = n64a((l>>12)&63);
		*p++ = n64a((l>>6)&63);
		*p++ = n64a(l&63);
	}
	switch(i) {
	case 2:
		l= *bp++ &255;
		l <<= 8;
		l |= *bp++ &255;
		l <<=2;
		*p++ = n64a((l>>12)&63);
		*p++ = n64a((l>>6)&63);
		*p++ = n64a(l&63);
		*p++ = tab64[64];
		break;
	case 1:
		l= *bp++ &255;
		l <<= 4;
		*p++ = n64a((l>>6)&63);
		*p++ = n64a(l&63);
		*p++ = tab64[64];
		*p++ = tab64[64];
	default: break;
	}
	*p=0;
	return str;
}
	
int a64_byte(char *byte,char *str)
{
char *bp,*p;
int i,len,size;
u_int l=0;
	size=0;
	bp=byte,p=str;
	len=strlen(str);
	for(i=0;i<len;i+=4) {
		l = a64n(*p++);
		l <<= 6;
		l |= a64n(*p++);
		if(*p==tab64[64]) break;
		l <<=6;
		l |= a64n(*p++);
		if(*p==tab64[64]) break;
		l <<=6;
		l |= a64n(*p++);
		*bp++ = l >> 16;
		*bp++ = ((l>>8) & 255);
		*bp++ = l & 255;
		size += 3;
	}
	i=strlen(p);
	switch(i) {
	case 1:
		l >>= 2;
		*bp++ = (l >> 8) & 255;
		*bp++ = l & 255;
		size += 2;
		break;
	case 2:
		l >>= 4;
		*bp++ = l & 255;
		size++;
	default:break;
	}
	return size;
}

