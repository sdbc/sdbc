
/************************************************
 *  For SDBC 4.0
 * 本程序组完成 串行数据对数据库SQL语句的映射。
 * 由于本组程序不支持绑定变量，因此不建议使用
 * 以DAU程序取代之
 *  For ORACLE
 ************************************************/

#include <pack.h>

/* at net_pack.c */
extern int strcpy_esc(char *dest,char *src,int len,char CURDML);

extern char * ext_copy(char *dest,const char *src);

/* 生成一个VALUES项，注意 ，typ是模板中的一个单项 */
char *mkvalue(char *vp,char *tmp,T_PkgType *typ)
{
int len,ccodd,clen;
char *p;

	if(!vp) return vp;
	*vp=0;
	if(typ->type<0) return tmp;
	if(!tmp||!*tmp) {
		vp=stpcpy(vp,"null");
		return vp;
	}
	switch(typ->type) {
	case CH_USEC:
		if(typ->format) {
			vp=stpcpy(ext_copy(stpcpy(stpcpy(stpcpy(vp,"TO_TIMESTAMP(\'"),tmp),"\',\'"),typ->format),"\')");
			break;
		}
		len=strlen(tmp);
		goto  do_char;
	case CH_DATE:
	case CH_JUL:
	case CH_MINUTS:
	case CH_TIME:
		if(typ->format) {
			if(is_timestamp((char *)typ->format))
			  vp=stpcpy(ext_copy(stpcpy(stpcpy(stpcpy(vp,"TO_TIMESTAMP(\'"),tmp),"\',\'"),typ->format),"\')");
			else vp=stpcpy(ext_copy(stpcpy(stpcpy(stpcpy(vp,"TO_DATE(\'"),tmp),"\',\'"),typ->format),"\')");
			break;
		}
	case CH_CJUL:
	case CH_CTIME:
	case CH_CMINUTS:
		len=strlen(tmp);
		goto do_char;
	case CH_CHAR:
		len=typ->len-1;
do_char:
		ccodd=clen=0;
		*vp++='\'';
		for(p=tmp;*p&&clen < len;p++,clen++) {
			if(!ccodd && *p&0x80) ccodd=1;
			else ccodd=0;
/*
if((unsigned char)*p < ' '||*p==0x7f) {
	vp+=sprintf(vp,"'||CHR(%d)||'",*p++);
	continue;
} else 
*/
			if((*p)=='\'') *vp++=*p;
			*vp++=*p;
		}
		if(ccodd) {
			if(clen<typ->len-1)
				*vp++=' ';
			else vp[-1] &= 0x7f; 
		}
		*vp++='\'';
		*vp=0;

		break;
	default:
		vp=stpcpy(vp,tmp);
		break;
	}
	return vp;
}
/* 直接从结构中生成Values 
*/

char *mk_values(char *values,void  *data, T_PkgType *tp)
{
T_PkgType *typ;
char *vp,tmp[4096];
int i;
	if(!values || !data || !tp ->type<0) return values;
	vp=values;
	*vp=0;
	if(tp->offset<0) set_offset(tp);
	for(i=0,typ=tp;typ->type != -1;i++,typ++) {
	    if((typ->bindtype&NOINS) || typ->type == CH_CLOB || !strcmp(typ->name,"ROWID") || *typ->name==' ') {
/* can not insert CLOB *p and ROWID into database,skip it*/
		continue;
	    }
		get_one(tmp,data,tp,i,0);
		if(vp!=values) {
			*vp++=',';
			*vp=0;
		}
		vp=mkvalue(vp,tmp,typ);
	}
	return vp;
}

char *mkvalues(char *values,char *str, T_PkgType *tp)
{
char tmp[4096],tmp1[4096];
T_PkgType *typ;
char *cp,*vp;
	
	if(!values || !str || !*str || !tp) return str;
	vp=values;
	cp=str;
	
	if(tp->offset<0) set_offset(tp);
	for(typ=tp;typ->type > -1;typ++) {
		if(!*cp) break;
	    if((typ->bindtype&NOINS) || typ->type == CH_CLOB || !strcmp(typ->name,"ROWID") || *typ->name==' ') {
/* can not insert CLOB *p and ROWID into database,skip it*/
		cp=stptok(cp,0,0,"|");
		if(*cp=='|') cp++;
		continue;
	    }
		cp=stptok(cp,tmp1,sizeof(tmp),"|");
		if(*cp) cp++;
		strcpy_esc(tmp,tmp1,0,'|');
		if(vp!=values) {
			*vp++=',';
			*vp=0;
		}
		vp=mkvalue(vp,tmp,typ);
	}
	return values;
}
char *mkupdate(char *str,char *data,T_PkgType *tp)
{
	if(!data || !*data || !str || !tp) return str;
	if(tp->offset<0) set_offset(tp);
	mkvalues(stpcpy(mkset(stpcpy(str,"SET("),tp),")=(SELECT "),data,tp);
	strcat(str," FROM DUAL)");
	return str;
}
/* 直接从结构中生成 Update 
*/
char *mk_update(char *str,void  *data,T_PkgType *tp)
{
	if(!data || !str || !tp) return str;
	return stpcpy(mk_values(stpcpy(mkset(stpcpy(str,"SET("),tp),")=(SELECT "),data,tp)," FROM DUAL)");
}

