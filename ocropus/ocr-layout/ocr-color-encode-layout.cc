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
// File: ocr-color-encode-layout.h
// Purpose: Color encode output of RAST layout analysis as per OCRopus
//          color coding conventions:
//          http://docs.google.com/Doc?id=dfxcv4vc_92c8xxp7
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace colib;

namespace ocropus {

    void ColorEncodeLayout::encode_textlines(){
        int color;
        int column_num = 0x00010000;
        makelike(outputImage,inputImage);
        //Comment out this loop when the input image is not inverted
        for(int i = 0, l = outputImage.length1d(); i<l; i++){
            if(!inputImage.at1d(i))
                inputImage.at1d(i) = 0xff;
            else
                inputImage.at1d(i) = 0;
        }
        for(int i = 0, l = outputImage.length1d(); i<l; i++){
            if(!inputImage.at1d(i))
                outputImage.at1d(i) = 0;
            else
                outputImage.at1d(i) = 0x00ffffff;
        }
        color = 1;
        // Limit the number of text columns to what can actually be encoded (30)
        int MAX_COLS = 30;
        ASSERTWARN(textcolumns.length()<=MAX_COLS);
        int num_cols = (textcolumns.length() < MAX_COLS)?textcolumns.length():MAX_COLS;
        if(num_cols!=textcolumns.length()) {
            fprintf(stderr,"\nWarning: Too many text columns: %d\n", textcolumns.length());
            fprintf(stderr,"         Max # of text columns: %d\n\n", MAX_COLS);
        }
        for(int i = 0, l = textlines.length(); i<l; i++){
            bool changed = false;
            rectangle r = textlines[i];
            int j;
            for(j = 0; j<num_cols; j++)
                if(textcolumns[j].includes(r))
                    break;
            column_num = (j+1)<<16;
            for(int x = r.x0, x1 = r.x1; x<x1; x++){
                for(int y = r.y0, y1 = r.y1; y<y1; y++){
                    if(!inputImage(x,y) && !outputImage(x,y)) {
                        outputImage(x,y) = (color|column_num);
                        changed = true;
                    }
                }
            }
            if(changed)
                color++;
        }

    }

    void ColorEncodeLayout::encode_gutters(){
        for(int i=0, l=gutters.length(); i<l; i++){
            rectangle r = gutters[i];
            for(int x = r.x0, x1 = r.x1; x<x1; x++){
                for(int y = r.y0, y1 = r.y1; y<y1; y++){
                    if(inputImage(x,y))
                        outputImage(x,y) = GUTTER_COLOR;
                }
            }
        }
    }
    
    void ColorEncodeLayout::encode_zones(rectarray &zones, int zone_color){
        sort_boxes_by_y0(zones);
        narray<bool> used;
        makelike(used,zones);
        fill(used,false);

        int num_cols = textcolumns.length();

        // First encode zones that belong to a single column and mark them
        for(int i=0; i<num_cols; i++){
            // Go through zones list in reverse order since they are sorted
            // w.r.t. image bottom
            int zone_num = 0;
            int column_num = i+1;
            for(int j=zones.length()-1; j>=0; j--){
                if(zones[j].fraction_covered_by(textcolumns[i]) >= 0.8){
                    used[j] = true;
                    zone_num++;
                    rectangle r = zones[j];
                    int zone_col = (column_num<<16) | zone_color | zone_num;
                    for(int x = r.x0, x1 = r.x1; x<x1; x++){
                        for(int y = r.y0, y1 = r.y1; y<y1; y++){
                            if(!inputImage(x,y) && !outputImage(x,y))
                                outputImage(x,y) = zone_col;
                        }
                    }
                    
                }
            }
        }

        // All unmarked zones do not belong to a single column so label them
        // as multi-column elements
        int n_multicol_zones = 0;
        for(int i=0; i<zones.length(); i++){
            if(!used[i]){
                rectangle r = zones[i];
                n_multicol_zones++;
                int zone_col = MULTI_COLUMN_ELEMENT_COLOR | 
                    zone_color | n_multicol_zones ;
                for(int x = r.x0, x1 = r.x1; x<x1; x++){
                    for(int y = r.y0, y1 = r.y1; y<y1; y++){
                        if(!inputImage(x,y) && !outputImage(x,y))
                            outputImage(x,y) = zone_col;
                    }
                }
            }
        }
    }
    
    void ColorEncodeLayout::encode_images(){
        encode_zones(images, IMAGE_COLOR);
    }
    
    void ColorEncodeLayout::encode_graphics(){
        encode_zones(graphics, GRAPHICS_COLOR);
    }
    
    void ColorEncodeLayout::encode_rulings(){
        encode_zones(rulings, RULING_COLOR);
    }
    
    void ColorEncodeLayout::encode(){
        encode_textlines();
        encode_gutters();
        encode_images();
        encode_graphics();
        encode_rulings();
    }

    ColorEncodeLayout *make_ColorEncodeLayout(){
        return new ColorEncodeLayout();
    }

}
