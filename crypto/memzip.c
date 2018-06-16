/**************************************************************************
 * memzip.c 开发者 余立华 2007.6.13
 * ver 0.0.3.0 使用了HASH表，支持滑动窗口。取消了窗口大小的设置。
 * 为在线交易写的一个压缩程序。压缩是基于字符串的，采用deflate算法
 * 即使压缩不了，也决不能变长，避免通信过程产生存储溢出
 * 暂时没有采用 huffuman算法，考虑到每个通信包传送huffman树，压缩将无利可图
 * 算法来源于gzip，改进之处在于加强了小匹配的效率，避免了为区别匹配子与原数据
 * 的数据标尺，它使压缩后的数据变大为9/8，
 * 本法采用标记字节来标记匹配子。
 * 如果原文中含有标记，则转义。
 * 本程序采用变长匹配子。最小的匹配子2字节，最大的4字节。
 **************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sc.h>

/* used by comprtok.flgbyte */
#define ESC  0X10		//转义字符
#define MFLG 0X11		//标记字符
#define MBASE 0x12		//短匹配标记字符，MBASE－MBASE+5,分别表示3-8字节的匹配长度。

#define MAX_DISTANCE 16576

#define HASH_BITS 15
#define HASH_SIZE (unsigned)(1<<HASH_BITS)
#define HASH_MASK (HASH_SIZE-1)
#define NIL (unsigned short)0xFFFF
#define DNIL ((NIL << 16)+NIL)
#define HASH(s,c) ((((unsigned)(s)<<5) ^ (unsigned)(c)) & HASH_MASK)

/* 匹配子的描述结构 */
struct comprtok {
	char flgbyte;	//标记字符
	char len;	//匹配长度，短匹配不使用。长匹配表示9－264字节的匹配。
/* 变长的匹配距离。如果3字节匹配，distance[0]=0~255,表示1－256字节范围内的匹配，
多字节匹配时：distance[0]={0-191},表示1-192范围内的匹配，一个字节。如果D7D6置1，
需要两个字节，表示193-MAX_DISTANCE的匹配距离 */
	unsigned char distance[2];
	char *strend;	//匹配子后面的字节
	char *strbegin; //窗口开始的位置
	unsigned short head[HASH_SIZE]; //hash head
	unsigned short prev[32768];	//link tab
	unsigned short h;		//HASH key
};

/* deflate预压缩，入口参数：
tokp:匹配子描述符。
strbegin:串开始的位置。
strstart:当前位置。
len:strstart之后的长度。
返回值:<0表示没有压缩。>=0表示压缩利润，匹配子描述在tokp里
*/
#define IBASE (MBASE-3)
static int deflate(register struct comprtok *tokp,register char *strstart,int len)
{
register char *p,*p1;
int i,gain,g1,j;
register unsigned int pos,b;
int distance,cur;
unsigned short *prev;
	if(len < 3) return -1;
	cur=strstart-tokp->strbegin;
	b=0;
	if(cur>MAX_DISTANCE) b=cur-MAX_DISTANCE;
	gain=-1;
	prev=tokp->prev;
	
	for(pos=tokp->head[tokp->h]; pos != NIL && pos>=b;
		pos=prev[pos]) {
/*
if(pos==tokp->prev[pos]) {
	printf("deflate pos=%d,cur=%d,hash=%d,head=%d\n",
		pos,cur,tokp->h,tokp->head[tokp->h]);
	break;
}
*/
	    p=tokp->strbegin+pos;
	    if(*strstart!=*p || strstart[1]!=p[1] || strstart[2]!=p[2])
			continue;
	    distance=cur-pos;
	    i=3;
	    p+=i;
		for( p1=strstart+i ; i<len && *p==*p1;p++,p1++)
		{
			if(++i == 264) {
				p++,p1++;
				break;
			}
	   	}
/* 计算利润 */
	    g1=i-2;
	    if(i==3) {
		if(distance>256) continue;

	    } else {
		if(distance>192) --g1;
	    	if(i>8) g1--;
	    }
	    j=(distance<i);	//重叠
	    if(g1>gain) {
		if(i<9) tokp->flgbyte=i+IBASE;
		else {
			tokp->flgbyte=MFLG;
			tokp->len=i-9;
		}
		if(i==3 || distance<193) {
			*tokp->distance=distance-1;
		} else {
			distance -= 193;
			*tokp->distance=(distance>>8) | 0xC0;
			tokp->distance[1]=distance;
		}
		tokp->strend=p1;
		gain=g1;
		if(j && g1>20) break; //重叠退出，否则，压缩略有提高，速度很慢
		if(i>263) break;
	    }
	}
/*
if(gain > 0) {
	if(tokp->flgbyte==MFLG) i=(tokp->len&255)+9;
	else i=tokp->flgbyte-IBASE;
	if(*tokp->distance<192) distance=*tokp->distance+1;
	else {
		distance=*tokp->distance & 0X3F;
		distance<<=8;
		distance += tokp->distance[1];
		distance += 193;
	}
	if(distance > 128 && distance < 193)
	    fprintf(stderr,"%d<%d,%d>\n",strstart-tokp->strbegin,
		i,distance,i,strstart);
}
*/
	return gain;
}

