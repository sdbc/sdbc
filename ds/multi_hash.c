#include <stdio.h>
#include <malloc.h>
#include <strproc.h>
#include <multi_hash.h>

typedef struct route_node {
	int rowno;
	int link;
	int count;
} multi_hash_node;

#define GETDATA(i) (para->getdata(para->data,(i)))
#define COMPARE(i,k) para->key_cmp(GETDATA(i),GETDATA(k))

//产生 hash表
int multi_hash(hash_paramiter *para)
{
	int conflict,i,hashnum;
	int *lp;
	multi_hash_node *hash,*top,*hp,*stack;

	if(!para) return -1;

	stack=malloc(sizeof(multi_hash_node) * para->key_count);
	if(!stack) return MEMERR;
	para->index=hash=(multi_hash_node *)malloc(para->key_count * sizeof(multi_hash_node));
	if(!hash) {
		free(stack);
		return MEMERR;
	}

	top=stack;
	for(i=0;i<para->key_count;i++) {
		hash[i].rowno=-1;
		hash[i].link=-1;
		hash[i].count=0;
	}
//ShowLog(5,"multi_hash:data_count=%d,key_count=%d",para->data_count,para->key_count);
	for(i=0;i<para->data_count;i++) {
		hashnum=para->do_hash(GETDATA(i),para->key_count);
		hp=&hash[hashnum];
		if(hp->rowno==-1) {	//没有散列冲突
			hp->rowno=i;
			hp->count=1;
		} else if(!COMPARE(i,hp->rowno)) {	//检查重码，构建重码链
			hp->count++;
			continue;
		} else {				//有散列冲突，存储冲突链
			if(top>stack&&!COMPARE(i,top[-1].rowno)) {
				top[-1].count++;
				continue;
			}
			top->rowno=i;
			top->link=hashnum;
			top->count=1;
			top++;
		}
	}
	conflict=top-stack;
	if(top > stack) { //有散列冲突，构建冲突链
		hp=hash;
		for(i=0;top>stack&&i<para->key_count;i++,hp++) {
			if(hp->rowno > -1) continue;
			top--;
//找到索引表里的空项
			hp->rowno=top->rowno;
			hp->count=top->count;
			hp->link=-1;
			for(lp=&top->link;*lp != -1;lp=&hash[*lp].link)
				;
			*lp=i;
		}
	}
	free(stack);
	return conflict;
}

int multi_hash_find(void *key,hash_paramiter *para,int *a_count)
{
	multi_hash_node *hp;

	if(a_count) *a_count=0;
	if(!para||!para->index) return -1;
	for(hp=((multi_hash_node *)para->index)+para->do_hash(key,para->key_count);
		hp->rowno>-1&&para->key_cmp(GETDATA(hp->rowno),key);
		hp=((multi_hash_node *)para->index)+hp->link) if(hp->link<0) return -1;
	if(hp->rowno<0) return -1;
	if(a_count) *a_count=hp->count;
	return hp->rowno;
}

