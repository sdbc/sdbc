/************************************************
 * SDBC 的核心传输程序。通信包分成包头和包体,
 * 包头是9个整数，被变换为48字节的定长头。
 * 包体是任意长度的字节流，只受资源限制。
 * 传输所需的内存是内部分配和管理的。
 * 包头和传输控制块的内容是向用户开放的。
 ************************************************/

//#include <malloc.h>
#include <unistd.h>
#include <ctype.h>
#include <sc.h>
#include <scry.h>
#include <crc32.h>
#include <bignum.h>
#include <quicklz.h>

static void showpack(int level,char *str,int i);

static int pack_encode(char *string,int length,T_Connect *conn)
{
	switch(conn->CryptFlg & DO_CRYPT) {
	case 1:
		enigma(conn->t.t,string,length);
		break;
	case 2:
		enigma_encrypt(conn->t.r,string,length);
		break;
	case 3:
		enigma2_encrypt(&conn->t,string,length);
		break;
	default:
		break;
	}
	return 0;
}
static int pack_decode(char *string,int length,T_Connect *conn)
{
	switch(conn->CryptFlg & DO_CRYPT) {
	case 1:
		enigma(conn->t.t,string,length);
		break;
	case 2:
		enigma_decrypt(conn->t.r,string,length);
		break;
	case 3:
		enigma2_decrypt(&conn->t,string,length);
		break;
	default:
		break;
	}
	return 0;
}

static int chknum(u_int *para,u_int init)
{
int i,old,sum,*ip;
        ip=(int *)para;
        old=*ip;
        *ip &= 0xFFFF;
	sum=(int)init;
        for(i=0;i<PARANUM;i++) {
		sum += *ip++;
        }
	*para |= sum << 16;
	return old;
}

static int NetHeadPack(char *buf,T_NetHead *nethead,u_int init)
{

	if(!buf) return POINTERR;
	//nethead->PROTO_NUM&=0X0000FFFF;
	chknum(nethead->para,init);
	str_n64a(PARANUM,nethead->para,buf);
	return 0;
}
static int NetHeadDispack(T_NetHead *nethead,char *buf,u_int init)
{
char *p;
int old;
	if(!buf)return POINTERR;
	if(isspace(*buf)) {
		return FORMATERR;
	}
	p=str_a64n(PARANUM,buf,nethead->para);
	old=p-buf;
	if(old<(HEADPACKLENGTH) && old<strlen(buf)) {
		return old;
	}
	old=chknum(nethead->para,init);
	if(old != nethead->para[0]) {
		ShowLog(1,"PKGERR:old=%08X,para=%08X",old,nethead->para[0]);
		return FORMATERR;
	}
	nethead->para[0] &= 0xFFFF;
	return 0;
}

int quick_send_pkg(T_Connect *connect,T_NetHead *nethead)
{
int i,ch;
char *p=NULL;

	if(nethead->PKG_LEN){
		p=connect->SendBuffer+HEADPACKLENGTH;
	   	pack_encode(p,nethead->T_LEN,connect);
		nethead->PKG_CRC=ssh_crc32((const unsigned char *)p, nethead->T_LEN);
		ch=*p;
	}
	i=NetHeadPack(connect->SendBuffer,nethead,connect->family[29]);
	if(p) *p=ch;
	i=SendNet(connect->Socket,connect->SendBuffer,
			HEADPACKLENGTH+nethead->T_LEN,connect->MTU);
//ShowLog(5,"SendPack:SendbBuffer=%s,i=%d,CRC=%08X",
//	connect->SendBuffer,i,nethead->PKG_CRC);
	if(connect->SendLen>32768) {
		free(connect->SendBuffer);
		connect->SendBuffer=0;
		connect->SendLen=0;
	}
	if(i<HEADPACKLENGTH+nethead->T_LEN) return i<0?SYSERR:LENGERR;
	else return 0;
}

