#ifndef MULTI_HASH
#define MULTI_HASH

/**
 * 产生 可以重复键值的静态hash表
 * 原数据必须是数组，重复键值必须排在一起
 * hash一旦建立，原数据不能增、删、改，否则重建hash表
 */
typedef struct {
	void *data;		//原数据数组
	int  data_count;	//原数据的尺寸
	int  key_count;		//键数量，如果没有重码，=data_count
	int (*do_hash)(void *key,int key_count);//hash函数
	void *(*getdata)(void *data,int n);//取数据的函数，返回&data[n]
	int (*key_cmp)(void *data,void *key);//比较&data[n]和key的函数,相等返回0
	void *index;//索引表，由multi_hash生成，用毕需自己释放
} hash_paramiter;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * 产生 可以重复键值的hash表
 * 原数据必须是数组，重复键值必须排在一起
 * para是hash参数表
 * mult_hash()返回冲突数，一般是key_count的1/3左右,供选择do_hash时做参考。
 */
int multi_hash(hash_paramiter *param);

/**
 * 检索mukti_hash
 * para是hash参数表,与multi_hash相同
 * 返回值是数据下标。-1没找到
 * a_count返回该key下数据的数量
 */
int multi_hash_find(void *key,hash_paramiter *para,int *a_count);

#ifdef __cplusplus
}
#endif

#endif
