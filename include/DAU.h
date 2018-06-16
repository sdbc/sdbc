/* DAU=Data Access Utility 
	2008-11-18 by ylh */


#ifndef DAUDEF

#define DAUDEF
#include <SRM.h>
#include <sqli.h>
#include <BB_tree.h>

typedef struct {
	SRM			srm;		//SRM=Struct Relational Map
	T_SQL_Connect		*SQL_Connect;
	char 			*tail;		//bind用临时指针
	sqlo_stmt_handle_t	cursor;		//prepare cursor
	T_Tree			*bt_pre;	//prepare bind tree
	sqlo_stmt_handle_t	ins_sth;	//insert cursor
	T_Tree			*bt_ins;	//insert bind tree
	sqlo_stmt_handle_t	upd_sth;	//update cursor
	T_Tree			*bt_upd;	//update bind tree
	sqlo_stmt_handle_t	del_sth;	//delete cursor
	T_Tree			*bt_del;	//delete bind tree
	unsigned int		pos;            //DAU池使用
} DAU;

// set col_to_lower = 1, 生成模板时列名改小写。
extern char col_to_lower;

#ifdef __cplusplus
extern "C" {
#endif

/********************************************
 * 初始化DAU，关联数据库和record
 ********************************************/
int DAU_init(DAU *DP,T_SQL_Connect *SQL_Connect,const char *tabname,void *record,T_PkgType *tp);

//msg 用于生成SQL语句和返回错误信息
int DAU_insert(DAU *DP,char *msg);

//where是where子句，同时利用这个空间生成完整的SQL语句
//如果已经是update开头的，就直接利用，不生成了
int DAU_update(DAU *DP,char *where);

//where是where子句，同时利用这个空间生成完整的SQL语句
//如果已经是delete开头的，就直接利用，不生成了
int DAU_delete(DAU *DP,char *where);
/****************************************************
 * 调用select语句，
 * where是where子句，同时利用这个空间生成完整的SQL语句
 * 如果已经是select开头的，就直接利用，不生成了
 ****************************************************/
int DAU_select(DAU *DP,char *where,int rownum);

/*****************************************************
 * 以游标方式打开结果集
 * where 用于生成语句， 还要利用这个空间生成完整的SQL语句
 * 同时也是存放Fetch结果集的buffer
 * 如果已经是select开头的，就直接利用，不生成了
 ****************************************************/
int DAU_prepare(DAU *DP,char *where);

//取结果集里的下一条记录，成功返回0
//能够自动识别DAU是select的还是prepare的
int DAU_next(DAU *DP);

/*****************************************************
 * 执行无返回结果集的SQL语句，使用DAU_delete的游标，
 * 不要与DAU_delete同时使用
 * DAU中的结构和模板需满足bind的需要。
 * 涉及表名的$DB.符号将用DBOWN取代。
 *****************************************************/
int DAU_exec(DAU *DP,char *stmt);
int bind_exec(DAU *DP,char *stmt);

/*********************************************
 * 多表的select
 * where是where子句，同时利用这个空间生成完整的SQL语句
 * 如果已经是select开头的，就直接利用，不生成了
 * 不支持bind 
 *********************************************/
int DAU_getm(int n,DAU DP[],char *where,int rownum);

//多表的select, 取结果集里的下一条记录，成功返回0
int DAU_nextm(int n,DAU *DP);

//释放DAU，在游标状态下关闭游标
void DAU_free(DAU DP[]);
extern void  DAU_freem(int n,DAU *DP);

/* 关于主键的函数，如果除主键外还有其他条件，放在stmt，否则*stmt=0 */
extern int select_by_PK(DAU *DP,char *stmt);
extern int prepare_by_PK(DAU *DP,char *stmt);
extern int update_by_PK(DAU *DP,char *stmt);
extern int delete_by_PK(DAU *DP,char *stmt);
/* 假修改，为解决插入时重码速度很慢的问题，注意与其他修改条件的冲突 */
extern int dummy_update(DAU *DP,char *stmt);
// at SRM_to_DAU.c
extern int SRM_to_DAU(DAU *DP,T_SQL_Connect *SQL_Connect,SRM *srmp);
extern int DAU_to_SRM(DAU *DP,SRM *srmp);
extern int SRM_next(SRM *srmp);
// at bind.c
extern int bind_ins(DAU *DP,char *buf);
extern int bind_select(DAU *DP,char *stmt,int recnum);
extern int print_bind(void *content);
/**
 * 打印 bind 值，在stmt中 
 */
extern int DAU_print_bind(DAU *DP,char *stmt);

int get_upd_returning(DAU *DP);
// at DAU_mk.c 用模板库生成DAU
int DAU_mk(DAU *DP,T_SQL_Connect *SQL_Connect,const char *tabname);

#ifdef __cplusplus
}
#endif

