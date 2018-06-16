/* 二叉树平衡删除 
 2010.1.18 by YLH */
#include <stdlib.h>
#include <string.h>
#include <BB_tree.h>
static T_Tree *R_rot(T_Tree *tp)
{
T_Tree *ttmp;

    ttmp=tp->Left;
    tp->Left=ttmp->Right;
	tp->Ldepth=ttmp->Rdepth;
    ttmp->Right=tp;
	ttmp->Rdepth=MAX(ttmp->Right->Rdepth,ttmp->Right->Ldepth)+1;
    return ttmp;
}

static T_Tree *L_rot(T_Tree *tp)
{
T_Tree *ttmp;

    ttmp=tp->Right;
    tp->Right=ttmp->Left;
	tp->Rdepth=ttmp->Ldepth;
    ttmp->Left=tp;
	ttmp->Ldepth=MAX(ttmp->Left->Rdepth,ttmp->Left->Ldepth)+1;
    return ttmp;
}

static T_Tree *R_blc(T_Tree *tp) //tp右高 
{
T_Tree *ttmp,*ftmp=0;
	ttmp=tp->Right;
	if(ttmp->Left) {
		ftmp=ttmp;
		ttmp=ttmp->Left; 
	}
	if(ftmp) {
		ftmp->Left=ttmp->Right;
		ftmp->Ldepth=ttmp->Rdepth;
		ttmp->Right=ftmp;
		if((ftmp->Rdepth - ftmp->Ldepth) > 1) {
			ttmp->Right=L_rot(ftmp);
		}
		ttmp->Rdepth=MAX(ttmp->Right->Rdepth,ttmp->Right->Ldepth)+1;
	}
	tp->Right=ttmp->Left;
	tp->Rdepth=ttmp->Ldepth;
	ttmp->Left=tp;
	if((tp->Ldepth - tp->Rdepth) > 1) ttmp->Left=R_rot(tp);
	ttmp->Ldepth=MAX(ttmp->Left->Rdepth,ttmp->Left->Ldepth)+1;
	return ttmp;
}

static T_Tree *L_blc(T_Tree *tp) //tp左高 
{
T_Tree *ttmp,*ftmp=0;
	ttmp=tp->Left;
	if(ttmp->Right) {
		ftmp=ttmp;
		ttmp=ttmp->Right; 
	}
	if(ftmp) {
		ftmp->Right=ttmp->Left;
		ftmp->Rdepth=ttmp->Ldepth;
		ttmp->Left=ftmp;
		if((ftmp->Ldepth - ftmp->Rdepth) > 1) {
			ttmp->Left=R_rot(ftmp);
		}
		ttmp->Ldepth=MAX(ttmp->Left->Rdepth,ttmp->Left->Ldepth)+1;
	} 
	tp->Left=ttmp->Right;
	tp->Ldepth=ttmp->Rdepth;
	ttmp->Right=tp;
	if((tp->Rdepth - tp->Ldepth) > 1) ttmp->Right=L_rot(tp);
	ttmp->Rdepth=MAX(ttmp->Right->Rdepth,ttmp->Right->Ldepth)+1;
	return ttmp;
}

static T_Tree *t_r(T_Tree **tp)
{
T_Tree *ttmp=0;
		if(!(*tp)->Right) return *tp;
		ttmp= t_r(&(*tp)->Right);
		if(ttmp==(*tp)->Right) { 
			(*tp)->Right=ttmp->Left;
			(*tp)->Rdepth=ttmp->Ldepth;
		} else (*tp)->Rdepth=MAX((*tp)->Right->Rdepth,(*tp)->Right->Ldepth)+1;
		if(((*tp)->Ldepth - (*tp)->Rdepth) >1) *tp=L_blc(*tp);

		return ttmp;
}

