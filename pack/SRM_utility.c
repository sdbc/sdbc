#include <SRM.h>

T_PkgType *SRM_getType(SRM *srmp,const char *key)
{
int i;
	if(!srmp) return NULL;
	i=index_col(srmp->colidx,abs(srmp->Aflg),(char *)key,srmp->tp);
	if(i<0) return NULL;
	return &srmp->tp[i];
}

int SRM_pkg_pack(SRM *srmp,char *buf,char dlimit)
{
	if(!srmp || !buf) return -1;
	if(!srmp->tp || !srmp->rec) return MEMERR;
	return pkg_pack(buf,srmp->rec,srmp->tp,dlimit);
}

int SRM_pkg_dispack(SRM *srmp,char *buf,char dlimit)
{
	if(!srmp || !buf) return -1;
	if(!srmp->tp || !srmp->rec) return MEMERR;
	return pkg_dispack(srmp->rec,buf,srmp->tp,dlimit);
}

char * SRM_getString(SRM *srmp,char *buf,char *key)
{
	if(!srmp || !buf) return NULL;
	if(!srmp->tp || !srmp->rec) return NULL;
	return getitem_idx(buf,srmp->rec,srmp->tp,key,srmp->colidx,abs(srmp->Aflg));
}

int SRM_setString(SRM *srmp,char *buf,char *key)
{
	if(!srmp || !buf) return -1;
	if(!srmp->tp || !srmp->rec) return MEMERR;
	return putitem_idx(srmp->rec,buf,srmp->tp,key,srmp->colidx,abs(srmp->Aflg));
}

int SRM_getOne(SRM *srmp,char *buf,int idx)
{
	if(!srmp || !buf) return -1;
	*buf=0;
	if(idx<0 || idx >= abs(srmp->Aflg)) return -1;
	return get_one_str(buf,srmp->rec,&srmp->tp[idx],0);
}

int SRM_putOne(SRM *srmp,char *buf,int idx)
{
	if(!srmp || !buf) return -1;
	if(!srmp->tp || !srmp->rec) return MEMERR;
	if(idx<0 || idx >= abs(srmp->Aflg)) return -1;
	return put_str_one(srmp->rec,buf,&srmp->tp[idx],0);
}

void *SRM_getP_by_index(SRM *srmp,int idx)
{
char *p;
	if(!srmp) return NULL;
	if(!srmp->tp || !srmp->rec) return NULL;
	if(idx<0 || idx >= abs(srmp->Aflg)) return NULL;
	p=(char *)srmp->rec+srmp->tp[idx].offset;
	return (void *)p;
}

void *SRM_getP_by_key(SRM *srmp,const char *key)
{
char *p;
int n;

	if(!srmp) {
		ShowLog(1,"%s:srmp is null",__FUNCTION__);
		return NULL;
	}
	if(!srmp->tp || !srmp->rec) {
		ShowLog(1,"%s:tp is nnull",__FUNCTION__);
		return NULL;
	}
	n=index_col(srmp->colidx,abs(srmp->Aflg),key,srmp->tp);
	if(n<0) return NULL;
	p=(char *)srmp->rec+srmp->tp[n].offset;
	return (void *)p;
}


