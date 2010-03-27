static char version_string[]="$Id: draw_line.c,v 1.1.1.1 1999/10/15 12:26:21 kise Exp $";
/*
   $Date: 1999/10/15 12:26:21 $
   $Revision: 1.1.1.1 $
   $Author: kise $
*/
#include <stdio.h>
#include <stdlib.h>
#include "color.h"
#include "function.h"

void draw_line(ImageData *imgd, int is, int js, int ie, int je,
	       int color, int width){

  int	x,y;
  int	d;
  int	w, h;
  int	xs,xe,ys,ye;
  int dx,dy;
  int from, to;

  int	odd;

  from = - (int)(width/2);
  to = (int)(width/2);
    
  odd = imgd->imax%2;

  w=abs(is-ie);
  h=abs(js-je);

  if(w>h){ /* 横長 */
    if(is<ie){ /* x座標の小さい方の点を(xs, ys)とする */
      xs=is; xe=ie; ys=js; ye=je;
    }
    else{
      xs=ie; xe=is; ys=je; ye=js;
    }
    if(ys<ye){		/* type 2 左上から右下への線分 */
      d=1;
    }
    else{			/* type 4 左下から右上への線分 */
      d=-1;
    }
    for(x=xs;x<=xe;x=x++){
      y=ys+d*(int)((float)h*(float)(x-xs)/(float)w+0.5);
      for(dx=from;dx<=to;dx++){
	for(dy=from;dy<=to;dy++){
	  if((x+dx >= 0)&&(x+dx < imgd->imax)&&
	     (y+dy >= 0)&&(y+dy < imgd->jmax)){
	    imgd->image[(x+dx)+(y+dy)*imgd->imax+odd*y] = color;
	  }
	}
      }
    }
  }
  else if (h!=0){  /* 縦長 */
    if(js<je){ /* y座標の小さい方の点を(xs, ys)とする */
      xs=is; xe=ie; ys=js; ye=je;
    }
    else{
      xs=ie; xe=is; ys=je; ye=js;
    }
    if(xs<xe){		/* type 1 左上から右下への線分 */
      d=1;
    }
    else{			/* type 3 右上から左下への線分 */
      d=-1;
    }
    for(y=ys;y<=ye;y++){
      x=xs+d*(int)((float)w*(float)(y-ys)/(float)h+0.5);
      for(dx=from;dx<=to;dx++){
	for(dy=from;dy<=to;dy++){
	  if((x+dx >= 0)&&(x+dx < imgd->imax)&&
	     (y+dy >= 0)&&(y+dy < imgd->jmax)){
	    imgd->image[(x+dx)+(y+dy)*imgd->imax+odd*y] = color;
	  }
	}
      }
    }
  }
}
