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
// Project:
// File: ocr-utils.cc
// Purpose: miscelaneous routines
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include <stdarg.h>
#include "ocropus.h"

using namespace iulib;
using namespace colib;

namespace ocropus {
    param_bool bgcheck("bgcheck", true, "abort on detecting an inverted image");

    void invert(bytearray &a) {
        int n = a.length1d();
        for (int i = 0; i < n; i++) {
            a.at1d(i) = 255 - a.at1d(i);
        }
    }

    void crop_masked(bytearray &result,
                     bytearray &source,
                     rectangle crop_rect,
                     bytearray &mask,
                     int def_val,
                     int pad) {
        CHECK_ARG(background_seems_black(mask));

        rectangle box(0, 0, source.dim(0), source.dim(1));
        box.intersect(crop_rect);
        result.resize(box.width() + 2 * pad, box.height() + 2 * pad);
        fill(result, def_val);
        for(int x = 0; x < box.width(); x++) {
            for(int y = 0; y < box.height(); y++) {
               if(mask(x + box.x0, y + box.y0))
                   result(x + pad, y + pad) = source(x + box.x0, y + box.y0);
            }
        }
    }


    int average_on_border(colib::bytearray &a) {
        int sum = 0;
        int right = a.dim(0) - 1;
        int top = a.dim(1) - 1;
        for(int x = 0; x < a.dim(0); x++)
            sum += a(x, 0);
        for(int x = 0; x < a.dim(0); x++)
            sum += a(x, top);
        for(int y = 1; y < top; y++)
            sum += a(0, y);
        for(int y = 1; y < top; y++)
            sum += a(right, y);
        // If average border intensity is between 127-128, inverting the
        // image does not work correctly
        float average_border_intensity = sum / ((right + top) * 2.0);
        ASSERTWARN(average_border_intensity<=127 || average_border_intensity>=128);
        return sum / ((right + top) * 2);
    }


    // FIXME/mezhirov use imgmorph stuff --tmb

    void blit2d(bytearray &dest, const bytearray &src, int shift_x, int shift_y) {
        int w = src.dim(0);
        int h = src.dim(1);
        for (int x=0;x<w;x++) for (int y=0;y<h;y++) {
            dest(x + shift_x, y + shift_y) = src(x,y);
        }
    }

    float median(intarray &a) {
        intarray s;
        copy(s, a);
        quicksort(s);
        int n = s.length();
        if (!n)
            return 0;
        if (n % 2)
            return s[n / 2];
        else
            return float(s[n / 2 - 1] + s[n / 2]) / 2;
    }



    void plot_hist(FILE *stream, floatarray &hist){
        if(!stream){
            fprintf(stderr,"Unable to open histogram image stream.\n");
            exit(0);
        }
        int maxval = 1000;
        int len    = hist.length();
        narray<unsigned char> image(len, maxval);
        fill(image,0xff);
        for(int x=0; x<len; x++){
            int top = min(maxval-1,int(hist[x]));
            for(int y=0; y<top; y++)
                image(x,y) = 0;
        }
        write_png(stream,image);
        fclose(stream);
    }

    void paint_box(intarray &image, rectangle r, int color, bool inverted){

        int width  = image.dim(0);
        int height = image.dim(1);
        int left, top, right, bottom;

        left   = (r.x0<0)  ? 0   : r.x0;
        top    = (r.y0<0)  ? 0   : r.y0;
        right  = (r.x1>=width) ? width-1 : r.x1;
        bottom = (r.y1>=height) ? height-1 : r.y1;

        if(right <= left || bottom <= top) return;

        for(int x=left; x<right; x++){
            for(int y=top; y<bottom; y++){
                if(!inverted)
                    image(x,y) &= color;
                else
                    image(x,y) |= color;
            }
        }
    }