//生成半个UPDATE语句:"UPDATE DBOWN.TABNAME "
#define DAU_mk_update(DP,buf) SRM_mk_update(&(DP)->srm,(DP)->SQL_Connect->DBOWN,(buf))
/* 对选择的列构建update语句，如果choose为空，全部列 ,返回尾部 */
#define DAU_mk_upd_col(DP,choose,stmt) SRM_mk_upd_col(&(DP)->srm,(DP)->SQL_Connect->DBOWN,(choose),(stmt));

#define DAU_mk_delete(DP,where) SRM_mk_delete(&(DP)->srm,(DP)->SQL_Connect->DBOWN,where)

#define DAU_pkg_pack(DP,buf,dlimit) SRM_pkg_pack(&(DP)->srm,(buf),(dlimit))
#define DAU_pkg_dispack(DP,buf,dlimit) SRM_pkg_dispack(&(DP)->srm,(buf),(dlimit))
#define DAU_pack(DP,buf) SRM_pack(&(DP)->srm,(buf))
#define DAU_dispack(DP,buf) SRM_dispack(&(DP)->srm,(buf))
#define DAU_getString(DP,buf,key) SRM_getString(&(DP)->srm,(buf),(key))
#define DAU_setString(DP,buf,key) SRM_setString(&(DP)->srm,(buf),(key))
#define DAU_getOne(DP,buf,key) SRM_getOne(&(DP)->srm,(buf),(key))
#define DAU_putOne(DP,buf,key) SRM_putOne(&(DP)->srm,(buf),(key))

//#define DAU_patt_copy(DP,tpl,choose) SRM_patt_copy(&(DP)->srm,(tpl),(choose))

/**
 *  以下函数取数据项的指针，数据类型、长度由应用软件负责
 */
// 按名字取指针，不合适的列名返回空指针
#define DAU_getP_by_key(DP,key) SRM_getP_by_key(&(DP)->srm,(key))
//按列号取指针，不合适的列号返回空指针
#define DAU_getP_by_index(DP,idx) SRM_getP_by_index(&(DP)->srm,(idx))

/* 将src中的同名成员拷贝到desc,格式自动转换 */
#define DAU_copy(desc,src,choose) SRM_copy(&(desc)->srm,&(src)->srm,(choose))

/* 带RETURNING子句的插入语句 */
#define DAU_ins_returning(DP,stmt) bind_ins((DP),(stmt))
/* 生成RETURNING子句，返回尾部 */
#define DAU_mk_returning(DP,choose,stmt) SRM_mk_returning(&(DP)->srm,(choose),(stmt))
/* DAU_setBind():设置模板中的bindtype,choose是列名列表,NULL全部列。
   bindtype可以是0,NOSELECT,NOINSERT,NOSELECT|NOINSERT....
*/
#define DAU_setBind(DP,bindtype,choose) SRM_setBind(&(DP)->srm,(bindtype),(choose))

#define DAU_RecSize(DP) SRM_RecSize(&(DP)->srm)
/* 按照key取模板 */
#define DAU_getType(DP,key) SRM_getType(&(DP)->srm,(key))
#define DAU_getRec(DP) ((DP)->srm.rec)
#define DAU_except_col(DP,buf,except) SRM_except_col(&(DP)->srm,(buf),(except))

#endif
