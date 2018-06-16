#include <SRM.h>
#include <json_pack.h>

#ifndef SRM_JSON
#define SRM_JSON

#ifdef __cplusplus
extern "C" {
#endif

JSON_OBJECT SRM_toJSON(SRM *srmp,JSON_OBJECT json,const char *choose);
int SRM_fromJSON(SRM *srmp,JSON_OBJECT json);
/* 将src中的同名成员拷贝到desc,格式自动转换 */
int SRM_copy(SRM *desc,SRM *src,const char *choose);

//根据JSON模板构建srm,成功返回0
int SRM_tpl(SRM *srmp,JSON_OBJECT tpl_json);
//从模板库构建srm,成功返回0
int SRM_mk(SRM *srmp,const char *tabname);

#ifdef __cplusplus
}
#endif

#endif
