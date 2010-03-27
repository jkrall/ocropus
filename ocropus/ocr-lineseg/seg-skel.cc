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


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "colib/colib.h"
#include "iulib/imgio.h"
#include "iulib/imglib.h"
#include "segmentation.h"

using namespace ocropus;
using namespace iulib;
using namespace colib;

namespace ocropus {
    bool is_singular0(bytearray &image,int i,int j) {
        if(i<1||i>=image.dim(0)-1||j<1||j>image.dim(1)-1) return 0;
        bytearray surround(8);
        int k = 0;
        surround(k++) = image(i+1,j);
        surround(k++) = image(i+1,j+1);
        surround(k++) = image(i,j+1);
        surround(k++) = image(i-1,j+1);
        surround(k++) = image(i-1,j);
        surround(k++) = image(i-1,j-1);
        surround(k++) = image(i,j-1);
        surround(k++) = image(i+1,j-1);
        // count the number of transitions
        int count = 0;
        for(k=0;i<8;k++) if(surround((k+1)%8)&&!surround(k)) count++;
        return count>2;
    }
    inline int neighbors(bytearray &image,int i,int j) {
        if(i<1||i>=image.dim(0)-1||j<1||j>image.dim(1)-1) return 0;
        if(!image(i,j)) return 0;
        int count = -1;
        for(int k=-1;k<=1;k++)
            for(int l=-1;l<=1;l++)
                if(image(i+k,j+l)) count++;
        return count;
    }
    void count_neighbors(bytearray &result,bytearray &image) {
        makelike(result,image);
        fill(result,0);
        int d=1;
        for(int i=d;i<image.dim(0)-d-1;i++) {
            for(int j=d;j<image.dim(1)-d-1;j++) {
                int n = neighbors(image,i,j);
                result(i,j) = n;
            }
        }
    }
    inline bool is_singular(bytearray &image,int i,int j) {
        return neighbors(image,i,j)>2;
    }
    void find_endpoints(bytearray &result,bytearray &image) {
        int d = 1;
        makelike(result,image);
        fill(result,0);
        for(int i=d;i<image.dim(0)-d-1;i++) {
            for(int j=d;j<image.dim(1)-d-1;j++) {
                int n = neighbors(image,i,j);
                CHECK_ARG(n<5);
                if(n==1) result(i,j) = 255;
            }
        }
    }
    void find_junctions(bytearray &result,bytearray &image) {
        int d=1;
        makelike(result,image);
        fill(result,0);
        for(int i=d;i<image.dim(0)-d-1;i++) {
            for(int j=d;j<image.dim(1)-d-1;j++) {
                int n = neighbors(image,i,j);
                CHECK_ARG(n<5);
                if(n>2) result(i,j) = 255;
            }
        }
    }
    void remove_singular_points(bytearray &image,int d) {
        for(int i=d;i<image.dim(0)-d-1;i++) {
            for(int j=d;j<image.dim(1)-d-1;j++) {
                if(is_singular(image,i,j)) {
                    for(int k=-d;k<=d;k++)
                        for(int l=-d;l<=d;l++)
                            image(i+k,j+l) = 0;
                }
            }
        }
    }
    class SkelSegmenter : public ISegmentLine {
        virtual const char *description() {
            return "skeleton segmenter";
        }
        const char *name() {
            return "skelseg";
        }
        virtual void init(const char **argv=0) {
        }

        virtual void charseg(intarray &segmentation,bytearray &image) {
            bytearray timage;
            copy(timage,image);
            thin(timage);
            //write_png(stdio("_thinned","w"),timage);
            remove_singular_points(timage,2);
            //write_png(stdio("_segmented","w"),timage);
            intarray tsegmentation;
            copy(tsegmentation,timage);
            label_components(tsegmentation);
            remove_small_components(tsegmentation,4,4);
            //write_png_rgb(stdio("_labeled","w"),tsegmentation);
            copy(segmentation,image);
            propagate_labels_to(segmentation,tsegmentation);
            //write_png_rgb(stdio("_propagated","w"),segmentation);
        }
    };
}

ISegmentLine *ocropus::make_SkelSegmenter() {
    return new SkelSegmenter();
}

