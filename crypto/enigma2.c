#include <string.h>
#include <crc32.h>
#include <enigma.h>

#ifndef MIN
#define MIN(a,b) ((a)>(b)?(b):(a))
#endif

static void b_revers(char *buf,int len)
{
char *p,*p1,c;
	if(!buf || len<2) return;
	p=buf;
	p1=p+len-1;
	while(p1>p) {
		c=*p1; *p1-- =*p; *p++=c;	
	}
	return;
}

void enigma_rev(ENIGMA ep,char *buf,int len)
{
int i;
char *p;
	if(!buf||len<=0) return;
	enigma_encrypt(ep,buf,len);
	b_revers(buf,len);
	len--;
	p=buf;
	for(i=0;i<len;i++) {
		p[1]^=*p; p++;
	}
}

void rev_enigma(ENIGMA ep,char *buf,int len)
{
int i;
char *p;
	if(!buf||len<=0) return;
	b_revers(buf,len);
	p=buf;
	for(i=0;i<len-1;i++) {
		*p^=p[1]; p++;
	}
	enigma_decrypt(ep,buf,len);
}

void enigma2_init(ENIGMA2 *ep,const char *bin_key,int len)
{
char rk[ROTORSZ];

	if(!ep||!bin_key) return;
	if(len<=0) len=strlen(bin_key);
	enigma_init(ep->t,bin_key,len);
	if(len>ROTORSZ) len=ROTORSZ;
	memcpy(rk,bin_key,len);
	b_revers(rk,len);
	enigma_encrypt(ep->t,rk,len);
	enigma_init(ep->r,rk,len);
	ep->crc=0X7FFFFFFF & ssh_crc32((unsigned char *)ep->t,sizeof(ep->t)<<1);
//ShowLog(5,"%s:crc=%d,len=%d",__FUNCTION__,ep->crc,len);
	return;
}

static void str_rotor(char *str,int len,int rotor)
{
	if(!rotor) return;
int l,i;
char *p,*p1;
	l=MIN(rotor,len-rotor);
       	p=str+rotor;
	p1=str;
	for(i=0;i<l;i++) {
		*p ^= *p1; *p1 ^= *p; *p++ ^= *p1++;
	}
}

void enigma2_encrypt(ENIGMA2 *ep,char *buf,int len)
{
	if(!ep||!len) return;
	enigma_rev(ep->t,buf,len);
	str_rotor(buf,len,ep->crc%len);
	enigma(ep->r,buf,len);
}

void enigma2_decrypt(ENIGMA2 *ep,char *buf,int len)
{
	if(!ep||!len) return;
	enigma(ep->r,buf,len);
	str_rotor(buf,len,ep->crc%len);
	rev_enigma(ep->t,buf,len);
}
