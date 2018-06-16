/* enigma º”√‹≤‚ ‘ */
#include <strproc.h>
#include <enigma.h>
#include <sys/time.h>

//#define TEST_SPEED

long interval(struct timeval *begtime,struct timeval *endtime)
{
long ret;
        ret=endtime->tv_sec-begtime->tv_sec;
        ret*=1000000;
        ret += endtime->tv_usec - begtime->tv_usec;
        return ret;
}


int main(int ac,char *av[])
{
char buf[131702];
int len,i,len1;
ENIGMA t;
ENIGMA2 egm;
struct timeval beg,end;

	enigma_init(t,"abcdefg",0);
	printf("t1:");
	for(i=0;i<256;i++) {
		if(!(i&7)) putchar('\n');
		printf("%3d:%02X\t",i,255&t[0][i]);
	}
	printf("\nt3:");
	for(i=0;i<256;i++) {
		if(!(i&7)) putchar('\n');
		printf("%3d:%02X\t",i,255&t[2][i]);
	}
	putchar('\n');

	enigma2_init(&egm,"abcdefg",0);
	printf("\ncrc=%d\n",egm.crc);
//	enigma2_init(&egm,"\x01\xff\x35\xf8\xef\x97\x22\x14\x80\x7f\t\b\r\n\377\177\225",17);
	memset(buf,'B',sizeof(buf));
	buf[sizeof(buf)-1]=0;
#ifndef TEST_SPEED
	while(!ferror(stdin)) {
		fgets(buf,sizeof(buf),stdin);
		if(feof(stdin)) break;
		TRIM(buf);
#endif
		len=strlen(buf);
		gettimeofday(&beg,0);
		enigma(t,buf,len);
		gettimeofday(&end,0);
		len1=len>32?32:len;
		printf("enigma encode    :");
		for(i=0;i<len1;i++) printf("%02X ",buf[i]&255);
		printf("\nenigma encode 64K:");
		for(i=0;i<len1;i++) printf("%02X ",buf[i+65536]&255);
		printf("\ntimeval=%ld\n",interval(&beg,&end));
		enigma(t,buf,len);
		printf("enigma decode:\n%.100s\n",buf);
//test frenz
		gettimeofday(&beg,0);
		enigma_encrypt(t,buf,len);
		gettimeofday(&end,0);
		printf("\nencrypt         :");
		for(i=0;i<len1;i++) printf("%02X ",buf[i]&255);
		printf("\nencrypt   64K:");
		for(i=0;i<len1;i++) printf("%02X ",buf[i+65536]&255);
		printf("\ntimeval=%ld\n",interval(&beg,&end));
		enigma_decrypt(t,buf,len);
		printf("decrypt    :\n%.100s\n",buf);
//test enigma2
		gettimeofday(&beg,0);
		enigma2_encrypt(&egm,buf,len);
		gettimeofday(&end,0);
		printf("enigma2 encode    :");
		for(i=0;i<len1;i++) printf("%02X ",buf[i]&255);
		printf("\nenigma2 encode 64K:");
		for(i=0;i<len1;i++) printf("%02X ",buf[i+65536]&255);
		printf("\ntimeval=%ld\n",interval(&beg,&end));
		enigma2_decrypt(&egm,buf,len);
		printf("enigma2 decode:\n%.100s\n",buf);
#ifndef TEST_SPEED
	}
#endif
	return 0;
}
