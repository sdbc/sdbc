#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <BB_tree.h>

/*-------------------- 插入记录比较=rec1-rec2 -------------------*/
static int Tree_Cmp(void *rec1,void *rec2,int len,
int (*Cmp_rec)(void *sp_content,void *key_content,int len))
{
	if(Cmp_rec) return Cmp_rec(rec1,rec2,len);
	int rc;
	return !(rc=memcmp(rec1,rec2,len))?0:(rc<0)?-2:2;
}

T_Tree *BB_Tree_Add( T_Tree *sp, void *content, int len,
	int (*Cmp_rec)(void *sp_content,void *content,int len),
	int (*user_add_tree)(T_Tree *sp,void *content,int len))
{
int rc;
T_Tree *tp;
    if(!sp){
	sp=(T_Tree *)malloc(sizeof(T_Tree)+len+1);
	if(!sp){
	    return sp;
	}
	memcpy(sp->Content,content,len);
	sp->Left=sp->Right=0;
	sp->Ldepth=sp->Rdepth=0;
    } else {
	    rc=Tree_Cmp(sp->Content,content,len,Cmp_rec);
	if(rc>0) {
		sp->Left=BB_Tree_Add( sp->Left, content, len,
			Cmp_rec, user_add_tree);
/* compute depth */
		tp=sp->Left;
		if(!tp) {
			sp->Ldepth=0;
			return sp;
		}
		sp->Ldepth=MAX(tp->Ldepth,tp->Rdepth)+1;
		if((sp->Ldepth - sp->Rdepth) <= 1) return sp;
		if(tp->Ldepth >= tp->Rdepth) { 
/* exchange */
			sp->Left=tp->Right;
			tp->Right=sp;
/* compute depth again */
			if(sp->Left)
			    sp->Ldepth=MAX(sp->Left->Ldepth,sp->Left->Rdepth)+1;
			else sp->Ldepth=0;
			tp->Rdepth=MAX(sp->Ldepth,sp->Rdepth)+1;
			return tp;
		}
/* Type I */
/* exchange 1 */
		tp=tp->Right;
		sp->Left->Right=tp->Left;
		tp->Left=sp->Left;
/* compute depth */
		if(sp->Left->Right)
		   sp->Left->Rdepth=MAX(sp->Left->Right->Ldepth,
				sp->Left->Right->Rdepth)+1;
		else sp->Left->Rdepth=0;
/* exchange 2 */
		sp->Left=tp->Right;
		if(sp->Left)
			sp->Ldepth=MAX(sp->Left->Ldepth,
				sp->Left->Rdepth)+1;
		else sp->Ldepth=0;
		tp->Right=sp;
/* compute depth of tp */
		tp->Ldepth=MAX(tp->Left->Ldepth,tp->Left->Rdepth)+1;
		tp->Rdepth=MAX(tp->Right->Ldepth,tp->Right->Rdepth)+1;
		return tp;
	} else if(rc<0) {
		sp->Right=BB_Tree_Add( sp->Right, content, len,
			Cmp_rec, user_add_tree);
/* compute depth */
		tp=sp->Right;
		if(!tp) {
			sp->Rdepth=0;
			return sp;
		}
		sp->Rdepth=MAX(tp->Ldepth,tp->Rdepth)+1;
		if((sp->Rdepth - sp->Ldepth) <= 1) return sp;
		if(tp->Rdepth >= tp->Ldepth) { 
/* exchange */

			sp->Right=tp->Left;
			tp->Left=sp;
/* compute depth again */
			if(sp->Right)
			  sp->Rdepth=MAX(sp->Right->Ldepth,sp->Right->Rdepth)+1;
			else sp->Rdepth=0;
			tp->Ldepth=MAX(sp->Ldepth,sp->Rdepth)+1;

			return tp;
		}
/* Type II */
/* exchange 1 */
		tp=tp->Left;
		sp->Right->Left=tp->Right;
		tp->Right=sp->Right;
/* compute depth */
		if(sp->Right->Left)
			sp->Right->Ldepth=MAX(sp->Right->Left->Ldepth,
				sp->Right->Left->Rdepth)+1;
		else sp->Right->Ldepth=0;
/* exchange 2 */
		sp->Right=tp->Left;
		if(sp->Right)
			sp->Rdepth=MAX(sp->Right->Ldepth,sp->Right->Rdepth)+1;
		else sp->Rdepth=0;
		tp->Left=sp;
/* compute depth of tp */
		tp->Ldepth=MAX(tp->Left->Ldepth,tp->Left->Rdepth)+1;
		tp->Rdepth=MAX(tp->Right->Ldepth,tp->Right->Rdepth)+1;
		return tp;
	} else { 
	    if(user_add_tree){
	        rc=user_add_tree(sp,content,len);
	    }
	}
    }
    return sp;
}

int BB_Tree_Scan(sp,proc)
T_Tree *sp;
int (*proc)(void *content);
{
int rc;
	if(!sp)return 0;
	if(sp->Left){
	    rc=BB_Tree_Scan(sp->Left,proc);
	    if(rc)return rc;
	}
	rc=proc(sp->Content);
        if(rc){
    	    return rc;
        }
	if(sp->Right) 
	    rc=BB_Tree_Scan(sp->Right,proc);
	return rc;
}


