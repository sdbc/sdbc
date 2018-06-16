#include <stdio.h>
#include <ctype.h>
#include <pack.h>
#include <json_pack.h>

static void add_field_to_object(JSON_OBJECT json,void *data,T_PkgType *typ)
{
char buf[50], *cp,*p;
	cp=data;
	switch(typ->type&127) {
	case CH_BYTE:
		cp=malloc((typ->len<<1)+1);
		get_one_str(cp,data,typ,0);
		json_object_object_add(json,plain_name(typ->name),json_object_new_string(cp));
		free(cp);
		break;
	case CH_INT:
	case CH_LONG:
	case CH_SHORT:
	case CH_TINY:
	case CH_INT64:
	case CH_DOUBLE:
	case CH_FLOAT:
		get_one_str(buf,data,typ,0);
		json_object_object_add(json,plain_name(typ->name),json_object_new_string(buf));
		break;
	case CH_CLOB:
		p=*(char **)(cp+typ->offset);
		if(p) json_object_object_add(json,plain_name(typ->name),json_object_new_string(p));
                break;
	default:
		json_object_object_add(json,plain_name(typ->name),json_object_new_string(cp+typ->offset));
		break;
	}
}
/*******************************************************************************
 * C struct to json object by SDBC patterntion
 *******************************************************************************/

JSON_OBJECT  stu_to_json(JSON_OBJECT json,void *data,T_PkgType * typ,const char *choose,char *colidx)
{
int i,colnum,n;
T_PkgType *tp;
char buf[100];
const char *cp;

	if(!json) return json;
	colnum=set_offset(typ);
	tp=typ;
	if(!choose||!*choose) {		// 没有选择，全部加入
	    for(i=0;tp->type>=0;i++,tp++) {
			if(tp->bindtype & NOSELECT) continue;
			if(choose&&isnull((char *)data+tp->offset,tp->type)) continue;
			if(tp->type==CH_STRUCT) {
			JSON_OBJECT sub=json_object_new_object();
				stu_to_json(sub,(char *)data+tp->offset,
					(T_PkgType *)tp->format,0,0);
				json_object_object_add(json,plain_name(tp->name),sub);
				continue;
			}
			add_field_to_object(json,data,tp);
	    }
	    return json;
	}
	cp=choose;
	do {
	char *p;
		*buf=0;
		cp=stptok(skipblk((char *)cp),buf,sizeof(buf),",|");	//可以用的分隔符
		p=buf;
		TRIM(p);
		if(!*p) continue;
		if(isdigit(*p)) {	//数字选择，字段号范围:"1-3,6-11"
		int ret,beg,end;
			end=beg=-1;
			ret=sscanf(buf,"%d - %d",&beg,&end);
			if(!ret) continue;
			if(ret==1) end=beg;
			if(end<beg) end=beg;
			for(ret=beg;ret<=end;ret++) {
				if(typ[ret].type==CH_STRUCT) {
				JSON_OBJECT sub=json_object_new_object();
					stu_to_json(sub,(char *)data+typ[ret].offset,
						(T_PkgType *)typ[ret].format,0,0);
					json_object_object_add(json,plain_name(typ[ret].name),sub);
					continue;
				}
				add_field_to_object(json,data,&typ[ret]);
			}
		} else {		//字母，字段名 :"colname1,colname2,..."
			n=index_col(colidx,colnum,p,typ);
			if(n<0) continue;
			if(typ[n].type==CH_STRUCT) {
			JSON_OBJECT sub=json_object_new_object();
				stu_to_json(sub,(char *)data+typ[n].offset,
					(T_PkgType *)typ[n].format,0,0);
				json_object_object_add(json,plain_name(typ[n].name),sub);
				continue;
			}
			add_field_to_object(json,data,typ+n);
		}
	} while(*cp++);
	return json;
}

char * json_get_string(JSON_OBJECT from,const char *key)
{
JSON_OBJECT json;
    json=json_object_object_get(from,key);
    if(!json) return NULL;
    return (char *)json_object_get_string(json);
}

