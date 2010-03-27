// -*- C++ -*-

// Copyright 2006-2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
// or its licensors, as applicable.
// Copyright 1995-2005 by Thomas M. Breuel
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
// Project:
// File:
// Purpose:
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites:


// MAYBE add garbage collection of unreachable components

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "colib/colib.h"
#include "iulib/iulib.h"
#include "ocropus.h"

using namespace ocropus;
using namespace colib;

#define MAXFLOAT 3.40282347e+38F

struct Rect {
    int x0,y0,x1,y1;
    Rect():x0(1),y0(1),x1(0),y1(0) {}
    Rect(int x0,int y0,int x1,int y1):x0(x0),y0(y0),x1(x1),y1(y1) {}
    void include(int x,int y) {
        if(x0>x1) {
            x0 = x;
            x1 = x+1;
            y0 = y;
            y1 = y+1;
        } else {
            x0 = min(x0,x);
            y0 = min(y0,y);
            x1 = max(x1,x+1);
            y1 = max(y1,y+1);
        }
    }
    void include(const Rect &other) {
        include(other.x0,other.y0);
        include(other.x1-1,other.y1-1); //otherwise include(include()) doesnt work
    }
    int width() {
        return x1-x0;
    }
    int height() {
        return y1-y0;
    }
};

struct Mean2 {
    float x,y,n,minx;
    Mean2() {
        x = 0;
        y = 0;
        n = 0;
        minx=MAXFLOAT;
    }
    void add(float nx,float ny) {
    if (nx<minx) minx=nx;
        x += nx;
        y += ny;
        n++;
    }
    float min_x() {
            return minx;
    }
    float mean_x() {
        return x/n;
    }
    float mean_y() {
        return y/n;
    }
};

namespace sort {
    float *compare_f;
    static int compare_float(const void *pi,const void *pj) {
        int i = *(int*)pi, j = *(int*)pj;
        float vi = compare_f[i], vj = compare_f[j];
        if(vi<vj) return -1;
        else if(vi>vj) return 1;
        else return 0;
    }
    void index(intarray &index,floatarray &values) {
        int n = values.length();
        index.resize(n);
        for(int i=0;i<n;i++) index(i) = i;
        compare_f = &values(0);
        qsort(&index(0),n,sizeof index(0),compare_float);
    }
};

void rescale(bytearray &out,bytearray &in) {
    for(int i=0;i<out.dim(0);i++)
        for(int j=0;j<out.dim(1);j++)
            out(i,j) = in((i*in.dim(0))/out.dim(0),(j*in.dim(1))/out.dim(1));
}

param_int maxcomp("maxcomp",4,"max number of components to group together");
param_int maxgap("maxgap",8,"max gap in pixels between bounding boxes grouped together");
param_int gwidth("gwidth",800,"width of output grid");
param_int gheight("gheight",3000,"max height of output grid");
param_string baseline("baseline",NULL,"baseline info in the form \"m,b,d,eps\"");
param_string size("size",NULL,"size range for resulting components \"w0,h0,w1,h1\"");
param_string smallsize("smallsize",NULL,"min size for subcomponents to be added\"w,h\"");
param_int maxonly("maxonly",0,"returns maximal possible connected component w/o maxgap in between");
param_int mergeabove("mergeabove",0,"only merge components if one is above the other");
param_string lineInfo("lineinfo",NULL,"line info in the form \"xLow yLow xHigh yHigh yIntercept slope descender xHeight\"");

