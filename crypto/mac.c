#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
//#include <sys/sockio.h>

#define MAXINTERFACES 8

/*
在linux下，判断网卡状态
ioctl(sockfd, SIOCGIFFLAGS, &ifr);
return ((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING));
可以看看这个目录，就知道有多少网卡
/sys/class/net/

*/
int get_mac(char* out)
{
char *mac;
register int fd,intrface;
struct ifreq buf[MAXINTERFACES];
struct ifconf ifc;

	*out=0;
	mac=out;
	if((fd=socket(AF_INET,SOCK_DGRAM,0))<0)
	{
		return -1;
	}
	ifc.ifc_len =  sizeof buf;
	ifc.ifc_buf = (caddr_t)buf;
	if(ioctl(fd,SIOCGIFCONF,(char *)&ifc))
	{
		close(fd);
		return -2;
	}
	// 获取端口信息
	intrface = ifc.ifc_len/sizeof(struct ifreq);
	// 根据端口信息获取设备IP和MAC地址
	while(intrface-- > 0 )
	{
 		if(!strcmp(buf[intrface].ifr_name,"lo")) {
			continue;
		}
/*
		// 获取设备名称
		if(!(ioctl(fd,SIOCGIFFLAGS,(char *)&buf[intrface])))
		{
			if(buf[intrface].ifr_flags & IFF_PROMISC)
			{
				retn++;
			}
		} else {
			ShowLog(1,"%s: ioctl device %s",__FUNCTION__,buf[intrface].ifr_name);
			continue;
		}
*/
		// 获取MAC地址
		if(ioctl(fd,SIOCGIFHWADDR,(char *)&buf[intrface]))
		{
			continue;
		}
		mac+=sprintf(mac,"%02X:%02X:%02X:%02X:%02X:%02X;",
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[0],
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[1],
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[2],
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[3],
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[4],
			(unsigned char)buf[intrface].ifr_hwaddr.sa_data[5]);
	}
	close(fd);
	return 0;
}