static T_Tree *t_l(T_Tree **tp)
{
T_Tree *ttmp=0;
		if(!(*tp)->Left) return *tp;
		ttmp= t_l(&(*tp)->Left);
		if(ttmp==(*tp)->Left) { 
			(*tp)->Left=ttmp->Right;
			(*tp)->Ldepth=ttmp->Rdepth;
		} else (*tp)->Ldepth=MAX((*tp)->Left->Rdepth,(*tp)->Left->Ldepth)+1;
		if(((*tp)->Rdepth - (*tp)->Ldepth) >1)  *tp=R_blc(*tp);

		return ttmp;
}
/* 删除当前节点，返回代替节点 */
static T_Tree *t_delete(T_Tree *tp,int (*user_free)(void *content))
{
T_Tree *ttmp;

/* user_free 返回0继续进行删除，否则不删，应用软件可以利用这一点处理特定节点 */
	if(user_free && user_free(tp->Content)) return (T_Tree *)-1;
	if(!tp->Right) {	//右子树为空
		ttmp=tp->Left;
	} else if(!tp->Left) {	//左子树为空
		ttmp=tp->Right;
	} else {	//左右子树均不空
		if(tp->Ldepth >= tp->Rdepth) {
		//寻找代替节点，即左孩子的最右下节点 
			ttmp=t_r(&tp->Left);
			if(ttmp != tp->Left) {
				ttmp->Left=tp->Left;
				ttmp->Ldepth=MAX(tp->Left->Rdepth,tp->Left->Ldepth)+1;
			}
			ttmp->Right=tp->Right;
			ttmp->Rdepth=tp->Rdepth;
		} else {
		//寻找代替节点，即右孩子的最左下节点 
			ttmp=t_l(&tp->Right);
			if(ttmp != tp->Right) {
				ttmp->Right=tp->Right;
				ttmp->Rdepth=MAX(tp->Right->Rdepth,tp->Right->Ldepth)+1;
			}
			ttmp->Left=tp->Left;
			ttmp->Ldepth=tp->Ldepth;
		}
	}
	free(tp);
	return ttmp;
}
/*---------------------删除记录比较---------------------*/
static int Tree_Cmp(rec1,rec2,len)
void *rec1;
void *rec2;
int len;
{
    int rc;
    rc=memcmp(rec1,rec2,len);
    if(rc){
        if(rc<0)return -2;
        else return 2;
    }
    return 0;
}

/* 删除指定节点，返回新的根节点,*flg初值=0,返回0未删除。否则表示在第几层删除 */
T_Tree * BB_Tree_Del(T_Tree *tp,void *content_key,int size_key,
			int (*Comp)(void *node,void *content,int size_content),
			int (*user_free)(void *content),int *flg)
{
int ret;

	if(!tp || !flg ||!content_key) {
		if(flg) *flg=0;
		return tp;
	}
	if(Comp) ret=Comp(tp->Content,content_key,size_key);
	else ret=Tree_Cmp(tp->Content,content_key,size_key);
	if(!ret) {	//根节点与之相等,直接删除
		 T_Tree *ttmp=t_delete(tp,user_free);
		 if(ttmp!=(T_Tree *)-1) {
		 	tp=ttmp;
		 	(*flg)=1;
		 } else *flg=0;
	} else if(ret>0) {	//被删节点在左子树上
		if(!tp->Left) {	//没找到
			*flg=0;
			return tp;
		}
		tp->Left=BB_Tree_Del(tp->Left,content_key,size_key,Comp,user_free,flg);
		if(*flg > 0) { //确实删除过 
			if(!tp->Left) tp->Ldepth=0;
			else tp->Ldepth=MAX(tp->Left->Rdepth,tp->Left->Ldepth)+1;
			if((tp->Rdepth - tp->Ldepth) > 1) tp=R_blc(tp);
			(*flg)++;
		}
	} else {	////被删节点在在右子树上
		if(!tp->Right)  {	//没找到
			*flg=0;
			return tp;
		}
		tp->Right=BB_Tree_Del(tp->Right,content_key,size_key,Comp,user_free,flg);
		if(*flg > 0) { //确实删除过 
			if(!tp->Right) tp->Rdepth=0;
			else tp->Rdepth=MAX(tp->Right->Rdepth,tp->Right->Ldepth)+1;
			if((tp->Ldepth - tp->Rdepth) > 1) tp=L_blc(tp);
			(*flg)++;
		}
	}
	return tp;
}
