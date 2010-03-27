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
// File: ocr-labels-to-grid.cc
// Purpose: output grid file corresponding to label file
// Responsible: tmb
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "imgio.h"
#include "imglib.h"

using namespace ocropus;
using namespace colib;

int grid_pack(bytearray &result,narray<bytearray> &components) {
    int x = 0;
    int y = 0;
    int max_ch = 0;
    fill(result,0);
    if(result.dim(0)>0 && result.dim(1)>0) result(0,0) = 255;
    for(int i=0;i<components.length();i++) {
        bytearray &comp = components[i];
        int cw = comp.dim(0);
        int ch = comp.dim(1);
        if(x+cw+1>result.dim(0)) {
            y = y + 1 + max_ch;
            max_ch = 0;
            x = 0;
        }
        CHECK_CONDITION(cw+1<result.dim(0));
        if(ch+y+1 < result.dim(1)) {
            result(x,y) = 255;
            for(int i=0;i<cw;i++) for(int j=0;j<ch;j++)
                result(x+1+i,y+1+j) = comp(i,j);
        }
        max_ch = max(max_ch,ch);
        x = x + cw + 1;
    }
    if(x>0) return y+1+max_ch;
    else return y;
}

void unpack_grid(narray<bytearray> &components,bytearray &image) {
    objlist<bytearray> clist;
    int y0 = 0;
    while(y0<image.dim(1)) {
        CHECK_ARG(image(0,y0)==255);
        int y1 = y0+1;
        while(y1<image.dim(1) && !image(0,y1)) y1++;
        int x0 = 0;
        while(x0<image.dim(0)) {
            int x1 = x0+1;
            while(x1<image.dim(0) && !image(x1,y0)) x1++;
            bytearray temp;
            extract_subimage(temp,image,x0,y0,x1,y1);
            move(clist.push(),temp);
        }
    }
}

int main(int argc,char **argv) {
    try {
        objlist<bytearray> clist;;
        for(int file=2;file<argc;file++) {
            intarray image;
            read_png_rgb(image,stdio(argv[file],"r"),true);
            // USAGE(max(image)>=1000000,"max(image)>=1M; make sure components are numbered sequentially from 1");
            narray<rectangle> bboxes;
            bounding_boxes(bboxes,image);
            fprintf(stderr,"# %s: %d boxes\n",argv[file],bboxes.length());
            for(int i=1;i<bboxes.length();i++) {
                rectangle r = bboxes(i);
                bytearray temp;
                extract_subimage(temp,image,r.x0,r.y0,r.x1,r.y1);
                for(int i=0;i<temp.length1d();i++)
                    temp.at1d(i) = 255 * !!temp.at1d(i);
                move(clist.push(),temp);
            }
        }
        narray<bytearray> components;
        move(components,clist);

        bytearray result;
        result.resize(1000,0);
        int h = grid_pack(result,components);
        result.resize(1000,h);
        grid_pack(result,components);

        write_png(stdio(argv[1],"w"),result);
    }
    catch(const char *oops) {
        fprintf(stderr,"oops: %s\n",oops);
    }
}
