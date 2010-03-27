static char version_string[]="$Id: draw_point.c,v 1.1.1.1 1999/10/15 12:26:21 kise Exp $";
/*
   $Date: 1999/10/15 12:26:21 $
   $Revision: 1.1.1.1 $
   $Author: kise $
*/
#include <stdlib.h>
#include "color.h"
#include "function.h"

void draw_point(ImageData *imgd, int x, int y, int color, int width)
{
  int	odd;
  int xx,yy;

  odd = imgd->imax%2;

  for(yy=y-width;yy<=y+width;yy++){
    for(xx=x-width;xx<=x+width;xx++){
      if(0 <= yy && yy < imgd->jmax &&
	 0 <= xx && xx < imgd->imax ){
	imgd->image[xx+yy*imgd->imax+odd*yy] = color;
      }
    }
  }
}

