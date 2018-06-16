/*****************************************
 * BULK.h ORACLE BULK read 
 * SDBC中ORACLE成组操作所需数据结构    
 * bind时需要的表示器、列级状态，列长度    
 * define时需要的表示器、列级状态，列长度    
 * 以及在SDBC中不能与ORACLE直接对应的类型   
 * 需要额外的字符缓冲区  
 * 本.h在#include <DAU.h> 之后使用    
 *****************************************/

#ifndef BULK_A_COL_LEN

#define BULK_A_COL_LEN 32

typedef struct {	//参与bind或define的，每列一个 
	char  *name;	//列名  
	short *ind;	//表示器, malloc(max_rows_of_batch * sizeof(short));
	short *r_code;	//列级状态码,  字符列用,      malloc(max_rows_of_batch * sizeof(short));
	short *r_len;	//返回的列长度,define_by_pos 字符列用,   malloc(max_rows_of_fetch * sizeof(short));
	char *a_col; 	//额外的字符缓冲区 malloc(max_rows_of_batch * BULK_A_COL_LEN); 
} col_bag;			// 列包   

typedef struct ora_bulk_desc {
	SRM *srm;			//SRM
	T_SQL_Connect *SQL_Connect;	//数据库句柄
	sqlo_stmt_handle_t sth;		//游标
	int bind_rows;			//绑定条件的最大行数
	int max_rows_of_fetch;		//每批读取的最大行数
	int cols;			//参与bind的列数(输入条件)
	int dcols;			//参与define的列数(输出结果)
	col_bag *cb;			//malloc(cols * sizeof(col_bag));
	col_bag *dcb;			//malloc(dcols * sizeof(col_bag));
	T_Tree *bind_tree;		//绑定树
	void *recs;			//数据记录 
	int prows;			//累计行数 
	int rows;			//实际操作的行数 
	int reclen;			//记录长度 
	int pos;			//BULK池使用
} BULK;

#define BULK_get_DAU(bulkp) (DAU *)((bulkp)->srm)

#ifdef __cplusplus
extern "C" {
#endif

//BULK_init后，DAU不可释放,BULK_free不释放DAU  
void BULK_init(BULK *bulk,DAU *DP,void *recs,int max_rows_of_fetch);
void BULK_free(BULK *bulk);
int BULK_prepare(BULK *bulkp,char *stmt,int bind_rows);
int BULK_fetch(BULK *bulk);
char * BULK_pkg_dispack(BULK *bulk,int n,char *buf,char delimit);
char * BULK_pkg_pack(BULK *bulk,int n,char *buf,char delimit);

#ifdef __cplusplus
}
#endif

#endif

