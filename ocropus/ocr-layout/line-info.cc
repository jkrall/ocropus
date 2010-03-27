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
// File: line-info.cc
// Purpose: getting line information from a single line
// Responsible: Faisal Shafait
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include <stdlib.h>
#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace iulib;
using namespace colib;

namespace ocropus {
    param_bool debug_line_info("debug_line_info",0,"Get line info image");

    void paint_line(intarray &image, TextLineExtended l){
        int width=image.dim(0);
        int height=image.dim(1);
        float y;
        float slope = l.m;
        float y_intercept = l.c;
        float descender = l.d;
        float ascender = l.a;
        float xheight = l.x;
        int   start =  (l.bbox.x0>0) ? (int) l.bbox.x0 : 0;
        int   end   =  (l.bbox.x1  <width) ? (int) l.bbox.x1  : width;
        int yl, yh, dl, dh, al, ah, xl, xh;
        bool baseline_only = false;
        //for horizontal lines

        for(int x=start;x<end;x++){
            y = slope * x + y_intercept;
            yl = (int) y;
            yh = yl +1;
            dl = (int) (y - descender); // Origin is in bottom left corner
            dh = dl +1;
            xl = (int) (y + xheight);
            xh = xl +1;
            al = (int) (y + xheight + ascender);
            ah = al +1;
            if( (yl >= 0) && (yl < height) )
                image(x,yl) &= 0xff0000ff;
            if( (yh >= 0) && (yh < height) )
                image(x,yh) &= 0xff0000ff;
            if(!baseline_only){
                if( (dl >= 0) && (dl < height) )
                    image(x,dl) &= 0xff00ffff;
                if( (dh >= 0) && (dh < height) )
                    image(x,dh) &= 0xff00ffff;
                if( (al >= 0) && (al < height) )
                    image(x,al) &= 0x00ff0000;
                if( (ah >= 0) && (ah < height) )
                    image(x,ah) &= 0x00ff0000;
                if( (xl >= 0) && (xl < height) )
                    image(x,xl) &= 0x0000ff00;
                if( (xh >= 0) && (xh < height) )
                    image(x,xh) &= 0x0000ff00;
            }
        }

    }

    static void set_default_line_info(float &intercept,
                                                float &slope,
                                                float &xheight,
                                                float &descender_sink,
                                                float &ascender_rise,
                                                intarray &charimage) {
        int ystart=-1,yend=-1;
        for(int y=0, h=charimage.dim(1); y<h; y++){
            for(int x=0, w=charimage.dim(0); x<w; x++){
                if(charimage(x,y)){
                    if(ystart==-1){
                        ystart = y;
                        break;
                    }
                    else
                        yend = y;
                }
            }
        }
        //fprintf(stderr,"ystart, yend = %d %d\n",ystart,yend);
        if(ystart!=-1 && yend!=-1){
            intercept = ystart;
            xheight = yend - ystart;
        }else{
            intercept = charimage.dim(1)/3.0;
            xheight = max(1.0,charimage.dim(1)/3.0);
        }
        //fprintf(stderr,"baseline = %.1f, xheight= %.1f\n",intercept,xheight);
        slope = 0;
        descender_sink = 0;
        ascender_rise = 0;

        if(debug_line_info){
            TextLineExtended t;
            t.c = intercept;
            t.m = slope;
            t.d = descender_sink;
            t.x = xheight;
            t.a = ascender_rise+xheight;
            intarray lineimage;
            makelike(lineimage, charimage);
            for(int i=0; i<charimage.length1d(); i++){
                lineimage.at1d(i) = 0x00ffffff*!charimage.at1d(i);
            }
            paint_line(lineimage, t);
            write_image_packed("line-info.png",lineimage);
        }
    }

    static bool internal_get_extended_line_info(float &intercept,
                                                float &slope,
                                                float &xheight,
                                                float &descender_sink,
                                                float &ascender_rise,
                                                intarray &charimage) {

        // Do connected component analysis
        if(charimage.dim(1) < 5){
            set_default_line_info(intercept, slope, xheight,
                                  descender_sink, ascender_rise, charimage);
            return true;
        }
        make_line_segmentation_black(charimage);
        label_components(charimage,false);

        // Clean non-text and noisy boxes and get character statistics
        rectarray bboxes;
        bounding_boxes(bboxes,charimage);
        if(!bboxes.length()){
            set_default_line_info(intercept, slope, xheight,
                                  descender_sink, ascender_rise, charimage);
            return true;
        }
        autodel<CharStats> charstats(make_CharStats());
        charstats->getCharBoxes(bboxes);
        if(!charstats->char_boxes.length()){
            set_default_line_info(intercept, slope, xheight,
                                  descender_sink, ascender_rise, charimage);
            return true;
        }
        charstats->calcCharStats();

        // Extract textlines
        autodel<CTextlineRASTExtended> ctextline(make_CTextlineRASTExtended());
        ctextline->min_q       = 2.0; // Minimum acceptable quality of a textline
        ctextline->min_count   = 2;   // ---- number of characters in a textline
        ctextline->min_length  = 30;  // ---- length in pixels of a textline
        ctextline->max_results = 1;
        ctextline->min_gap     = 500;
        ctextline->epsilon     = 2;

        narray<TextLineExtended> textlines;
        ctextline->extract(textlines,charstats);

        // Return the info
        if(!textlines.length()) {
            set_default_line_info(intercept, slope, xheight,
                                  descender_sink, ascender_rise, charimage);
        }else{
            TextLineExtended &t = textlines[0];
            intercept = t.c;
            slope = t.m;
            descender_sink = t.d;
            xheight = t.x;
            ascender_rise = t.x+t.a;
            if(debug_line_info){
                intarray lineimage;
                makelike(lineimage, charimage);
                for(int i=0; i<charimage.length1d(); i++){
                    lineimage.at1d(i) = 0x00ffffff*!charimage.at1d(i);
                }
                for(int i=0; i<textlines.length(); i++){
                    paint_line(lineimage, textlines[i]);
                }
                write_image_packed("line-info.png",lineimage);
            }
        }


        return true;
    }

    bool get_extended_line_info(float &intercept, float &slope,
                                float &xheight, float &descender_sink,
                                float &ascender_rise, intarray &charimage) {

        intarray c;
        copy(c, charimage);
        return internal_get_extended_line_info(intercept, slope, xheight,
                                               descender_sink, ascender_rise, c);
    }

    bool get_extended_line_info_using_ccs(float &intercept, float &slope,
                                          float &xheight, float &descender_sink,
                                          float &ascender_rise,
                                          bytearray &charimage) {

        intarray c;
        copy(c, charimage);
        label_components(c);
        return internal_get_extended_line_info(intercept, slope, xheight,
                                               descender_sink, ascender_rise, c);
    }

    bool get_extended_line_info(TextLineExtended &t, intarray &seg) {
        return get_extended_line_info(t.c, t.m, t.x, t.d, t.a, seg);
    }

    bool get_extended_line_info_using_ccs(TextLineExtended &t,
                                          bytearray &image) {
        return get_extended_line_info_using_ccs(t.c, t.m, t.x, t.d, t.a, image);
    }
}