int SendPack(T_Connect *connect,T_NetHead *nethead)
{
int i,len;
char *p=NULL;

//释放接收buf
	if(connect->RecvLen > 32768) {
		if(connect->RecvBuffer) free(connect->RecvBuffer);
		connect->RecvBuffer=0;
		connect->RecvLen=0;
	}

	len=(connect->CryptFlg&UNDO_ZIP)?nethead->T_LEN:nethead->PKG_LEN;
	i=connect->SendLen-len;
        if(i<(100+HEADPACKLENGTH)||i>SDBC_BLKSZ) {
                connect->SendLen=len+400+HEADPACKLENGTH;
	        if(!connect->SendBuffer)
			connect->SendBuffer=malloc((size_t)connect->SendLen);
                else connect->SendBuffer=realloc((char *)connect->SendBuffer,
					(size_t)connect->SendLen);
        }
	if(!connect->SendBuffer) {
		connect->SendLen=0;
		return MEMERR;
	}
	if(nethead->PKG_LEN){
		p=connect->SendBuffer+HEADPACKLENGTH;
		if(connect->CryptFlg & DO_ZIP && !(connect->CryptFlg & UNDO_ZIP)) {
			nethead->T_LEN=qlz_compress(nethead->data,p,nethead->PKG_LEN);
/*
			   if(nethead->PKG_LEN!=nethead->T_LEN)
				ShowLog(6,"SendPack PKG_LEN=%d,T_LEN=%d",
					nethead->PKG_LEN,nethead->T_LEN);
*/
		} else {
			memcpy((void *)p,nethead->data,nethead->PKG_LEN);
			nethead->T_LEN=nethead->PKG_LEN;
		}
	} else nethead->T_LEN=0;
	if(connect->Socket<0) return INTNULL;
	return quick_send_pkg(connect,nethead);
}

