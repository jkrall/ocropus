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


#include "colib/colib.h"
#include "iulib/iulib.h"
#include "ocropus.h"

using namespace ocropus;
using namespace colib;

int main(int argc,char **argv) {
    if(argc==1&&isatty(0)) {
        fprintf(stderr,"WARNING: this is not exactly the same algorithm as described in the paper\n");
        fprintf(stderr,"arguments: < pnm-image > segmented-pnm-image\n\n");
        exit(1);
    }
    try {
        bytearray image;
        intarray segmentation;
        // read image
        if(argc>1) read_pnm_gray(stdio(argv[1],"r"), image);
        else read_pnm_gray(stdin, image);
        autodel<ISegmentLine> seg(make_CurvedCutSegmenter());
        seg->charseg(segmentation, image);
        write_ppm_packed(stdout,segmentation);

//        intarray segmentation(image.dim(0),image.dim(1));
//        segmentation = 0;
//        //incorporate the cuts into the image by setting them to zero in segmentation
//        //and using that to compute the "new" (cut) connected components
//        //the artifically created cut has width 3
//        for(int i=0;i<image.dim(0);i++) for(int j=0;j<image.dim(1);j++)
//            segmentation(i,j) = image(i,j)?0xffffffff:0;
//        for(int y=0;y<image.dim(1);y++) {
//            for(int r=0;r<segmenter.bestcuts.length();r++) {
//                int c = segmenter.bestcuts(r);
//                int x = segmenter.cuts(c)(y).x;
//                int clear = 0x00000000;
//
//                //only set to zero (clear) if it is an actual cut through a component
//                //don't cut a vertical one pixel line, which would result in a deletion if you did
//                // w/o doing this single pixels between characters belong to one of them
//                int next_x = segmenter.cuts(c)(min(y+1,image.dim(1)-1)).x;
//                int next_y = min(image.dim(1)-1,y+1);
//                int left_x = max(0,x-1);
//                int right_x= min(x+1,image.dim(0)-1);
//                if ( !segmentation(x,y) || (segmentation(x,y) &&
//                        ( segmentation(right_x,y) || segmentation(right_x,max(0,y-1)) ||
//                        ( segmentation(right_x,next_y) && (right_x != next_x) ) ||
//                        ( segmentation(x,next_y)       && (x != next_x) )
//                        || ( segmentation(left_x,next_y)  && (left_x != next_x) )
//                        )) ) {
//                    for(int i=max(0,x-2);i<=x;i++) segmentation(i,y) = clear;
//            } else {DBG(500) << "Didn't cut: " << VAR(next_x) << VAR(x) << VAR(y) << ::std::endl;}
//            }
//        }
//        intarray iimage;
//        copy(iimage,image);
//        label_components(segmentation);
//        propagate_labels_to(iimage,segmentation);
//        write_ppm(stdout, iimage);
    } catch(const char *msg) {
        fprintf(stderr,"exception: %s\n",msg);
        exit(1);
    }
}
