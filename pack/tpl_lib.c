#include <pack.h>
#include <json_pack.h>

static pthread_rwlock_t tpl_lock = PTHREAD_RWLOCK_INITIALIZER;
static JSON_OBJECT tpl_lib=NULL;

T_PkgType * mk_tpl(const char *tabname)
{
JSON_OBJECT json;
T_PkgType *tpl=NULL;

	pthread_rwlock_rdlock(&tpl_lock);
	if(tpl_lib && NULL != (json=json_object_object_get(tpl_lib,tabname))) {
		tpl=new_tpl_fromJSON(json);
		pthread_rwlock_unlock(&tpl_lock);
	} else pthread_rwlock_unlock(&tpl_lock);
	return tpl;
}

int tpl_to_lib(T_PkgType *tpl,const char *tabname)
{
JSON_OBJECT json;
int ret;

	json=json_object_new_array();
	ret=tpl_to_JSON(tpl,json);

	pthread_rwlock_wrlock(&tpl_lock);
	if(!tpl_lib) tpl_lib=json_object_new_object();
	json_object_object_add(tpl_lib,tabname,json);
	pthread_rwlock_unlock(&tpl_lock);

	return ret;
}

void tpl_lib_cancel()
{
	pthread_rwlock_wrlock(&tpl_lock);
	if(tpl_lib) {
		json_object_put(tpl_lib);
		tpl_lib=NULL;
	}
	pthread_rwlock_unlock(&tpl_lock);
}
