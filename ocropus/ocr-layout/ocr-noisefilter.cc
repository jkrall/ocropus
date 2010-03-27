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
// File: ocr-noisefilter.cc
// Purpose: Docuemnt image cleanup using projection profiles and
//          connected component filtering.
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace ocropus;
using namespace colib;
namespace ocropus {


    NoiseFilter::NoiseFilter() {
        xstep=5;
        ystep=5;
        xwidth=5;
        yheight=5;
        bfthreshold=0.70;
        wfthreshold=0.995;
        max_width=-1;
        max_height=-1;
        border_margin=25;
    }


    NoiseFilter::~NoiseFilter() {}




    float NoiseFilter::blackRatio(int x0,int y0,int x1,int y1,bytearray &image){
        float result=0.0;
        int blackpixels=0;
        int total = (x1-x0+1) * (y1-y0+1);
        for(int i=x0 ;i<=x1 ;i++) {
            for(int j=y0; j<=y1 ;j++) {
                if(image(i,j)==black) {blackpixels++;}
            }
        }
        result=(float)blackpixels/(float)total;
        return result;
    }

    void NoiseFilter::remove(int x0,int y0,int x1,int y1,bytearray &image) {

        int white = 0xff;
        int black = 0x00;
        for (int i=x0 ; i<=x1 ;i++) {
            for (int j=y0; j<=y1 ;j++) {
                if(image(i,j)==black){image(i,j)=white;}
            }
        }
    }

    void NoiseFilter::blackFilter(bytearray &out , bytearray &original,
                                  float threshold, int xstep, int ystep,
                                  int xwidth, int yheight){
        bytearray in,in_removable;
        copy(in_removable,original);
        copy(in,original);
        int ximage=in.dim(0);
        int yimage=in.dim(1);
        int left;
        int right;
        int top;
        int bottom;
        int leftr=0;
        int rightr=ximage-1;
        float blackratio=0.0;
        //scan the left border first
        int start = (int) ximage/3;
        right=start;
        bottom=0;
        top=yimage-1;
        left=start-xwidth+1;
        while(left>=0) {
            blackratio=blackRatio(left,bottom,right,top,in);
            if(blackratio>threshold) {
                remove(0,0,right,top,in_removable);
                leftr=right;
                break;
            }
            right=right-xstep;
            left=left-xstep;
        }
        //scan the right border now
        start=(int) (ximage*2)/3;
        left=start;
        bottom=0;
        top=yimage-1;
        right=start+xwidth-1;
        while(right<ximage) {
            blackratio=blackRatio(left,bottom,right,top,in);
            if(blackratio>threshold) {
                remove(left,bottom,ximage-1,top,in_removable);
                rightr=left;
                break;
            }
            right=right+xstep;
            left=left+xstep;
        }
        //scan the top border now
        start = (int) (yimage*24)/25;
        left=leftr;
        right=rightr;
        bottom=start;
        top=start+yheight-1;
        while(top<yimage) {
            blackratio=blackRatio(left,bottom,right,top,in);
            if(blackratio>threshold){
                remove(0,bottom,ximage-1,yimage-1,in_removable);
                break;
            }
            top=top+ystep;
            bottom=bottom+ystep;
        }
        //scan bottom border
        start = (int) yimage/25;
        left=leftr;
        right=rightr;
        top=start;
        bottom=start-yheight+1;
        while(bottom>=0){
            blackratio=blackRatio(left,bottom,right,top,in);
            if(blackratio>threshold){
                remove(0,0,ximage-1,top,in_removable);
                break;
            }
            top=top-ystep;
            bottom=bottom-ystep;
        }
        copy(out,in_removable);
    }