    void paint_box(bytearray &image, rectangle r, byte color, bool inverted){

        int width  = image.dim(0);
        int height = image.dim(1);
        int left, top, right, bottom;

        left   = (r.x0<0)  ? 0   : r.x0;
        top    = (r.y0<0)  ? 0   : r.y0;
        right  = (r.x1>=width) ? width-1 : r.x1;
        bottom = (r.y1>=height) ? height-1 : r.y1;

        if(right <= left || bottom <= top) return;

        for(int x=left; x<right; x++){
            for(int y=top; y<bottom; y++){
                if(!inverted)
                    image(x,y) &= color;
                else
                    image(x,y) |= color;
            }
        }
    }

    void paint_box_border(intarray &image, rectangle r, int color, bool inverted){

        int width  = image.dim(0);
        int height = image.dim(1);
        int left, top, right, bottom;

        left   = (r.x0<0)  ? 0   : r.x0;
        top    = (r.y0<0)  ? 0   : r.y0;
        right  = (r.x1>=width) ? width-1 : r.x1;
        bottom = (r.y1>=height) ? height-1 : r.y1;
        if(right < left || bottom < top) return;
        int x,y;
        if(!inverted){
            for(x=left; x<=right; x++){ image(x,top)     &=color; }
            for(x=left; x<=right; x++){ image(x,bottom)  &=color; }
            for(y=top; y<=bottom; y++){ image(left,y)    &=color; }
            for(y=top; y<=bottom; y++){ image(right,y)   &=color; }
        }else{
            for(x=left; x<=right; x++){ image(x,top)     |=color; }
            for(x=left; x<=right; x++){ image(x,bottom)  |=color; }
            for(y=top; y<=bottom; y++){ image(left,y)    |=color; }
            for(y=top; y<=bottom; y++){ image(right,y)   |=color; }
        }
    }

    void paint_box_border(bytearray &image, rectangle r, byte color, bool inverted){

        int width  = image.dim(0);
        int height = image.dim(1);
        int left, top, right, bottom;

        left   = (r.x0<0)  ? 0   : r.x0;
        top    = (r.y0<0)  ? 0   : r.y0;
        right  = (r.x1>=width) ? width-1 : r.x1;
        bottom = (r.y1>=height) ? height-1 : r.y1;
        if(right < left || bottom < top) return;
        int x,y;
        if(!inverted){
            for(x=left; x<=right; x++){ image(x,top)     &=color; }
            for(x=left; x<=right; x++){ image(x,bottom)  &=color; }
            for(y=top; y<=bottom; y++){ image(left,y)    &=color; }
            for(y=top; y<=bottom; y++){ image(right,y)   &=color; }
        }else{
            for(x=left; x<=right; x++){ image(x,top)     |=color; }
            for(x=left; x<=right; x++){ image(x,bottom)  |=color; }
            for(y=top; y<=bottom; y++){ image(left,y)    |=color; }
            for(y=top; y<=bottom; y++){ image(right,y)   |=color; }
        }
    }

    static void subsample_boxes(narray<rectangle> &boxes, int factor) {
        int len = boxes.length();
        if (factor == 0) return;
        for(int i=0; i<len; i++){
            boxes[i].x0 = boxes[i].x0/factor;
            boxes[i].x1 = boxes[i].x1/factor;
            boxes[i].y0 = boxes[i].y0/factor;
            boxes[i].y1 = boxes[i].y1/factor;
        }
    }


    void draw_rects(colib::intarray &out, colib::bytearray &in,
                    colib::narray<colib::rectangle> &rects,
                    int downsample_factor,  int color){
        int ds = downsample_factor;
        if(ds <= 0)
            ds = 1;
        int width  = in.dim(0);
        int height = in.dim(1);
        int xdim   = width/ds;
        int ydim   = height/ds;
        out.resize(xdim, ydim);
        for(int ix=0; ix<xdim; ix++)
            out(ix,ydim-1)=0x00ffffff;
        for(int x=0,ix=0; x<width-ds; x+=ds, ix++) {
            for(int y=0,iy=0; y<height-ds; y+=ds, iy++){
                out(ix,iy)=in(x,y)*0x00010101;
            }
        }
        narray<rectangle> boxes;
        copy(boxes,rects);
        if(ds > 1)
            subsample_boxes(boxes, ds);

        for(int i=0, len=boxes.length(); i<len; i++)
            paint_box_border(out, boxes[i], color);

    }

