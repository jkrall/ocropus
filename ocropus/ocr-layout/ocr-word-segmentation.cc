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
// This version of the xycut algorithm has been implemented according to
// the algorithm described in "Empirical Performance Evaluation Methodology and
// Its Application to Page Segmentation Algorithms" by Mao and Kanungo(Figure 3)
//
// Project: OCRopus
// File: ocr-word-segmentation.cc
// Purpose: Word segmentation using smearing of bounding boxes
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace iulib;
using namespace colib;
using namespace ocropus;

namespace {
    struct SegmentWords : ISegmentPage {
        SegmentWords();

        ~SegmentWords() {}

        const char *name() {
            return "segwords";
        }

        const char *description() {
            return "segment words by smearing\n";
        }

        void segment(colib::intarray &image,colib::bytearray &in);
    };

    SegmentWords::SegmentWords(){
    }

    void SegmentWords::segment(intarray &image,bytearray &in_not_inverted) {

        bytearray in;
        copy(in, in_not_inverted);
        binarize_simple(in);
        optional_check_background_is_lighter(in);
        invert(in);
        // Do connected component analysis
        intarray charimage;
        copy(charimage,in);
        label_components(charimage,false);
        rectarray bboxes;
        bounding_boxes(bboxes,charimage);
        if(bboxes.length()==0){
            makelike(image,in);
            fill(image,0x00ffffff);
            return ;
        }
        //fprintf(stderr,"Time elapsed (bounding_boxes): %.3f \n",(clock()/float(CLOCKS_PER_SEC)) - startTime);
        autodel<CharStats> charstats(make_CharStats());
        charstats->getCharBoxes(bboxes);
        charstats->calcCharStats();
        //if(debug_layout>=2)
        //    charstats->print();
        fill(in,0);
        for(int i=0,l=charstats->char_boxes.length(); i<l; i++){
            rectangle r = charstats->char_boxes[i];
            for(int x=r.x0; x<r.x1; x++)
                for(int y=r.y0; y<r.y1; y++)
                    in(x,y)=255-in_not_inverted(x,y);
            paint_box_border(in,r,0xff,true);
        }

        int dot_dilation = 2*charstats->char_spacing;
        for(int i=0, l=charstats->dot_boxes.length(); i<l; i++){
            rectangle d = charstats->dot_boxes[i].dilated_by(dot_dilation,dot_dilation,0,0);
            if(d.x0<0) d.x0=0;
            if(d.y0<0) d.y0=0;
            bool white_pixel_found = false;
            for(int x=d.x0; x<d.x1 && !white_pixel_found; x++){
                for(int y=d.y0; y<d.y1 && !white_pixel_found; y++){
                    if(in(x,y)==255)
                        white_pixel_found = true;
                }
            }
            if(white_pixel_found){
            for(int x=d.x0; x<d.x1; x++)
                for(int y=d.y0; y<d.y1; y++)
                    in(x,y)=255-in_not_inverted(x,y);
                paint_box_border(in,d,0xff,true);
            }
        }
        //write_image_gray("inverted.png",in);
        binary_close_rect(in,2*charstats->char_spacing,charstats->char_spacing);
        copy(charimage,in);
        label_components(charimage,false);
        rectarray wordboxes;
        bounding_boxes(wordboxes,charimage);
        makelike(image,in_not_inverted);
        for(int i=0,l=in_not_inverted.length1d(); i<l; i++){
            if(in_not_inverted.at1d(i) == 0xff || charimage.at1d(i)==0)
                image.at1d(i) = 0x00ffffff;
            else
                image.at1d(i) = charimage.at1d(i);
        }
    }
}

namespace ocropus {
    ISegmentPage *make_SegmentWords() {
        return new SegmentWords();
    }

    bool segment_words_by_projection(intarray &gapimage,
                                     bytearray &in_not_inverted,
                                     int num_words) {
        bytearray in;
        copy(in, in_not_inverted);
        invert(in);
        binarize_simple(in);

        bytearray projection;
        projection.resize(in.dim(0));

        int col_sum=0;
        for(int x=0,w=in.dim(0); x<w; x++){
            col_sum=0;
            for(int y=0,h=in.dim(1); y<h; y++){
                if(in(x,y))
                    col_sum ++;
            }
            projection[x]=col_sum;
        }
        heap<int,true> gaps;
        //find gaps in projection
        int gapstart=-1, gapend=-1;
        for(int i=1,l=projection.length()-1; i<l; i++){
            if(projection[i] == 0 && projection[i-1] != 0)
                gapstart = i;
            if(gapstart != -1 && projection[i] != 0 && projection[i-1] == 0){
                gapend = i;
                int gappos = (gapend+gapstart)/2;
                gaps.insert(gappos, double(gapend-gapstart));
                gapstart = -1;
            }
        }

        makelike(gapimage, in);
        fill(gapimage,1);

        // no of gaps is always 1 less than the no of words
        if(gaps.length()<num_words-1){
            fprintf(stderr,"Less gaps than desired no of words.\n");
            return false;
        }

        for(int i=0; i<num_words-1; i++){
            int gappos = gaps.extractMax();
            for(int y=0,h=in.dim(1); y<h; y++){
                gapimage(gappos,y) = 0;
            }
        }
        label_components(gapimage,false);
        // simple_recolor(gapimage);
        for(int x=0,w=in.dim(0); x<w; x++){
            for(int y=0,h=in.dim(1); y<h; y++){
                if(!in(x,y))
                    gapimage(x,y) = 0x00ffffff;
            }
        }
        return true;
    }

}


