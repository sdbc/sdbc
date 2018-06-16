/*******************************************************
 * Secure Database Connect
 * SDBC 7.0 for ORACLE 
 * 2012.9.19 by ylh
 *******************************************************/

#ifndef PACKDEF
#define PACKDEF

#include <sys/types.h>
#include <datejul.h>

#define CH_BYTE 0
#define CH_CHAR 1
#define CH_TINY 2
#define CH_SHORT 3
#define CH_INT 4
#define CH_LONG 5
#define CH_INT64 6
#define	CH_FLOAT 7
#define CH_DOUBLE 8
#define CH_LDOUBLE 9
#define CH_STRUCT 125
#define CH_CLOB	126

extern const char NAN_NULL[sizeof(double)];

#define DOUBLENULL (*(double *)NAN_NULL)
#define FLOATNULL (*(float *)NAN_NULL)
#define SHORTNULL (short)(1<<(8*sizeof(short)-1)) 
#define INTNULL   (int)(1<<(8*sizeof(int)-1)) 
#define LONGNULL  (1L<<(8*sizeof(long)-1))
#define INT64NULL  ((INT64)1<<(8*sizeof(INT64)-1))
#define TINYNULL  0X80


/*****************************************************
 * define extend data type for net_pack()
 *****************************************************/
#define CH_DATE		(CH_CHAR|0x80) 
#define CH_CNUM		(CH_CHAR|0x100) 
#define CH_JUL  	(CH_INT4|0X80)
#define CH_MINUTS	(CH_INT4|0x100)
#define CH_TIME		(CH_INT64|0x80)
#define CH_USEC		(CH_INT64|0x200)
/* used by varchar2 as date(year to day),default format is "YYYYMMDD" */
#define CH_CJUL		(CH_INT4|0x200)
#define CH_CMINUTS	(CH_INT4|0x400)
/* used by varchar2 as date(year to sec),default format is "YYYYMMDDHH24MISS" */
#define CH_CTIME	(CH_INT64|0x100)

#define YEAR_TO_DAY_LEN 11
#define YEAR_TO_MIN_LEN 17
#define YEAR_TO_SEC_LEN 20
#define YEAR_TO_USEC_LEN 27

extern const char YEAR_TO_DAY[];
extern const char YEAR_TO_MIN[];
extern const char YEAR_TO_SEC[];
extern const char YEAR_TO_USEC[];
//这个就是所谓的“模板”了，它使我们能够"看"到未知结构的内容。
typedef struct {
	    INT4 type;
	    INT4 len; // in byte
	    const char *name;
	    const char *format;
	    INT4 offset;
	    int bindtype; //default=0
} T_PkgType;

//T_PkgType 的模板
extern T_PkgType tpl_tpl[];

// for bindtype
#define RETURNING 1     //this column only for returnning
#define NOINS	  2	//this column don't insert or update
#define NOSELECT  4     //this column don't select only used by bind

#ifdef NETVAR
#define VAR
#else
#ifdef __cplusplus
#define VAR extern "C" 
#else
#define VAR extern
#endif
#endif


VAR T_PkgType ByteType[];
VAR T_PkgType CharType[];
VAR T_PkgType TinyType[];
VAR T_PkgType ShortType[];
VAR T_PkgType IntType[];
VAR T_PkgType LongType[];
VAR T_PkgType DoubleType[];
VAR T_PkgType FloatType[];
#define MINUTSNULL INTNULL

extern T_PkgType SqlVarType[];/* in net_pack.c*/
typedef struct {
    char  sqlname[49];
    int sqltype;
    int sqllen;
    char  sqlformat[49];
} T_SqlVar;

extern T_PkgType SqlDaType[]; /* in pack.c */

typedef struct {
    int  cursor_no;
    int cols;
    T_SqlVar *sqlvar;
} T_SqlDa;