    void draw_filled_rects(colib::intarray &out, colib::bytearray &in,
                           colib::narray<colib::rectangle> &rects,
                           int downsample_factor, int color, int border_color){
        int ds = downsample_factor;
        if(ds <= 0)
            ds = 1;
        int width  = in.dim(0);
        int height = in.dim(1);
        int xdim   = width/ds;
        int ydim   = height/ds;
        out.resize(xdim, ydim);
        for(int ix=0; ix<xdim; ix++)
            out(ix,ydim-1)=0x00ffffff;
        for(int x=0,ix=0; x<width-ds; x+=ds, ix++) {
            for(int y=0,iy=0; y<height-ds; y+=ds, iy++){
                out(ix,iy)=in(x,y)*0x00010101;
            }
        }
        narray<rectangle> boxes;
        copy(boxes,rects);
        if(ds > 1)
            subsample_boxes(boxes, ds);

        for(int i=0, len=boxes.length(); i<len; i++){
            paint_box(out, boxes[i], color);
            paint_box_border(out, boxes[i], border_color);
        }

    }

    // FIXME/mezhirov add comments --tmb

    void get_line_info(float &baseline, float &xheight, float &descender, float &ascender, intarray &seg) {
        narray<rectangle> bboxes;
        bounding_boxes(bboxes, seg);

        intarray tops, bottoms;
        makelike(tops,    bboxes);
        makelike(bottoms, bboxes);

        for(int i = 0; i < bboxes.length(); i++) {
            tops[i] = bboxes[i].y1;
            bottoms[i] = bboxes[i].y0;
        }

        baseline = median(bottoms) + 1;
        xheight = median(tops) - baseline;

        descender = baseline - 0.4 * xheight;
        ascender  = baseline + 2 * xheight;
    }

    // FIXME/mezhirov add comments --tmb

    static const char *version_string = NULL;

    // FIXME/mezhirov add comments --tmb

    const char *get_version_string() {
        return version_string;
    }

    // FIXME/mezhirov add comments --tmb

    void set_version_string(const char *new_version_string) {
        if (version_string) {
            ASSERT(new_version_string && !strcmp(version_string, new_version_string));
        } else {
            version_string = new_version_string;
        }
    }

    void align_segmentation(intarray &segmentation,narray<rectangle> &bboxes) {
        intarray temp;
        make_line_segmentation_black(segmentation);
        renumber_labels(segmentation,1);
        int nsegs = max(segmentation)+1;
        intarray counts;
        counts.resize(nsegs,bboxes.length());
        fill(counts,0);
        for(int i=0;i<segmentation.dim(0);i++) {
            for(int j=0;j<segmentation.dim(1);j++) {
                int cs = segmentation(i,j);
                if(cs==0) continue;
                for(int k=0;k<bboxes.length();k++) {
                    if(bboxes[k].contains(i,j))
                        counts(cs,k)++;
                }
            }
        }
        intarray segmap;
        segmap.resize(counts.dim(0));
        for(int i=0;i<counts.dim(0);i++) {
            int mj = -1;
            int mc = 0;
            for(int j=0;j<counts.dim(1);j++) {
                if(counts(i,j)>mc) {
                    mj = j;
                    mc = counts(i,j);
                }
            }
            segmap(i) = mj;
        }
        for(int i=0;i<segmentation.dim(0);i++) {
            for(int j=0;j<segmentation.dim(1);j++) {
                int cs = segmentation(i,j);
                if(cs) continue;
                segmentation(i,j) = segmap(cs)+1;
            }
        }
    }

