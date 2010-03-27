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
// File: ocr-pageseg-wcuts.cc
// Purpose: Page segmentation using whitespace cuts
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

namespace ocropus {
    using namespace iulib;
    using namespace colib;

    param_int  debug_tiseg_intermediate("debug",0,"print intermediate results to stdout");

    static void invertbg(intarray &image){
        simple_recolor(image);
        for(int i=0; i<image.length1d(); i++)
            if(image.at1d(i) == 0x00800000)
                image.at1d(i) = 0x00ffffff;
        return;
    }

    WhitespaceCuts::WhitespaceCuts(){
        max_aspect = 0.5;
        min_boxes  = 0;
    }

    void WhitespaceCuts::get_vborder(rectarray &columns,
                                     CharStats &charstats){
        int charlen  = charstats.char_boxes.length();
        int imwidth  = charstats.img_width;
        int imheight = charstats.img_height;

        int border_limit = 25;

        rectangle b = rectangle();
        for(int i=0; i<charlen; i++){
            b.include(charstats.char_boxes[i]);
        }

        if(b.x0 <= border_limit)
            b.x0 = border_limit;
        if(b.x1 >= imwidth-border_limit)
            b.x1 = imwidth-border_limit;

        columns.push(rectangle(1,1,b.x0-1,imheight-1));
        columns.push(rectangle(b.x1+1,1,imwidth-1,imheight-1));
    }

    void WhitespaceCuts::find_hspaces(rectarray &hspaces,
                                      rectarray &whitespace_boxes,
                                      CharStats &charstats){

        int   num_chars       = charstats.char_boxes.length();
        int   maxthreshold    = max_boxes; //Minimum number of text boxes on each side
        int   minthreshold    = min_boxes;

        int margin = (charstats.xheight)>>1;

        int top_neighbors=0, bottom_neighbors=0;
        int wlen = whitespace_boxes.length();
        for(int i=0; i<wlen; i++){
            if(whitespace_boxes[i].aspect() >= max_aspect)
                continue;

            if(whitespace_boxes[i].height() <= (0.4*charstats.line_spacing) )
                continue;

            rectangle top   = rectangle(whitespace_boxes[i]);
            top.x0   += margin;
            top.x1   -= margin;
            top.y0   -= margin;
            top.y1   -= margin;

            rectangle bottom  = rectangle(whitespace_boxes[i]);
            bottom.x0  += margin;
            bottom.x1  -= margin;
            bottom.y0  += margin;
            bottom.y1  += margin;

            //Check if first bounding box is the page dimensions
            int start = ( (charstats.char_boxes[0].x0==0) &&
                          (charstats.char_boxes[0].y0==0) ) ? 1 : 0;
            for(int j=start; j< num_chars; j++){
                if( top.overlaps(charstats.char_boxes[j]) ){  top_neighbors++;  }
                if( bottom.overlaps(charstats.char_boxes[j]) ){ bottom_neighbors++; }
            }


            if( (max(top_neighbors, bottom_neighbors) >= maxthreshold) &&
                (min(top_neighbors, bottom_neighbors) >= minthreshold) ){
                hspaces.push(whitespace_boxes[i]);
            }
        }
    }


    void WhitespaceCuts::filter_hspaces(rectarray &hspaces,
                                        CharStats &charstats){
        int len=hspaces.length();
        heap<rectangle> hcutsheap;
        float overlapt = 0.3;
        for(int i=0; i<len; i++){
            hcutsheap.insert(hspaces[i],hspaces[i].width());
        }

        rectarray hcutsfiltered;
        float hlen = hcutsheap.length();
        bool overlap = false;
        int cflen = 0;

        //Filtering out overlapping candidates
        for(int i=0; i<hlen; i++){
            rectangle temp = hcutsheap.extractMax();
            cflen = hcutsfiltered.length();
            overlap = false;
            for(int j=0; j<cflen; j++){
                if(temp.fraction_covered_by(hcutsfiltered[j]) > overlapt){
                    overlap = true;
                    break;
                }
            }
            if(!overlap)
                hcutsfiltered.push(temp);
        }

        cflen = hcutsfiltered.length();
        hspaces.clear();
        for(int i=0; i<cflen; i++){
            hspaces.push(hcutsfiltered[i]);
        }
    }

    // FIXME/faisal method naming --tmb

