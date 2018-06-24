
#ifndef SRMDEF

#define SRMDEF

#include <pack.h>

typedef struct {
	T_PkgType	*tp;			//模板
	char 		*result;		//结果集
	char		*rp;			//结果集解析指针
	void		*rec;			//数据记录
	const char	*hint;			//select之后的提示，如，distinct,/*+rule*/
	int		Aflg;			//列数，+tp，rec为存储分配
	const char	*pks;			//主键
	const char	*tabname;
	char		*colidx;
	const char	*befor;			//主语句前置内容，如 with...as...select...
} SRM;

/* 把SRM模板按照选择符生成子模板 */
#define SRM_patt_copy(srm,tp,choose) patt_copy_col((tp),(srm).tp,(choose),(srm).colidx)

/* SRM_setBind:设置模板中的bindtype,choose是列名列表,NULL全部列。
   bindtype可以是0,NOSELECT,NOINSERT,NOSELECT|NOINSERT....
*/
#define SRM_setBind(srm,bindtype,choose) set_bindtype_idx((srm)->tp,(bindtype),(choose),abs((srm)->Aflg),(srm)->colidx)

#ifdef __cplusplus
extern "C" {
#endif

void SRM_init(SRM *srmp,void *record,T_PkgType *tp);
void SRM_free(SRM *srmp);
void PartternFree(SRM *srmp);
int mk_sdbc_type(char *type_name);

/* 按照key取模板 */
T_PkgType *SRM_getType(SRM *srmp,const char *key);

int SRM_pkg_pack(SRM *srmp,char *buf,char dlimit);
int SRM_pkg_dispack(SRM *srmp,char *buf,char dlimit);
#define SRM_pack(srmp,buf) SRM_pkg_pack((srmp),(buf),'|')
#define SRM_dispack(srmp,buf) SRM_pkg_dispack((srmp),(buf),'|')
char * SRM_getString(SRM *srmp,char *buf,char *key);
int SRM_setString(SRM *srmp,char *buf,char *key);
int SRM_getOne(SRM *srmp,char *buf,int idx);
int SRM_putOne(SRM *srmp,char *buf,int idx);
/**
 *  以下函数取数据项的指针，数据类型、长度由应用软件负责
 */
// 按名字取指针，不合适的列名返回空指针
void *SRM_getP_by_key(SRM *srmp,const char *key);
//按列号取指针，不合适的列号返回空指针
void *SRM_getP_by_index(SRM *srmp,int idx);

/* 生成bind WHERE 子句 */
char * mk_where(const char *keys,char *stmt);
char * SRM_mk_returning(SRM *srmp,const char *keys,char *stmt);
int SRM_mk_select(SRM *srmp,char *DBOWN,char *where);
int SRM_mk_delete(SRM *srmp,char *DBOWN,char *where);
//生成半个UPDATE语句:"UPDATE DBOWN.TABNAME "
char * SRM_mk_update(SRM *srmp,char *DBOWN,char *where);
/* 对选择的列构建update语句，如果choose为空，全部列 ,返回尾部 */
char * SRM_mk_upd_col(SRM *srmp,char *DBOWN,const char *choose,char *stmt);

#ifdef __cplusplus
}
#endif

#define SRM_RecSize(srmp) ((srmp)->tp[abs((srmp)->Aflg)].offset)
#define SRM_except_col(srmp,buf,except) except_col((buf),((srmp)->tp),(except))

#endif

