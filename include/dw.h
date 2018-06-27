#include <stdio.h>
#include <BB_tree.h>
/* default DWNUM,over by file: DWNUM=??? */
#define DWNUM 32

struct dwnode {
	int num;
	char *name;
};
typedef struct {
	T_Tree *node;
} DWS;

#ifdef __cplusplus
extern "C" {
#endif

char *getdw(int k,DWS *dwp);
/*   initdw(),return 0 success */
int initdw(char *flnm,DWS *dwp);
void freedw(DWS *dwp);

#ifdef __cplusplus
}
#endif
