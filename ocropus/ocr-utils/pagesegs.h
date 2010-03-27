// -*- C++ -*-

// Copyright 2006-2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
// or its licensors, as applicable.
//
// You may not use this file except under the terms of the accompanying license.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You may
// obtain a copy of the License at http:  www.apache.org/licenses/LICENSE-2.0
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
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_pagesegs__
#define h_pagesegs__

#include "ocropus.h"

namespace ocropus {

    using namespace colib;
    using namespace iulib;

    void check_page_segmentation(intarray &cseg);
    void make_page_segmentation_black(intarray &a);
    void make_page_segmentation_white(intarray &a);

    inline int pseg_pixel(int column,int paragraph,int line) {
        ASSERT((column > 0 && column < 32) || column == 254 || column == 255);
        ASSERT((paragraph >= 0 && paragraph < 64) || (paragraph >=251 && paragraph <= 255));
        ASSERT(line>=0 && line<256);
        return (column<<16) | (paragraph<<8) | line;
    }

    inline int pseg_pixel(int column,int line) {
        ASSERT(column>0 && column<32);
        ASSERT(line>=0 && line<64*256);
        return (column<<16) | line;
    }

    inline int pseg_column(int pixel) {
        return (pixel>>16)&0xff;
    }

    inline int pseg_paragraph(int pixel) {
        return (pixel>>8) & 0x3f;
    }

    inline int pseg_line(int pixel) {
        return pixel & 0xff;
    }

    inline int pseg_pline(int pixel) {
        return pixel & 0x3fff;
    }

    inline int cseg_pixel(int chr) {
        ASSERT(chr>0 && chr<4096);
        return (1<<12) | chr;
    }

    inline void pseg_columns(intarray &a) {
        for(int i=0;i<a.length1d();i++) {
            int value = a.at1d(i);
            if(value==0xffffff) value = 0;
            value = pseg_column(value);
            if(value>=32) value = 0;
            a.at1d(i) = value;
        }
    }

    inline void pseg_plines(intarray &a) {
        for(int i=0;i<a.length1d();i++) {
            int value = a.at1d(i);
            if(value==0xffffff) value = 0;
            if(pseg_column(value)>=32) value = 0;
            value = pseg_pline(value);
            a.at1d(i) = value;
        }
    }

    struct RegionExtractor {
        intarray segmentation;
        narray<rectangle> boxes;
        void setImage(intarray &image) {
            copy(segmentation,image);
            renumber_labels(segmentation,1);
            bounding_boxes(boxes,segmentation);
        }
        void setImageMasked(intarray &image,int mask,int lo,int hi) {
            makelike(segmentation,image);
            fill(segmentation,0);
            for(int i=0;i<image.length1d();i++) {
                int pixel = image.at1d(i);
                if(pixel<lo || pixel>hi) continue;
                segmentation.at1d(i) = (pixel & mask);
            }
            renumber_labels(segmentation,1);
            bounding_boxes(boxes,segmentation);
        }
        void setPageColumns(intarray &image) {
            makelike(segmentation,image);
            fill(segmentation,0);
            for(int i=0;i<image.length1d();i++) {
                int pixel = image.at1d(i);
                int col = pseg_column(pixel);
                if(col<1||col>=32) continue;
                int par = pseg_paragraph(pixel);
                if(par>=64) continue;
                segmentation.at1d(i) = col;
            }
            renumber_labels(segmentation,1);
            bounding_boxes(boxes,segmentation);
        }
        void setPageParagraphs(intarray &image) {
            makelike(segmentation,image);
            fill(segmentation,0);
            for(int i=0;i<image.length1d();i++) {
                int pixel = image.at1d(i);
                int col = pseg_column(pixel);
                if(col<1||col>=32) continue;
                int par = pseg_paragraph(pixel);
                if(par>=64) continue;
                segmentation.at1d(i) = (col<<8) | par;
            }
            renumber_labels(segmentation,1);
            bounding_boxes(boxes,segmentation);
        }
        void setPageLines(intarray &image) {
            makelike(segmentation,image);
            fill(segmentation,0);
            for(int i=0;i<image.length1d();i++) {
                int pixel = image.at1d(i);
                int col = pseg_column(pixel);
                if(col<1||col>=32) continue;
                int par = pseg_paragraph(pixel);
                if(par>=64) continue;
                segmentation.at1d(i) = pixel;
            }
            renumber_labels(segmentation,1);
            bounding_boxes(boxes,segmentation);
        }
        int length() {
            return boxes.length();
        }
        rectangle bbox(int i) {
            return boxes[i];
        }
        void bounds(int i,int *x0=0,int *y0=0,int *x1=0,int *y1=0) {
            *x0 = boxes[i].x0;
            *y0 = boxes[i].y0;
            *x1 = boxes[i].x1;
            *y1 = boxes[i].y1;
        }
        int x0(int i) {
            return boxes[i].x0;
        }
        int y0(int i) {
            return boxes[i].y0;
        }
        int x1(int i) {
            return boxes[i].x1;
        }
        int y1(int i) {
            return boxes[i].y1;
        }
        template <class S,class T>
        void extract(narray<S> &output,narray<T> &input,int index,int margin=0) {
            rectangle r = boxes[index].grow(margin);
            r.intersect(rectangle(0,0,input.dim(0),input.dim(1)));
            CHECK_CONDITION(!r.empty());
            extract_subimage(output,input,r.x0,r.y0,r.x1,r.y1);
        }
        template <class S>
        void mask(narray<S> &output,int index,int margin=0) {
            rectangle r = boxes[index].grow(margin);
            r.intersect(rectangle(0,0,segmentation.dim(0),segmentation.dim(1)));
            CHECK_CONDITION(!r.empty());
            output.resize(r.x1-r.x0,r.y1-r.y0);
            fill(output,0);
            for(int i=r.x0;i<r.x1;i++) for(int j=r.y0;j<r.y1;j++) {
                    if(segmentation(i,j)==index)
                        output(i-r.x0,j-r.y0) = 255;
                }
        }
    };
    inline void write_page_segmentation(FILE *stream,intarray &a) {
        check_page_segmentation(a);
        write_image_packed(stream,a,"png");
    }
    inline void read_page_segmentation(intarray &a,FILE *stream) {
        read_image_packed(a,stream,"png");
        check_page_segmentation(a);
    }
}

#endif
