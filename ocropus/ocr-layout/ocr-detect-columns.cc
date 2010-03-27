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
// File: ocr-detect-columns.cc
// Purpose: Group textlines into text columns 
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace colib;

namespace ocropus {

    // Get text columns from an array of text-line bounding boxes and an array
    // of whitespace column separators
    void get_text_columns(rectarray &textcolumns,
                          rectarray &textlines,
                          rectarray &gutters){

        if(!textlines.length())  return;
        if(!gutters.length()){
            rectangle column = rectangle();
            for(int i=0; i<textlines.length(); i++)
                column.include(textlines[i]);
            textcolumns.push(column);
            return;
        }

        rectangle column = rectangle(textlines[0]);
        rectangle tempcolumn =
        rectangle(textlines[0].dilated_by(-10,-2,-10,-2));
        for(int i=1; i<textlines.length(); i++){
            tempcolumn.include(textlines[i].dilated_by(-10,-2,-10,-2));
            bool intersects_gutter = false;
            bool gutter_penetrating_from_below = false;
            bool gutter_penetrating_from_above = false;
            for(int j=0; j<gutters.length(); j++){
                point top    = point(gutters[j].xcenter(),gutters[j].y1) ;
                point bottom = point(gutters[j].xcenter(),gutters[j].y0) ;
                if(tempcolumn.overlaps(gutters[j])){
                    intersects_gutter = true;
                    if(textlines[i].contains(top))
                        gutter_penetrating_from_below = true;
                    if(textlines[i].contains(bottom))
                        gutter_penetrating_from_above = true;
                    break;
                }
            }
            if(intersects_gutter && !gutter_penetrating_from_below){
                textcolumns.push(column);
                column = rectangle(textlines[i]);
                if(!gutter_penetrating_from_above)
                    tempcolumn=rectangle(textlines[i].dilated_by(-10,-2,-10,-2));
                else
                    tempcolumn=rectangle();
            } else{
                column.include(textlines[i]);
            }
        }
    }

    static void get_bounding_box(rectangle &bbox,rectarray &bboxes){
        //first copying into arrays for x0,x1,y0,y1
        bbox = rectangle();
        for(int i = 0; i<bboxes.length();i++){
            bbox.include(bboxes[i]);
        }
    }
        
    static float get_overlap(rectangle current,rectangle previous){
        int   intersection = (min(current.x1,previous.x1)-max(current.x0,previous.x0));
        float width_sum = (current.x1-current.x0)+(previous.x1-previous.x0);
        if(width_sum)
            return (2*intersection)/ width_sum;
        else
            return 0;
    }
    
    // Get text columns from an array of paragraphs or zones using their alignment
    void get_text_columns(rectarray &columns,
                          rectarray &paragraphs){
        objlist< rectarray > tempcol;
        objlist< narray<float> > floatcol;
        rectarray temp;
        narray<float> temp1;
        rectarray probablecol;
        rectangle previous;
        rectangle current;
        rectangle tempt;
        // Overlap threshold for grouping paragraphs into columns
        float column_threshold = 0.6;

        if(paragraphs.length() == 0)
                     return ;
       
        //first separating on the basis of y coordinate
        previous = paragraphs[0];
        temp.push(previous);
        temp1.push(1.0);
        for(int i = 1;i<paragraphs.length();i++){
            current = paragraphs[i];
            if(current.y1 > previous.y1){
                move(tempcol.push(),temp);
                move(floatcol.push(),temp1);
                temp.push(current);
                temp1.push(get_overlap(current,previous));
                previous = current;
            }else{
                temp1.push(get_overlap(current,previous));
                temp.push(current);
                previous = current;
            }
        }
        move(tempcol.push(),temp);
        move(floatcol.push(),temp1);

        // now grouping on the basis of overlap and getting bounding boxes of the columns
       
        FILE *colfile = fopen("columns.dat","w");
        for(int i=0; i<tempcol.length(); i++){
                probablecol.push(tempcol[i][0]);
            for(int j = 1;j<tempcol[i].length();j++){
                if(floatcol[i][j]>column_threshold){
                    probablecol.push(tempcol[i][j]);
                }else{
                    get_bounding_box(tempt,probablecol);
                    columns.push(tempt);
                    probablecol.dealloc();
                    probablecol.push(tempcol[i][j]);
                }
            }
            get_bounding_box(tempt,probablecol);
            probablecol.dealloc();
            columns.push(tempt);
            tempt.println(colfile);
        }
        fclose(colfile);
    }
}
