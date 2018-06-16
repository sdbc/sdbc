#include <string.h>
#include <crc32.h>
#include <enigma.h>
/*
 *	A one-rotor machine designed along the lines of Enigma
 *	but considerably trivialized.
 */

#define MASK 0377
// 随机串长度，如果你想使用MD5，加大这个值 

/* 生成密码轮 */   
void enigma_init(ENIGMA t,const char *bin_key,int len)
{
int		ic, i, k,random,seed;
signed char	temp,*t1,*t2,*t3;

	if(!t || !bin_key) return;
	if(len<=0 && !(len=strlen(bin_key))) return;
	seed=(int)ssh_crc32((unsigned char *)bin_key,len);
	t1=t[0]; t2=t[1]; t3=t[2];
// 设置ic，初始化的自旋 
	ic=seed; 
	for(i=0;i<len;i++) ic += bin_key[i];
	ic &= MASK;
//printf("%s:len=%d,seed=%d,ic=%d\n",__FUNCTION__,len,seed,ic);
// ic，初始化的自旋 
	for(i=0;i<ROTORSZ;i++) {
		t1[i] = (i+ic) & MASK;
		t3[i] = 0;
	}

	if(len>ROTORSZ) {//如果密钥长于ROTORSZ,使用最后一部分
		bin_key += len-ROTORSZ;
		len = ROTORSZ;
	}
	for(i=0;i<ROTORSZ;i++) {
		seed = (seed<<1) + (signed)bin_key[i%len];
		random = (seed&0X7FFFFFFF) % 65529;	//random(key);
// 以上生成尽可能随机的random，你有充分的自由度选择你的算法 
/* 生成主编码轮 t1 */
		k = ROTORSZ-1 - i;
		ic = random % (k+1);

		temp = t1[k]; t1[k] = t1[ic]; t1[ic] = temp;
/************************************************************************
 * 生成反射轮 反射轮只要不重不漏的把各点两两连接起来即可,
 ************************************************************************/
		if(t3[k]!=0) continue;
		ic = (random&MASK) % k;
		while(t3[ic]!=0) ic = (ic+1) % k;
		t3[k] = ic; t3[ic] = k;
	}
/* t2为t1的逆 */
	for(i=0;i<ROTORSZ;i++)
		t2[t1[i]&MASK] = i;
/*
char buf[200],*cp;
cp=buf;
for(i=0;i<32;i++)
	cp+=sprintf(cp,"%02X ",t1[i]&255);
ShowLog(5,"%s:len=%d,t1=%s",__FUNCTION__,len,buf);
*/
}

void enigma(ENIGMA t,char *string,int len)
{
register int  n1,n2, k;
signed char *t1,*t2,*t3;

	if(!t || !string || len <= 0) return;
	t1=t[0]; t2=t[1]; t3=t[2];
//初始位置和与len和T有关，不知道T就不知道它，也无法通过明文、密文的关系推断T。
	n2=t[len&1][(len>>9)&MASK]&MASK;
	n1=t3[(len>>1)&MASK]&MASK;
//printf("%s:len=%d,n1=%d,n2=%d\n",__FUNCTION__,len,n1,n2);
	for(k=0;k<len;k++,string++){
		*string = t2[(t3[(t1[((signed char)*string+n1)&MASK]+n2)&MASK]-n2)&MASK]-n1;
		if (++n1 >= ROTORSZ) {
			n1 &= MASK;
			if(++n2==ROTORSZ) n2 = 0;
		}
	}
}

void enigma_encrypt(ENIGMA t,char *string,int len)
{
register signed char *p;
register int x,n1;       //x旋转因子
signed char *t1,*t2,*t3;
int  n2, k;
//int r=0;

        if(!t || !string || len <= 0) return;
        t1=t[0]; t2=t[1]; t3=t[2];

	n2=t[len&1][(len>>9)&MASK]&MASK;
	n1=t3[(len>>1)&MASK]&MASK;

	p=(signed char *)string;
	for(k=0;k<len;k++,p++){
		x=t1[(*p+n1)&MASK];
		*p = t2[(t3[(x+n2)&MASK]-n2)&MASK]-n1;
		n1 += (x&0x1F) + 1;
		if (n1 >= ROTORSZ) {
			n1 &= MASK;
			if(++n2==ROTORSZ) n2 = 0;
		}
	}
}

void enigma_decrypt(ENIGMA t,char *string,int len)
{
register signed char *p;
register int n1, n2, k,x;
signed char *t1,*t2,*t3;

        if(!t || !string || len <= 0) return;
        t1=t[0]; t2=t[1]; t3=t[2];
	n2=t[len&1][(len>>9)&MASK]&MASK;
	n1=t3[(len>>1)&MASK]&MASK;

	p=(signed char *)string;
	for(k=0;k<len;k++,p++){
		x=t3[(t1[(*p+n1)&MASK]+n2)&MASK]-n2;
		*p = t2[x&MASK]-n1;
		n1 += (x&0x1F)+1;
		if (n1 >= ROTORSZ) {
			n1 &= MASK;
			if(++n2==ROTORSZ) n2 = 0;
		}
	}
}

