// -*- C++ -*-

// Copyright 2006-2008 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
// or its licensors, as applicable.
//
// You may not use this file except under the terms of the accompanying license.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You may
// obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Project: OCRopus
// File: ocr-binarize-otsu.cc
// Purpose: An implementation of Otsu's global thresholding method
//
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"

#define MAXVAL 256

using namespace iulib;
using namespace colib;

namespace ocropus {

    param_string debug_otsu("debug_otsu",0,"output the result of binarization");

    struct BinarizeByOtsu : IBinarize {
        ~BinarizeByOtsu() {}

        const char *description() {
            return "An implementation of Otsu's binarization algorithm.\n";
        }

        const char *name() {
            return "binotsu";
        }

        BinarizeByOtsu() {
        }

        void binarize(bytearray &out, floatarray &in){
            bytearray image;
            copy(image,in);
            binarize(out,image);
        }

        void binarize(bytearray &bin_image, bytearray &gray_image){
            if(bin_image.length1d()!=gray_image.length1d())
                makelike(bin_image,gray_image);

            if(contains_only(gray_image,byte(0),byte(255))){
                copy(bin_image,gray_image);
                return ;
            }

            int image_width  = gray_image.dim(0);
            int image_height = gray_image.dim(1);
            int    hist[MAXVAL];
            double pdf[MAXVAL]; //probability distribution
            double cdf[MAXVAL]; //cumulative probability distribution
            double myu[MAXVAL];   // mean value for separation
            double max_sigma, sigma[MAXVAL]; // inter-class variance

            /* Histogram generation */
            for(int i=0; i<MAXVAL; i++){
                hist[i] = 0;
            }
            for(int x=0; x<image_width; x++){
                for(int y=0; y<image_height; y++){
                    hist[gray_image(x,y)]++;
                }
            }

            /* calculation of probability density */
            for(int i=0; i<MAXVAL; i++){
                pdf[i] = (double)hist[i] / (image_width * image_height);
            }

            /* cdf & myu generation */
            cdf[0] = pdf[0];
            myu[0] = 0.0;       /* 0.0 times prob[0] equals zero */
            for(int i=1; i<MAXVAL; i++){
                cdf[i] = cdf[i-1] + pdf[i];
                myu[i] = myu[i-1] + i*pdf[i];
            }

            /* sigma maximization
               sigma stands for inter-class variance
               and determines optimal threshold value */
            int threshold = 0;
            max_sigma = 0.0;
            for(int i=0; i<MAXVAL-1; i++){
                if(cdf[i] != 0.0 && cdf[i] != 1.0){
                    double p1p2 = cdf[i]*(1.0 - cdf[i]);
                    double mu1mu2diff = myu[MAXVAL-1]*cdf[i]-myu[i];
                    sigma[i] = mu1mu2diff * mu1mu2diff / p1p2;
                }
                else
                    sigma[i] = 0.0;
                if(sigma[i] > max_sigma){
                    max_sigma = sigma[i];
                    threshold = i;
                }
            }


            for(int x=0; x<image_width; x++){
                for(int y=0; y<image_height; y++){
                     if (gray_image(x,y) > threshold)
                        bin_image(x,y) = MAXVAL-1;
                    else
                        bin_image(x,y) = 0;
                }
            }

            if(debug_otsu) {
                fprintf(stderr,"Otsu threshold value = %d\n", threshold);
                write_png(stdio(debug_otsu, "w"), bin_image);
            }
        }

    };

    IBinarize *make_BinarizeByOtsu() {
        return new BinarizeByOtsu();
    }

} //namespace