    float NoiseFilter::whiteRatio(int x0,int y0,int x1,int y1,bytearray &image) {
        float whiteratio= (1.0-blackRatio(x0,y0,x1,y1,image));
        return whiteratio;
    }
    void NoiseFilter::whiteFilter(bytearray &out,bytearray &original,
                                  float threshold, int xstep, int ystep,
                                  int xwidth, int yheight) {
        bytearray in,in_removable;
        copy(in,original);
        copy(in_removable,original);
        int ximage=in.dim(0);
        int yimage=in.dim(1);
        int left;
        int right;
        int top;
        int bottom;
        int leftr=0;
        int rightr=ximage-1;
        float whiteratio;
        //scanning the left border first
        int start= (int) ximage/5;
        right=start;
        bottom=0;
        top=yimage-1;
        left=start-xwidth+1;
        while(left>=0){
            whiteratio=whiteRatio(left,bottom,right,top,in);
            if(whiteratio>threshold){
                remove(0,0,right-xwidth+1,top,in_removable);
                leftr=right;
                //printf("removed left %g\n",whiteratio);
                break;
            }
            right=right-xstep;
            left=left-xstep;
        }
        //scan the right border now
        start=(int) (ximage*4)/5;
        left=start;
        bottom=0;
        top=yimage-1;
        right=start+xwidth-1;
        while(right<ximage) {
            whiteratio=whiteRatio(left,bottom,right,top,in);
            if(whiteratio>threshold) {
                remove(left+xwidth-1,bottom,ximage-1,top,in_removable);
                rightr=left;
                //printf("removed right %g\n",whiteratio);
                break;
            }
            right=right+xstep;
            left=left+xstep;
        }
        //scan the top border now
        start = (int) (yimage*24)/25;
        left=leftr;
        right=rightr;
        bottom=start;
        top=start+yheight-1;
        while(top<yimage) {
            whiteratio=whiteRatio(left,bottom,right,top,in);
            if(whiteratio>threshold){
                remove(0,bottom+yheight-1,ximage-1,yimage-1,in_removable);
                break;
            }
            top=top+ystep;
            bottom=bottom+ystep;
        }
        //scan the bottom border now
        start =(int) (yimage)/50;
        left=leftr;
        right=rightr;
        top=start;
        bottom=start-yheight+1;
        while(bottom>=0){
            whiteratio=whiteRatio(left,bottom,right,top,in);
            if(whiteratio>threshold){
                remove(0,0,ximage-1,top-yheight+1,in_removable);
                //printf("removed bottom %g\n",whiteratio);
                break;
            }
            top=top-ystep;
            bottom=bottom-ystep;
        }
        copy(out,in_removable);
    }

    void NoiseFilter::ccanalysis(bytearray &out,bytearray &in,rectarray &bboxes){
        //max_width = (int) (2*in.dim(0)/3.0);
        //max_height = (int) (2*in.dim(1)/3.0);
        max_width = in.dim(0);
        max_height = in.dim(1);
        for(int i=0; i<bboxes.length(); i++){
            if(bboxes(i).width()  >= max_width)  continue;
            if(bboxes(i).height() >= max_height) continue;
            if(bboxes(i).x0 < border_margin )    continue;
            if(bboxes(i).y0 < border_margin )    continue;
            if(bboxes(i).x1 > in.dim(0) - border_margin )   continue;
            if(bboxes(i).y1 > in.dim(1) - border_margin )   continue;

            for(int y=bboxes(i).y0; y<bboxes(i).y1; y++){
                for(int x=bboxes(i).x0; x<bboxes(i).x1; x++){
                    out(x,y) = in(x,y);
                }
            }
        }
    }


    void NoiseFilter::blackFilter(bytearray &out,bytearray &in){
        blackFilter(out,in,bfthreshold,xstep,ystep,xwidth,yheight);
    }
    void NoiseFilter::whiteFilter(bytearray &out,bytearray &in){
        whiteFilter(out,in,wfthreshold,xstep,ystep,xwidth,yheight);
    }
    bool is_line_image(bytearray &in){
        int line_threshold=500;
        return (in.dim(1) < line_threshold);
    }

    NoiseFilter *make_NoiseFilter() {
        return new NoiseFilter();
    }

}
