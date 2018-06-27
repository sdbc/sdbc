#include <bignum.h>

extern char *DES_fcrypt(char *key,char *salt,char *buf);
/*
#include <string.h>
static void showhex(int level,char *title,char *str,int i)
{
int j;
char errbuf[400];
char *p;
        strcpy(errbuf,title);
        strcat(errbuf," ");
        p=errbuf+strlen(errbuf);
        while(i>64) {
                i-=64;
                for(j=0;j<64;j++) p+=sprintf(p,"%02X ",*str++ & 255);
                ShowLog(level,"%s",errbuf);
                p=errbuf;
        }
        for(j=0;j<i;j++) p+=sprintf(p,"%02X ",*str++ & 255);
        ShowLog(level,"%s",errbuf);
}
*/

char * prikey128(char *keybuf,u_int ind[4],u_int *family)
{
int i,b,c;
char salt[2],key[9];
u_int d[3];

	salt[0]=rshift(4,ind,7);;
	if(salt[0]==0) salt[0]=0x41;
	salt[1]=rshift(4,ind,7);
	if(salt[1]==0) salt[1]=0x41;
	b=rshift(4,ind,5);
	c=rshift(4,ind,5);
	for(i=0;i<3;i++) {
		d[i]=family[b];
		if(++b == 32) b=0;
	}
	if(c) rshift(3,d,c);
	for(i=0;i<8;i++) {
		key[i] = rshift(3,d,6)+0x21;
	}
	key[i]=0;
	DES_fcrypt(key,salt,keybuf);
	for(i=13;i<26;i++) keybuf[i]=rshift(4,ind,8);
	keybuf[i]=0;
//	showhex(5,(char *)__FUNCTION__,keybuf,26);
	return keybuf;
}