void BB_Tree_Free(T_Tree **sp,void (*user_free)(void *))
{
	if(!(*sp))return;
	if((*sp)->Left)BB_Tree_Free(&(*sp)->Left,user_free);
	if((*sp)->Right)BB_Tree_Free(&(*sp)->Right,user_free);
	if ((*sp)) {
		if(user_free) user_free((*sp)->Content);
		free((*sp));
		(*sp)=0;
	}
	return;
}

T_Tree * BB_Tree_Find(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len))
{
        while(sp) {
//rc=sp-key
		int rc=Tree_Cmp(sp->Content,content_key,len,Cmp_rec);
		if(!rc) return sp;
		if(rc<=0) sp=sp->Right;
                else sp=sp->Left;
        }
	return NULL;
/*
	return !sp?sp:!(rc=Tree_Cmp(sp->Content,content_key,len,Cmp_rec))?sp:
		(rc>0)?BB_Tree_Find(sp->Left,content_key,len,Cmp_rec):
		BB_Tree_Find(sp->Right,content_key,len,Cmp_rec);
*/
}

//返回>key的节点
T_Tree * BB_Tree_GT(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len))
{
T_Tree *t=NULL;
//试验一下非递归的算法，似乎性能改善有限
        while(sp) {
		int rc=Tree_Cmp(sp->Content,content_key,len,Cmp_rec);
		if(rc<=0) sp=sp->Right;
                else {
			t=sp;
			sp=sp->Left;
                }
        }
        return t;
}
/* 这是原来的递归算法
T_Tree *t;
	return !sp?sp:(0>=Tree_Cmp(sp->Content,content_key,len,Cmp_rec))?
		BB_Tree_GT(sp->Right,content_key,len,Cmp_rec):
		(t=BB_Tree_GT(sp->Left,content_key,len,Cmp_rec))?t:sp;
}
*/
//返回>=key的节点
T_Tree * BB_Tree_GTEQ(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len))
{
T_Tree *t=NULL;
        while(sp) {
		int rc=Tree_Cmp(sp->Content,content_key,len,Cmp_rec);
		if(!rc) return sp;
		if(rc<0) sp=sp->Right;
                else {
			t=sp;
			sp=sp->Left;
                }
        }
        return t;
/*
	return !sp?sp:!(rc=Tree_Cmp(sp->Content,content_key,len,Cmp_rec))?sp:
		(rc<0)?BB_Tree_GTEQ(sp->Right,content_key,len,Cmp_rec):
		(t=BB_Tree_GTEQ(sp->Left,content_key,len,Cmp_rec))?t:sp;
*/
}

//返回<key的节点
T_Tree * BB_Tree_LT(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len))
{
T_Tree *t=NULL;
//试验一下非递归的算法，似乎性能改善有限
        while(sp) {
		if(0<=Tree_Cmp(sp->Content,content_key,len,Cmp_rec))
			sp=sp->Left;
                else {
			t=sp;
			sp=sp->Right;
                }
        }
        return t;
/*
	return !sp?sp:(0<=Tree_Cmp(sp->Content,content_key,len,Cmp_rec))?
		BB_Tree_LT(sp->Left,content_key,len,Cmp_rec):
		(t=BB_Tree_LT(sp->Right,content_key,len,Cmp_rec))?t:sp;
*/
}

//返回<=key的节点
T_Tree * BB_Tree_LTEQ(T_Tree *sp,void *content_key,int len,
		int (*Cmp_rec)(void *s1,void *s2,int len))
{
T_Tree *t=NULL;

        while(sp) {
		int rc=Tree_Cmp(sp->Content,content_key,len,Cmp_rec);
		if(!rc) return sp;
		if(rc>0) sp=sp->Left;
                else {
			t=sp;
			sp=sp->Right;
                }
        }
        return t;
/*
	return !sp?sp:!(rc=Tree_Cmp(sp->Content,content_key,len,Cmp_rec))?sp:
		(rc>0)?BB_Tree_LTEQ(sp->Left,content_key,len,Cmp_rec):
		(t=BB_Tree_LTEQ(sp->Right,content_key,len,Cmp_rec))?t:sp;
*/
}

T_Tree * BB_Tree_MAX(T_Tree *sp)
{
T_Tree *t;
	return !sp?sp:(t=BB_Tree_MAX(sp->Right))?t:sp;
}

T_Tree * BB_Tree_MIN(T_Tree *sp)
{
T_Tree *t;
	return !sp?sp:(t=BB_Tree_MIN(sp->Left))?t:sp;
}

//bt为根结点的指针，返回值为bt的节点数
// context为应用提供的上下文数据，由counter使用
//counter由应用提供，判断是否符合计数条件，不符合返回0.
int BB_Tree_Count(T_Tree *  bt,void *context,
	int (*counter)(T_Tree *bt,void *context))
{
	return !bt?0:(BB_Tree_Count(bt->Left,context,counter) +
		((counter)?counter(bt,context):1) +
		BB_Tree_Count(bt->Right,context,counter));
}


