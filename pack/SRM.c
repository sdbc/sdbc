#define _GNU_SOURCE
#include <SRM.h>
#include <ctype.h>

T_PkgType *patt_dup(T_PkgType *tp)
{
int n=set_offset(tp);
T_PkgType *new_tp;

	new_tp=(T_PkgType *)malloc(sizeof(T_PkgType) * (n+1));
	if(!new_tp) return NULL;
	patt_copy(new_tp,tp,0);
	return new_tp;
}

static const char pre_tabname[]="$DB.";

void SRM_init(SRM *srmp,void *record,T_PkgType *tp)
{

	if(!srmp) return;
	srmp->rec=record;
	srmp->tp=NULL;
	if(tp) {
		srmp->tp=patt_dup(tp); //为了线程安全
		if(!srmp->tp) return;
		srmp->Aflg=-set_offset(srmp->tp);
		srmp->result=0;
		srmp->rp=0;
		srmp->colidx=mk_col_idx(srmp->tp);
		srmp->tabname=(char *)tp[abs(srmp->Aflg)].name;
		srmp->pks=(char *)tp[abs(srmp->Aflg)].format;
		srmp->befor=0;
		srmp->hint=0;
	}
}

static char * addown_to_name(char *buf,char *DBOWN,const char *name)
{
char *p,*p1,*p2;

	p=buf;
	p1=(char *)name;
	do {
		p=strtcpy(p,&p1,*pre_tabname);
		if(*p1 && !strncmp(p1+1,pre_tabname+1,3)) {
			if(*DBOWN) {
				p2=DBOWN;
				p=strtcpy(p,&p2,0);
				p1+=3;
			} else p1+=4;
		} else if(*p1) {
			*p++=*p1++;
			*p=0;
		}
	} while(*p1) ;
	return p;
}

void set_dbo(char *buf,char *DBOWN)
{
char *p;
	p=buf;
	while(0!=(p=strcasestr(p,pre_tabname))) {
		if(*DBOWN) {
			p=strsubst(p,3,DBOWN);
			p++;
		} else {
			p=strsubst(p,4,0);
		}
	}
}

int SRM_mk_select(SRM *srmp,char *DBOWN,char *where)
{
register char *p,*p1;
char *whp=0;

	if(!where) return -1;
	if(*where && (toupper(*where)=='S' && toupper(where[1])=='E')) return 0; //如果是select,不作处理
	if(!srmp->tp) return FORMATERR;
	if(*where) {
                whp=strdup(where);
                if(!whp) {
                  return MEMERR;
		}
	}
	p=where;
	if(srmp->befor) {
		p=stpcpy(p,srmp->befor);
		*p++ = ' ';
		srmp->befor=0;
	}
	p=stpcpy(p,"SELECT ");
	if(srmp->hint) {
		p=stpcpy(p, srmp->hint);
		*p++ = ' ';
	}
	mkfield(p,srmp->tp,0);
	p1=where;
	while((p1=strcasestr((const char *)p1,(const char *)pre_tabname)) != 0) {
		if(*DBOWN) p1=strsubst(p1,3,DBOWN);
		else	   p1=strsubst(p1,4,DBOWN);
	}
	p+=strlen(p);
	p=stpcpy(p," FROM ");
		
	p1=stptok(srmp->tabname,0,0," ,.");
	if(!*p1) {			//简单表名 
	      	if(DBOWN && *DBOWN) {
       			p=stpcpy(p, DBOWN);
				*p++='.';
				*p=0;
		}
		p=stpcpy(p,srmp->tabname);
		*p++=' ';
		*p=0;
	} else {
		p=addown_to_name(p,DBOWN,srmp->tabname);
		*p++=' ';
		*p=0;
	}
	if(whp) {
		p=addown_to_name(p,DBOWN,whp);
		free(whp);
	}
	return 0;
}
// 生成半个 update 语句 
char * SRM_mk_update(SRM *srmp,char *DBOWN,char *where)
{
register char *p;

	p=where;
	if(srmp->befor) {
		p=stpcpy(p,srmp->befor);
		*p++ = ' ';
		srmp->befor=0;
	}
	p=stpcpy(p,"UPDATE ");
	if(srmp->hint) {
		p=stpcpy(p,srmp->hint);
		*p++=' ';
		*p=0;
		srmp->hint=NULL;
	}
	if(DBOWN && *DBOWN && !strchr(srmp->tabname,'.')) {
		p=stpcpy(p, DBOWN);
		*p++='.';
		*p=0;
	}
	p=stpcpy(p, srmp->tabname);
	*p++=' ';
	*p=0;
	return p;
}

