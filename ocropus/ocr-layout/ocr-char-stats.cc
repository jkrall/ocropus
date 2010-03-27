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
// Project: OCRopus
// File: ocr-char-stats.cc
// Purpose: calculate character statistics from bounding boxes
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace ocropus;
using namespace iulib;
using namespace colib;

namespace ocropus {
    extern param_int debug_layout;

    void sort_boxes_by_x0(rectarray &boxes) {
        int len = boxes.length();
        if(!len)
            return;
        intarray values,index;
        makelike(values,boxes);
        for(int i=0; i<len; i++)
            values[i] = boxes[i].x0;
        quicksort(index,values);
        permute(boxes,index);
    }
    
    void sort_boxes_by_y0(rectarray &boxes) {
        int len = boxes.length();
        if(!len)
            return;
        intarray values,index;
        makelike(values,boxes);
        for(int i=0; i<len; i++)
            values[i] = boxes[i].y0;
        quicksort(index,values);
        permute(boxes,index);
    }
    
    static void calc_hist(floatarray &hist, floatarray &distances,
                          int minval, int maxval){
        hist.clear();
        hist.resize(maxval+1);
        fill(hist,0.0);
        int len = distances.length();
        for(int i=0; i<len; i++){
            if(distances[i]<=minval || distances[i]>=maxval)
                continue;
            hist[int(distances[i])]++;
        }
    }

    static void calc_hist(floatarray &hist, floatarray &distances){
        float maxval=-10000;
        int   len = distances.length();
        for(int i=0; i<len; i++)
            maxval = max(maxval, distances[i]);
        
        hist.clear();
        hist.resize((int)maxval+1); //Not interested in -ive values
        for(int i=0; i<=maxval; i++)
            hist[i] = 0;
        for(int i=0; i<len; i++){
            hist[(int)distances[i]] ++;
        }
    }
    
    // estimate the height of a line of text using bounding boxes of connected components
    int calc_xheight(rectarray &bboxes) {
        int i;
        int best_i;
        float best_v;
        floatarray heights;
        floatarray histogram;
        int h=200, min_height=2;
        float smooth=4.0;
        int len = bboxes.length();
        for(int i=0; i< len; i++){
            if(bboxes[i].height()<h)
                heights.push(bboxes[i].height());
        }
        calc_hist(histogram,heights);
        gauss1d(histogram,smooth);
        
        best_i = -1;
        best_v = -1.0;
        for(i=min_height;i<histogram.length();i++) {
            if(histogram[i]<best_v) continue;
            best_i = i;
            best_v = histogram[i];
        }
        
        return best_i;
    }

    static inline bool x_overlap(rectangle a, rectangle b){
        return ( (a.x1 >= b.x0) && (b.x1 >= a.x0) );
    }
    
    static inline bool y_overlap(rectangle a, rectangle b){
        return ( (a.y1 >= b.y0) && (b.y1 >= a.y0) );
    }
    
    static void calc_distances(floatarray &hdist,
                               floatarray &vdist,
                               rectarray &cboxes){
        int    num_chars = cboxes.length();
        float  maxdist = 200;
        float  temp = 0,minhdist,minvdist;
        int    start;
        //Start with i=1 to skip first bounding box which is page size.
        start = ( (cboxes[0].x0==0) && (cboxes[0].y0==0) ) ? 1 : 0;
        for(int i=start; i< num_chars; i++){
            rectangle r = cboxes[i];
            minhdist = 10000;
            minvdist = 10000;
            for(int j=start; j< num_chars; j++){
                rectangle cand = cboxes[j];
                if(y_overlap(r,cand) && (cand.x0 > r.x1)){
                    temp = cand.x0 - r.x1;
                    if(temp < minhdist)
                        minhdist = temp;
                }
                if(x_overlap(r,cand) && (cand.y0 > r.y1)){
                    temp = cand.y0 - r.y1;
                    if(temp < minvdist)
                        minvdist = temp;
                }
            }
            if(minhdist && (minhdist<maxdist))
                hdist.push(minhdist);
            if(minvdist && (minvdist<maxdist))
                vdist.push(minvdist);
        }
    }


    CharStats::CharStats() {
        img_height    = -1;
        img_width     = -1;
        xheight       = -1;
        char_spacing  = -1;
        word_spacing  = -1;
        line_spacing  = -1;
        char_boxes.clear();
        dot_boxes.clear();
        large_boxes.clear();
    }

    CharStats::CharStats(CharStats &c) {
        img_height    = c.img_height;
        img_width     = c.img_width;
        xheight       = c.xheight;
        char_spacing  = c.char_spacing;
        word_spacing  = c.word_spacing;
        line_spacing  = c.line_spacing;
        copy(char_boxes,c.char_boxes);
        copy(dot_boxes,c.dot_boxes);
        copy(large_boxes,c.large_boxes);
    }

    CharStats::~CharStats() {
        concomps.dealloc();
        char_boxes.dealloc();
        dot_boxes.dealloc();
        large_boxes.dealloc();
    }

