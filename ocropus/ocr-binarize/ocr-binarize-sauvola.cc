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
// File: ocr-binarize-sauvola.cc
// Purpose: An efficient implementation of the Sauvola's document binarization
//          algorithm based on integral images as described in
//          F. Shafait, D. Keysers, T.M. Breuel. "Efficient Implementation of
//          Local Adaptive Thresholding Techniques Using Integral Images".
//          Document Recognition and Retrieval XV, San Jose.
//
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "colib/iarith.h"

#define MAXVAL 256

using namespace iulib;
using namespace colib;

namespace ocropus {

    param_float sauvola_k("sauvola_k",0.3,"Weighting factor");
    param_int   sauvola_w("sauvola_w",40,"Local window size. Should always be positive");
    param_string debug_binarize("debug_binarize",0,"output the result of binarization");

    struct BinarizeBySauvola : IBinarize {
        float k;
        int w;
        int whalf; // Half of window size
        ~BinarizeBySauvola() {}

        const char *description() {
            return "An efficient implementation of the Sauvola's document \
binarization algorithm based on integral images.\n";
        }

        const char *name() {
            return "binsauvola";
        }

        void set(const char *key,double value) {
            if(!strcmp(key,"k")) this->k = value;
            else if(!strcmp(key,"w")) this->w = int(value);
            else throw "unknown parameter";
        }

        BinarizeBySauvola() {
            w = sauvola_w;
            k = sauvola_k;
        }

        void binarize(bytearray &out, floatarray &in){
            bytearray image;
            copy(image,in);
            binarize(out,image);
        }

        void binarize(bytearray &bin_image, bytearray &gray_image){
            whalf = w>>1;
            // fprintf(stderr,"[sauvola %g %d]\n",k,w);
            CHECK_ARG(k>=0.05 && k<=0.95);
            CHECK_ARG(w>0 && k<1000);
            if(bin_image.length1d()!=gray_image.length1d())
                makelike(bin_image,gray_image);

            if(contains_only(gray_image,byte(0),byte(255))){
                copy(bin_image,gray_image);
                return ;
            }

            int image_width  = gray_image.dim(0);
            int image_height = gray_image.dim(1);
            whalf = w>>1;

            // Calculate the integral image, and integral of the squared image
            narray<int64_t> integral_image,rowsum_image;
            narray<int64_t> integral_sqimg,rowsum_sqimg;
            makelike(integral_image,gray_image);
            makelike(rowsum_image,gray_image);
            makelike(integral_sqimg,gray_image);
            makelike(rowsum_sqimg,gray_image);
            int xmin,ymin,xmax,ymax;
            double diagsum,idiagsum,diff,sqdiagsum,sqidiagsum,sqdiff,area;
            double mean,std,threshold;

            for(int j=0; j<image_height; j++){
                rowsum_image(0,j) = gray_image(0,j);
                rowsum_sqimg(0,j) = gray_image(0,j)*gray_image(0,j);
            }
            for(int i=1; i<image_width; i++){
                for(int j=0; j<image_height; j++){
                    rowsum_image(i,j) = rowsum_image(i-1,j) + gray_image(i,j);
                    rowsum_sqimg(i,j) = rowsum_sqimg(i-1,j) + gray_image(i,j)*gray_image(i,j);
                }
            }

            for(int i=0; i<image_width; i++){
                integral_image(i,0) = rowsum_image(i,0);
                integral_sqimg(i,0) = rowsum_sqimg(i,0);
            }
            for(int i=0; i<image_width; i++){
                for(int j=1; j<image_height; j++){
                    integral_image(i,j) = integral_image(i,j-1) + rowsum_image(i,j);
                    integral_sqimg(i,j) = integral_sqimg(i,j-1) + rowsum_sqimg(i,j);
                }
            }

            //Calculate the mean and standard deviation using the integral image

            for(int i=0; i<image_width; i++){
                for(int j=0; j<image_height; j++){
                    xmin = max(0,i-whalf);
                    ymin = max(0,j-whalf);
                    xmax = min(image_width-1,i+whalf);
                    ymax = min(image_height-1,j+whalf);
                    area = (xmax-xmin+1)*(ymax-ymin+1);
                    // area can't be 0 here
                    // proof (assuming whalf >= 0):
                    // we'll prove that (xmax-xmin+1) > 0,
                    // (ymax-ymin+1) is analogous
                    // It's the same as to prove: xmax >= xmin
                    // image_width - 1 >= 0         since image_width > i >= 0
                    // i + whalf >= 0               since i >= 0, whalf >= 0
                    // i + whalf >= i - whalf       since whalf >= 0
                    // image_width - 1 >= i - whalf since image_width > i
                    // --IM
                    ASSERT(area);
                    if(!xmin && !ymin){ // Point at origin
                        diff   = integral_image(xmax,ymax);
                        sqdiff = integral_sqimg(xmax,ymax);
                    }
                    else if(!xmin && ymin){ // first column
                        diff   = integral_image(xmax,ymax) - integral_image(xmax,ymin-1);
                        sqdiff = integral_sqimg(xmax,ymax) - integral_sqimg(xmax,ymin-1);
                    }
                    else if(xmin && !ymin){ // first row
                        diff   = integral_image(xmax,ymax) - integral_image(xmin-1,ymax);
                        sqdiff = integral_sqimg(xmax,ymax) - integral_sqimg(xmin-1,ymax);
                    }
                    else{ // rest of the image
                        diagsum    = integral_image(xmax,ymax) + integral_image(xmin-1,ymin-1);
                        idiagsum   = integral_image(xmax,ymin-1) + integral_image(xmin-1,ymax);
                        diff       = diagsum - idiagsum;
                        sqdiagsum  = integral_sqimg(xmax,ymax) + integral_sqimg(xmin-1,ymin-1);
                        sqidiagsum = integral_sqimg(xmax,ymin-1) + integral_sqimg(xmin-1,ymax);
                        sqdiff     = sqdiagsum - sqidiagsum;
                    }

                    mean = diff/area;
                    std  = sqrt((sqdiff - diff*diff/area)/(area-1));
                    threshold = mean*(1+k*((std/128)-1));
                    if(gray_image(i,j) < threshold)
                        bin_image(i,j) = 0;
                    else
                        bin_image(i,j) = MAXVAL-1;
                }
            }
            if(debug_binarize) {
                write_png(stdio(debug_binarize, "w"), bin_image);
            }
        }

    };

    IBinarize *make_BinarizeBySauvola() {
        return new BinarizeBySauvola();
    }

} //namespace
