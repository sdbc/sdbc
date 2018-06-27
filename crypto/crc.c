/* CRC -- CCITT */
#include <crc.h>
#if defined (DEBUG)
#include <stdio.h>
main()
{
unsigned short crc,cc;
char aa[1000000];
long tim,time();

	crc=gencrc("UDFFaaa",7); /* CRC=0x7ADD */
	printf("UDFFaaa crc=%04hX\n",crc);
	crc=gencrc("UDFFaaa\x65\xea",9); /* CRC=0xF0B8 */
	printf("UDFFaaa\\x65\\xea crc=%04hX\n",~crc);
	tim=time((long *)0);
		crc=gencrc(aa,sizeof(aa));
	printf("time=%lds!\n",time((long *)0) - tim);
}
#endif


static long const crctab[]={
	0X0000,0x1081,0x2102,0x3183,0x4204,0x5285,0x6306,0x7387,
	0x8408,0x9489,0xA50A,0xB58B,0xC60C,0XD68D,0xE70E,0xF78F
};

unsigned short gencrc(unsigned char *p,int len)
{
unsigned char tmpchar;
unsigned long i,index;
unsigned short crctmp;
	crctmp=0xFFFF;
	for(i=0;i<len;i++,p++) {
		tmpchar=*p;
		index=((crctmp ^ tmpchar) & 0x000f);
		crctmp = ((crctmp >> 4) & 0xfff) ^ crctab[index];
		tmpchar >>= 4;
		index=((crctmp ^ tmpchar) & 0x000f);
		crctmp = ((crctmp >> 4) & 0xfff) ^ crctab[index];
	}
	return ~crctmp;
}

