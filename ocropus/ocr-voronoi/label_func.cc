/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
  label_func.c
  ラベル画像 *limg を扱うファイル
*/

#include <stdio.h>
#include <stdlib.h>
#include "const.h"
#include "defs.h"
#include "extern.h"
#include "function.h"

namespace voronoi{
    Label      *limg;
    Coordinate imax, jmax;

    /*
     * label format
     * ラベル画像limg を初期化する関数
     */
    void lab_format(ImageData *imgd)
    {
        NumPixel i;

        imax=imgd->imax;
        jmax=imgd->jmax;

        // Faisal Shafait's modification: changed sizeof(short) to sizeof(int)	
        limg=(Label *)myalloc(jmax*imax*sizeof(unsigned int));

        if(sizeof(Label)==sizeof(int)){
            for(i=0;i<jmax*imax;i++)
                *(limg+i)=NOLABEL;
        }
        else if(sizeof(Label)==sizeof(short)){
            /* just for efficiency */
            /* Faisal Shafait's modification to fix segmentation fault on 64-bit machines*/
            int factor = sizeof(long)/sizeof(short);
            for(i=0;i<(int)(jmax*imax)/factor;i++)
                *((unsigned long *)limg+i)=UNSIGNED_MAX;
            if((jmax*imax)%factor!=0){
                for(i=1;i<factor;i++)
                    *(limg+jmax*imax-i)=NOLABEL;
            }
            /* End of Faisal Shafait's modification*/
        }
        else{
            fprintf(stderr,"lab_format : the size of Label must be short or int\n");
            exit(1);
        }
    }

    /*
     * label get
     * ラベル画像limg の画素(i,j)の値を返す関数
     */
    Label lab_get(Coordinate i, Coordinate j)
    {
        return(*(limg+imax*j+i));
    }

    /*
     * label set
     * ラベル画像limg の画素(i,j)の値をln にセットする関数
     */
    void lab_set(Coordinate i, Coordinate j, Label ln)
    {
        *(limg+imax*j+i) = ln;
    }

    /*
     * free limg
     * ラベル画像limg の領域解放
     */
    void free_limg()
    {
        free(limg);
    }
}