#ifdef __cplusplus
extern "C" {
#endif

/*********************************************
 * netvar.c
 *********************************************/
int set_offset(T_PkgType *pkg_type);
int isnull(void *vp,int type);

/***************************************************************
 * char *str="aaa|bbb||123||";
 * make values to char *values
 * values:  'aaa','bbb','',123,null
 **************************************************************/
extern char *mkvalues(char *values,char *str, T_PkgType *tp);
/* 直接从结构中生成Values
*/

extern char *mk_values(char *values,void  *data, T_PkgType *tp);
/* 生成一个vALUES项，注意，typ是模板中的一个单项 返回value的尾部 */
extern char *mkvalue(char *value,char *data,T_PkgType *typ);

/*******************************************************
 * 模版拷贝，choose是选择符，如果为空，全拷贝
 * choose:"0-3,6-11",按列号选择。"devid,flag"按列名选择
 * dest必须具有足够的空间
 *******************************************************/
int patt_copy_col(T_PkgType * dest,T_PkgType * src,const char *choose,char *colidx);
#define patt_copy(dest,src,choose) patt_copy_col((dest),(src),(choose),0)


/*************************************************************
 * create for UPDATE SET SUBstmt or INSERT
 * returned str="...,...,.."
 ************************************************************/
extern char *mkset(char *str, T_PkgType *tp);

/*************************************************************
 * create for SELECT
 * if tabname is null returned field="...,...,.."
 * else  returned field="tabname.field1,tabname.field2,..."
 ************************************************************/
extern char *mkfield(char *field, T_PkgType *tp,const char *tabname);

/*************************************************************
 * create UPDATE SUBstmt for ORACLE
 * data="...|...|...|"
 * returned str="SET(..,..,..)=(SELECT ..,..,.. FROM DUAL)"
 *************************************************************/
extern char *mkupdate(char *str, char *data,T_PkgType *tp);
/* 直接从结构中生成 Update
*/
extern char *mk_update(char *str,void  *data,T_PkgType *tp);


/*************************************************************
 * pkg_pack 2009.5.13 by ylh
 *************************************************************/
extern int pkg_dispack(void *net_struct,char *buf,T_PkgType *pkg_type,char delimit);
#define net_dispack(stu,buf,tpl) pkg_dispack((stu),(buf),(tpl),'|')
extern int pkg_pack(char *buf,void *net_struct,T_PkgType *pkg_type,char delimit);
#define net_pack(buf,stu,tpl) pkg_pack((buf),(stu),(tpl),'|')
extern int get_one_str(char *str,void *data,T_PkgType *pkg_type,char dlmt);
#define get_one(str,data,pkg_type,i,dlmt) get_one_str((str),(data),&(pkg_type)[(i)],(dlmt))
extern int put_str_one(void *data,char *str,T_PkgType *pkg_type,char dlmt);
#define put_one(data,str,pkg_type,i,dlmt) put_str_one((data),(str),&(pkg_type)[(i)],(dlmt))
/*******************************************
 * 找别名
 *******************************************/
extern const char *plain_name(const char *name);
extern int pkg_getnum(const char *key,T_PkgType *tpl);
extern T_PkgType * pkg_getType(const char *key,T_PkgType *tpl);
/* getitem: if key exist return content(in buf),else return NULL */
extern char *getitem_idx(char *buf,void *stu,T_PkgType *tpl,const char *key,const char *colidx,int colnum);
#define getitem(buf,stu,tpl,key) getitem_idx((buf),(stu),(tpl),(key),0,0)
/* putitem: if key exist return fieldnumber(>=0),else return -1 */
extern int putitem_idx(void *stu,char *buf,T_PkgType *tpl,const char *key,const char *colidx,int colnum);
#define putitem(stu,buf,tpl,key) getitem_idx((stu),(buf),(tpl),(key),0,0)
extern int data_init(void *data,T_PkgType *type);
extern char *mk_col_idx(T_PkgType *tpl);
/* 双写src中的单引号 返回尾部 */
char * ext_copy(char *dest,const char *src);
/* 判断格式语句是否timestamp 类型 */
char * is_timestamp(const char *format);
extern int index_col(const char *idx,int colnum,const char *key,T_PkgType *tp);
/* 从模板中选取除去except之外的列名->buf,返回列数 */
int except_col(char *buf,T_PkgType *tp,const char *except);
/* 清除模板中的绑定标志，flg=NOINS or RETURNING or NOSELECT */
#define ALL_BINDTYPE -1
void clean_bindtype(T_PkgType *tp,int flg);
int set_bindtype_idx(T_PkgType *tp,int bindtype,const char *choose,int cols,char *idx);

#ifdef __cplusplus
}
#endif
/* set_bindtype():设置模板中的bindtype,choose是列名列表,NULL全部列。
   bindtype可以是0,NOSELECT,NOINSERT,NOSELECT|NOINSERT....
*/
#define set_bindtype(tp,bindtype,choose) set_bindtype_idx((tp),(bindtype),(choose),0,NULL)

#endif