    void WhitespaceCuts::filter_hanging_hspaces(rectarray &hspaces,
                                                rectarray &columns,
                                                CharStats &charstats){
        int hlen     = hspaces.length();
        int clen     = columns.length();
        int imwidth  = charstats.img_width;

        rectarray hcutsfiltered;
        int counter;
        int xmin, xmax;

        for(int i=0; i<hlen; i++){
            counter = 0;
            xmin    = imwidth;
            xmax    = -1;
            for(int j=0; j<clen; j++){
                if(hspaces[i].overlaps(columns[j])){
                    counter ++;
                    rectangle temp = hspaces[i].intersection(columns[j]);
                    xmin = (xmin < temp.x0) ? xmin : temp.x0;
                    xmax = (xmax > temp.x1) ? xmax : temp.x1;
                }
            }
            if(counter >= 2){
                hspaces[i].x0 = xmin;
                hspaces[i].x1 = xmax;
                hcutsfiltered.push(hspaces[i]);
            }
        }

        int cflen = hcutsfiltered.length();
        hspaces.clear();
        for(int i=0; i<cflen; i++){
            hspaces.push(hcutsfiltered[i]);
        }
    }

    // FIXME/faisal method naming --tmb

    void WhitespaceCuts::get_whitespace_cuts(rectarray &wcuts,
                                             CharStats &charstats){
        wcuts.clear();
        rectarray hspaces,columns,colcandidates; // FIXME/faisal column_candidates --tmb

        // Remove very small and very big connected components
        rectarray concomps;     // FIXME/faisal pick better variable name --tmb
        for(int i=0, l=charstats.concomps.length(); i<l; i++){
            rectangle r = charstats.concomps[i];
            // FIXME/faisal maybe pull out the magic, tunable parameters and make them consts --tmb
            if(r.area() > 2*charstats.xheight &&
               r.area() < charstats.img_width * charstats.img_height/2)
                concomps.push(r);
        }

        // Compute Whitespace Cover
        rectangle r = rectangle(0,0,charstats.img_width-1,charstats.img_height-1);
        autodel<WhitespaceCover> whitespaces(make_WhitespaceCover(r));
        rectarray whitespaceboxes;
        whitespaces->setMinWeight(500);
        whitespaces->compute(whitespaceboxes,concomps);

        // FIXME/faisal code layout: no fancy spacing --tmb
        max_boxes = 4;
        //min_boxes = 2;
        min_space = 15;
        max_space = 30;
        width     = 1.5;
        findGutters(colcandidates,whitespaceboxes,charstats);
        filterOverlaps(columns,colcandidates);
        get_vborder(columns,charstats);

        find_hspaces(hspaces, whitespaceboxes, charstats);
        filter_hspaces(hspaces, charstats);
        filter_hanging_hspaces(hspaces, columns, charstats);

        int clen = columns.length();
        int hlen = hspaces.length();
        for(int i=0; i<clen; i++)
            wcuts.push(columns[i]);
        for(int i=0; i<hlen; i++)
            wcuts.push(hspaces[i]);

    }

    WhitespaceCuts *make_WhitespaceCuts(){
        return new WhitespaceCuts();
    }

    static void filter_zones(rectarray &zonesfiltered,
                             rectarray &zones){
        //Filter zones completely contained inside other zones.
        int zlen = zones.length();
        bool contained = false;
        float overlapt = 0.8;

        for(int i=0; i<zlen; i++){
            rectangle temp = zones[i];
            contained = false;
            for(int j=0; j<zlen; j++){
                if(i==j) continue;
                if(temp.fraction_covered_by(zones[j]) > overlapt){
                    contained = true;
                    break;
                }
            }
            if(!contained)
                zonesfiltered.push(temp);
        }
    }

    // FIXME/faisal
    // method naming
    // documentation--what does this do? "refine" how?
    // document the arguments
    // --tmb

