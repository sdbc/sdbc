
#include <unistd.h>
#include <arpa/inet.h>
#include <sccli.h>
#include <bignum.h>
#include <crc.h>

extern char * prikey128(char *keybuf,u_int ind[4],u_int *family);
//客户端用，与服务器协商并取得密钥 
int get_clikey(T_Connect *conn)
{
int i,kw,a2,b1;
u_int x[4],ax[4],ay[4],m[4];
int crymode=0;
INT64 tim;
char keybuf[52],cmd[256];
ENIGMA2 t;
unsigned short crc;

	tim=now_usec();
	x[0]=(0XFFFFFFF&((int)tim ));
	x[1]=0XFFFFFFFF & (tim>>32);
	x[2]=getpid();
	x[3]=0X7FFFFFFF&conn->family[31&x[0]];
	a2=(x[3]^(tim>>31))&0X1F;
	b1=(x[3]^(tim>>36))&0X1F;
	kw=conn->family[b1];
	n_zero(4,ax);
	i=conn->family[a2];
	if(!i) i=65537;
	ax[0]=i;
	ax[1]=kw;
	m[1]=m[2]=m[3]=0XFFFFFFFF;
	m[0]=0xFFFFFF61;
	_e_m_(4,ax,x,m,ay);
	n2byte(4,ay,keybuf);
	keybuf[16]=a2&255;
	keybuf[17]=b1&255;
	
	byte_a64(cmd,keybuf,18);
	crc=gencrc((unsigned char *)cmd,24);
	*(short *)(cmd+24)=htons(crc);
	i=SendNet(conn->Socket,cmd,26,0);
	if(i!=26) {
		ShowLog(1,"%s:SenNet(%d) i=%d,err=%d,%s",__FUNCTION__,conn->Socket,i,errno,strerror(errno));
	}
	i=RecvNet(conn->Socket,cmd,68,120);
	if(i!=68) {
		ShowLog(1,"%s:read(%d) len=%d,Error=%d,%s!",__FUNCTION__,
			conn->Socket,i,errno,strerror(errno));
		return SYSERR;
	}
	cmd[68]=0;
	i=a64_byte(keybuf,cmd);
	if(i!=50) {
		ShowLog(1,"%s:%s len=%d can not get ind!\n",
			__FUNCTION__,cmd,i);
		return LENGERR;
	}

	crymode=keybuf[49];
	if(crymode & DO_CRYPT) {
		byte2n(4,ay,keybuf);
		_e_m_(4,ay,x,m,ax);
		prikey128(cmd,ax,conn->family);
		enigma2_init(&t,cmd,0);
		enigma2_decrypt(&t,keybuf+16,32);
/*
char errbuf[72];
strhex(8,(u_int *)(keybuf+16),errbuf);
ShowLog(5,"%s:key=%s,len=%d",__FUNCTION__,errbuf,strlen(keybuf+16));
*/
		enigma2_init(&conn->t,keybuf+16,32);
	} 
	conn->CryptFlg=crymode;
	return crymode;
}