static void win_slip(struct comprtok *tokp)
{
int i;
register unsigned short *usp,*udp;
	usp=tokp->head;
	for(i=0;i<HASH_SIZE;i++,usp++) {
		if(*usp==NIL) continue;
		if(*usp<16384) *usp=NIL;
		else *usp-=16384;
	}
	udp=tokp->prev;
	usp=udp+16384;
	for(i=0;i<16384;i++,usp++,udp++) {
		if(*usp==NIL || *usp<16384) *udp=NIL;
		else *udp=*usp-16384;
	}
	tokp->strbegin+=16384;
}
/* memzip,主要的压缩程序
 入口参数：
src：被压缩的串。
srcLen：串的字节数。
dest：压缩后的串存放于此，其长度不小于srcLen
返回值：压缩后的长度，如果==srcLen，没有压缩
*/
int memzip(char *dest,char *src,int srcLen)
{
register unsigned char *p,*pd,*p0,*pe;
int gain=0,totalgain=0;
unsigned cur,i;
struct comprtok comtok;
	if(!src || !dest) return 0;
    {
	register unsigned *up;
	register unsigned num;
	num=HASH_SIZE>>1;
	up=(unsigned *)comtok.head;
	do *up++ = DNIL; while(--num);
    }
	p=(unsigned char *)src;pd=(unsigned char *)dest;
	comtok.strbegin=src;
	comtok.h=HASH(HASH(*p,p[1]),p[2]);
	comtok.head[comtok.h]=0;
	*comtok.prev=NIL;
	*pd++=*p++;
	p0=(unsigned char *)src+srcLen;
	pe=(unsigned char *)dest+srcLen;
	while(p<p0) {
		if(p<p0-2) comtok.h=HASH(comtok.h,p[2]);
		if((cur=p-(unsigned char *)comtok.strbegin) > 32504) { //窗口滑动16384
//fprintf(stderr,"slip cur=%u\n",cur);

			win_slip(&comtok);
			cur -= 16384;
/*
printf("win_slip cur=%d,strbegin=%d,srcLen=%d,p=%d\n",
cur,comtok.strbegin-src,srcLen,p-src);
*/
		}
		gain=deflate(&comtok,(char *)p,p0-p);
/* 当前字符加入HASH表 */
		comtok.prev[cur]=comtok.head[comtok.h];
		comtok.head[comtok.h]=cur;
		if(gain>0) {	//压缩。
		    p=(unsigned char *)comtok.strend;
			i=p-(unsigned char *)comtok.strbegin;  //能够HASH的最后字节
/* 匹配区内所有字符都要加入HASH表 */	
			while(++cur < i) {
				comtok.h=HASH(comtok.h,
				    *((unsigned char *)comtok.strbegin+cur+2));
/*
if(cur & 0XFFFFC000)
	fprintf(stderr,"cur=%u\n",cur);
*/
				comtok.prev[cur]=comtok.head[comtok.h];
				comtok.head[comtok.h]=cur;
			}
		    *pd++ = comtok.flgbyte;
		    if(comtok.flgbyte == MFLG) 	//长匹配
			    *pd++ = comtok.len;
		    *pd++ =*comtok.distance;
		    if(comtok.flgbyte!=MBASE && *comtok.distance > 191) {
				*pd++ = comtok.distance[1];	//长距离
		    }
		    totalgain += gain;
		    continue;
		}
		if(*p>=ESC && *p <(ESC+8))  {
			totalgain--;
			if(pd>pe) //  防止越界
			{
				totalgain=-1;
				break;
			}
			*pd++=ESC; //转义
		}
		if(pd>pe) {	//防止越界
			totalgain=-1;
			break;
		}
		*pd++ = *p++; //原文复制
	}
	if(totalgain <=0 && p != p0) {
		memcpy(dest,src,srcLen);
		return srcLen;
	}
	return pd-(unsigned char *)dest;
}

/* 主要的解压缩程序
入口参数：
compr_str：经过memzip压缩的字符串。
t_len:compr_str的长度。
buf：解压后的串。
pkg_len:buf的长度，应该就是压缩前的长度。
返回值：解压后的长度,应该等于压缩前的长度。
-1:出现不明错误，这个串不是memzip压缩的。
*/
int memunzip(char *compr_str,int t_len,char *buf,int pkg_len)
{
register unsigned char *p,*p1,*p2,*p0;
int n;
int distance;
	if(!compr_str || !buf) return 0;
	if(t_len==pkg_len) {
		if(compr_str != buf)
			memcpy(buf,compr_str,t_len);
		return(t_len);
	}
	p=(unsigned char *)compr_str;
	p1=(unsigned char *)buf;
	*p1++ = *p++;
	p0=(unsigned char *)(compr_str+t_len);
	while(p<p0) {
		if(p1 == p) {
			return pkg_len;
		}
		if((p1-(unsigned char *)buf)>=pkg_len) {
//fprintf(stderr,"出现不明错误，解码超长%d！t_len=%d,possion=%d\n",
//		p1-buf-pkg_len,t_len,p-(unsigned char *)compr_str);
			return -1;
		}
		switch((unsigned char)*p) {
		case MBASE:
		case MBASE+1:
		case MBASE+2:
		case MBASE+3:
		case MBASE+4:
		case MBASE+5:	//短匹配
			n=*p-IBASE;
			p++;
			if(n != 3) goto get_distance;
			else { //三字节distance:=0-255;
				distance=(*p++ & 255)+1;
				goto byte3;
			}

		case MFLG:	//长匹配
			p++;
			n=(*p&255)+9;
			p++;
get_distance:
			if(*p > 191) {
				distance= *p&0X3F;
				distance <<= 8;
				p++;
				distance += *p&255;
				p++;
				distance += 193;
			} else {
				distance=(*p)+1;
				p++;
			}
byte3:
			p2=p1-distance;
			while(n--) *p1++ = *p2++;
			break;
		case ESC:	//解除转义
			p++;
		default:
			*p1++ = *p++;
			break;	
		}
	}
	return p1-(unsigned char *)buf;
}
