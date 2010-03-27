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
// File: ocr-extract-lines.cc
// Purpose: given a color layout analysis map, output images of the individual lines
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "imgio.h"
#include "imglib.h"

using namespace ocropus;
using namespace colib;

param_int minwidth("minwidth",50,"minimum line width");
param_int minheight("minheight",10,"minimum line height");
param_int padding("padding",5,"padding for each line image");

int main(int argc,char **argv) {
    try {
        intarray image;
        inthash<rectangle> bboxes;
        read_png_rgb(image,stdio(argv[1],"r"),true);
        for(int i=0;i<image.dim(0);i++) for(int j=0;j<image.dim(1);j++) {
            int pixel = image(i,j);
            rectangle &r = bboxes(pixel);
            r.include(i,j);
        }
        intarray keys;
        bboxes.keys(keys);
        for(int ki=0;ki<keys.length();ki++) {
            int key = keys[ki];
            if(key==0) continue;
            rectangle box = bboxes(key);
            if(box.width()<minwidth || box.height()<minheight) continue;
            intarray line(box.width(),box.height());
            // FIXME/tmb add greyscale extraction here --???
            for(int i=0;i<box.width();i++) for(int j=0;j<box.height();j++) {
                line(i,j) = 0xffffff * !!image(i+box.x0,j+box.y0);
            }
            pad_by(line,padding,padding,0);
            int r = ((key>>16)&255);
            int g = ((key>>8)&255);
            int b = (key&255);
            char name[10000];
            sprintf(name,"%s-%d-%d-%03d.png",argv[2],r,g,b);
            write_png_rgb(stdio(name,"w"),line);
        }
    }
    catch(const char *oops) {
        fprintf(stderr,"oops: %s\n",oops);
    }
}
