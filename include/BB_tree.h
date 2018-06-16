
#ifndef BB_TREE_H
#define BB_TREE_H

#include <pthread.h>

struct B_Tree {
	int Ldepth;
	int Rdepth;
	struct B_Tree *Left;
	struct B_Tree *Right;
//	void *Content;
	char Content[0];
};
typedef struct B_Tree T_Tree;

#ifdef __cplusplus
extern "C" {
#endif
/*********************************************************************
 * BB_Tree_Add:将内容加到树
 * 参数：
 * sp:根节点，content:节点内容，可以是任何数据结构
 * len:content的长度,Cmp_rec:记录比较器，sp_content>content时返回>0,
 * <时返回<0,=时返回0，user_add_tree插入节点时重码后的处理程序a
 * 返回新的根节点 
 ************************************************************************/
T_Tree * BB_Tree_Add( T_Tree *sp,void *content,int len, 
		int (*Cmp_rec)(void *sp_content,void *content,int len),
		int (*user_add_tree)(T_Tree *sp,void *content,int len));
/* BB_Tree_Scan: proc(): if return 0  continue sacn ; else break scan  */
int BB_Tree_Scan(T_Tree *sp, int (*proc)(void *content));
void BB_Tree_Free(T_Tree **sp,void (*user_free)(void *val));
//返回=key的节点
T_Tree * BB_Tree_Find(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *sp_content,void *content_key,int len));
//返回>key的节点
T_Tree * BB_Tree_GT(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len));
//返回>=key的节点
T_Tree * BB_Tree_GTEQ(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len));
//返回<key的节点
T_Tree * BB_Tree_LT(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len));
//返回<=key的节点
T_Tree * BB_Tree_LTEQ(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len));
#ifndef MAX
#define MAX(a,b) (((a)>(b))?a:b)
#endif

/* 删除指定节点，返回新的根节点,*flg初值=0,返回0未删除。否则表示在第几层删除 ,
   tp根节点,content_key查找键值,size_key,content_key的长度,Comp比较器，
   user_free用户的节点内容删除函数,返回0删除成功,返回非0节点将不被删除.
   返回值:新的根节点 */

T_Tree * BB_Tree_Del(T_Tree *tp,void *content_key,int size_key,
            int (*Comp)(void *node,void *content,int size_content),
            int (*user_free)(void *content),int *flg);
T_Tree * BB_Tree_MAX(T_Tree *sp);
T_Tree * BB_Tree_MIN(T_Tree *sp);
//bt为根结点的指针，返回值为bt的节点数*/
// context为应用提供的上下文数据，由counter使用
//counter由应用提供，判断是否符合计数条件，不符合返回0.
int BB_Tree_Count(T_Tree *  bt,void *context,
	int (*counter)(T_Tree *bt,void *context));

#ifdef __cplusplus
}
#endif

#endif
