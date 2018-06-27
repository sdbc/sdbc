/*****************************************
 * OAD.h ORACLE Array Describe
 * SDBC中ORACLE成组操作所需数据结构
 * bind时需要的表示器、列级状态，列长度
 * 以及在SDBC中不能与ORACLE直接对应的类型
 * 需要额外的字符缓冲区
 * 本.h在#include <DAU.h> 之后使用
 *****************************************/

#ifndef A_COL_LEN

#define A_COL_LEN 32

typedef struct {	//参与bind的，每列一个
	char  *name;	//列名
	short *ind;		//表示器, malloc(max_rows_of_batch * sizeof(short));
	short *r_code;	//列级状态码,  字符列用,      malloc(max_rows_of_batch * sizeof(short));
	short *r_len;	//返回的列长度,define_by_pos 字符列用,   malloc(max_rows_of_batch * sizeof(short));
	char *a_col; 	//额外的字符缓冲区 malloc(max_rows_of_batch * A_COL_LEN);
} col_bag;			// 列包

typedef struct ora_array_desc {
	SRM *srm;			//SRM
	T_SQL_Connect *SQL_Connect;	//数据库句柄
	sqlo_stmt_handle_t sth;		//游标
	int max_rows_of_batch;		//每批最大行数
	int cols;			//参与bind的列数
	col_bag *cb;			//malloc(cols * sizeof(col_bag));
	T_Tree *bind_tree;		//绑定树
	void *recs;			//数据记录
	int begin;			//exec时的起始行号
	int rows;			//实际操作的行数
	int reclen;			//记录长度
	int a_col_flg;			//额外列操作
	unsigned int pos;		//OAD池使用
} OAD;

#define OAD_get_DAU(oadp) (DAU *)((oadp)->srm)

#ifdef __cplusplus
extern "C" {
#endif

//OAD_init后，DAU不可释放,OAD_free不释放DAU
void OAD_init(OAD *oad,DAU *DP,void *recs,int max_rows_of_batch);
void OAD_free(OAD *oad);
int OAD_mk_ins(OAD *oad,char *stmt);
int OAD_mk_update(OAD *oadp,char *stmt);
int OAD_mk_del(OAD *oadp,char *where);
int OAD_exec(OAD *oad,int begin,int rowno);
char * OAD_pkg_dispack(OAD *oad,int n,char *buf,char delimit);
char * OAD_pkg_pack(OAD *oad,int n,char *buf,char delimit);

#ifdef __cplusplus
}
#endif

#endif