    static void refine_zones(rectarray &zonesfiltered,
                             rectarray &page_blocks,
                             autodel<CharStats> &charstats){

        float t_snug  = 0.5;
        int   mingap  = 200;
        int   usegaps = 0;
        int slen = page_blocks.length();
        int clen = charstats->concomps.length();
        rectarray zones, members, gaps;
        narray<bool> used;
        used.resize(clen);
        fill(used,false);
        for(int i=0; i<slen; i++){
            rectangle temp = rectangle(page_blocks[i]);
            members.clear();
            gaps.clear();
            for(int j=0; j<clen; j++){
                if(used[j]) continue;
                if(charstats->concomps[j].fraction_covered_by(page_blocks[i]) >= t_snug){
                    temp.include(charstats->concomps[j]);
                    members.push(charstats->concomps[j]);
                    used[j] = true;
                }
            }
            if(members.length() == 1 || usegaps==0){
                zones.push(temp);
            }
            else if(members.length() >= 2){
                sort_boxes_by_x0(members);
                for(int j=1, mlen=members.length(); j<mlen; j++){
                    int gap = members[j].x0 - members[j-1].x1;
                    if(gap >= mingap)
                        gaps.push(rectangle(members[j-1].x1,temp.y0,members[j].x0,temp.y1));
                }
                if(gaps.length()){
                    int left = temp.x0;
                    for(int j=0; j<gaps.length(); j++){
                        zones.push(rectangle(left, temp.y0, gaps[j].x0, temp.y1));
                        left = gaps[j].x1;
                    }
                    zones.push(rectangle(left, temp.y0, temp.x1, temp.y1));
                }
                else
                    zones.push(temp);
            }

        }

        filter_zones(zonesfiltered, zones);
    }

    void SegmentPageByWCUTS::segment(intarray &image,bytearray &in_not_inverted) {

        bytearray in;
        copy(in, in_not_inverted);
        binarize_simple(in);
        optional_check_background_is_lighter(in);
        invert(in);

        // Do connected component analysis
        intarray charimage;
        copy(charimage,in);
        label_components(charimage,false);

        // Clean non-text and noisy boxes and get character statistics
        rectarray bboxes;
        bounding_boxes(bboxes,charimage);
        if(bboxes.length()<=1){
            makelike(image,in);
            fill(image,0x00ffffff);
            return ;
        }
        autodel<CharStats> charstats(make_CharStats());
        charstats->getCharBoxes(bboxes);
        charstats->calcCharStats();
        if(debug_tiseg_intermediate>=2){
            charstats->print();
        }

        // Find whitespace cuts
        autodel<WhitespaceCuts> gutters(make_WhitespaceCuts());
        rectarray wcuts;
        gutters->get_whitespace_cuts(wcuts,*charstats);
        if(debug_tiseg_intermediate){
            for(int i=0; i<wcuts.length();i++){
                wcuts[i].println();
            }
        }

        // Find page segments
        autodel<WhitespaceCover> page_seg(make_WhitespaceCover(0,0,in.dim(0),in.dim(1)));
        rectarray page_blocks;
        page_seg->setGreedy(true);
        page_seg->setMaxResults(100);
        page_seg->setMinWeight(5000);
        page_seg->setMinWidth(1);
        page_seg->setMinHeight(1);
        page_seg->compute(page_blocks,wcuts);
        //         if(debug_tiseg_intermediate){
        //             for(int i=0; i<page_blocks.length();i++){
        //                 page_blocks[i].println();
        //             }
        //         }

        rectarray zonesfiltered;
        refine_zones(zonesfiltered, page_blocks, charstats);


        int cflen = zonesfiltered.length();
        int clen  = charstats->concomps.length();
        float overlapt = 0.8;

        makelike(image,in);
        fill(image,0x00ffffff);
        for(int i=0; i<cflen; i++){
            // Do a logical OR of zone index with R=1 to assign all
            // zones to the first column
            int color = (i+1)|(0x00010000);
            rectangle r = rectangle();
            for(int j=0; j<clen; j++) {
                if(charstats->concomps[j].fraction_covered_by(zonesfiltered[i]) > overlapt)
                    r.include(charstats->concomps[j]);
            }

            for(int x=r.x0; x<r.x1; x++){
                for(int y=r.y0; y<r.y1; y++){
                    if(in_not_inverted(x,y) == 0)
                        image(x,y) = color ;
                }
            }
        }

        if(debug_tiseg_intermediate){
            intarray recolored_image;
            copy(recolored_image,image);
            invertbg(recolored_image);
            write_image_packed("whitespace-cuts.png",recolored_image);
        }
    }

    ISegmentPage *make_SegmentPageByWCUTS() {
        return new SegmentPageByWCUTS();
    }


}

