#include <Binary_search.h>
// =key
int lowerBound(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n))
{
int middle,start=0,end=data_count-1,val;

    if(!key||!data) return -1;
    while (start <= end) {
	middle = start + ((end-start) >> 1);
	val=compare(key,data,middle); //data - key
	if (!val && (!middle||compare(key,data,middle - 1) < 0)) return middle;
	if (val>=0) end = middle - 1;
	else start = middle + 1;
    }
    return -1;//²»´æÔÚ
}
// >key
int upperBound(void *key,void *data,int data_count,int (*compare)(void *key,void *data,int n))
{
int middle,start=0,end=data_count-1,val;
int result=-1;

	if(!key||!data) return -1;
	while(start <= end) {
        	middle = start + ((end-start) >> 1);
                val=compare(key,data,middle);
		if(val>0)  {
			if(!middle||compare(key,data,middle - 1) <= 0)
				result=middle;
			end=middle-1;
		} else start=middle+1;
	};
	return result;
}
