/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
*/
#include <stdio.h>
#include <stdlib.h>
#include "const.h"

namespace voronoi{
    void usage()
    {
        fprintf(stderr,"\nUsage : be ");
        fprintf(stderr,"<input_file> <output_file>\n");
        fprintf(stderr," input_file  : a binary doc. image (sun raster or tiff format)\n");
        fprintf(stderr," output_file : line segments(ascii)\n");
        fprintf(stderr," options : [-<argument> (<data type>)] : <meaning>:<<default value>>\n");
        fprintf(stderr," [-sr (int)]  : sampling rate                        :<%d>\n",
                SAMPLE_RATE);
        fprintf(stderr," [-nm (int)]  : max size (area) of noise CC          :<%d>\n",
                NOISE_MAX);
        fprintf(stderr," [-fr (float)]: threshold setting param. for Td2     :<%.3f>\n",
                FREQ_RATE);
        fprintf(stderr," [-ta (int)]  : threshold value for area ratio       :<%d>\n",
                Ta_CONST);
        fprintf(stderr," [-sw (int)]  : size of smoothing window of distance :<%d>\n",
                SMWIND);
        fprintf(stderr," [-dparam]    : display parameters                   :<NO>\n");
        exit(-1);
    }
}






