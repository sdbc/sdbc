#include <stdio.h>
#include <sys/types.h>

typedef unsigned int u_int;
#define LOW 0
#define HIGH 1
#define MAXNUMBER 128
#define BITS 32

#ifdef __cplusplus
extern "C" {
#endif

u_int *mulm(u_int n,u_int *pa,u_int *pb,u_int *pm);
u_int *mul1(u_int n,u_int *pa,u_int b,u_int *pm);
u_int divm(u_int n,u_int *div,u_int *by,u_int *quot,u_int *rem);
u_int * d_to_b(u_int n,char *cp,u_int *dp);
char *b_to_d(u_int n,u_int *pb,char *d);
u_int addm(u_int n,u_int *add_to,u_int *addf_from);
u_int * incm(u_int n,u_int *num);
u_int subm(u_int n,u_int *sub_to,u_int *subf_from);
u_int * decm(u_int n,u_int *num);
int numcmp(u_int n,u_int *cmp,u_int *cmp_from);
void numcpy(u_int n,u_int *to,u_int *from);
u_int * n_zero(u_int n,u_int *to);
u_int * n_one(u_int n,u_int *to);
u_int * n_ff(u_int n,u_int *to);
u_int *n_not(u_int n,u_int *to);
u_int n_iszero(u_int n,u_int *pa);
u_int rshift(u_int n,u_int *up,int i);
u_int lshift(u_int n,u_int *up,int i);
u_int div1(u_int n,u_int *pa,u_int b,u_int *pq);

u_int *_m_m_(int n,u_int *a,u_int *b,u_int *mi,u_int *ret);
u_int *mulmod(int n,u_int *a,u_int *b,u_int *mi,u_int *ret);
u_int *_e_m_(int n,u_int *a,u_int *z,u_int *m,u_int *ret);
u_int *expmod(int n,u_int *a,u_int *z,u_int *m,u_int *ret);

/* a64n.c */
char n64a(int n);
int a64n(char a);
char * str_n64a(int n,u_int *num,char *str);
char * str_a64n(int n,char *str,u_int *num);
void loadnum(int len,char *str,u_int *num);
char *strhex(int n,u_int *num,char *str);
void byte2n(int n,u_int *num,char *bytestr);
void n2byte(int n,u_int *num,char *bytestr);
char * revers(char *str);

char * byte_a64(char* str,char *byte,int len);
int a64_byte(char *byte,char *str);


#ifdef __cplusplus
}
#endif

