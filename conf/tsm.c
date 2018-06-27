/******************************************
 * Thread Saved Memory manager
 ******************************************/
#include <strproc.h>

pthread_mutex_t tsm_mtx=PTHREAD_MUTEX_INITIALIZER;

char * tsm_alloc(size_t n)
{
char *p;

	pthread_mutex_lock(&tsm_mtx);
	p=malloc(n);
	pthread_mutex_unlock(&tsm_mtx);
	return p;
}

char * tsm_free(char *p)
{

	if(!p) return NULL;
	pthread_mutex_lock(&tsm_mtx);
        free(p);
        pthread_mutex_unlock(&tsm_mtx);
	return NULL;
}

char *tsm_realloc(char *alrd,size_t n)
{
char *p;
	pthread_mutex_lock(&tsm_mtx);
        p=realloc(alrd,n);
        pthread_mutex_unlock(&tsm_mtx);
        return p;
}

char *thr_strdup(char *cp)
{
char *p;

	pthread_mutex_lock(&tsm_mtx);
	p=strdup(cp);
	pthread_mutex_unlock(&tsm_mtx);
	return p;
}