int RecvPack(T_Connect *connect,T_NetHead *nethead)
{
char headbuf[HEADPACKLENGTH+1],addr[16];
int i,n;
u_int crc;
char *zbuf;

	n_zero(PARANUM,nethead->para);
	nethead->data=0;
	memset(headbuf,0,sizeof(headbuf));
	i=RecvNet(connect->Socket,headbuf,HEADPACKLENGTH,connect->timeout);
//free SendBuffer
	if(i<HEADPACKLENGTH){
		if(i==TIMEOUTERR) {
			ShowLog(1,"%s:head TIMEOUT %d second's",__FUNCTION__,
				connect->timeout);
			return i;
		}
		ShowLog(1,"RecvPack Head LENERR i=%d,err=%d,%s",i,errno,strerror(errno));
		return LENGERR;
	}
	headbuf[HEADPACKLENGTH]=0;
	i=NetHeadDispack(nethead,headbuf,connect->family[29]);
	if(i!=0) {
		peeraddr(connect->Socket,addr);
		ShowLog(1,"aft NetHeadDispack len=%d,PKGERR %s:%.48s",i, addr,headbuf);
		showpack(1,headbuf,HEADPACKLENGTH);
		return(FORMATERR);
	}
	if(!nethead->T_LEN) return 0;
	i=((connect->CryptFlg&UNDO_ZIP)?nethead->T_LEN:nethead->PKG_LEN)+1;
	n=connect->RecvLen-i;
	if(n<0 || n>SDBC_BLKSZ){
		if( connect->RecvBuffer)
			free(connect->RecvBuffer);
		connect->RecvBuffer=0;
		connect->RecvLen=0;
		connect->RecvBuffer=malloc(i);
		if(!connect->RecvBuffer) return MEMERR;
		connect->RecvLen=i;
	}
	if(!(connect->CryptFlg&UNDO_ZIP) && nethead->T_LEN != nethead->PKG_LEN) {
		zbuf=malloc(nethead->T_LEN);
		if(!zbuf) {
			RecvNet(connect->Socket,connect->RecvBuffer,nethead->T_LEN,3);
			return MEMERR;
		}
	} else zbuf=connect->RecvBuffer;
	i=RecvNet(connect->Socket,zbuf,nethead->T_LEN,3);
	if(i < (nethead->T_LEN)) {
		if(TIMEOUTERR == i) {
			ShowLog(1,"%s:recv body TIMEOUT",__FUNCTION__);
			return i;
		}
		if(zbuf!=connect->RecvBuffer) free(zbuf);
		ShowLog(1,"%s,Recv Body T_LEN=%d i=%d,errno=%d",__FUNCTION__,
					nethead->T_LEN,i,errno);
		free(connect->RecvBuffer);
		connect->RecvBuffer=0;
		connect->RecvLen=0;
		return i<0?SYSERR:LENGERR;
	}
	crc=ssh_crc32((const unsigned char *)zbuf, nethead->T_LEN);
	if((connect->CryptFlg & CHECK_CRC) && (crc != nethead->PKG_CRC)) {
		ShowLog(1,"RecvPack:PKG_CRC=%08X,crc=%08X,PKG_LEN=%d,T_LEN=%d,head=%s",
			nethead->PKG_CRC,crc,nethead->PKG_LEN,nethead->T_LEN,headbuf);
			return CRCERR;
	}
        pack_decode(zbuf, nethead->T_LEN,connect);
	if(zbuf != connect->RecvBuffer) {
		if(nethead->T_LEN<9 || nethead->PKG_LEN != qlz_size_decompressed(zbuf)) {
			ShowLog(1,"unzip error T_LEN=%d,ADDR=%s",nethead->T_LEN, addr);
			return FORMATERR;
		}
		i=qlz_decompress(zbuf,connect->RecvBuffer);
		free(zbuf);
		if(i!=nethead->PKG_LEN) {
		    ShowLog(1,"RecvPack:PKG_LEN=%d,T_LEN=%d,unzip_size=%d",
			nethead->PKG_LEN,nethead->T_LEN,i);
			return LENGERR;
		}
	} 
	connect->RecvBuffer[nethead->PKG_LEN]=0;
	nethead->data=connect->RecvBuffer;
	return 0;
}

void initconnect(T_Connect *connect)
{
    connect->SendBuffer=0;
    connect->RecvBuffer=0;
    connect->SendLen=0;
    connect->RecvLen=0;
    connect->Var=0;
    connect->freevar=0;
    connect->Socket=-1;
    connect->CryptFlg=0;
    connect->only_do=0;
    connect->timeout=0;
    connect->Event_proc=0;
    connect->MTU=0;
    connect->status=0;
}
extern void FreeVar(void *);
void freeconnect(T_Connect *conn)
{
	
    if(conn->Var) {
    	if(conn->freevar) {conn->freevar(conn->Var);}
	else FreeVar(conn->Var);
	conn->Var=NULL;
    }
    if(conn->Socket > -1) close(conn->Socket);
    conn->Socket=-1;
    memset(&conn->t,0,sizeof(conn->t));
    if(conn->SendBuffer) {
	free(conn->SendBuffer);
    }
    conn->SendBuffer=0;
    conn->SendLen=0;
    if(conn->RecvBuffer) {
	free(conn->RecvBuffer);
    }
    conn->RecvBuffer=0;
    conn->RecvLen=0;
    conn->Var=NULL;
    conn->status=0;
}
static void showpack(int lever,char *str,int i)
{
int j,k;
char errbuf[512];
	while(i>64) {
		i-=64;
		k=0;
		for(j=0;j<64;j++) k+=sprintf(errbuf+k,"%02X ",*str++ & 255);
		ShowLog(lever,"%s",errbuf);
	}
	k=0;
	for(j=0;j<i;j++) k+=sprintf(errbuf+k,"%02X ",*str++ & 255);
	ShowLog(lever,"%s",errbuf);
}
