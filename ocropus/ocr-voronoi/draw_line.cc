#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "defs.h"
#include "const.h"
#include "function.h"

namespace voronoi{
    unsigned char bitmask[8];
    unsigned char not_bitmask[8];

    int draw_bit_get( ImageData *imgd, register int i, register int j ,int odd){
        if(odd){
            /* Sun Raster の２値画像は, 横幅が16の倍数でない場合,
               (横幅分のビット数)＋(データが途中まで入った2byte 分を最後まで
               埋める0)
            */

            if( *( imgd->image+(imgd->imax/(2*BYTE)+1)*2*j+i/BYTE)
                & bitmask[i%BYTE] )
                return(1);
            else
                return(0);
        }
        else{
            if( *( imgd->image+(j*imgd->imax+i)/BYTE) & bitmask[i%BYTE] )
                return(1);
            else
                return(0);
        }
    }

    void make_bitmask(void){
        int 	i;

        for(i=0;i<8;i++){
            bitmask[i]=(0x01 << (BYTE-i-1));
            not_bitmask[i]=~bitmask[i];
        }
    }

    /* 2値画像を8bit カラー画像に変更 */
    void bit_to_byte( ImageData *in_imgd, ImageData *out_imgd, int noimage){

        int		i, j;
        int		odd;

        odd = in_imgd->imax%2;

        make_bitmask();
    
        for(j=0; j<in_imgd->jmax; j++){
            for(i=0; i<in_imgd->imax; i++){
                if(noimage==YES){
                    out_imgd->image[i+(j*in_imgd->imax)+odd*j] = WHITE;
                }
                else{
                    if( draw_bit_get( in_imgd, i, j, odd ) ){
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

    void draw_line(ImageData *imgd, int is, int js, int ie, int je, int color, int width){

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

        if(w>h){
            if(is<ie){
                xs=is; xe=ie; ys=js; ye=je;
            }
            else{
                xs=ie; xe=is; ys=je; ye=js;
            }
            if(ys<ye){		/* type 2 */
                d=1;
            }
            else{			/* type 4 */
                d=-1;
            }
            for(x=xs;x<=xe;x++){
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
        else if (h!=0){ 
            if(js<je){ 
                xs=is; xe=ie; ys=js; ye=je;
            }
            else{
                xs=ie; xe=is; ys=je; ye=js;
            }
            if(xs<xe){		/* type 1 */
                d=1;
            }
            else{			/* type 3 */
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
}