    namespace {
        void getrow(intarray &a,intarray &m,int i) {
            a.resize(m.dim(1));
            for(int j=0;j<m.dim(1);j++) a(j) = m(i,j);
        }
        void getcol(intarray &a,intarray &m,int j) {
            a.resize(m.dim(0));
            for(int i=0;i<m.dim(0);i++) a(i) = m(i,j);
        }
    }

    void evaluate_segmentation(int &nover,int &nunder,int &nmis,intarray &model_raw,intarray &image_raw,float tolerance) {
        CHECK_ARG(samedims(model_raw,image_raw));

        intarray model,image;
        copy(model,model_raw);
        replace_values(model, 0xFFFFFF, 0);
        int nmodel = renumber_labels(model,1);
        CHECK_ARG(nmodel<100000);

        copy(image,image_raw);
        replace_values(image, 0xFFFFFF, 0);
        int nimage = renumber_labels(image,1);
        CHECK_ARG(nimage<100000);

        intarray table(nmodel,nimage);
        fill(table,0);
        for(int i=0;i<model.length1d();i++)
            table(model.at1d(i),image.at1d(i))++;

//         for(int i=1;i<table.dim(0);i++) {
//             for(int j=1;j<table.dim(1);j++) {
//                 printf(" %3d",table(i,j));
//             }
//             printf("\n");
//         }

        nover = 0;
        nunder = 0;
        nmis = 0;

        for(int i=1;i<table.dim(0);i++) {
            intarray row;
            getrow(row,table,i);
            row(0) = 0;
            double total = sum(row);
            int match = argmax(row);
            // printf("[%3d,] %3d: ",i,match); for(int j=1;j<table.dim(1);j++) printf(" %3d",table(i,j)); printf("\n");
            for(int j=1;j<table.dim(1);j++) {
                if(j==match) continue;
                int count = table(i,j);
                if(count==0) continue;
                if(count / total > tolerance) {
                    nover++;
                } else {
                    nmis++;
                }
            }
        }
        for(int j=1;j<table.dim(1);j++) {
            intarray col;
            getcol(col,table,j);
            col(0) = 0;
            double total = sum(col);
            int match = argmax(col);
            // printf("[,%3d] %3d: ",j,match); for(int i=1;i<table.dim(0);i++) printf(" %3d",table(i,j)); printf("\n");
            for(int i=1;i<table.dim(0);i++) {
                if(i==match) continue;
                int count = table(i,j);
                if(count==0) continue;
                if(count / total > tolerance) {
                    nunder++;
                } else {
                    nmis++;
                }
            }
        }
    }
    void ocr_bboxes_to_charseg(intarray &cseg,narray<rectangle> &bboxes,intarray &segmentation) {
        make_line_segmentation_black(segmentation);
        CHECK_ARG(max(segmentation)<100000);
        intarray counts(max(segmentation)+1,bboxes.length());
        fill(counts,0);
        for(int i=0;i<segmentation.dim(0);i++) for(int j=0;j<segmentation.dim(1);j++) {
            int value = segmentation(i,j);
            if(value==0) continue;
            for(int k=0;k<bboxes.length();k++) {
                rectangle bbox = bboxes[k];
                if(bbox.includes(i,j))
                    counts(value,k)++;
            }
        }
        intarray valuemap(max(segmentation)+1);
        fill(valuemap,0);
        for(int i=1;i<counts.dim(0);i++)
            valuemap(i) = rowargmax(counts,i)+1;
        makelike(cseg,segmentation);
        for(int i=0;i<segmentation.dim(0);i++) for(int j=0;j<segmentation.dim(1);j++) {
            cseg(i,j) = valuemap(segmentation(i,j));
        }
    }

