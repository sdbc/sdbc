/*************************************************
 * 一个ENIGMA密码机算法
 * 一个转轮，一个反射轮
 * 256个点位,可进行字节的二进制加密/解密
 * 码空间:256! × 255 × 253 ....... ×3     
 * 主要用于通信系统加密
 *************************************************/
#include <sys/types.h>
#ifndef ENIGMA_H
#define ENIGMA_H

#define ROTORSZ 256

typedef signed char ENIGMA[3][ROTORSZ];
typedef struct {
	ENIGMA t;
	ENIGMA r;
	u_int crc;
} ENIGMA2;

#ifdef __cplusplus
extern "C" {
#endif
/* 改进的 ENIGMA 程序 序列长度：64K */
void enigma_init(ENIGMA t,const char *bin_key,int len);		//初始化
void enigma(ENIGMA t,char *buf,int len);			//加密/解密
//疯狂旋转的转轮机，完全屏蔽了列表特征，不存在周期性
void enigma_encrypt(ENIGMA t,char *string,int len);		//加密
void enigma_decrypt(ENIGMA t,char *string,int len);		//解密 

/* 加密后扰码，用于配合多种加密手段 */
void enigma_rev(ENIGMA t,char *buf,int len);
/* 解扰后解密，用于解密 */
void rev_enigma(ENIGMA t,char *buf,int len);

/* 加强的ENIGMA程序，完全消除了密文与相似明文的对应关系 */
void enigma2_init(ENIGMA2 *ep,const char *bin_key,int len);	//初始化
void enigma2_encrypt(ENIGMA2 *ep,char *buf,int len);		//加密
void enigma2_decrypt(ENIGMA2 *ep,char *buf,int len);		//解密 

#ifdef __cplusplus
}
#endif

#endif
