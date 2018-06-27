
#include <SRM.h>


static T_PkgType SDBC_type[]={
        {CH_BYTE,-1,"CH_BYTE",0,-1},
        {CH_CHAR,-1,"CH_CHAR"},
        {CH_DATE,-1,"CH_DATE"},
        {CH_CLOB,-1,"CH_CLOB"},
        {CH_STRUCT,-1,"CH_STRUCT"},
        {CH_TINY,1,"CH_TINY"},
        {CH_SHORT,sizeof(short),"CH_SHORT"},
        {CH_INT,sizeof(int),"CH_INT"},
        {CH_INT4,sizeof(INT4),"CH_INT4"},
        {CH_LONG,sizeof(long),"CH_LONG"},
        {CH_DOUBLE,sizeof(double),"CH_DOUBLE"},
        {CH_FLOAT,sizeof(float),"CH_FLOAT"},
        {CH_JUL,sizeof(INT4),"CH_JUL",YEAR_TO_DAY},
        {CH_MINUTS,sizeof(INT4),"CH_MINUTS",YEAR_TO_MIN},
        {CH_CJUL,sizeof(INT4),"CH_CJUL","YYYYMMDD"},
        {CH_CMINUTS,sizeof(INT4),"CH_CMINUTS","YYYYMMDDHH24MI"},
        {CH_INT64,sizeof(INT64),"CH_INT64"},
        {CH_TIME,sizeof(INT64),"CH_TIME",YEAR_TO_SEC},
        {CH_CTIME,sizeof(INT64),"CH_CTIME","YYYYMMDDHH24MISS"},
        {CH_USEC,sizeof(INT64),"CH_USEC","YYYY-MM-DD HH24:MI:SS.FF6"},
        {CH_CNUM,35,"CH_CNUM"},
        {-1,0}
};

int mk_sdbc_type(char *typename)
{
T_PkgType *tp;

	tp=pkg_getType(typename,SDBC_type);
	return tp->type;
}

