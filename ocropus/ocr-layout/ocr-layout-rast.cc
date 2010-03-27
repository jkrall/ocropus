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
// File: ocr-layout-rast.cc
// Purpose: perform layout analysis by RAST
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include <time.h>
#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace iulib;
using namespace colib;

namespace ocropus {

    param_string debug_segm("debug_segm",0,"output segmentation file");
    param_int  debug_layout("debug_layout",0,"print the intermediate results to stdout");

    SegmentPageByRAST::SegmentPageByRAST(){
        max_results = 1000;
        gap_factor = 10;
        use_four_line_model = false;
    }

    // FIXME/faisal refactor this
    // make the arguments instance variables
    // make separate private methods out of the individual processing steps
    // give descriptive names to each such private method
    // this documents the individual steps and makes the overall algorithm easier to follow
    // --tmb

    void SegmentPageByRAST::segmentInternal(intarray &visualization,
                                            intarray &image,
                                            bytearray &in_not_inverted,
                                            bool need_visualization,
                                            rectarray &extra_obstacles) {

        // FIXME/faisal remove this dead code --tmb
        //float startTime = clock()/float(CLOCKS_PER_SEC);
        const int zero   = 0;
        const int yellow = 0x00ffff00;
        bytearray in;
        copy(in, in_not_inverted);
        make_page_binary_and_black(in);
        //fprintf(stderr,"Time elapsed (autoinvert): %.3f \n",(clock()/float(CLOCKS_PER_SEC)) - startTime);

        // Do connected component analysis
        intarray charimage;
        copy(charimage,in);
        label_components(charimage,false);
        //fprintf(stderr,"Time elapsed (label_components): %.3f \n",(clock()/float(CLOCKS_PER_SEC)) - startTime);

        // Clean non-text and noisy boxes and get character statistics
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
        if(debug_layout>=2){
            charstats->print();
        }
        //fprintf(stderr,"Time elapsed (charstats): %.3f \n",(clock()/float(CLOCKS_PER_SEC)) - startTime);

        // Compute Whitespace Cover
        autodel<WhitespaceCover> whitespaces(make_WhitespaceCover(0,0,in.dim(0),in.dim(1)));
        rectarray whitespaceboxes;
        whitespaces->compute(whitespaceboxes,charstats->char_boxes);
        //fprintf(stderr,"Time elapsed (whitespaces): %.3f \n",(clock()/float(CLOCKS_PER_SEC)) - startTime);

        // Find whitespace column separators (gutters)
        autodel<ColSeparators> whitespace_obstacles(make_ColSeparators());
        rectarray gutters,gutter_candidates;
        whitespace_obstacles->findGutters(gutter_candidates,whitespaceboxes,*charstats);
        whitespace_obstacles->filterOverlaps(gutters,gutter_candidates);
        if(debug_layout){
            for(int i=0; i<gutters.length();i++){
                gutters[i].println(stdout);
            }
        }

        // Separate horizontal/vertical rulings from graphics
        rectarray hor_rulings;
        rectarray vert_rulings;
        rectarray graphics;
        autodel<ExtractRulings> rulings(make_ExtractRulings());
        rulings->analyzeObstacles(hor_rulings,vert_rulings,graphics,
                                  extra_obstacles,charstats->xheight);
        rulings->analyzeObstacles(hor_rulings,vert_rulings,graphics,
                                  charstats->large_boxes,charstats->xheight);

        // add whitespace gutters and the user-supplied obstacles to a list of
        // obstacles
        rectarray textline_obstacles;
        for(int i=0;i<gutters.length();i++)
            textline_obstacles.push(gutters[i]);
        for(int i=0;i<extra_obstacles.length();i++)
            textline_obstacles.push(extra_obstacles[i]);
        for(int i=0;i<vert_rulings.length();i++)
            textline_obstacles.push(vert_rulings[i]);
        //fprintf(stderr,"Time elapsed (gutters): %.3f \n",(clock()/float(CLOCKS_PER_SEC)) - startTime);

        // Extract textlines
        narray<TextLine> textlines;

        if(use_four_line_model){
            narray<TextLineExtended> textlines_extended;
            autodel<CTextlineRASTExtended> ctextline(make_CTextlineRASTExtended());
            ctextline->min_q     = 2.0; // Minimum acceptable quality of a textline
            ctextline->min_count = 2;   // ---- number of characters in a textline
            ctextline->min_length= 30;  // ---- length in pixels of a textline

            ctextline->max_results = max_results;
            ctextline->min_gap = gap_factor*charstats->xheight;

            ctextline->extract(textlines_extended,textline_obstacles,charstats);
            for(int i=0,l=textlines_extended.length();i<l;i++)
                textlines.push(textlines_extended[i].getTextLine());
        }else{
            autodel<CTextlineRAST> ctextline(make_CTextlineRAST());
            ctextline->min_q     = 2.0; // Minimum acceptable quality of a textline
            ctextline->min_count = 2;   // ---- number of characters in a textline
            ctextline->min_length= 30;  // ---- length in pixels of a textline

            ctextline->max_results = max_results;
            ctextline->min_gap = gap_factor*charstats->xheight;

            ctextline->extract(textlines,textline_obstacles,charstats);
        }


        // Sort textlines in reading order
        autodel<ReadingOrderByTopologicalSort>
            reading_order(make_ReadingOrderByTopologicalSort());
        reading_order->sortTextlines(textlines,gutters,hor_rulings,vert_rulings,*charstats);
        //fprintf(stderr,"Time elapsed (ctextline): %.3f \n",(clock()/float(CLOCKS_PER_SEC)) - startTime);

        rectarray textcolumns;
        rectarray paragraphs;
        rectarray textline_boxes;
        for(int i=0, l=textlines.length(); i<l; i++)
            textline_boxes.push(textlines[i].bbox);

        // Group textlines into text columns
        //Since vertical rulings has the same role as whitespace gutters, just
        //add them to vertical separators list
        rectarray vert_separators;
        for(int i=0,l=vert_rulings.length(); i<l; i++){
            vert_separators.push(vert_rulings[i]);
        }
        for(int i=0,l=gutters.length(); i<l; i++){
            vert_separators.push(gutters[i]);
        }

        get_text_columns(textcolumns,textline_boxes,vert_separators);

        // Color encode layout analysis output
        autodel<ColorEncodeLayout> color_encoding(make_ColorEncodeLayout());
        copy(color_encoding->inputImage,in);
        for(int i=0, l=textlines.length(); i<l; i++)
            color_encoding->textlines.push(textlines[i].bbox);
        for(int i=0, l=textcolumns.length(); i<l; i++)
            color_encoding->textcolumns.push(textcolumns[i]);
        for(int i=0, l=gutters.length(); i<l; i++)
            color_encoding->gutters.push(gutters[i]);
        for(int i=0, l=vert_rulings.length(); i<l; i++)
            color_encoding->rulings.push(vert_rulings[i]);
        for(int i=0, l=hor_rulings.length(); i<l; i++)
            color_encoding->rulings.push(hor_rulings[i]);
        for(int i=0, l=graphics.length(); i<l; i++)
            color_encoding->graphics.push(graphics[i]);

        color_encoding->encode();
        copy(image,color_encoding->outputImage);

        //fprintf(stderr,"Time elapsed (find-columns): %.3f \n",(clock()/float(CLOCKS_PER_SEC)) - startTime);
        if(debug_layout){
            for(int i=0; i<textlines.length();i++)
                textlines[i].print();
            for (int i=0; i<textcolumns.length();i++)
                textcolumns[i].println();
        }
        replace_values(image,zero,yellow);
        if(need_visualization) {
            visualize_layout(visualization, in_not_inverted, textlines,
                             vert_separators, extra_obstacles, *charstats);
        }
    }

    void SegmentPageByRAST::segment(intarray &result,
                                    bytearray &in_not_inverted,
                                    rectarray &obstacles) {
        intarray debug_image;
        if(debug_segm) {
            segmentInternal(debug_image, result, in_not_inverted, true, obstacles);
            write_image_packed(debug_segm,debug_image);
        } else {
            segmentInternal(debug_image, result, in_not_inverted, false, obstacles);
        }
    }

    void SegmentPageByRAST::segment(intarray &result, bytearray &in_not_inverted) {
        rectarray obstacles;
        segment(result,in_not_inverted,obstacles);
    }

    void SegmentPageByRAST::visualize(intarray &result,
                                      bytearray &in_not_inverted,
                                      rectarray &obstacles) {
        intarray segmentation;
        segmentInternal(result, segmentation, in_not_inverted, true,obstacles);
    }

    ISegmentPage *make_SegmentPageByRAST() {
        return new SegmentPageByRAST();
    }

}

