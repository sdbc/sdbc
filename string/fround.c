#include <math.h>

double f_round(double x,int flg,int dig);

double f_round(x,flg,dig)
double x;
int flg,dig;
{
double r,y,z;
int i;
	if(!x) return x;
	z=x;
	if(dig > 0) {
		for(i=0;i<dig;i++) z *= 10;
	} else if(dig < 0) {
		for(i=0;i<dig;i++) z /= 10;
	} else ;
	r=modf(z,&y);
	switch(flg) {
	case 1:    /*    to - */
		if(r<0) y--;
		break;
	case 2:    /*    to + */
		if(r>0) y++;
		break;
	case 3: ;  /*    to 0 */
		break;
	case 4:    /*    to +- */
		if(z>=0) {
			if(r>0) y++;
		} else {
			if(r<0) y--;
		}
		break;
	case 5:   /*    4s5r */
	default:
		if(z>=0) {
			if(r>=0.5) y++;
		} else {
			if(r<=-0.5) y--;
		}
	}
	if(dig > 0) {
		for(i=0;i<dig;i++) y /= 10;
	} else if(dig < 0) {
		for(i=0;i<dig;i++) y *= 10;
	} else ;
	return(y);
}

