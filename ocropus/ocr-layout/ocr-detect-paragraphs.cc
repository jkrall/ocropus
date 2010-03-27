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
// File: ocr-detect-paragraphs.cc
// Purpose: Group textlines into paragraphs 
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace iulib;
using namespace colib;

namespace ocropus {

    const int LEFT_ALIGNED   = 1;
    const int RIGHT_ALIGNED  = 2;
    const int CENTER_ALIGNED = 3;
    const int JUSTIFIED      = 4;
    const int NOT_ALIGNED    = 0;

    static void get_bounding_box(rectangle &bbox,rectarray &bboxes){
        //first copying into arrays for x0,x1,y0,y1
        bbox = rectangle();
        for(int i = 0; i<bboxes.length();i++){
            bbox.include(bboxes[i]);
        }
    }
        
    static int alignment(bool left, bool right, bool center){
        if(left == true   &&  right == true   && center == true)  { return JUSTIFIED;}
        if(left == false  &&  right == false  && center == true)  { return CENTER_ALIGNED;}
        if(left == true   &&  right == false  && center == false) { return LEFT_ALIGNED;}
        if(left == false  &&  right == true   && center == false) { return RIGHT_ALIGNED;}
        return NOT_ALIGNED;
    }

    static int get_relative_alignment(rectangle &current,rectangle &previous){
        bool left = false;
        bool right = false;
        bool center = false;
        int align_range = 10;
        if(current.x0<(previous.x0 + align_range) && current.x0>(previous.x0-align_range))
            {left = true;}
        

        if(current.x1<(previous.x1+align_range) && current.x1>(previous.x1-align_range))
            {right = true;}
        
        if(current.xcenter() < (previous.xcenter() + align_range) && 
           current.xcenter()  >(previous.xcenter() - align_range))
            {center = true;}
        
        return alignment(left, right, center);
    }

    static void get_alignment(objlist< narray <int> > &tempalign, 
                         objlist< narray <rectangle> > &temppara){
        narray<int> talign;
        rectangle current,previous;
        for(int i = 0;i<temppara.length();i++){
            talign.push(NOT_ALIGNED);
            previous = temppara[i][0];
            for(int j = 1;j<temppara[i].length();j++){
                current = temppara[i][j];
                talign.push(get_relative_alignment(current,previous));
                previous = current;
            }
            move(tempalign.push(),talign);
        }
    }
               
    static void merge_single_line_paras(objlist<rectarray > &amcolumns,
                                        objlist<rectarray > &finalpara,
                                        objlist<narray<int> > &finalalign){
        objlist< narray <int> > amalignment;
        rectarray current_tpara;
        narray<int> current_talign;
        
        for(int i = 0;i<finalpara.length();i++){
            if(finalpara[i].length() == 1 && finalalign[i][0] == 1){
                if((amcolumns.length()-1) >= 0){
                    amcolumns[amcolumns.length()-1].push(finalpara[i][0]);
                    amalignment[amcolumns.length()-1].push(finalalign[i][0]);
                }
            }else if(finalpara[i].length() == 2 && finalalign[i][0] == 0 &&
                     finalalign[i][1] == 2 && (i+1)<finalpara.length()){
                current_tpara.push(finalpara[i][0]);
                current_tpara.push(finalpara[i][1]);
                current_talign.push(finalalign[i][0]);
                current_talign.push(finalalign[i][1]);
                for(int j = 0;j<finalpara[(i+1)].length();j++){
                    current_tpara.push(finalpara[i+1][j]);
                    current_talign.push(finalalign[i+1][j]);
                }
                move(amcolumns.push(),current_tpara);
                move(amalignment.push(),current_talign);
                i = i+1;
            }else{
                copy(current_tpara,finalpara[i]);
                move(amcolumns.push(),current_tpara);
                copy(current_talign,finalalign[i]);
                move(amalignment.push(),current_talign);
            }
        }
    }

    static bool matches_previous(int previous_alignment, int tempalign){
        return (previous_alignment == 0 && tempalign != 0) ||
            (previous_alignment != 0 && tempalign == previous_alignment);
    }
      
    static bool does_not_match_previous(int previous_alignment, int tempalign){
        return (previous_alignment == 0 && tempalign == 0) ||
            (previous_alignment != 0 && tempalign != previous_alignment);
    }
      
    static bool new_column(rectangle current, rectangle previous){
        if(current.y0 > previous.y1){return true;}
        return false;
    }
    
    
    // FIXME
    // method too long
    // --tmb

    void get_paragraphs(rectarray &paragraphs ,
                        rectarray &textlines,
                        CharStats &charstats){

        if(textlines.length() == 0)
            return ;
        rectangle current,previous;
        //initializing previous
        objlist< narray <rectangle> > temppara;
        objlist< narray <int> > tempalign;
        objlist< narray <rectangle> > finalpara;
        objlist< narray <int> > finalalign;
        rectarray current_tpara;
        narray<int> current_talign;
        
        
        //since the textlines are sorted we group them on the basis of y
        //coordinate and gaps between them. 
 
        previous = textlines[0];
        current_tpara.push(previous);
        for(int i = 1;i<textlines.length();i++){
            current = textlines[i];
            if(!new_column(current,previous) && 
               (previous.y0-current.y1 < 2*charstats.line_spacing)){
                current_tpara.push(current);
                previous = current;
            }else{
                move(temppara.push(),current_tpara);
                current_tpara.push(current);
                previous = current;
            }
            if(i+1 == textlines.length()){move(temppara.push(),current_tpara);}
        }
        
        // now get alignment for the temporary paragraphs
    
        get_alignment(tempalign,temppara);
        
        //now divide into groups where alignment changes
        
        int previous_alignment;
        for(int i = 0;i<temppara.length();i++){
            current_tpara.push(temppara[i][0]);
            current_talign.push(tempalign[i][0]);
            previous_alignment = tempalign[i][0];
            for(int j = 1;j<temppara[i].length();j++){
                if(matches_previous(previous_alignment, tempalign[i][j])){
                    current_tpara.push(temppara[i][j]);
                    current_talign.push(tempalign[i][j]);
                    previous_alignment = tempalign[i][j];
                }
                if(does_not_match_previous(previous_alignment, tempalign[i][j])){
                    move(finalpara.push(),current_tpara);
                    move(finalalign.push(),current_talign);
                    current_tpara.push(temppara[i][j]);
                    current_talign.push(tempalign[i][j]);
                    previous_alignment = tempalign[i][j];
                }
            }
            move(finalpara.push(),current_tpara);
            move(finalalign.push(),current_talign);
        }
        
        
        //now  merge single and double lines 
        objlist< narray <rectangle> > amcolumns;
        merge_single_line_paras(amcolumns,finalpara,finalalign);
       
        // get the final bounding boxes of the paragraphs
        rectangle temp;
        for(int i = 0; i<amcolumns.length();i++){
            get_bounding_box(temp,amcolumns[i]);
            paragraphs.push(temp);
        }
    }
}
