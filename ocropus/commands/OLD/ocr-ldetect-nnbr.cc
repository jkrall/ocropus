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
// File: ocr-ldetect-nnbr.cc
// Purpose: perform language detection by nearest neighbor classification (IMMATURE)
// Responsible: tmb
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "imgio.h"
#include "imglib.h"

using namespace ocropus;
using namespace colib;

param_int cw("cwidth",10,"rescaled character width");
param_int ch("cheight",10,"rescaled character height");
param_int nclass("nclass",3,"maximum number of classes");
param_int nclusters("nclusters",20,"maximum number of classes");
param_int nrounds("nrounds",10,"maximum number of classes");
param_int ntrials("ntrials",10,"maximum number of classes");

struct Rectangle {
    int x0,y0,x1,y1;
    Rectangle() {
        x0 = 1;
        x1 = 0;
    }
    void include(int x,int y) {
        if(x0>x1) {
            x0 = x;
            y0 = y;
            x1 = x+1;
            y1 = y+1;
        } else {
            x0 = min(x,x0);
            y0 = min(y,y0);
            x1 = max(x+1,x1);
            y1 = max(y+1,y1);
        }
    }
    int width() {
        return x1-x0;
    }
    int height() {
        return y1-y0;
    }
    int area() {
        return width()*height();
    }
};

typedef narray< autoref< bytearray > > imagelist;

void extract_images(imagelist &result,intarray &image) {
    result.dealloc();
    int n = max(image)+1;
    narray<Rectangle> bounds(n);
    for(int i=0;i<image.dim(0);i++) for(int j=0;j<image.dim(1);j++) {
        int pixel = image(i,j);
        if(pixel) bounds(pixel).include(i,j);
    }
    for(int k=0;k<bounds.dim(0);k++) {
        int x = bounds(k).x0;
        int y = bounds(k).y0;
        int w = bounds(k).width();
        int h = bounds(k).height();
        if(w*h<=0) continue;
        bytearray subimage(w,h);
        for(int i=0;i<w;i++) for(int j=0;j<h;j++) {
            subimage(i,j) = 255 * (image(i+x,j+y)==k);
        }
        result.push();
        move(*result.last(),subimage);
    }
}

inline void add(floatarray &result,floatarray &v) {
    if(result.length1d()==0) {
        makelike(result,v);
        fill(result,0);
    }
    for(int i=0;i<result.length1d();i++)
        result.at1d(i) += v.at1d(i);
}

inline int iclamp(double x,int n) {
    return max(0,min(int(x+0.5),n-1));
}

void fit_into(floatarray &out,bytearray &in) {
    double scale = max(in.dim(0)/double(out.dim(0)),
                       in.dim(1)/double(out.dim(1)));
    int dx = int(max((in.dim(0)/scale-out.dim(0))/2,0));
    int dy = int(max((in.dim(1)/scale-out.dim(1))/2,1));
    fill(out,0);
    for(int i=0;i<out.dim(0);i++) for(int j=0;j<out.dim(1);j++) {
        int x = iclamp(i*scale,in.dim(0));
        int y = iclamp(j*scale,in.dim(1));
        out(i,j) = !!in(x,y);        // MAYBE add dx,dy here
    }
}

int main(int argc,char **argv) {
    try {
        char file[10000];
        int cls;
        intarray counts(1000);
        fill(counts,0);

        autodel<IClassifier> neighbors;
#if 1
        neighbors = make_KmeansClassifier();
        neighbors->param("nclusters",nclusters);
        neighbors->param("ntrials",ntrials);
        neighbors->param("nrounds",nrounds);
#else
        neighbors = make_KnnClassifier();
        neighbors->param("k",5);
#endif

        neighbors->start_training();
        while(fscanf(stdin,"%s %d\n",file,&cls)==2) {
            fprintf(stderr,"# %s %d\n",file,cls);
            intarray image;
            read_png_rgb(image,stdio(file,"r"),true);
            renumber_labels(image,1);
            imagelist chars;
            extract_images(chars,image);
            fprintf(stderr,"# got %d images\n",chars.length());
            counts(cls) += chars.length();
            for(int i=0;i<chars.length();i++) {
                floatarray fchar(cw,ch);
                fit_into(fchar,*chars[i]);
                fchar.reshape(cw*ch);
                perturb(fchar,1e-4);
                neighbors->add(fchar,cls);
            }
        }

        neighbors->start_classifying();
        floatarray result;
        for(int i=1;i<argc;i++) {
            intarray image;
            read_png_rgb(image,stdio(argv[i],"r"),true);
            renumber_labels(image,1);
            imagelist chars;
            extract_images(chars,image);
            fprintf(stderr,"# %s: got %d images\n",argv[i],chars.length());
            for(int i=0;i<chars.length();i++) {
                floatarray fchar(cw,ch);
                fit_into(fchar,*chars[i]);
                fchar.reshape(cw*ch);
                perturb(fchar,1e-4);
                floatarray values;
                neighbors->score(values,fchar);
                add(result,values);
            }
            printf("%s",argv[1]);
            for(int i=0;i<result.dim(0);i++)
                printf(" %g (%g)",result(i)/max(1,counts(i)),result(i));
            printf("\n");
        }
    }
    catch(const char *oops) {
        fprintf(stderr,"oops: %s\n",oops);
    }
}
