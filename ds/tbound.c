/*************************************************
 * 可以看到如何用lowerBound()和upperBound组合成
 * <  <=  > 功能
 ************************************************/
#include <stdio.h>
#include <Binary_search.h>
#include <pack.h>

// <key的最后元素
int less_than(void *key,void *data,int data_siz,int cmp(void *key,void *data,int n))
{
int ret;
	if(0>(ret=lowerBound(key,data,data_siz,cmp)) &&
	   0>(ret=upperBound(key,data,data_siz,cmp))) {
		ret=data_siz;
	}
	return --ret;
}

// <=key的最后元素
int less_eq(void *key,void *data,int data_siz,int cmp(void *key,void *data,int n))
{
int ret;
	if(0>(ret=upperBound(key,data,data_siz,cmp)))
		ret=data_siz;
	return --ret;
}

// >=key的第一个元素
int great_eq(void *key,void *data,int data_siz,int cmp(void *key,void *data,int n))
{
int ret;
	return (ret=lowerBound(key,data,data_siz,cmp))>=0?ret:
		upperBound(key,data,data_siz,cmp);
}

//static int tab[]={-1,-1,0,1,1,1,1,2,2,2,2,3,3,4,5,5,5,5,5,5,6,6,6,7};
static int tab[]={0,0,2,2,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,7,10,10};
#define COUNT sizeof(tab)/sizeof(int)
static int tab1[]={1,3,5,7,9,11,13,15,17,19,21,23,25,27,29,31,33,35,37,39};
#define CNT1 sizeof(tab1)/sizeof(int)

static int int_cmp(void *key,void *data,int n)
{
int *d=(int *)data+n;
printf("data[%d]=%d,key=%d\n",n,*d,*(int*)key);
	if(*d > *(int*)key) return 1;
	if(*d < *(int*)key) return -1;
	return 0;
}

int main(int argc,char *argv[])
{
int ret,low,up,key;

	key=10;
	low=lowerBound(&key,tab,COUNT,int_cmp);
printf("------------\n");
	up=upperBound(&key,tab,COUNT,int_cmp);
printf("------------\n");
	ret=less_than(&key,tab,COUNT,int_cmp);
printf("------------\n");
	
	printf("key=%d,low=%d,up=%d,less=%d,less_eq=%d,great_eq=%d\n",key,low,up,ret,
		less_eq(&key,tab,COUNT,int_cmp),
		great_eq(&key,tab,COUNT,int_cmp));

	key=0;
	ret=Binary_GTEQ(&key,tab1,CNT1,int_cmp);
	printf("GTEQ:key=%d,ret=%d,data=%d\n",key,ret,(ret>=0)?tab1[ret]:INTNULL);
	return 0;
}

