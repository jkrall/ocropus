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
// File: ocr-extract-gutters.cc
// Purpose: Filter whitespace cover rectangles to extract gutters or column separators
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace ocropus;
using namespace colib;

namespace ocropus {

ColSeparators::ColSeparators(){
    min_aspect = 3.0;
    min_space  = 10;
    max_space  = 25;
    width      = 1.0;
    max_boxes  = 6;
    min_boxes  = 3;
}
    
// FIXME/faisal
// why does a method called "findGutters" return an array of columns?
// explain how this works
// make terminology consistent: does this find column separators, gutters, or what?
// name: don't abbreviate Column
// document what the arguments represent, or pick much more descriptive names
// --tmb

void ColSeparators::findGutters(rectarray &columns,
                                rectarray &whitespace_boxes,
                                CharStats &charstats) {
    // FIXME/faisal remove fancy spacing and alignment --tmb
    int   num_chars       = charstats.char_boxes.length();
    int   maxthreshold    = max_boxes; //Minimum number of text boxes on each side
    int   minthreshold    = min_boxes;
    int   word_spacing    = charstats.word_spacing;
        
    // FIXME/faisal
    // variable name is non-descriptive and doesn't conform to our conventions
    // why is there a >>1? do you mean /2?
    // better: int ... = int(neighbor_frac_for_... * charstats.xheight);
    //      and make neighbor_frac_for_... a constant with documentation
    // use US spelling consistently throughout
    // --tmb
    
    int neighbourT = (charstats.xheight)>>1;
        
    word_spacing = min(word_spacing, max_space);
    word_spacing = max(word_spacing, min_space);
        
    int left_neighbors, right_neighbors;
    int wlen = whitespace_boxes.length();
    for(int i=0; i<wlen; i++){
        if(whitespace_boxes[i].aspect() <= min_aspect)
            continue;
            
        if(whitespace_boxes[i].width() <= (width*word_spacing) )
            continue;
            
        left_neighbors  = 0;
        right_neighbors = 0;
            
        // FIXME/faisal you don't need the extra constructor; rectangle is a value type --tmb
        rectangle left   = rectangle(whitespace_boxes[i]);
        // FIXME/faisal
        // remove fancy spacing
        // what does this do?
        // --tmb
        left.x0   -= neighbourT;
        left.x1   -= neighbourT;
        left.y0   += neighbourT;
        left.y1   -= neighbourT;
            
            
        rectangle right  = rectangle(whitespace_boxes[i]);
        right.x1  += neighbourT;
        right.x0  += neighbourT;
        right.y0  += neighbourT;
        right.y1  -= neighbourT;
            
        //Check if first bounding box is the page dimensions
        int start = ( (charstats.char_boxes[0].x0==0) && (charstats.char_boxes[0].y0==0) ) ? 1 : 0;
        for(int j=start; j< num_chars; j++){
            if( left.overlaps(charstats.char_boxes[j]) ){  left_neighbors++;  }
            if( right.overlaps(charstats.char_boxes[j]) ){ right_neighbors++; }
        }
            
        if( (max(left_neighbors, right_neighbors) >= maxthreshold) &&
            (min(left_neighbors, right_neighbors) >= minthreshold) ){
            columns.push(whitespace_boxes[i]);
        }
    }
}
    
// FIXME/faisal explain what this does and what it's used for --tmb

void ColSeparators::filterOverlaps(rectarray &colboxes, rectarray &colcandidates){

    int len=colcandidates.length();
    heap<rectangle>   colheap;
    float        overlapt = 0.3; // FIXME/faisal no fancy spacing in variable declarations
    rectarray colfiltered;
    bool         overlap = false;
    int          cflen = 0;
        
    for(int i=0; i<len; i++){
        colheap.insert(colcandidates[i],colcandidates[i].height());
    }
    float        hlen = colheap.length(); // FIXME/faisal spacing

    //Filtering out overlapping candidates
    for(int i=0; i<hlen; i++){
        rectangle temp = colheap.extractMax();
        cflen = colfiltered.length();
        overlap = false;
        for(int j=0; j<cflen; j++){
            if(temp.fraction_covered_by(colfiltered[j]) > overlapt){
                overlap = true;
                //printf("Candidate # %d with weight %d failed.\n",turn++,temp.area()*temp.height());
                break;
            }
        }
        if(!overlap){
            colfiltered.push(temp);
        }
    }
        
    cflen = colfiltered.length();
    colboxes.clear();
    for(int i=0; i<cflen; i++){
        colboxes.push(colfiltered[i]);
    }    
}
    
ColSeparators *make_ColSeparators(){
    return new ColSeparators();
}
} // end namespace