int main(int argc,char **argv) {
    if(argc!=3) {
            fprintf(stderr,"arguments: group-image-out group-info-out\n");
            fprintf(stderr,"Parameters (passed in environment variables):\n");
            fprintf(stderr,"   maxcomp=4      max number of components to group together\n");
            fprintf(stderr,"   maxgap=8       max gap in pixels between bounding boxes grouped together\n");
            fprintf(stderr,"   gwidth=800     width of output grid\n");
            fprintf(stderr,"   gheight=3000   max height of output grid\n");
            fprintf(stderr,"   baseline=NONE  baseline info in the form \"m,b,d,eps\"\n");
            fprintf(stderr,"   size=NONE      size range for resulting components \"w0,h0,w1,h1\"\n");
            fprintf(stderr,"   smallsize=NONE min size for subcomponents to be added\"w,h\"\n");
            fprintf(stderr,"   maxonly=0      returns maximal possible connected component w/o maxgap in between\n");
            fprintf(stderr,"   mergeabove=0   only merge components if one is above the other\n");
        fprintf(stderr,"   lineinfo=NONE   line info in the form \"xLow yLow xHigh yHigh yIntercept slope descender xHeight\"\n");
        exit(1);
    }
    try {
#if 0
        int maxcomp = igetenv("maxcomp",4);
        int maxgap = igetenv("maxgap",8);
        int maxonly = igetenv("maxonly",0);
        int mergeabove = igetenv("mergeabove",0);
#endif

        Grid result;
        result.create(gwidth,gheight);

        float bm,bb,bd,be;
        if(baseline) {
            if(sscanf(baseline,"%f,%f,%f,%f",&bm,&bb,&bd,&be)!=4)
            throw "baseline info format error";
            bd = bb-bd;
        }
        int w0,h0,w1,h1;
        if(size) {
            if(sscanf(size,"%d,%d,%d,%d",&w0,&h0,&w1,&h1)!=4)
            throw "size info format error";
        }

        int sw, sh;
        if(smallsize) {
            if(sscanf(smallsize,"%d,%d",&sw,&sh)!=2)
            throw "smallsize info format error";
        }

        intarray image;
        read_ppm_packed(stdin, image);
        int nlabels = renumber_labels(image,1);
        //DBG(500) << VAR(nlabels) << ::std::endl;
        stdio info(argv[2],"w");

        // renumber components by x coordinate

        narray<Mean2> means(nlabels);
        for(int i=0;i<image.dim(0);i++) for(int j=0;j<image.dim(1);j++) {
            int pixel = image(i,j);
            if(pixel==0) continue;
            means(pixel).add(i,j);
        }
        floatarray xs(nlabels);
        xs(0) = -1;
        for(int i=1;i<nlabels;i++) {
            xs(i) = means(i).mean_x(); //less sensitive to tails
            if (maxonly==1) xs(i) = means(i).min_x();
        }
        intarray sorted;
        sort::index(sorted,xs);
        for(int i=0;i<image.dim(0);i++) for(int j=0;j<image.dim(1);j++) {
            int pixel = image(i,j);
            if(pixel==0) continue;
            image(i,j) = sorted(pixel);
        }

        // compute bounding boxes
        //DBG(500) << "compute bounding boxes" << ::std::endl;
        narray<Rect> bboxes(nlabels);
        for(int i=0;i<image.dim(0);i++) for(int j=0;j<image.dim(1);j++) {
            int pixel = image(i,j);
            if(pixel==0) continue;
            bboxes(pixel).include(i,j);
        }

        //-- write line information into first field of the grid: -------------
        //-- (descender, baseline, x-height, ascender) ------------------------
        // get line info from env
        int xLow = 0, yLow = 0, xHigh = 0, yHigh = 0, xHeight = 0;
        float yIntercept = 0.f, slope = 0.f, desc = 0.f;
        int maxShift = 0;
        int basepoint=0;
        if (lineInfo) {
            int count = sscanf(lineInfo,"%d %d %d %d %f %f %f %d",
                                &xLow, &yLow, &xHigh, &yHigh, &yIntercept,
                                &slope, &desc, &xHeight);
            //DBG(500) << "lineinfo: xLow=" << xLow << " yLow=" << yLow <<
            //        " xHigh=" << xHigh << " yHigh=" << yHigh <<
            //       " yIntercept=" << yIntercept << " slope=" << slope <<
            //        " descender=" << desc << " xHeight=" << xHeight << std::endl;
            if (count != 8) {
                throw "line info format error";
            }
            // !!! IMPORTANT: The line image has 2 extra pixels sourrounding
            // it from crop-image. So we really need to add them to work on
            // the right thing!
            xLow -= 2;
            yLow -= 2;
            xHigh += 2;
            yHigh += 2;
            // -- mark baseline as basepoint --
            // basepoint is the intercept of the baseline with the
            // (left side of the) bounding box
            basepoint = (int) ((slope * ((float) xLow) + yIntercept) - (float) yLow);
            // calculate maximum possible shift due to slope correction
            // (right-most pixel)
            // this is neccessary to keep all pixel inside the bbox,
            maxShift = abs((int) (slope * (float) image.dim(0)));
            //std::cout << VAR(maxShift) << std::endl;
            // shift basepoint by the maximum possible shift
            // (only if slope is positive)
            if (slope > 0.f) basepoint += maxShift;
            //DBG(500) << "basepoint: " << basepoint << std::endl;
            // basepoint and descender must not be at the same pixel!
            if (basepoint == 0) basepoint = 1;

            // lineInfo is just one column in the height of the line image
            bytearray lineInfoImage(1, image.dim(1) + maxShift + 1);
            // initialize "all white"
            fill(lineInfoImage, 255);
            lineInfoImage(0, basepoint) = 0;

            // -- mark descender --
            int descender = basepoint - (int) desc;
            // descender must not be below bounding box!
            if (descender < 0 || descender == basepoint) descender = 0;
            lineInfoImage(0, descender) = 0;

            // -- mark x-height --
            // xHeight and basepoint must not be at the same pixel!
            if (xHeight == 0) xHeight = 1;
            // limit x-height to not exceed bounding box
            if (basepoint + xHeight >= lineInfoImage.dim(1)) {
                xHeight = lineInfoImage.dim(1) - 2 - basepoint;
            }
            //            std::cout << VAR(basepoint) << std::endl;
            //            std::cout << VAR(xHeight) << std::endl;
            //            std::cout << VAR(lineInfoImage.dim(1)) << std::endl;
            lineInfoImage(0, basepoint + xHeight) = 0;
            // -- mark ascender (for now top) --
            lineInfoImage(0, lineInfoImage.dim(1)-1) = 0;
            // add the info as first element to the grid
            result.add(lineInfoImage);
        }


        // extract ranges of connected components
        //DBG(500) << "start combination of bboxes..." << ::std::endl;
        // Note: the older skript "classify_simple.sh" relied on a slightly different version of the following code
        //       if you intend to use a classifier based on connected components, please check out revision 180

        int offset=1;
        for(int i=1;i<nlabels;i+=offset) {
            printf("i=%d\n", i);
            for(int j=i;j<min(nlabels,i+maxcomp);j++) {
                if (maxonly==1) {
                    j=min(nlabels,i+maxcomp)-1;
                    offset=j-i+1;
                }
                Rect r;
                bool skip = false;
                for(int k=i;k<=j;k++) {
                    //if(k>i && (bboxes(k).x0-bboxes(k-1).x1) > maxgap) {
                    if(k>i && (((bboxes(k).x0-r.x1) > maxgap) ||
                        //since bboxes(k-1).x1 might be smaller than r.x1
                            (mergeabove && (r.x1>=r.x0) && (bboxes(k).y0<r.y1 && bboxes(k).y1>r.y0)))) {
                                // only merge if one cc is above other
                        // not sure if (r.x1>=r.x0) is necessary here, because we already know that k>i ...
                        if (maxonly==1) offset=(k)-i;
                        skip = true;
                        break;
                    }
                    if (smallsize) { // don't add small connected "sub"-components, e.g. single pixel...
                        if(bboxes(k).width()<sw||bboxes(k).height()<sh) continue;
                    }
                    r.include(bboxes(k));
                    //DBG(700) << VAR(bboxes(k).x1) << VAR(r.x1) << ::std::endl;
                }
                printf("r=(%d %d %d %d)\n", r.x0, r.y0, r.x1, r.y1);
                if(skip && maxonly==0) break;

                // the merged component must be on the baseline
                if(baseline) {
                    float x = (r.x0+r.x1)/2;
                    float y = r.y0;
                    float py = bm*x+bb;
                    float pyd = bm*x+bd;
                    float error = min(fabs(py-y),fabs(pyd-y));
                    if(error>be) continue;
                }

                //DBG(40) << "Bef: " << VAR(r.y0) << VAR(r.y1) << VAR(r.height()) << VAR(r.width()) << ::std::endl;
                //r.include(r.x0,r.y0-10 < 0 ? 0 : r.y0-10);
                //DBG(40) << "Aft: " << VAR(r.y0) << VAR(r.y1) << VAR(r.height()) << VAR(r.width()) << ::std::endl;

                // the merged component must be within size range
                if(size) {
                    if(r.width()<w0||r.width()>w1||r.height()<h0||r.height()>h1) continue;
                }

                // reset bottom of bounding box to zero, to keep y position of
                // the character (sub image)
                // backup variables for the original bounding box of the merged
                // components to write in the group info file
                int bottomBak = 0, topBak = 0;
                if (lineInfo) {
                    bottomBak = r.y0;
                    topBak = r.y1;
                    r.y0 = 0;
                    r.y1 = image.dim(1);
                }
                printf("r=(%d %d %d %d)\n", r.x0, r.y0, r.x1, r.y1);

                /////////////////////////////////////////////////////
                // extract pixels belonging to the merged component
                //DBG(600) << "Extract pixels belonging to the merged component..." << ::std::endl;
                bytearray sub;
                sub.resize(r.width(), r.height() + maxShift + 1);
                // 'erase' sub => paint all white
                fill(sub, 255);
                int numpixels=0;
                int shift = 0;
                for(int x=r.x0;x<r.x1;x++) for(int y=r.y0;y<r.y1;y++) {
                    //DBG(600) << VAR(r.x0) << VAR(r.x1) << VAR(x) << VAR(image.dim(0)) << ::std::endl;
                    int pixel = image(x,y);
                    // only take pixels of the currently merged components
                    pixel = (pixel>=i&&pixel<=j);
                    int pX, pY;
                    pX = x-r.x0;
                    pY = y-r.y0;
                    // shift the sub image according to slope
                    // (use x center of bbox for correction)
                    if (lineInfo && x==r.x0) {
                        shift = 0;
                        if (slope > 0.f) shift += maxShift;
                        //shift -= (int) (slope * ((float) (r.x0+r.x1))/2.f);
                        shift -= (int) (slope * (float)x);
                        //std::cout << "\tshift: " << shift;
                    }
                    // draw baseline (for debugging)
                    //if (pY + shift == basepoint) sub(pX, pY + shift) = 200;
                    if (pixel) {
                        // paint data in black
                        sub(pX, pY + shift) = 0;
                        numpixels++;
                    }
                    }

                // restore original bounding box of merged components
                if (lineInfo) {
                    r.y0 = bottomBak;
                    r.y1 = topBak;
                }
                    printf("numpixels: %d\n",numpixels);
                // only add merged components that have a minimum number of pixels
                if (numpixels<4) continue;
                printf("survived the first continue\n");

                // only add merged components that have a minimum "blackness"
                float blackness=((float)numpixels)/(float)(r.width()*r.height());
                if (blackness<0.1) continue;
                printf("survived the second continue\n");

                // scaled is not used...
                // bytearray scaled(16,16);
                // rescale(scaled,sub);
                //DBG(600) << VAR(j) << VAR(i) << VAR(offset) << ::std::endl;
                if(!result.add(sub)) break;
                //DBG(600) << "Wrote: " << VAR(j) << VAR(i) << VAR(offset) << ::std::endl;
                fprintf(info,"%d:%d %d,%d,%d,%d\n",i,j+offset-1+((maxonly==1&&i==nlabels-1)?1:0),r.x0,r.y0,r.x1,r.y1);
            }
        }
        // write the result
        result.save(argv[1]);
    } catch(const char *msg) {
        fprintf(stderr,"exception: %s\n",msg);
        exit(1);
    }
}
