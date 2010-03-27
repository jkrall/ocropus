/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $

  cline.c
  引数の解析  analysis of arguments
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "const.h"
#include "extern.h"
#include "function.h"

namespace voronoi{
    void analyze_cline(char **argv, int *ifargc, int *ofargc)
    {
        int i = 1;
        int j = 0;
        int argc_tmp[2];

        while( argv[i] != NULL) {
            if(strcmp(argv[i],"-sr")==0) {
                i++;
                if(argv[i] == NULL) usage();
                sample_rate = atoi(argv[i]);
                i++;
            }
            else if(strcmp(argv[i],"-nm")==0) {
                i++;
                if(argv[i] == NULL) usage();
                noise_max = atoi(argv[i]);
                i++;
            }
            else if(strcmp(argv[i],"-fr")==0) {
                i++;
                if(argv[i] == NULL) usage();
                freq_rate = atof(argv[i]);
                i++;
            }
            else if(strcmp(argv[i],"-ta")==0) {
                i++;
                if(argv[i] == NULL) usage();
                Ta = atoi(argv[i]);
                i++;
            }
            else if(strcmp(argv[i],"-sw")==0) {
                i++;
                if(argv[i] == NULL) usage();
                smwind = atoi(argv[i]);
                i++;
            }
            else if(strcmp(argv[i],"-dparam")==0) {
                display_parameters = YES;
                i++;
            }
            else {
                argc_tmp[j] = i;
                j++;
                i++;
            }
        }
        if( j != 2 ) usage();
        else {
            *ifargc = argc_tmp[0];
            *ofargc = argc_tmp[1];
        }
    }
}