    template <class T>
    void remove_small_components(narray<T> &bimage,int mw,int mh) {
        intarray image;
        copy(image,bimage);
        label_components(image);
        narray<rectangle> rects;
        bounding_boxes(rects,image);
        bytearray good(rects.length());
        for(int i=0;i<good.length();i++)
            good[i] = 1;
        for(int i=0;i<rects.length();i++) {
            if(rects[i].width()<mw && rects[i].height()<mh) {
                // printf("*** %d %d %d\n",i,rects[i].width(),rects[i].height());
                good[i] = 0;
            }
        }
        for(int i=0;i<image.length1d();i++) {
            if(!good(image.at1d(i)))
                image.at1d(i) = 0;
        }
        for(int i=0;i<image.length1d();i++)
            if(!image.at1d(i)) bimage.at1d(i) = 0;
    }
    template void remove_small_components<byte>(narray<byte> &,int,int);
    template void remove_small_components<int>(narray<int> &,int,int);

    template <class T>
    void remove_marginal_components(narray<T> &bimage,int x0,int y0,int x1,int y1) {
        intarray image;
        copy(image,bimage);
        label_components(image);
        narray<rectangle> rects;
        bounding_boxes(rects,image);
        if(rects.length()>0) {
            x1 = bimage.dim(0)-x1;
            y1 = bimage.dim(1)-y1;
            bytearray good(rects.length());
            fill(good, 1);
            for(int i=0;i<rects.length();i++) {
                rectangle r = rects[i];
                // >= ?
                if(r.x1 < x0 || r.x0 > x1 || r.y1 < y0 || r.y0 > y1) {
                    good[i] = 0;
                }
            }
            for(int i=0;i<image.length1d();i++) {
                if(!good(image.at1d(i)))
                    bimage.at1d(i) = 0;
            }
        }
    }
    template void remove_marginal_components<byte>(narray<byte> &,int,int,int,int);
    template void remove_marginal_components<int>(narray<int> &,int,int,int,int);

    void remove_neighbour_line_components(bytearray &line) {
        invert(line);
        intarray image;
        copy(image,line);
        label_components(image);
        narray<rectangle> rects;
        bounding_boxes(rects,image);
        if(rects.length()>0) {
            int h = line.dim(1);
            int lower = int(h*0.33);
            int upper = int(h*0.67);
            bytearray good(rects.length());
            fill(good, 1);
            for(int i=0;i<rects.length();i++) {
                rectangle r = rects[i];
                if( ((r.y0 == 0) && (r.y1 < lower)) ||
                    ((r.y1 == h) && (r.y0 > upper)) )
                    good[i] = 0;
            }
            for(int i=0;i<image.length1d();i++)
                if(!good(image.at1d(i)))
                    line.at1d(i) = 0;
        }
        invert(line);
    }

    /// Analogous to python's split().
    void split_string(narray<iucstring> &components,
                      const char *s,
                      const char *delimiters) {
        components.clear();
        if(!*s) return;
        while(1) {
            const char *p = s;
            while(*p && !strchr(delimiters, *p))
                p++;
            int len = p - s;
            if(len) {
                components.push().append(s, len);
            }
            if(!*p) return;
            s = p + 1;
        }
    }

    int binarize_simple(bytearray &result, bytearray &image) {
        int threshold = (max(image)+min(image))/2;
        makelike(result,image);
        for(int i=0;i<image.length1d();i++)
            result.at1d(i) = image.at1d(i)<threshold ? 0 : 255;
        return threshold;
    }

    int binarize_simple(colib::bytearray &image) {
        return binarize_simple(image, image);
    }

    void binarize_with_threshold(bytearray &out, bytearray &in, int threshold) {
        makelike(out,in);
        for(int i=0;i<in.length1d();i++)
            out.at1d(i) = in.at1d(i)<threshold ? 0 : 255;
    }

    void binarize_with_threshold(bytearray &in, int threshold) {
        binarize_with_threshold(in, threshold);
    }

