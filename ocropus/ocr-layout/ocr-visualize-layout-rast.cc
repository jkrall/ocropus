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
// File: ocr-visualize-layout-rast.cc
// Purpose: Visualize output of layout analysis using RAST in the same way
//          as the web demo:
//          http://demo.iupr.org/layout/layout.php
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

namespace ocropus {

    static void paint_line(intarray &image, line l){
        int width=image.dim(0);
        int height=image.dim(1);
        float y;
        float slope = l.m;
        float y_intercept = l.c;
        float descender = l.d;
        int   start =  (l.start>0) ? (int) l.start : 0;
        int   end   =  (l.end  <width) ? (int) l.end   : width;
        int yl, yh, dl, dh;
        bool baseline_only = false;
        //for horizontal lines
    
        for(int x=start;x<end;x++){
            y = slope * x + y_intercept;
            yl = (int) y;
            yh = yl +1;
            dl = (int) (y - descender); // Origin is in bottom left corner
            dh = dl +1;
            if( (yl >= 0) && (yl < height) )
                image(x,yl) &= 0xff0000ff;
            if( (yh >= 0) && (yh < height) )
                image(x,yh) &= 0xff0000ff;
            if(!baseline_only){
                if( (dl >= 0) && (dl < height) )
                    image(x,dl) &= 0xff00ffff;
                if( (dh >= 0) && (dh < height) )
                    image(x,dh) &= 0xff00ffff;
            }
        }
        
    }

    // paint a line connecting the centers of two consecutive lines in reading
    // order

    static void connect_line_centers(intarray &image, line a, line b){
        int width=image.dim(0);
        float x1 = (a.start + a.end)/2.0;
        float x2 = (b.start + b.end)/2.0;
        float y1 = a.m * x1 + a.c ;
        float y2 = b.m * x2 + b.c ;

        if (y2==y1) return;
        float slope_inverse = ((y2 - y1) != 0)? (x2 - x1)/(y2 - y1) : HUGE_VAL;

        //if (x1 > x2) swap(x1, x2);
        //if (y1 > y2) swap(y1, y2);

        int linewidth = 1; //actual line width = 2*linewidth +1
        int thickness  = 2 * linewidth + 1;
        int yoffset = 10; //Height of arrow head

        float x,y;
        if(y1 < y2){
            for (y = y1; y<= y2; y++){
                x = slope_inverse * (y - y1) + x1;
                x = (x <  2*linewidth)   ? 2*linewidth : x;
                x = (x >= width-2*linewidth) ? width-2*linewidth-1 : x;
                for(int i = 0; i < thickness; i++)
                    image((int)x-linewidth+i ,(int)y) &= 0xffff00ff;
                if (y >= y2 - yoffset){
                    for(int i = 0; i < 2*thickness; i++)
                        image((int)x-2*linewidth+i ,(int)y) &= 0xffff00ff;
                }
            }
        }else{
            for (y = y1; y>= y2; y--){
                x = slope_inverse * (y - y1) + x1;
                x = (x <  2*linewidth)   ? 2*linewidth : x;
                x = (x >= width-2*linewidth) ? width-2*linewidth-1 : x;
                for(int i = 0; i < thickness; i++)
                    image((int)x-linewidth+i ,(int)y) &= 0xffff00ff;
                if (y <= y2 + yoffset){
                    for(int i = 0; i < 4*thickness; i++)
                        image((int)x-4*linewidth+i ,(int)y) &= 0xffff0000;
                }
            }
        }
    }

    static void paint_reading_order(intarray &image, narray<line> &lines_ordered){
        int size = lines_ordered.length();
        for(int i=0; i<size-1; i++){
            connect_line_centers(image, lines_ordered[i], lines_ordered[i+1]);
        }
    }

    void visualize_layout(intarray &debug_image,
                          bytearray &in_not_inverted,
                          narray<TextLine> &textlines,
                          rectarray &gutters, 
                          rectarray &extra_obstacles, 
                          CharStats &charstats) {
        makelike(debug_image,in_not_inverted);
        int v0 = min(in_not_inverted);
        int v1 = max(in_not_inverted);
        int threshold = (v1+v0)/2;
        for(int i=0; i<in_not_inverted.length1d(); i++)
            debug_image.at1d(i)=int((in_not_inverted.at1d(i)<threshold)?0:0x00ffffff);
        narray<line> lines;
        for(int i = 0; i<textlines.length(); i++)
            lines.push(line(textlines[i]));

        //Since vertical rulings has the same role as whitespace gutters, just
        //add them to vertical separators list
        rectarray vert_separators;
        for(int i=0,l=extra_obstacles.length(); i<l; i++){
            vert_separators.push(extra_obstacles[i]);
        }
        for(int i=0,l=gutters.length(); i<l; i++){
            vert_separators.push(gutters[i]);
        }

        if(lines.length() > 1){
            for(int i=0,l=lines.length();i<l;i++){
                paint_line(debug_image,lines[i]);
            }
            if(vert_separators.length()){
                sort_boxes_by_x0(vert_separators);
                extend_lines(lines, vert_separators, charstats.img_width);
            }

            paint_reading_order(debug_image,lines);
        }

        for(int i=0; i<gutters.length(); i++){
            paint_box(debug_image,gutters[i],0x00ffff00);
            paint_box_border(debug_image,gutters[i],0x0000ff00);
        }
            
        for(int i=0, l=extra_obstacles.length(); i<l; i++)
            paint_box(debug_image,extra_obstacles[i],0x00ff0000);
    }
    
    void visualize_segmentation_by_RAST(intarray &result, 
                                        bytearray &in_not_inverted) {
        SegmentPageByRAST s;
        rectarray obstacles;
        s.visualize(result, in_not_inverted, obstacles);
    }

    void visualize_segmentation_by_RAST(intarray &result, 
                                        bytearray &in_not_inverted,
                                        rectarray &extra_obstacles) {
        SegmentPageByRAST s;
        s.visualize(result, in_not_inverted, extra_obstacles);
    }

}
