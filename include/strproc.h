/*****************************************************************************
          DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
                   Version 6, January 2014

Copyright (C) 2014 Lihua-Yu <ylh2@sina.com>

Everyone is permitted to copy and distribute verbatim or modified
copies of this license document, and changing it is allowed as long
as the name is changed.

           DO WHAT THE FUCK YOU WANT TO PUBLIC LICENSE
  TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION

 0. You just DO WHAT THE FUCK YOU WANT TO.
********************************************************************************/
// For SDBC 7.0
#ifndef STRPROC_H
#define STRPROC_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <inttypes.h>  //C99

#if defined _LP64 || defined __LP64__ || defined __64BIT__ || _ADDR64 || defined _WIN64 || defined __arch64__ || __WORDSIZE == 64 || (defined __sparc && defined __sparcv9) || defined __x86_64 || defined __amd64 || defined __x86_64__ || defined _M_X64 || defined _M_IA64 || defined __ia64 || defined __IA64__
#define SDBC_PTR_64
#endif
#define SYSERR (-1)
#define LENGERR (-2)
#define POINTERR (-3)
#define MEMERR (-4)
#define THREAD_ESCAPE (-5)
#define NOTLOGIN (-6)
#define FORMATERR (-7)
#define CRCERR (-8)
#define TIMEOUTERR (-11)

#if defined WIN32 || defined _WIN64
#include <winsock2.h>
#define INT64  __int64
typedef int pthread_t;
typedef int pthread_mutex_t;
#define timezone _timezone
typedef unsigned int u_int;
#else
#       include <pthread.h>
#define INT64 int64_t
#endif

#define CH_INT4 CH_INT
#define INT4 int32_t

extern char *(*encryptproc)(char *mstr);
/***************************************************************************
 * 一些函数需避免GBK字符集里边的英文字母被误判，使用GBK字符集时需设该标志为1
 * 其缺省值为0
 ***************************************************************************/
extern char GBK_flag;

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/*function:stptok 分解字符串为一组标记串		                        */
/*description:根据分隔符'tok'，从'src'分解到'trg'中，如果'tok'不设置，则*/
/*根据'src'与len的最小值，将'src'拷贝到'trg'中。可以解决汉字中出现的与  */
/*'tok'相同的分隔符被分解。返回指向出现'tok'字符的指针					*/
/*特殊：																*/
/*	char *p=stptok(from,0,0,char *tok); 即当分配的目标串和长度为0时,	*/
/*	则只返回出现'tok'字符的指针											*/
/************************************************************************/
/*function:iscc 判断是否是汉字*/
int iscc(unsigned char ch);
/*function:firstcc 汉字的第一个字节*/
int firstcc(unsigned char *bp,unsigned char *bufp);
/*function:secondcc 汉字的第二个字节*/
int secondcc(unsigned char *bp,unsigned char *bufp);
char *stptok(const char *src,char *trg,int len,const char *tok);
/**********************************************************************
 * stctok:类似strtok,提供线程安全的，支持汉字token的字符串分解函数。
 * 前几个参数与strtok_r相同,int *by:指示器，表示字符串是由tok中那个字符截断的
 * -1表示不是被tok截断的。
 *********************************************************************/
extern char *stctok(const char *str,char *ctok,char **savep,int *by);
extern char *cstrchr(const char *from,char *tok);

/************************************************************************/
/*function:strsubst 替换字符串函数				                        */
/*description:用'str'替换'from'的前cnt个字符							*/
/*返回指向替换后的下一个字符的指针										*/
/************************************************************************/
char *strsubst(char *from,int cnt,char *str);

/************************************************************************/
/*function:strsub 截取字符串函数                                        */
/*description:'src'中截取从start开始的cnt个字符到'dest'中(start起点为0)	*/
/************************************************************************/
void strsub(char *dest, const char *src,int start,int cnt);

/************************************************************************/
/*function:strins 插入字符函数                                          */
/*description:在'str'前插入字符'ch',返回下一个字符的指针				*/
/************************************************************************/
char *strins(char *str,char ch);

/************************************************************************/
/*function:strdel 删除字符函数		                                    */
/*description:删除'str'头一个字符										*/
/************************************************************************/
#define strdel(str) strsubst((str),1,(char *)0)

/************************************************************************/
/*function:trim 去除字符串前后空格	                                    */
/************************************************************************/
char *trim(char *src);

/************************************************************************/
/*function:rtrim 去除字符串后的空格	 	                                */
/************************************************************************/
char *rtrim(char *str);

/************************************************************************/
/*function:ltrim 去除字符串前的空格		                                */
/************************************************************************/
char *ltrim(char* str);

/************************************************************************/
/*function:strupper 将字符串中的小写字母转换成大写                      */
/************************************************************************/
char *strupper(char *str);

/************************************************************************/
/*function:strlower 将字符串中的大写字母转换成小写                      */
/************************************************************************/
char *strlower(char *str);
/**************************************************
 * TRIM: delete suffix space of str
 *****************************************************/
char * TRIM(char *str);
char *trim_all_space(char *str);
char *sc_basename(char *path);
/******************************************************
 * skipblk: skip prefix space of str
 *****************************************/
char *skipblk(const char *str);
/* copy *src to dst break by c return tail of dst,*src to tail  */
char *strtcpy(char *dst,char **src,char c);
#ifndef  __USE_XOPEN2K8
char *stpcpy(char *dest,const char *src);
#endif

char *strrevers(char *str);


/***********************************************
 * ShowLog: write debug or log message to stderr
 * logfile name  defined by enveroment varibale LOGFILE
 * LOGLEVEL defined by enveroment varibale LOGLEVEL
 * if DEBUG_level <= LOGLEVEL then write message to stderr
 * example:
 * LOGLEVEL=3
 * LOGFILE=/tmp/rec
 * today is 2001-02-08 14:29'03
 * then stderr=/tmp/rec08.log
 * if DEBUG_level <=3 ,then write the message to /tmp/rec08.log as:
 * DEBUG_level Showid yyyy/mm/dd 14:29'03 message
 *
 ***********************************************/
extern char *Showid;
extern char LOGFILE[];
int ShowLog(int DEBUG_level,const char *fmt,...);
//设置日志级别,允许程序动态设置日志级别
//返回原先的级别
int setShowLevel(int new_level);

/*************************************************************
 *  多线程日志,如果每个线程代表一个客户身份
 * 在认证客户身份完成后调用 mthr_showid_add,将Showid加入系统
 * 该线程消亡前，调用 mthr_showid_del清除该Showid
 * mthr_showid_del() 返回删除的层号，0=没删
 ************************************************************/
void mthr_showid_add(pthread_t tid,char *showid);
int mthr_showid_del(pthread_t tid);

//读配置文件建立环境变量
int envcfg(char *configfile);
int strcfg(char *str);

int isrfile(char *path);
int isdir(char *path);
/* 整型变字符串，返回尾部　*/
char * itoStr(register int n, char *s);
char * lltoStr(register INT64 n, char *s);
/********************************************
 *  fround.c
 * flg:0-科学舍入,1-向负,2-向正,3-只舍,4-只入,5-四舍五入,
 ********************************************/
double f_round(double x,int flg,int dig_num);
#define sc_round(x) f_round((x),5,0)
// used by open_auth() and ldap_auth()
int open_auth(char * authfile,char *name, char *DNS,char *UID,char *Pwd,char *DBOWN);
extern char *decodeprepare(char *dblabel);

#ifdef __cplusplus
}
#endif
#define SQL_AUTH open_auth

#endif
