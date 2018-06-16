/*****************************************************
 * For SDBC 4.0
 * 数据库帐号口令的获取函数
 * SDBC封装了数据库的帐号口令。
 * 客户端需要以连接串关联数据库
 * 本程序就是利用LDAP，以连接串提取帐号口令。
 *****************************************************/

#include <stdlib.h>
#include <string.h>
#include <strproc.h>
#include <ldap_auth.h>
extern char *(*encryptproc)(char *mstr);
#define strdel(str) strsubst((str),1,(char *)0)

LDAP * ldap_connect(char *host,int port,char *user_dn,char *user_pwd)
{
LDAP *ld;
int rc;
	 //get a handle to a LDAP connection
    if(!(ld=ldap_init((LDAP_CONST char *)host,port))) {
        ShowLog(1,"ldap_connect:ldap_init %s/%d fail!",host,port);
        return NULL;
    }

    rc=ldap_simple_bind_s(ld,user_dn,user_pwd);
    if(rc!=LDAP_SUCCESS) {
        ShowLog(1,"ldap_connect: ldap_simple_bind_s err=%s,user_dn='%s',pwd=%s",
					ldap_err2string(rc),user_dn,user_pwd);
        ldap_unbind(ld);
        return NULL;
	}
	return ld;
}

int ldap_auth(LDAP *ld,char *auth_dn,char *dsn,char *uid,char *pwd,char *dbown)
{
LDAPMessage *result,*e;
BerElement *ber;

char *p;
char **vals;
int i,rc;

    if((rc=ldap_search_ext_s(ld,auth_dn,LDAP_SCOPE_BASE,
		"(objectclass=*)",NULL,0,NULL,NULL,
		LDAP_NO_LIMIT,LDAP_NO_LIMIT,&result))!=LDAP_SUCCESS) {
   			ShowLog(1,"ldap_auth:ldap_search_ext_s %s",ldap_err2string(rc));
			return(-1);
    }

    //since we are doing a base search ,there should be only one matching entry
    e=ldap_first_entry(ld,result);
    if(e==NULL) {
		ShowLog(1,"ldap_auth:ldap_first_entry err=there is no record can match your request!");
    	ldap_msgfree(result);
		return (-2);
	}
    
    //Iterate through each attribute in the entry
    for(p=ldap_first_attribute(ld,e,&ber);p!=NULL;p=ldap_next_attribute(ld,e,ber)) {
    	//for each attribute ,print the attribute name and values
    	if((vals=ldap_get_values(ld,e,p))!=NULL) {
            for(i=0;vals[i]!=NULL;i++) {
				if(!strcmp(p,"SID")) {
						strcpy(dsn,vals[i]);
				} else if(!strcmp(p,"USERNAME")) {
						strcpy(uid,vals[i]);
						if(vals[i][0]=='@' && *encryptproc) {
								strdel(uid);
								encryptproc(uid);
						}
				} else if(!strcmp(p,"PWD")) {
						strcpy(pwd,vals[i]);
						if(vals[i][0]=='@' && *encryptproc) {
								strdel(pwd);
								encryptproc(pwd);
						}
				} else if(!strcmp(p,"DBOWN")) {
						strcpy(dbown,vals[i]);
				} else {
					ShowLog(5,"ldap_auth get_values:name=%s, value=%s",p,vals[i]);
				}
			}
            ldap_value_free(vals);
        }
        ldap_memfree(p);
    }
    if(ber!=NULL) ber_free(ber,0);
    ldap_msgfree(result);
    return(0);
}