/* 对选择的列构建update语句，如果choose为空，全部列 ,返回尾部 */
char * SRM_mk_upd_col(SRM *srmp,char *DBOWN,const char *choose,char *stmt)
{
char *p,*p1;
char col_name[49];
T_PkgType *tp;

    if(!srmp||!stmt) {
        return stmt;
    }
    p=SRM_mk_update(srmp,DBOWN,stmt);
    p=stpcpy(p,"SET ");
    p1=(char *)choose;
    if(p1 && *p1) do {
	if(!*p1) break;
        p1=stptok(skipblk(p1),col_name,sizeof(col_name),",|");
	*p++ = '$';
	p=stpcpy(stpcpy(stpcpy(p,col_name),"=:"),col_name);
	*p++ = ',';
    } while (*p1++);
	else  for(tp=srmp->tp;tp->type>=0;tp++) {
		if(tp->type==CH_CLOB || tp->bindtype & NOINS) continue;
		p1=(char *)tp->name;
		p=stpcpy(stpcpy(strtcpy(p,&p1,' '),"=:"),plain_name(tp->name));
		*p++ = ',';
	}
	*p=0;
    p[-1]=' ';
    return p;
}

int SRM_mk_delete(SRM *srmp,char *DBOWN,char *where)
{
register char *p,*whp;

	whp=0;
	if(*where && (toupper(*where)=='D')) return 0;//如果是delete,不作处理
        if(*where) {
          	whp=strdup(where);
          	if(!whp) {
           		return MEMERR;
            	}
        }
	p=where;
	if(srmp->befor) {
		p=stpcpy(p,srmp->befor);
		*p++ = ' ';
		srmp->befor=0;
	}
	p=stpcpy(p,"DELETE ");
	if(srmp->hint && *srmp->hint) {
		p=stpcpy(p,srmp->hint);
		*p++ = ' ';
		srmp->hint=NULL;
	}
	p=stpcpy(p,"FROM ");
	if(DBOWN && *DBOWN &&  !strchr(srmp->tabname,'.')) {
		p=stpcpy(p,DBOWN);
		*p++ = '.';
	}
	p=stpcpy(p, srmp->tabname);
	*p++ = ' ';
	*p=0;
	if(whp) {
         	p=stpcpy(p,whp);
           	free(whp);
	}
	return 0;
}

void PatternFree(SRM *srmp)
{
int i;
	if(srmp->Aflg<=0) {
		srmp->Aflg=0;
		srmp->pks=0;
		if(srmp->tp) {
			free(srmp->tp);
			srmp->tp=NULL;
		}
		return;
	}
	if(srmp->tp) {
		for(i=0;i<srmp->Aflg;i++) {
			if(srmp->tp[i].name) free((char *)srmp->tp[i].name);
			if(srmp->tp[i].format) free((char *)srmp->tp[i].format);
		}
		free(srmp->tp);
		srmp->tp=0;
	}
	if(srmp->rec) free(srmp->rec);
	srmp->rec=0;
	if(srmp->pks) free((char*)srmp->pks);
	srmp->pks=0;
	srmp->Aflg=0;
}

void SRM_free(SRM *srmp)
{
	if(!srmp) return;
	if(srmp->result) free(srmp->result);
	srmp->result=0;
	srmp->rp=0;
	if(srmp->colidx) free(srmp->colidx);
	srmp->colidx=0;
	if(srmp->tp) clean_bindtype(srmp->tp,ALL_BINDTYPE);
	PatternFree(srmp);
}