    void CharStats::getCharBoxes(rectarray &comps){

        if(!comps.length()){
            fprintf(stderr,"Empty connected components array. Aborting ...\n");
            exit(1);
        }
        img_width  = comps[0].x1;
        img_height = comps[0].y1;

        for(int i=1;i<comps.length();i++)
            if(comps[i].area())
                concomps.push(comps[i]);
        
        int histsize  = 200;
        int mindim    = 5;
        floatarray xhist,yhist,widtharray,heightarray;
        for(int i=0;i<concomps.length();i++) {
            widtharray.push(concomps[i].width());
            heightarray.push(concomps[i].height());
        }
        calc_hist(xhist,widtharray,mindim,histsize);
        calc_hist(yhist,heightarray,mindim,histsize);
        //plot_hist(fopen("xhist-original.png","w"),xhist);
        //plot_hist(fopen("yhist-original.png","w"),yhist);
        //gauss1d(xhist,1.0);
        //gauss1d(yhist,1.0);
        //plot_hist(fopen("xhist.png","w"),xhist);
        //plot_hist(fopen("yhist.png","w"),yhist);
        int xmode = argmax(xhist);
        int ymode = argmax(yhist);
        if(debug_layout>=2){
            printf("x-mode = %d, y-mode =%d, x-peak = %.2f, y-peak=%.2f\n",
                   xmode,ymode,xhist[xmode],yhist[ymode]);
        }

        int minarea    = ymode*2;
        int mindotarea = ymode/3;
        int maxaspect  = 10;
        int maxheight  = ymode*10;
        int maxwidth   = xmode*10;
        int minheight  = ymode>>1;
        int minwidth   = xmode>>1;
        for(int i=0;i<concomps.length();i++) {
            int bw = concomps[i].width();
            int bh = concomps[i].height();
            if(bw*bh<minarea){
                if(bw*bh>mindotarea)
                    dot_boxes.push(concomps[i]);
                continue;
            }
            if(bw<minwidth && bh<minheight){
                dot_boxes.push(concomps[i]);
                continue;
            }
            if(bw/bh>maxaspect || bh/bw>maxaspect ||
               bw>maxwidth || bh>maxheight){
                large_boxes.push(concomps[i]);
                continue;
            }
            char_boxes.push(rectangle(concomps[i]));
        }

    }
    
    void CharStats::calcCharStats(){
        calcCharStats(char_boxes);
    }
    
    void CharStats::calcCharStats(rectarray &cboxes){
        if(!cboxes.length()){
            fprintf(stderr,"No character boxes found! ...\n");
            return ;
        }
        
        xheight = calc_xheight(cboxes);
        sort_boxes_by_y0(cboxes);
        
        floatarray   vdist,hdist;
        floatarray   vhist,hhist;
        float        maxdist = 200;
        float        smooth = 2.0; 
        
        calc_distances(hdist, vdist, cboxes);
        calc_hist(hhist, hdist);
        calc_hist(vhist, vdist);
        gauss1d(hhist,smooth);
        gauss1d(vhist,smooth);
        intarray modes;
        peaks(modes, hhist, 1, int(maxdist));
        if(modes.length()<2){
            if(debug_layout)
                fprintf(stderr,"Warning: No peaks found in character spacing histogram!\n");
            char_spacing = xheight / 4;
            word_spacing = xheight / 2;
        }
        else{
            char_spacing = modes[0];
            word_spacing = modes[1];
        }
        float mv = -1;
        for(int i=0, l=vhist.length(); i<l; i++){
            if(vhist[i] > mv){
                mv = vhist[i];
                line_spacing = i;
            }
        }
    }
    
    void CharStats::calcCharStatsForOneLine(){
        calcCharStatsForOneLine(char_boxes);
    }

    void CharStats::calcCharStatsForOneLine(rectarray &cboxes){
        if(!cboxes.length()){
            fprintf(stderr,"No character boxes found! ...\n");
            return ;
        }
        
        xheight = calc_xheight(cboxes);
        sort_boxes_by_x0(cboxes);
        
        floatarray   hdist,vdist;
        floatarray   hhist;
        float        maxdist = 200;
        
        calc_distances(hdist, vdist, cboxes);
        calc_hist(hhist, hdist);
        intarray modes;
        peaks(modes, hhist, 1, int(maxdist));
        if(modes.length()<2){
            if(debug_layout)
                fprintf(stderr,"Warning: No peaks found in character spacing histogram!\n");
            char_spacing = xheight / 4;
            word_spacing = xheight / 2;
        }
        else{
            char_spacing = modes[0];
            word_spacing = modes[1];
        }
    }

    void CharStats::print() {
        printf("Image Size   = %d x %d\n",img_width,img_height);
        printf("xheight      = %d\n",xheight);
        printf("Char spacing = %d\n",char_spacing);
        printf("Word spacing = %d\n",word_spacing);
        printf("Line spacing = %d\n",line_spacing);
        printf("Number of Characters = %d\n",char_boxes.length());
        printf("Number of dots = %d\n",dot_boxes.length());
        printf("Number of large components = %d\n",large_boxes.length());
    }

    CharStats *make_CharStats() {
        return new CharStats();
    }
    CharStats *make_CharStats(CharStats &c) {
        return new CharStats(c);
    }
}