int json_toStr(char *buf,JSON_OBJECT json,T_PkgType *tp)
{
char *p,*p1;
int i;
T_PkgType *typ,Char_Type[2];

    if(!buf || !json) return 0;
        Char_Type[0].type=CH_CHAR;
        Char_Type[0].len=-1;
        Char_Type[1].len=0;
        Char_Type[0].offset=0;
        Char_Type[1].offset=-1;

    p=buf;
    typ=tp;
    for(i=0;typ->type >= 0;i++,typ++) {
	if(typ->bindtype & NOSELECT) continue;
	if(typ->type == CH_STRUCT) {
	JSON_OBJECT val=json_object_object_get(json,plain_name(typ->name));
		if(!val) continue;
		p+=json_toStr(p,val,(T_PkgType *)typ->format);
		continue;
	}
        p1=json_get_string(json,plain_name(typ->name));
        if(p1) p+=get_one_str(p,p1,Char_Type,'|'); // 组串时需要转义
        *p++ = '|';
        *p=0;
    }
    return p-buf;
}


/*************************************************************************
 * json_object to sdbc string...
 *************************************************************************/

char * json_to_sdbc(char *tmp,JSON_OBJECT json)
{
T_PkgType Char_Type[2];
char *p1;
char *cp=tmp;
	*tmp=0;
	if(!json) return tmp;
        Char_Type[0].type=CH_CHAR;
        Char_Type[0].len=-1;
        Char_Type[1].len=0;
        Char_Type[0].offset=0;
        Char_Type[1].offset=-1;
        *cp=0;
      {
        json_object_object_foreach(json, key, val) {
                p1= (char *)json_object_get_string(val);        //取原始串
                if(p1) {
                        cp+=get_one_str(cp,p1,Char_Type,'|'); // 组串时需要转义
                        *cp++ = '|';
                        *cp=0;
                } else cp+=sprintf(cp,"|");
        }
      }
        return tmp;
}

/**********************************************************************************
 * json object to C struct by SDBC patterntion
 **********************************************************************************/

int json_to_struct(void *data,JSON_OBJECT json,T_PkgType *typ)
{
T_PkgType *tp;
int i,num;
JSON_OBJECT val;
char *cp;
	num=0;
	if(!json) return 0;
	if(typ->offset<0) set_offset(typ);
	tp=typ;
	for(i=0;tp->type>=0;i++,tp++) {
		if(tp->type == CH_STRUCT) {
			val=json_object_object_get(json,plain_name(tp->name));
			if(!val) continue;
			json_to_struct((char *)data+tp->offset,val,(T_PkgType *)tp->format);
			continue;
		}
		if((val=json_object_object_get(json,plain_name(tp->name)))!=NULL) {
			cp=(char *)json_object_get_string(val);
			put_str_one(data,cp,tp,0);
			num++;
		}
	}
	return num;
}
/***********************************************************************
 * 往存储分配的串里追加数据 ,多分配了100字节，事后可以追加少量数据。
 ***********************************************************************/

char *strappend(char *allocated,char *tobeapp)
{
char *p;
int len;
        if(!allocated) {
		allocated=malloc(strlen(tobeapp)+100);
		if(!allocated) return NULL;
		p=allocated;
		len=0;
	} else {
		len=strlen(allocated);
        	p=realloc(allocated,strlen(tobeapp)+len+100);
        	if(!p) return(NULL);
	}
	sprintf(p+len,"%s\n",tobeapp);
        return p;
}

int tpl_to_JSON(T_PkgType *tp,JSON_OBJECT result)
{
int i;
JSON_OBJECT json;

	set_offset(tpl_tpl);
	for(i=0;tp->type>-1;i++,tp++) {
		json=json_object_new_object();
		struct_to_json(json,tp,tpl_tpl,NULL);
		json_object_array_add(result,json);
	}
	json=json_object_new_object();
	struct_to_json(json,tp,tpl_tpl,NULL);
	json_object_array_add(result,json);
	return i;
}

T_PkgType * new_tpl_fromJSON(JSON_OBJECT tpl_json)
{
int n,i,ret;
T_PkgType *typ,*tp;
JSON_OBJECT json1;
	if(!tpl_json) return NULL;
	n=json_object_array_length(tpl_json);
	if(n<=0) return NULL;
	i=sizeof(T_PkgType) * n;
	typ=(T_PkgType *)malloc(i);
	if(!typ) return NULL;
	memset(typ,0,i);
	tp=typ;
	for(i=0;i<n;i++,tp++) {
		json1=json_object_array_get_idx(tpl_json,i);
		ret=json_to_struct(tp,json1, tpl_tpl);
		if(ret<=0) {
			free(typ);
			return NULL;
		}
		if(i==0) tp->offset=-1;
		tp->name=strdup(tp->name);
		if(tp->format) {
                	if(*tp->format) tp->format=strdup(tp->format);
			else tp->format=NULL;
		}
	}
	tp[-1].type=-1;
	return typ;
}	
