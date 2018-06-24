#ifndef JSON_PACK
#define JSON_PACK

#ifndef  __STRICT_ANSI__
#define __STRICT_ANSI__
#endif

#include <json.h>
/**********************************************************************************
 * 这一组工具函数用于SDBC与JSON之间的数据转换。
 * 原有SDBC模板的name可以是各种内容，如：字段名称、常数、函数，或decode语句等等,
 * 当使用本组函数，要求name必须是JSON名字格式。
 *******************************************************************************/

typedef struct json_object *JSON_OBJECT;
#define struct_to_json(json,data,typ,choose) stu_to_json((json),(data),(typ),(choose),0)
/***********************************************************************
 * 往存储分配的串里追加数据 ,多分配了100字节，事后可以追加少量数据
 ***********************************************************************/

#define add_string_to_json(json,name,val) json_object_object_add((json),(name),json_object_new_string(val))


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * C struct to json object by SDBC parttention
 * choose为NULL，选择全部字段。为"",选择全部非空字段
 * choose 可以是选择的字段名称列表，用,或|隔开。
 * 也可以是字段顺号，顺号范围，可以与名字列表混用，如："0-5,8,11,zip,code"
 *******************************************************************************/

JSON_OBJECT stu_to_json(JSON_OBJECT json,void *data,T_PkgType * typ,const char *choose,char *colidx);

/*************************************************************************
 * json_object to sdbc string...
 *************************************************************************/

char * json_to_sdbc(char *tmp,JSON_OBJECT json);

/**********************************************************************************
 * json object to C struct by SDBC parrtention
 **********************************************************************************/

int json_to_struct(void *data,JSON_OBJECT json,T_PkgType *typ);

char * json_get_string(JSON_OBJECT from,const char *key);
int json_to_Str(char *buf,JSON_OBJECT json,T_PkgType *tp);

//T_PkgType --> JSON result:[]
int tpl_to_JSON(T_PkgType *tp,JSON_OBJECT result);
JSON_OBJECT jerr(int jerrno,const char *errmsg);
//失败返回NULL，成功后，需应用者自行释放
T_PkgType * new_tpl_fromJSON(JSON_OBJECT tpl_json);
// at tpl_lib.c
T_PkgType * mk_tpl(const char *tabname);
int tpl_to_lib(T_PkgType *tpl,const char *tabname);
void tpl_lib_cancel(void);

#ifdef __cplusplus
}
#endif

#endif
