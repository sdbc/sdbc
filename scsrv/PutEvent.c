#include <scsrv.h>

int PutEvent(T_Connect *conn,int Evtno)
{
	if(!conn->Event_proc) return 0;
	return conn->Event_proc(conn,Evtno);
}

