#include <strproc.h>
#include <scry.h>

int fingerprint(char *out)
{
int ret;
char buf[256];
	ret=get_mac(buf);
	byte_a64(out,buf,strlen(buf));
	return ret;
}
