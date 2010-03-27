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
// File: voronoi-ocropus.cc
// Purpose: Wrapper class for voronoi code
//
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "iulib/imgio.h"
#include "iulib/imglib.h"
#include "voronoi-ocropus.h"
#include "defs.h"
#include "function.h"
#include "ocropus.h"

#define MAXVAL 256

using namespace colib;
using namespace iulib;
using namespace voronoi;

namespace ocropus {

    param_string debug_voronoi("debug_voronoi",0,"output voronoi diagram image");

    static void bytearray2img(ImageData *imgd, bytearray &image){
        int width  = image.dim(0);
        int height = image.dim(1);

        if((imgd->image=(char *)malloc(width*height))==NULL){
            fprintf(stderr,"bytearray2imgd: not enough memory for image\n");
            exit(1);
        }
        /* setting dimension */
        imgd->imax=width;
        imgd->jmax=height;

        for(int y=0; y<height; y++){
            for(int x=0; x<width; x++){
               imgd->image[x+y*width] = image(x,y);
            }
        }

        /* cleaning the right edge of the image */
        char            emask = 0x00;
        unsigned long   h=imgd->jmax;
        /*for( i = 0 ; i < w%BYTE ; i++)
          emask|=(0x01 << (BYTE-i-1));*/
        for(int j = 0 ; j < h ; j++)
            *((imgd->image)+(j+1)*imgd->imax-1)&=emask;

        imgd->imax = image.dim(0)*8;

    }

    static void img2bytearray(intarray &image, ImageData *imgd){
        int width  = imgd->imax;
        int height = imgd->jmax;
        image.resize(width,height);

        for(int y=0; y<height; y++){
            for(int x=0; x<width; x++){
               unsigned char val = imgd->image[x+y*width];
               if(val == WHITE)
                   image(x,height-y-1) = 0x00ffffff;
               else if(val == BLACK)
                   image(x,height-y-1) = 0;
               else
                   image(x,height-y-1) = val;
            }
        }

    }

    static void byte2bit(bytearray &cimg, bytearray &in_img){
        // compress to 1bit/pixel and invert as required by voronoi code
        bytearray img;
        copy(img,in_img);
        int width  = in_img.dim(0);
        int height = in_img.dim(1);

        for(int x=0; x<width; x++)
            for(int y=0; y<height; y++)
                if(img(x,y)>128)
                    img(x,y)=0;
                else
                    img(x,y)=1;

        unsigned char b0,b1,b2,b3,b4,b5,b6,b7;
        // Add a white strip if image width is not a multiple of 8
        int offset = img.dim(0)%8;
        int newxdim = img.dim(0)>>3;
        if(offset)
            newxdim++;
        cimg.resize(newxdim,img.dim(1));
        for(int y=0,yi=height-1; y<height; y++,yi--){
            int x,xi,w=cimg.dim(0);
            for(x=0,xi=0; (x<w && xi+7<img.dim(0)); x++, xi+=8){
                b7 = (img(xi,yi)<<7)   & 0x80;
                b6 = (img(xi+1,yi)<<6) & 0x40;
                b5 = (img(xi+2,yi)<<5) & 0x20;
                b4 = (img(xi+3,yi)<<4) & 0x10;
                b3 = (img(xi+4,yi)<<3) & 0x08;
                b2 = (img(xi+5,yi)<<2) & 0x04;
                b1 = (img(xi+6,yi)<<1) & 0x02;
                b0 = (img(xi+7,yi)<<0) & 0x01;
                cimg(x,y)= b0| b1| b2| b3| b4| b5| b6| b7 ;
            }
            // If image width is not a multiple of 8
            if(offset){
                unsigned char lastbyte=0;
                while(xi<img.dim(0)){
                    lastbyte |= img(xi++,yi);
                    lastbyte = lastbyte << 1;
                }
                if(x<w)
                    cimg(x,y)=lastbyte;
            }
        }
    }

    void SegmentPageByVORONOI::segment(intarray &out_image,bytearray &in_image){

        if(!contains_only(in_image,byte(0),byte(255))){
            fprintf(stderr,"Voronoi algorithm needs binary input image.\n");
            exit(1);
        }

        ImageData imgd_in,imgd_out;
        bytearray in_bitimage;
        intarray voronoi_diagram_image;

        byte2bit(in_bitimage,in_image);

        bytearray2img(&imgd_in,in_bitimage);

        set_param(nm,sr,fr,ta);
        voronoi_colorseg(&imgd_out,&imgd_in,remove_noise);

        img2bytearray(voronoi_diagram_image,&imgd_out);
        //simple_recolor(voronoi_diagram_image);
        if(debug_voronoi)
            write_image_packed(debug_voronoi,voronoi_diagram_image);

        intarray voronoi_zones;
        makelike(voronoi_zones,voronoi_diagram_image);
        for(int i=0; i<voronoi_diagram_image.length1d(); i++){
            if(voronoi_diagram_image.at1d(i)==0x00ffffff ||
               voronoi_diagram_image.at1d(i)==0)
                voronoi_zones.at1d(i) = 1;
            else
                voronoi_zones.at1d(i) = 0;
        }

        label_components(voronoi_zones,false);

        makelike(out_image,in_image);
        ASSERT(in_image.length1d()<=voronoi_zones.length1d());
        for(int x=0,w=in_image.dim(0);x<w;x++){
            for(int y=0,h=in_image.dim(1);y<h;y++){
                if(in_image(x,y)==0 &&
                   voronoi_zones(x,y)!=0 &&
                   voronoi_diagram_image(x,y)!=0x00ffffff)
                    out_image(x,y) = voronoi_zones(x,y);
                else
                    out_image(x,y) = 0x00ffffff;
            }
        }

        free(imgd_in.image);
        free(imgd_out.image);
    }

    ISegmentPage *make_SegmentPageByVORONOI() {
        return new SegmentPageByVORONOI();
    }

} //namespace
