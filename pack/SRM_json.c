#include <SRM.h>
#include <SRM_json.h>

JSON_OBJECT  SRM_toJSON(SRM *srmp,JSON_OBJECT json,const char *choose)
{
	if(!srmp || !json) return NULL;
	if(!srmp->tp || !srmp->rec) return NULL;
	return stu_to_json(json,srmp->rec,srmp->tp,choose,srmp->colidx);
}

int SRM_fromJSON(SRM *srmp,JSON_OBJECT json)
{
	if(!srmp || !json) return -1;
	if(!srmp->tp || !srmp->rec) return MEMERR;
	return json_to_struct(srmp->rec,json,srmp->tp);
}
/* 将src中的同名成员拷贝到desc,格式自动转换 */
int SRM_copy(SRM *desc,SRM *src,const char *choose)
{
JSON_OBJECT json;
int ret;
	if(!desc || !src) return -1;
	if(!src->tp || !src->rec) return MEMERR;
	if(!desc->tp || !desc->rec) return MEMERR;
	json=json_object_new_object();
	if(!json) return MEMERR;
	ret=json_to_struct(desc->rec,
		stu_to_json(json,src->rec,src->tp,choose,src->colidx),
		desc->tp);
	json_object_put(json);
	return ret;
}

static int mk_srm(SRM *srmp)
{
int j;

	srmp->Aflg=set_offset(srmp->tp);
        srmp->rec=(char *)malloc(srmp->tp[srmp->Aflg].offset+1);
        if(!srmp->rec) {
          for(j=0;j<srmp->Aflg;j++) {
                if(srmp->tp[j].name) free((char *)srmp->tp[j].name);
                if(srmp->tp[j].format) free((char *)srmp->tp[j].format);
          }
          free(srmp->tp);
          ShowLog(1,"%s:  malloc fault!",__FUNCTION__);
          srmp->tp=NULL;
          return MEMERR;
        }
	srmp->result=0;
        srmp->rp=0;
        srmp->befor=0;
        srmp->hint=0;

	srmp->colidx=mk_col_idx(srmp->tp);
	srmp->tabname=srmp->tp[srmp->Aflg].name;
	srmp->pks=srmp->tp[srmp->Aflg].format;
	return 0;
}

int SRM_tpl(SRM *srmp,JSON_OBJECT tpl_json)
{
	srmp->tp=new_tpl_fromJSON(tpl_json);
	if(!srmp->tp) return -1;
        return mk_srm(srmp);
}

int SRM_mk(SRM *srmp,const char *tabname)
{
	if(!srmp || !tabname || !*tabname) return -1;
	if(!(srmp->tp=mk_tpl(tabname))) {
		return -2;
	}
        return mk_srm(srmp);
}

