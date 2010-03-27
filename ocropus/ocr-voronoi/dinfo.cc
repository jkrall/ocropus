/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
  dinfo.c
*/
#include <stdio.h>
#include "extern.h"

namespace voronoi{
    void dparam()
    {
        fprintf(stderr,"Parameters : \n");

        fprintf(stderr,
                "sampling rate\t%d\nnoise max\t%d\n",sample_rate,noise_max);
        fprintf(stderr,"freq_rate\t%f\n",freq_rate);
        fprintf(stderr,"Ta\t\t%d\n",Ta);
        fprintf(stderr,"smwind\t\t%d\n",smwind);
    }

    void dnumber(int imax, int jmax)
    {
        fprintf(stderr,"---------------------------------------------------------\n");
        fprintf(stderr,"image size \t\t\t\t\t%d x %d\n",imax,jmax);
        fprintf(stderr,"black pixels\t\t\t\t\t%d\n",BPnbr);
        fprintf(stderr,"connected components\t\t\t\t%d\n",LABELnbr);
        fprintf(stderr,"sample points\t\t\t\t\t%d\n",sample_pix);
        fprintf(stderr,"point Voronoi line segments\t\t\t%d\n",point_edge);
        fprintf(stderr,"area Voronoi line segments (before erasing)\t%d\n",LINEnbr);
        fprintf(stderr,"area Voronoi line segments (after erasing)\t%d\n",edge_nbr);
        fprintf(stderr,"---------------------------------------------------------\n");
    }

#ifdef TIME
    void dtime()
    {
        fprintf(stderr,"\timg_to_site\tvoronoi\t\terase\t\toutput\n");
        fprintf(stderr,"Time\t%.3f\t\t%.3f\t\t%.3f\t\t%.3f\n",
                b_s_time,v_time,e_time,o_time);
    }
#endif
}
