/*********************************************************
 * DAU的C++接口
 *********************************************************/

#ifndef DAUDEF

#include <DAU.h>
#include <DAU_json.h>

class SdbcDAO {
	protected:
		DAU dau;
	public:
//构造函数和析构函数
//空模板
	SdbcDAO(void)
        {
            DAU_init(&dau,NULL,NULL,NULL,NULL);
        }

//从数据库获取模板
	SdbcDAO(T_SQL_Connect *SQL_Connect,char *tabname)
        {
            int ret;

            ret=DAU_mk(&dau,SQL_Connect,tabname);
            if(ret<0) {
                throw Showid;
            }
        }
//使用静态模板，并从模板获取表名
	SdbcDAO(T_SQL_Connect *SQL_Connect,void *rec,T_PkgType *tpl)
        {
            init(SQL_Connect,NULL,rec,tpl);
        }

//用表名取代模板里的表名
	SdbcDAO(T_SQL_Connect *SQL_Connect,char *tabname,void *rec,T_PkgType *tpl)
	{
            int ret;
            ret=init(SQL_Connect,tabname,rec,tpl);
            if(ret<0) {
                throw Showid;
	    }
	}
//对已存在的对象重新初始化
	int  init(T_SQL_Connect *SQL_Connect,char *tabname,void *rec,T_PkgType *tpl)
	{
	 	DAU_init(&dau,SQL_Connect,tabname,rec,tpl);
	}

	void Data_init(void)
	{
            if(dau.srm.tp) data_init(dau.srm.rec,dau.srm.tp);
	}

//析构函数
	~SdbcDAO(void)
        {
            DAU_free(&dau);
        }

//bean函数
	DAU *getDAU()
	{
	 return &dau;
	}

	T_SQL_Connect *getSqlConnect(void)
	{
	 return dau.SQL_Connect;
	}

	T_PkgType *getTemplate()
	{
	 return dau.srm.tp;
	}

	T_PkgType *getTemplate(const char *key)
	{
	int n;
           n=index_col(dau.srm.colidx,abs(dau.srm.Aflg),key,dau.srm.tp);
           if(n<0) return NULL;
           return &dau.srm.tp[n];
	}

	int getTplNo(const char *colName)
	{
	 return index_col(dau.srm.colidx,abs(dau.srm.Aflg),colName,dau.srm.tp);
	}

	int getColNum(void)
	{
	 return abs(dau.srm.Aflg);
	}

	int getRecSize(void)
	{
	 if(!dau.srm.tp) return -1; 
	 return dau.srm.tp[abs(dau.srm.Aflg)].offset;
	}

	void *getRec(void)
	{
	 return dau.srm.rec;
	}

	void *getRec(char *colname)
	{
	 return DAU_getP_by_key(&dau,colname);
	}

	void *getRec(int col_no)
	{
	 return DAU_getP_by_index(&dau,col_no);
	}

	void setHint(const char *hint)
	{
	 dau.srm.hint=hint;
	}

	void setBefor(const char *befor)
	{
	 dau.srm.befor=befor;
	}

//操作函数
	int select(char *stmt)
	{
	  return DAU_select(&dau,stmt,0);
	}

	int select(char *stmt,int recnum)
	{
	  return DAU_select(&dau,stmt,recnum);
	}

	int nextRow(void)
	{
	  return DAU_next(&dau);
	}

	int prepare(char *stmt)
	{
	  return DAU_prepare(&dau,stmt);
	}

	int insert(char *stmt)
	{
	  return DAU_insert(&dau,stmt);
	}

	int update(char *stmt)
	{
	  return DAU_update(&dau,stmt);
	}

	int Delete(char *stmt)
	{
	  return DAU_delete(&dau,stmt);
	}

	int exec(char *stmt)
	{
	  return DAU_exec(&dau,stmt);
	}

//设置选择的列输出
	int selectCol(const char *choose)
	{
	char stmt[10240];
	int ret=DAU_except_col(&dau,stmt,choose);
    		if(ret<=0) return ret;
		DAU_setBind(&dau,NOSELECT,stmt);
	}

	int distinctCol(const char *choose)
	{
	  dau.srm.hint="DISTINCT";
	  return selectCol(choose);
	}

//对选择的列构建update语句，如果choose为空，全部列 ,返回尾部
	char *mkUpdateByCol(const char *choose,char *stmt)
	{
	  return DAU_mk_upd_col(&dau,choose,stmt);
	}

	int setBind(int bindtype,const char *choose)
	{
	  return DAU_setBind(&dau,bindtype,choose);
	}

	int exceptCol(char *buf,char *choose)
	{
	  return DAU_except_col(&dau,buf,choose);
	}

//打印上述操作的绑定变量值
	int printBind(char *buf)
	{
	  return DAU_print_bind(&dau,buf);
	}

//序列化/反序列化
	int pack(char *buf)
	{
	  return DAU_pack(&dau,buf);
	}

	int pack(char *buf,char delimit)
	{
	  return DAU_pkg_pack(&dau,buf,delimit);
	}

	int unpack(char *buf)
	{
	  return DAU_dispack(&dau,buf);
	}

	int unpack(char *buf,char delimit)
	{
	  return DAU_pkg_dispack(&dau,buf,delimit);
	}

	int getOne(char *buf,T_PkgType *tpl)
	{
	  return get_one_str(buf,dau.srm.rec,tpl,0);
	}

	int getOne(char *buf,const char *col_name)
	{
	  return getOne(buf,getTemplate(col_name));
	}

	int putOne(char *buf,T_PkgType *tpl)
	{
	  return put_str_one(dau.srm.rec,buf,tpl,0);
	}

	int putOne(char *buf,const char *col_name)
	{
	  return putOne(buf,getTemplate(col_name));
	}

#ifdef DAU_JSON
	JSON_OBJECT toJSON(JSON_OBJECT json,char *choose)
	{
	  return DAU_toJSON(&dau,json,choose);
	}

	int fromJSON(JSON_OBJECT json)
	{
	  return DAU_fromJSON(&dau,json);
	}
#endif
};

#endif
