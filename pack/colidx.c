/********************************************
 * 快速查找模板，利用hash算法
 *******************************************/
//#include <alloca.h>
#include <pack.h>

typedef struct {
	char *name;
	int colno;
	int link;
} colidx;

typedef struct {
	char *name;
	int colno;
	int hash;
} hashnode;

static int hash_name(const char *name,int mod)
{
register int hashval=0;
	if(!mod) return -1;
	while(*name) hashval += *(unsigned char *)name++; 
	return (hashval&0x7FFFFFFF)%mod;
}
//产生索引 
char *mk_col_idx(T_PkgType *tpl)
{
int coln,i;
colidx *colp;
T_PkgType *tp;
int *lp;

	coln=set_offset(tpl);
	colp=(colidx *)malloc(coln * sizeof(colidx));
	if(!colp) return NULL;

//#ifdef __GNUC__
hashnode *top,stack[coln];
//#else
//hashnode *top,*stack=(struct hashnode *)alloca(coln * sizeof(hashnode));
//	if(!stack) {
//		free(colp);
//		return NULL;
//	}
//#endif

	top=stack;
	for(i=0;i<coln;i++) {
		colp[i].colno=-1;
		colp[i].link=-1;
	}
	i=0;
	for(tp=tpl;tp->type>=0;tp++,i++) {
	char *p=(char *)plain_name(tp->name);
	int hashnum=hash_name(p,coln);
	    if(colp[hashnum].colno==-1) {	//没有散列冲突 
		colp[hashnum].name=p;
		colp[hashnum].colno=i;
	    } else {				//有散列冲突，存储冲突链
//printf("%s:name[%d]=%s,%d,--%s\n",__FUNCTION__,i,p,hashnum,colp[hashnum].name);
		top->name=p;
		top->colno=i;
		top->hash=hashnum;
		top++;
	    }
	}
	if(top != stack) { //有散列冲突，构建冲突链 
		for(i=0;i<coln;i++) {
			if(colp[i].colno != -1) continue;
			top--;
//找到索引表里的空项 
			colp[i].name=top->name;
			colp[i].colno=top->colno;
/*
			for(lp=&colp[top->hash].link;*lp != -1;lp=&colp[*lp].link)
				;
*/
			lp=&colp[top->hash].link;
			colp[i].link=*lp;
			*lp=i;
		}
	}

	return (char *)colp;
}
//查找列号 
int index_col(const char *idx,int colnum,const char *key,T_PkgType *tp)
{
register colidx *cp;
#define colp ((colidx *)(idx))
	
	if(!tp) return -1;
	if(!idx) {
	int i;
		i=pkg_getnum(key,tp);
		if(tp[i].type==-1) return -1;
		return i;
	}
	for(cp=colp+hash_name(key,colnum);
	   strcmp(cp->name,key); cp=colp+cp->link) {
		if(cp->link==-1) return -1;
	} 
	return cp->colno;
}

