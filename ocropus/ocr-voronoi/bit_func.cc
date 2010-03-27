/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
  bit_func.c
  1画素1ビットの画像用のライブラリ

  使用例
  bit_get(imgd,i,j)
  bit_set(imgd,i,j,1)
  frame(imgd)

*/

#include <stdio.h>
#include "const.h"
#include "defs.h"
#include "extern.h"

namespace voronoi{
    /* make_mask() を廃止して、データとして書くことにした。*/
    unsigned char mask[8]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
    unsigned char not_mask[8]={0x7f,0xbf,0xdf,0xef,0xf7,0xfb,0xfd,0xfe};

    /*
     * bit get
     * 画像imgd の画素(i,j)の値を返す関数
     * this function returns the value (1 or 0) of a pixel (i,j) in imgd
     */
    int bit_get( ImageData *imgd, Coordinate i, Coordinate j )
    {
        if( *( imgd->image+(imgd->imax*j+i)/BYTE) & mask[i%BYTE] )
            return(1);
        else
            return(0);
    }

    /*
     * bit set (b=1) & reset (b=0)
     * 画像imgd の画素(i,j)をb の値にセットする.
     * this function sets the value of a pixel (i,j) in imgd to b
     */ 
    void bit_set( ImageData *imgd, Coordinate i, Coordinate j ,int b )
    {
        if(b)
            *( imgd->image+(j*imgd->imax+i)/BYTE) |= mask[i%BYTE];
        else
            *( imgd->image+(j*imgd->imax+i)/BYTE) &= not_mask[i%BYTE];
    }

    /* 
     * １バイトのデータのなかで、kから(kを含む)最初にビットが
     * 立っている場所を返す。k以降がすべて０ならば、BYTE(８)を返す。
     */
    int byte_pos (char byte, int k)
    {
        int i;
        for(i=k;i<BYTE;i++){
            if(byte&mask[i])
                return(i);
        }
        return(BYTE);
    }

    /* 画像の枠をvalue の値に設定する関数 */
    void frame( ImageData *imgd, int width, int value)
    {

        Coordinate i,j;

        /* 周囲をvalueにセット */
        for( i=0 ; i<imgd->imax; i++){
            for( j=0 ; j<width ; j++){
                bit_set( imgd, i, j, value );
                bit_set( imgd, i, imgd->jmax-1-j, value );
            }
        }
        for( j=0; j<imgd->jmax; j++){
            for( i=0 ; i<width ; i++){
                bit_set( imgd, i, j, value );
                bit_set( imgd, imgd->imax-1-i, j, value );
            }
        }
    }
}

