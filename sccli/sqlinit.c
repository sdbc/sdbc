#include <sccli.h>

/* used by client */
void Init_CLI_Var(T_CLI_Var *CLI_Var)
{
        *CLI_Var->ErrMsg=0;
        CLI_Var->Errno=0;
        CLI_Var->NativeError=0;
        *CLI_Var->SqlState=0;
    	*CLI_Var->UID=0;
    	*CLI_Var->PWD=0;
    	*CLI_Var->DSN=0;
    	*CLI_Var->DBOWN=0;
    	CLI_Var->var=NULL;
	CLI_Var->svc_tbl=NULL;
//	CLI_Var->status=0;
}
