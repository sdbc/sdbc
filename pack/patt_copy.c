#include <stdio.h>
#include <ctype.h>
#include <pack.h>

static void tp_cp(T_PkgType * dest,T_PkgType * src,int i)
{
T_PkgType *tp;

	tp=&src[i];
	dest->type=tp->type;
	dest->len=tp->len;
	dest->name=tp->name;
	dest->format=tp->format;
	dest->offset=tp->offset;
	dest->bindtype=0;
}
/*******************************************************************************
 * C struct to json object by SDBC parttention
 *******************************************************************************/

int patt_copy_col(T_PkgType * dest,T_PkgType * src,const char *choose,char *idxcol)
{
int i,num;
T_PkgType *tp,*dp;
char buf[100];
const char *cp;
int colnum;

	if(!src || !dest) return 0;
	tp=src;
	dp=dest;
	if(!choose||!*choose) {		// 没有选择，全部加入
	    for(i=0;tp[i].type>=0;i++,dp++) {
		tp_cp(dp,tp,i);
	    }
	    tp_cp(dp,tp,i);
	    return i;
	}
	num=0;
	cp=choose;
	colnum=set_offset(src);
	do {
	char *p;
		*buf=0;
		cp=stptok(skipblk((char *)cp),buf,sizeof(buf),",|");	//可以用的分隔符
		p=buf;
		TRIM(p);
		if(!*p) continue;
		if(isdigit(*p)) {	//数字选择，字段号范围
		int ret,beg,end;
			end=beg=-1;
			ret=sscanf(buf,"%d - %d",&beg,&end);
			if(!ret) continue;
			if(ret==1) end=beg;
			if(end<beg) end=beg;
			for(ret=beg;ret<=end;ret++,dp++) {
	    			tp_cp(dp,tp,ret);
				num++;
			}
		} else {		//字母，字段名
			i=index_col(idxcol,colnum,p,src);
			if(src[i].type<0) continue;
	    		tp_cp(dp,tp,i);
			dp++;
			num++;
		}
	} while(*cp++);
	tp_cp(dp,tp,colnum);
	return num;
}

