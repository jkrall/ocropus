static char version_string[]="$Id: draw_rect.c,v 1.1.1.1 1999/10/15 12:26:21 kise Exp $";
/*
   $Date: 1999/10/15 12:26:21 $
   $Revision: 1.1.1.1 $
   $Author: kise $
*/

#include <stdio.h>
#include <stdlib.h>
#include "color.h"
#include "function.h"

void draw_rect(ImageData *imgd, int is, int js, int ie, int je,
	       int color, int width){

  int	x,y;
  int	odd;

  odd = imgd->imax%2;

  /* 横線 */
  for(y=js;y<=je&&y<=js+width;y++){
    for(x=is;x<=ie;x++){
      imgd->image[x+y*imgd->imax+odd*y] = color;
    }
  }
  for(y=je;y>=js&&y>=je-width;y--){
    for(x=is;x<=ie;x++){
      imgd->image[x+y*imgd->imax+odd*y] = color;
    }
  }
  /* 縦線 */
  for(x=is;x<=ie&&x<=is+width;x++){
    for(y=js;y<=je;y++){
      imgd->image[x+y*imgd->imax+odd*y] = color;
    }
  }
  for(x=ie;x>=is&&x>=ie-width;x--){
    for(y=js;y<=je;y++){
      imgd->image[x+y*imgd->imax+odd*y] = color;
    }
  }
}





