// -*- C++ -*-

// Copyright 2006-2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz 
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
// Project: roughocr -- mock OCR system exercising the interfaces and useful for testing
// File: ocr-erase-nonchar-components.cc
// Purpose: erase non-character sized components from an image
// Responsible: tmb
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "imgio.h"
#include "imglib.h"

using namespace ocropus;
using namespace colib;

point estimate_size(intarray &image) {
        narray<rectangle> bboxes;
        bounding_boxes(bboxes,image);
        floatarray xhist(200),yhist(200);
        for(int i=1;i<bboxes.length();i++) {
            ext(xhist,bboxes[i].width())++;
            ext(yhist,bboxes[i].height())++;
        }
        simple_recolor(image);
        gauss1d(xhist,5.0);
        gauss1d(yhist,5.0);
        intarray xps,yps;
        peaks(xps,xhist,10);
        peaks(yps,yhist,15);
        return point(xps[0],yps[0]);
}

param_bool colorful("colorful",0,"make output colorful");

int main(int argc,char **argv) {
    try {
        intarray image;
        read_png_rgb(image,stdio(argv[1],"r"),true);
        autoinvert(image);

        label_components(image,false);
        
        point size = estimate_size(image);

        fprintf(stderr,"estimated_character_size=%d %d\n",size.x,size.y);

        narray<rectangle> bboxes;
        bounding_boxes(bboxes,image);
        bytearray erase(bboxes.length());
        fill(erase,0);
        for(int i=0;i<bboxes.length();i++) {
            int bw = bboxes[i].width();
            int bh = bboxes[i].height();
            int bad = 0;
            if(bw<0 || bh<0) continue;
            if(bw/bh>4 || bh/bw>4) bad = 1;
            if(bw>size.x*3) bad = 1;
            if(bh>size.y*3) bad = 1;
            if(bw<size.x/2 && bh<size.y/2) bad = 1;
            erase[i] = bad;
        }
        for(int i=0;i<image.length1d();i++)
            if(erase(image.at1d(i)))
                image.at1d(i) = 0;

        if(colorful) 
            simple_recolor(image);
        write_png_rgb(stdio(argv[2],"w"),image);
    }
    catch(const char *oops) {
        fprintf(stderr,"oops: %s\n",oops);
    }
}
