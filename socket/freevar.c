#include <stdio.h>
#include <strproc.h>

void FreeVar(void *p) 
{
	if(p) {
		ShowLog(0,"T_Connect->Var be used,but Has't free,you should offer a Function FreeVar(void *p) Free it!");
	}
}

