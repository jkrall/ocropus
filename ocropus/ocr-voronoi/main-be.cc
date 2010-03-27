/*
   $Date: 1999/10/15 12:40:27 $
   $Revision: 1.1.1.1 $
   $Author: kise $
   main.c
   メインプログラム
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "defs.h"
#include "extern.h"
#include "const.h"
#include "function.h"

using namespace voronoi;

int main(int argc, char **argv) {
    FILE		*ofp;
    int 		i;
    int                 ifargc, ofargc;
    ImageData		imgd1;

    /* analysis of arguments */
    analyze_cline(argv,&ifargc,&ofargc);
    
    /* ファイルオープン
       opening the output file */
    if((ofp = fopen(argv[ofargc],"w"))==NULL) {
	fprintf(stderr,"can't open output-file\n");
	exit(1);
    }

    /* reading the image data */
    read_image(argv[ifargc],&imgd1);

    unsigned int nlines=0;
    LineSegment	 *mlineseg;
    voronoi_pageseg(&mlineseg,&nlines,&imgd1);
    for(i=0;i<nlines;i++) {
	if(mlineseg[i].yn == OUTPUT &&
	   (mlineseg[i].xs != mlineseg[i].xe
	    || mlineseg[i].ys != mlineseg[i].ye)) {
	    fprintf(ofp,"%d %d %d %d\n",
		    mlineseg[i].xs,mlineseg[i].xe,
		    mlineseg[i].ys,mlineseg[i].ye);
	}
    }
    /* ファイルクローズ */
    fclose(ofp);
    free(bpx);
    free(noise_comp);
    free(imgd1.image);
    if(!nlines)
        free(mlineseg);
#ifdef TIME
    dtime();
#endif
}
