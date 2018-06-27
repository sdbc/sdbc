#include <stdio.h>
#include <sys/types.h>

#define LOW 0
#define HIGH 1
#define MAXNUMBER 128
#define BITS 32

void numcpy(u_int n,u_int *to,u_int *from);
u_int * n_zero(u_int n,u_int *to);
u_int * n_one(u_int n,u_int *to);
u_int * n_ff(u_int n,u_int *to);
u_int *n_not(u_int n,u_int *to);
int rshift(int n,u_int *up,int i);
int lshift(int n,u_int *up,int i);

u_int *_m_m_(int n,u_int *a,u_int *b,u_int *mi,u_int *ret);
u_int *_e_m_(int n,u_int *a,u_int *z,u_int *m,u_int *ret);

#define RSALEN 1024/BITS
#define PACKLEN 1024/8
int vnt_ca(char *keyfile,char *code,char result[PACKLEN]);
extern int errno;
void loadnum(int len,char *str,u_int *num);
