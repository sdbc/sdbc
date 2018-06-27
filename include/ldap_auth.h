#include <ldap/ldap.h>

#ifdef __cplusplus
extern "C" {
#endif

LDAP * ldap_connect(char *host,int port,char *user_dn,char *user_pwd);
int ldap_auth(LDAP *ld,char *auth_dn,char *dsn,char *uid,char *pwd,char *dbown);

#ifdef __cplusplus
}
#endif
