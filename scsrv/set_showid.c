/**************************************************
 * 为每个线程服务建立Showid，由tpool.c引用 
 * Showid的位置由应用程序掌握，这里并不知道
 * 这里给出空函数，应该由应用系统重载 
#include "../thread/thrsrv.h"
 **************************************************/

#include <sys/types.h>

#ifdef __cplusplus
extern "C" 
#endif

void set_showid(void *ctx)
{
//pthread_t tid=pthread_self();
//GDA *gp=(GDA *)ctx;
	if(!ctx) return;
//	mthr_showid_add(tid,gp->ShowID);
}
