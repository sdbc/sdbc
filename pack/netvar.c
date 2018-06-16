#define NETVAR
#include <pack.h>

#define M_ST_OFF(struct_type, member)    \
    	(int)((long)(void *) &(((struct_type*) 0)->member))

const char NAN_NULL[sizeof(double)]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};

T_PkgType LongType[]={
	{CH_LONG,sizeof(long),0,0,-1},
	{-1,0}
};
T_PkgType CharType[]={
	{CH_CHAR,-1,0,0,-1},
	{-1,-1}
};
T_PkgType ByteType[]={
	{CH_BYTE,-1,0,0,-1},
	{-1,-1}
};
T_PkgType DoubleType[]={
	{CH_DOUBLE,sizeof(double),0,0,-1},
	{-1,0}
};
T_PkgType FloatType[]={
	{CH_FLOAT,sizeof(float),0,0,-1},
	{-1,0}
};
T_PkgType IntType[]={
	{CH_INT,sizeof(int),0,0,-1},
	{-1,0}
};
T_PkgType ShortType[]={
	{CH_SHORT,sizeof(short),0,0,-1},
	{-1,0}
};
T_PkgType TinyType[]={
	{CH_TINY,1,0,0,-1},
	{-1,0}
};

int isnull(void * vp,int type)
{
	if(!vp) return 1;
	switch(type&127) {
	case CH_CHAR:
		return (!*(char *)vp);
	case CH_CLOB:
		return (!*(char **)vp);
	case CH_TINY:
		return(*(unsigned char *)vp == TINYNULL);
	case CH_SHORT:
		return(*(short *)vp == SHORTNULL);
	case CH_INT:
		return(*(int *)vp == INTNULL);
	case CH_LONG:
		return(*(long *)vp == LONGNULL);
	case CH_INT64:
		return(*(INT64 *)vp == INT64NULL);
	case CH_DOUBLE:
	case CH_LDOUBLE:
		return(!memcmp(NAN_NULL,(char *)vp,sizeof(double)));
	case CH_FLOAT:
		return(!memcmp(NAN_NULL,(char *)vp,sizeof(float)));
	default:
		return 0;
	}
}
static int cnt_type(register T_PkgType *tp)
{
int i;
	for(i=0;tp[i].type>-1;i++);
	return i;
}

void clean_bindtype(T_PkgType *tp,int flg)
{
T_PkgType *typ;

	if(!tp) return;
	for(typ=tp;typ->type>-1;typ++) {
		typ->bindtype &= ~flg;
	}
	typ->bindtype &= ~flg;
}
//这些结构用来测试各种数据的边界对齐规则，不要猜，要让编译器自己说。
typedef struct {
	char a;
	int b;
} align;
typedef struct {
	char a;
	double b;
} dalign;
typedef struct {
	char a;
	long double b;
} dfali;
typedef struct {
	char a;
	INT64 b;
} ll_ali;
typedef struct {
	char a;
	long b;
} l_ali;

int set_offset(T_PkgType *pkg_type)
{
int i,k,l;
int ali,dali,lali,llali,ldli;
int max_align=1;

	if(!pkg_type) return -1;
	if(pkg_type->offset>-1) return cnt_type(pkg_type);
	ali= M_ST_OFF(align,b) - 1;
	dali=M_ST_OFF(dalign,b) - 1;
	lali= M_ST_OFF(l_ali,b) - 1;
	llali= M_ST_OFF(ll_ali,b) - 1;
	ldli= M_ST_OFF(dfali,b) - 1;
	k=0;
	switch(pkg_type->type &127) {
	case CH_SHORT:
		max_align=2;
			break;
	case CH_INT:
		max_align=sizeof(int);
			break;
	case CH_LONG:
		max_align=sizeof(long);
			break;
	case CH_INT64:
		max_align=llali+1;
			break;
	case CH_DOUBLE:
		max_align=(dali+1);
			break;
	case CH_LDOUBLE:
		max_align=sizeof(long double);
			break;
	case CH_FLOAT:
		max_align=sizeof(float);
			break;
	case CH_CLOB:
		max_align=sizeof(char *);
			break;
	case CH_STRUCT:
		l=set_offset((T_PkgType *)pkg_type->format);
		pkg_type->len=((T_PkgType *)pkg_type->format)[l].offset;
		l=((T_PkgType *)pkg_type->format)[l].bindtype;
		max_align=l;
		break;
	default:
		max_align=1;
		break;
	}
	for(i=0;pkg_type[i].type>-1;i++){
		if(i>0) pkg_type[i].offset=k;
		pkg_type[i].bindtype=0;
		if((pkg_type[i].type&127)!=CH_CLOB)k+=pkg_type[i].len;
		else k+=sizeof(char *);
		switch(pkg_type[i+1].type&127) {
			case 127:
				break;
			case CH_CHAR:
			case CH_BYTE:
			case CH_TINY:
				break;
			case CH_INT64:
				k=(k+llali)&~llali;
				max_align=(max_align>(llali+1))?max_align:(llali+1);
//				max_align=(max_align>sizeof(INT64))?max_align:sizeof(INT64);
				break;
			case CH_LDOUBLE:
				k=(k+ldli)&~ldli;
				max_align=(max_align>sizeof(long double))?max_align:sizeof(long double);
				break;
			case CH_DOUBLE:
				k=(k+dali)&~dali;
				max_align=(max_align>(dali+1))?max_align:(dali+1);
				break;
			case CH_SHORT:
				k=(k+1)&~1;
				max_align=(max_align>sizeof(short))?max_align:sizeof(short);
				break;
			case CH_CLOB:
				k=(k+(sizeof(char *)-1)) & ~(sizeof(char *)-1);
				max_align=(max_align>sizeof(char *))?max_align:sizeof(char *);
				break;
			case CH_FLOAT:
			case CH_INT:
				k=(k+ali)&~ali;
				max_align=(max_align>sizeof(int))?max_align:sizeof(int);
				break;
			case CH_STRUCT:
				l=set_offset((T_PkgType *)pkg_type[i+1].format);
				pkg_type[i+1].len=((T_PkgType *)pkg_type[i+1].format)[l].offset;
				l=((T_PkgType *)pkg_type[i+1].format)[l].bindtype;
				max_align=(max_align>l)?max_align:l;
				if(l>1) {
					l--;
					k=(k+l)&~l;
				}
				break;
			case CH_LONG:
				max_align=(max_align>(lali+1))?max_align:(lali+1);
				k=(k+lali)&~lali;
				break;
			default:
				k=(k+ali)&~ali;
				break;
		}
	}
// 2009-06-22 by ylh,adjuct to struct align
	pkg_type[i].bindtype=max_align;
	if(max_align>1) {
		max_align--;
		pkg_type[i].offset=(k+max_align)&~max_align;
	} else  pkg_type[i].offset=k;
	pkg_type->offset=0;
	return i;
}

