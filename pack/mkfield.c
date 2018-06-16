
/************************************************
 *  For SDBC 4.0
 * 本程序组完成 串行数据对数据库SQL语句的映射。
 *  For ORACLE
 ************************************************/
#define _GNU_SOURCE
#include <pack.h>

/* at net_pack.c */
extern int strcpy_esc(char *dest,char *src,int len,char CURDML);

char * is_timestamp(const char *format)
{
	return strstr(format,"FF");
}

char * ext_copy(char *dest,const char *src)
{
	while(*src) {
		*dest++ = *src;
		if(*src == '\'') *dest++=*src;
		++src;
	}
	*dest=0;
	return dest;
}
	
char *mkset(char *str, T_PkgType *tp)
{
register T_PkgType *typ;
char *cp,*p;

	if(!str || !tp) return str;
	if(tp->offset<0) set_offset(tp);
	cp=str;
	for(typ=tp;typ->type>=0;typ++) {
	    if((typ->bindtype&NOINS) || typ->type == CH_CLOB || !strcmp(typ->name,"ROWID") || *typ->name==' ') continue;
/* 提取真实列名 */
		p=(char *)typ->name;
		cp=strtcpy(cp,&p,' ');
		*cp++ = ',';
		*cp=0;
	}
	*--cp=0;
	return cp;
}

static char *mk_date(char *buf,char *name,const char *format)
{
char *p,*cp;
	
	p=(char *)plain_name(name);
	cp=stpcpy(buf,"TO_CHAR(");
	if(p==name)  cp=stpcpy(cp,name);
	else  //复杂的列表达式 
		cp+=sprintf(cp,"%.*s",(int)(p-name-1),name);
	cp=stpcpy(stpcpy(ext_copy(stpcpy(cp,",'"),format),"') "),p);
	return cp;
}

static char *_mkfield(char *field, T_PkgType *tp)
{
register T_PkgType *typ;
register char *cp;

	if(!field || !tp ) return field;
	if(tp->offset<0) set_offset(tp);
	cp=field;
	for(typ=tp;typ->type>=0;typ++) {
		if(typ->bindtype & NOSELECT) continue;
		switch(typ->type) {
		case CH_MINUTS:
		case CH_DATE:
		case CH_JUL:
		case CH_TIME:
		case CH_USEC:
			if(typ->format) {
				cp=mk_date(cp,skipblk((char *)typ->name),typ->format);
/*
			char *p0;
				p0=skipblk(typ->name);
				cp=stpcpy(ext_copy(stpcpy(strtcpy(stpcpy(cp,"TO_CHAR("),&p0,' '),",\'"),typ->format),"\') ");
				if(*p0==' ') cp=stpcpy(cp,++p0);
				else cp=stpcpy(cp,typ->name);
*/
				break;
			}
		default:
			cp=stpcpy(cp,typ->name);
			break;
		}
		*cp++=',';
		*cp=0;
	}
	*(--cp)=0;
	return cp;
}
char *mkfield(char *field, T_PkgType *tp,const char *tabname)
{
T_PkgType *typ;
char *cp;

	if(!field || !tp ) return field;
	if(tp->offset<0) set_offset(tp);
	if(!tabname || !*tabname) return (_mkfield(field,tp));
	cp=field;
	for(typ=tp;typ->type>=0;typ++) {
		if(typ->bindtype & NOSELECT) continue;
		switch(typ->type) {
		case CH_MINUTS:
		case CH_DATE:
		case CH_JUL:
		case CH_TIME:
		case CH_USEC:
			if(typ->format) {//不能支持太复杂的日期表达式 
			char *p=(char *)typ->name;
				cp=stpcpy(stpcpy(cp,"TO_CHAR("),tabname);
				*cp++='.';
				cp=stpcpy(ext_copy(stpcpy(strtcpy(cp,&p,' '),",\'"),typ->format),"\') ");
				if(*p==' ') cp=stpcpy(cp,++p);
				else cp=stpcpy(cp,typ->name);
				break;
			}
		default:
			cp=stpcpy(cp,tabname);
			*cp++='.';
			cp=stpcpy(cp,typ->name);
		}
		*cp++=',';
	}
	*(--cp)=0;
	return cp;
}
