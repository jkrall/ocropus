static char version_string[]="$Id: util.c,v 1.1.1.1 1999/10/15 12:26:21 kise Exp $";
/*
   $Date: 1999/10/15 12:26:21 $
   $Revision: 1.1.1.1 $
   $Author: kise $
  
   1画素1ビットの画像用のライブラリ
   
   使用例
   bit_get(imgd,i,j,odd)
   make_mask()

   注)make_mask()はbit_setを使う前に
   main関数内で呼び出すこと。
*/
#include<stdio.h>
#include "color.h"
#include "function.h"

unsigned char mask[8];
unsigned char not_mask[8];

int bit_get( ImageData *imgd, register int i, register int j ,int odd){
  if(odd){
    /* Sun Raster の２値画像は, 横幅が16の倍数でない場合,
       (横幅分のビット数)＋(データが途中まで入った2byte 分を最後まで
       埋める0)
       */

    if( *( imgd->image+(imgd->imax/(2*BYTE)+1)*2*j+i/BYTE)
	& mask[i%BYTE] )
      return(1);
    else
      return(0);
  }
  else{
    if( *( imgd->image+(j*imgd->imax+i)/BYTE) & mask[i%BYTE] )
      return(1);
    else
      return(0);
  }
}

void make_mask()
{
  int 	i;

  for(i=0;i<8;i++){
    mask[i]=(0x01 << (BYTE-i-1));
    not_mask[i]=~mask[i];
  }
}

/* 2値画像を8bit カラー画像に変更 */
void bit_to_byte( ImageData *in_imgd, ImageData *out_imgd, int noimage){

  int		i, j;
  int		odd;

  odd = in_imgd->imax%2;

  make_mask();
    
  for(j=0; j<in_imgd->jmax; j++){
    for(i=0; i<in_imgd->imax; i++){
      if(noimage==YES){
	out_imgd->image[i+(j*in_imgd->imax)+odd*j] = WHITE;
      }
      else{
	if( bit_get( in_imgd, i, j, odd ) ){
	  out_imgd->image[i+(j*in_imgd->imax)+odd*j] = BLACK;
	}
	else{
	  out_imgd->image[i+(j*in_imgd->imax)+odd*j] = WHITE;
	}
      }
    }
    if( (j<(in_imgd->jmax - 1)) && odd ){
      out_imgd->image[(j+1)*in_imgd->imax+j] = 0x00;
    }
  }
}

/* 画像の周囲waku画素分をLINE_C にセットする.
   プログラムに依存しているので, 他では使用しにくい. */
void frame(ImageData *out_imgd, int waku, int value){
    
  int i,j;
  int	odd;

  odd = out_imgd->imax%2;
    
  /* 周囲をvalueにセット */
  for( i=0 ; i<out_imgd->imax; i++){
    for( j=0 ; j<waku ; j++){
      out_imgd->image[i+(j*out_imgd->imax)+odd*j] = value;
      out_imgd->image[i+((out_imgd->jmax-1-j)*out_imgd->imax)
		     +odd*(out_imgd->jmax-1-j)] = value;
    }
  }
  for( j=0; j<out_imgd->jmax; j++){
    for( i=0 ; i<waku ; i++){
      out_imgd->image[i+(j*out_imgd->imax)+odd*j] = value;
      out_imgd->image[(out_imgd->imax-1-i)+(j*out_imgd->imax)
		     +odd*j] = value;	    
    }
  }
}  
