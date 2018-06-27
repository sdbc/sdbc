#include <ldap_auth.h>

#define MAX_DN 64

extern T_PkgType ldap_auth_tpl[];

typedef struct {
        char host[256];
        short port;
        char user_dn[MAX_DN];
        char pwd[16];
} ldap_auth_stu;

#ifdef __cplusplus
extern "C" {
#endif

extern int ldap_dbopen(T_SQL_Connect *SQL_Connect,LDAP *ld,char *dblabel);


#ifdef __cplusplus
}
#endif
