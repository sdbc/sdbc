/*
static void rdy_add(TCB *en)
{
        if(!rpool.queue) {
                rpool.queue=en;
                en->next=en;
        } else {
                en->next=rpool.queue->next;
                rpool.queue->next=en;
                rpool.queue=en;
        }
}

static TCB *rdy_get()
{
TCB *enp;
        if(!rpool.queue) return NULL;
        enp=rpool.queue->next;
        if(enp->next == enp) rpool.queue=NULL;
        else rpool.queue->next=enp->next;
        enp->next=NULL;
        return enp;
}
*/

#define LNKTAB_ADD(LNKTAB,NODE)			\
{						\
	if(!(LNKTAB)) {				\
		(LNKTAB)=(NODE);		\
		(NODE)->next=(NODE);		\
	} else {				\
		(NODE)->next = (LNKTAB)->next;	\
		(LNKTAB)->next = (NODE);	\
		(LNKTAB) = (NODE);		\
	}					\
}

#define LNKTAB_GET(LNKTAB,NODE)			\
{						\
	if(!(LNKTAB)) tmp=NULL;			\
	(NODE)=(LNKTAB)->next;			\
	if((LNKTAB)->next == (LNKTAB))		\
		(LNKTAB)=NUL;			\
	else (LNKTAB)->next=(LNKTAB)->next->next;\
	(NODE)->next=NULL;			\
}