    void runlength_histogram(floatarray &hist,
                             bytearray &img,
                             rectangle box,
                             bool white,
                             bool vertical){
        fill(hist,0);
        white = !!white;
        if(!vertical) {
            for(int j=box.y0;j<box.y1;j++){
                int flag = 0;
                int runlength = 0;
                for(int k=box.x0;k<box.x1;k++) {
                    if (!!img(k,j)==white) {
                        runlength++;
                        flag = 1;
                    }
                    if (!!img(k,j) != white && flag) {
                        flag = 0;
                        if(runlength < hist.length())
                            hist(runlength)++;
                        runlength = 0;
                    }
                }
                if (flag && runlength < hist.length())
                    hist(runlength)++;
            }
        } else {
            for(int k=box.x0;k<box.x1;k++) {
                int flag = 0;
                int runlength = 0;
                for(int j=box.y0;j<box.y1;j++){
                    if (!!img(k,j)==white) {
                        runlength++;
                        flag = 1;
                    }
                    if (!!img(k,j) != white && flag) {
                        flag = 0;
                        if(runlength < hist.length())
                            hist(runlength)++;
                        runlength = 0;
                    }
                }
                if (flag && runlength < hist.length())
                    hist(runlength)++;
            }
        }
    }

    /// Return the index of the histogram median.
    int find_median_in_histogram(narray<float> &hist){
        int index= 0;
        float partial_sum = 0, sum = 0 ;
        for(int i = 0; i < hist.length(); i++) sum += hist(i);
        for(int j = 0; j < hist.length(); j++) hist(j) /= sum;
        while(partial_sum < 0.5) {
            partial_sum += hist(index);
            index++;
        }
        return index;
    }

    void throw_fmt(const char *format, ...) {
        va_list v;
        va_start(v, format);
        static char buf[1000];
        vsnprintf(buf, sizeof(buf), format, v);
        va_end(v);
        throw (const char *) buf;
    }

    void optional_check_background_is_darker(colib::bytearray &a) {
        if(bgcheck) {
            CHECK_CONDITION(background_seems_black(a));
        }
    }
    void optional_check_background_is_lighter(colib::bytearray &a) {
        if(bgcheck) {
            CHECK_CONDITION(background_seems_white(a));
        }
    }

    void paint_rectangles(intarray &image,rectarray &rectangles) {
        int w = image.dim(0), h = image.dim(1);
        for(int i=0;i<rectangles.length();i++) {
            rectangle r = rectangles[i];
            r.x0 = r.x0<0?0:r.x0;
            r.y0 = r.y0<0?0:r.y0;
            r.x1 = r.x1>=w?w-1:r.x1;
            r.y1 = r.y1>=h?h-1:r.y1;
            int color = i+1;
            for(int x=r.x0;x<r.x1;x++)
                for(int y=r.y0;y<r.y1;y++)
                    image(x,y) = color;
        }
    }

    template<class T>
    void rotate_90(narray<T> &out, narray<T> &in) {
        out.resize(in.dim(1),in.dim(0));
        for (int x=0;x<in.dim(0);x++)
            for (int y=0;y<in.dim(1);y++)
                out(y,in.dim(0)-x-1) = in(x,y);
    }
    template void rotate_90<byte>(narray<byte> &,narray<byte> &);


    template<class T>
    void rotate_270(narray<T> &out, narray<T> &in) {
        out.resize(in.dim(1), in.dim(0));
        for (int x=0;x<in.dim(0);x++) {
            for (int y=0; y<in.dim(1);y++)
                out(in.dim(1)-y-1,x) = in(x,y);
        }
    }
    template void rotate_270<byte>(narray<byte> &,narray<byte> &);

    template<class T>
    void rotate_180(narray<T> &out, narray<T> &in) {
        out.resize(in.dim(0), in.dim(1));
        for (int x=0;x<in.dim(0);x++) {
            for (int y=0; y<in.dim(1);y++)
                out(in.dim(0)-x-1,in.dim(1)-y-1) = in(x,y);
        }
    }
    template void rotate_180<byte>(narray<byte> &,narray<byte> &);
}
