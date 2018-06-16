#include <stdio.h>
#include <sc.h>
#include <datejul.h>
#include <bignum.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <crc32.h>
#include <crc.h>
#include <enigma.h>
#include <ctype.h>

extern char * prikey128(char *keybuf,u_int ind[4],u_int *family);

static char * clikey(char *keybuf,u_int *family)
{
ENIGMA2 t;
INT64 i64;
u_int key[8]; //256bit key

	i64=now_usec();
	key[0]^=(u_int)i64;
	key[1]^=(u_int)(i64 >> 32);
	key[2]^=getpid();
	key[3]=family[key[1]&0x1F];
	key[4]^=*(int *)keybuf;
	key[5]^=*(int *)(keybuf+4);
	key[6]^=*(int *)(keybuf+8);
	key[7]=ssh_crc32((unsigned char *)key,sizeof(key));
	enigma2_init(&t,(const char *)key,sizeof(key));
	enigma2_encrypt(&t,(char *)key,sizeof(key));
	memcpy(keybuf,key,sizeof(key));
	return keybuf;
}
//服务器用，与客户端协商密钥。 
int mk_clikey(int socket,ENIGMA2 *tc,u_int *family)
{
ENIGMA2 t;
char buf[100],keybuf[56],cli_k[52],*cp;
int i,len;
int crymode=15;
unsigned short crc,recv_crc;
char addr[16];

	cp=getenv("CRYPTFLG");
	if(cp && isdigit(*cp)) crymode=atoi(cp)&0xf;
	i=0;
	*buf=0;
	peeraddr(socket,addr);
	len=RecvNet(socket,buf,26,15);
	i=-1;
	if(len!=26 || !(i=strncmp(buf,"hello world",11))) {
		if(len==SYSERR) return SYSERR;
		ShowLog(1,"%s:read %s:%s len %d error!",__FUNCTION__,addr,buf,len);
		if(i==0) {
			sprintf(buf,"%-67s\n","Goodbye!\n");
			SendNet(socket,buf,68,0);
		}
		return FORMATERR;
	}
	crc=gencrc((unsigned char *)buf,24);
	recv_crc=ntohs(*(short *)(buf+24));
	if(crc != recv_crc) {
		ShowLog(1,"%s %s:PKGERR crc错!",__FUNCTION__,addr);
		return FORMATERR;
	}
	buf[24]=0;
	len=a64_byte(cli_k,buf);
	if(len!=18) {
//		sprintf(buf,"%-67s\n","Goodbye!");
//		SendNet(socket,buf,68,0);
		ShowLog(1,"%s %s:a64 len %d error!",__FUNCTION__,addr,len);
		return LENGERR;
	}

	if(crymode & DO_CRYPT) {
	INT64 tim;
	u_int ax[4],ay[4],y[4],m[4];
	int kw;
		tim=now_usec();
		y[0]^=tim;
		y[1]^=(tim>>32);
		y[2]^=getpid();
		y[3]=0x7FFFFFFF&family[31&y[0]];
		m[1]=m[2]=m[3]=0xFFFFFFFF;
		m[0]=0xFFFFFF61;
		byte2n(4,ax,cli_k);
		_e_m_(4,ax,y,m,ay);
/*
b_to_d(4,ay,buf);
ShowLog(5,"%s:axy=%s",__FUNCTION__,buf);
ShowLog(5,"%s:hex axy=%s",__FUNCTION__,strhex(4,ay,buf));
*/

		prikey128(keybuf,ay,family);
		enigma2_init(&t,keybuf,0);
//找a
		kw=family[31&cli_k[17]];
       		i=family[31&cli_k[16]];
       		if(!i) i=65537;
		n2byte(4,ax,cli_k+16);
		n_zero(4,ax);
		ax[0]=i;
		ax[1]=kw; //ax=a
		_e_m_(4,ax,y,m,ay);
		n2byte(4,ay,cli_k);
/*
b_to_d(4,ay,buf);
ShowLog(5,"%s:ay=%s",__FUNCTION__,buf);
*/
		clikey(cli_k+16,family);
/*
cp=buf;
for(i=0;i<32;i++) {
	cp+=sprintf(cp,"%02X ",255& *(cli_k+16+i));	
}
ShowLog(5,"%s:clikey=%s",__FUNCTION__,buf);
*/

		enigma2_init(tc,cli_k+16,32);
		enigma2_encrypt(&t,cli_k+16,32);
	} else {
		memcpy(keybuf,cli_k,6);
		cli_k[48]=0;
	}
	cli_k[49]=crymode;
	byte_a64(keybuf,cli_k,50);
	SendNet(socket,keybuf,68,0);
	
	return crymode;
}

