/********************************************************
 * Binary_search.h,通用二分法查找程序
 ********************************************************/
#ifndef BINARY_SEARCH
#define BINARY_SEARCH

#ifdef __cplusplus
extern "C" {
#endif
//返回数组的下标，-1没找到
int Binary_EQUAL(void *key,		//查找键值，类型与data相同
		  void *data,		//排序的数据数组，类型由应用自定义
		  int data_count,	//数组的大小
//比较函数。如果数组是增序排列,返回data[n]-*key，否则返回*key-data[n]。
		  int (*compare)(void *key,void *data,int n));
//GT,LT等大、小指的数组顺序的大、小，注意反序的数组
int Binary_GT(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));
int Binary_GTEQ(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));
int Binary_LT(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));
int Binary_LTEQ(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));
//以下适用于带重复值的有序数组
//=key的第一个元素的下标，-1=没找到,注意：与STL不同
int lowerBound(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));
//>key的第一个元素的下标，-1=没找到 这样可以组合出各种查询条件如：>,>=,<,<=.
int upperBound(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n));

#ifdef __cplusplus
}
#endif

#endif
