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
// File: ocr-reading-order.cc
// Purpose: Topological sorting reading order algrithm for arranging
//          a group of textlines into reading order.
//          For more information, please refer to the paper:
//          T. M. Breuel. "High Performance Document Layout Analysis",
//          Symposium on Document Image Understanding Technology, Maryland.
//          http://pubs.iupr.org/DATA/2003-breuel-sdiut.pdf
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace colib;

namespace ocropus {

    //Assuming horizontal lines with slope in the interval [-0.05, 0.05]
    static int wbox_intersection(line l, rectangle wbox){

        float y = l.m * wbox.xcenter() + l.c;
        return ( (y > wbox.y0) && (y < wbox.y1) );
    }

    //Extend textlines to the nearest whitespace gutter or image start/end
    void extend_lines(narray<line> &lines,
                      rectarray &wboxes,
                      int image_width){

        int num_lines = lines.length();
        int num_wboxes = wboxes.length();

        // NOTE: maybe extend to image start/width if not intersecting with column separator
        // if we observe problems in benchmarks
        for(int i = 0; i<num_lines; i++){
            float new_start = 0;
            float new_end = image_width-1;
            bool start_updated = false;
            bool end_updated = false;
            for(int j = 0; j<num_wboxes; j++){
                if(wbox_intersection(lines[i], wboxes[j])){
                    if(wboxes[j].x0 <= lines[i].start) {
                        new_start = (new_start > wboxes[j].x1) ? new_start : wboxes[j].x1;
                        start_updated = true;
                    }
                    else {
                        new_end = (new_end   < wboxes[j].x0) ? new_end   : wboxes[j].x0;
                        end_updated = true;
                    }
                }
            }
            lines[i].start = (lines[i].start > new_start) ? new_start : lines[i].start;
            lines[i].end = (lines[i].end   < new_end  ) ? new_end   : lines[i].end  ;
            if (start_updated)
                lines[i].start = new_start;
            if (end_updated)
                lines[i].end = new_end;
            //printf("%.0f %.0f %.0f %.0f \n",lines[i].start,lines[i].top,lines[i].end,lines[i].bottom);
        }
    }

    static inline bool x_overlap(line a, line b){
        return ( (a.end >= b.start) && (b.end >= a.start) );
    }

    static bool separator_segment_found(line a, line b, narray<line> &lines){
        int lines_length = lines.length();
        float y_min = (a.c < b.c) ? a.c : b.c;
        float y_max = (a.c > b.c) ? a.c : b.c;

        for(int i = 0; i<lines_length; i++)
            if( x_overlap(lines[i],a) && x_overlap(lines[i],b) )
                if( (lines[i].c > y_min) && (lines[i].c < y_max) )
                    return true;

        return false;

    }

    static void construct_graph(narray<line> &lines, narray<bool> &lines_dag){
        //lines_dag(i,j) = 1 iff there is a directed edge from i to j
        int graph_length = lines.length();

        for(int i = 0; i<graph_length; i++){
            for(int j = i; j<graph_length; j++){

                if(i == j){ lines_dag(i,j) = 1; continue; }

                if( x_overlap(lines[i],lines[j]) ){
                    //assuming parallel horizontal lines and page origin and bottom left corner
                    if(lines[i].top > lines[j].top) { lines_dag(i,j) = 1; }
                    else { lines_dag(j,i) = 1; }
                }

                else{
                    if( separator_segment_found(lines[i],lines[j],lines) )        continue;
                    else if(lines[i].end <= lines[j].start)  { lines_dag(i,j) = 1; }
                    else  { lines_dag(j,i) = 1; }
                }
            }
        }
    }

    ReadingOrderByTopologicalSort::ReadingOrderByTopologicalSort(){
        id = 0;
    }

    void ReadingOrderByTopologicalSort::visit(int k, narray<bool> &lines_dag){
        int size = lines_dag.dim(0);
        val(k) = ++id;
        for (int i = 0; i< size; i++){
            if(lines_dag(k,i) != 0)
                if(val(i) == 0)
                    visit(i, lines_dag);
        }
        ro_index.push(k);
    }

    void ReadingOrderByTopologicalSort::depthFirstSearch(narray<bool> &lines_dag){
        int size = lines_dag.dim(0);
        val.resize(size);
        fill(val,false);
        for (int k = 0; k< size; k++)
            if (val(k) == 0)
                visit(k, lines_dag);
    }

    void ReadingOrderByTopologicalSort::sortTextlines(narray<TextLine> &textlines,
                                                      rectarray &gutters,
                                                      CharStats &charstats){
        id = 0;
        val.clear();
        ro_index.clear();
        narray<line> lines;
        for(int i = 0; i<textlines.length(); i++)
            lines.push(line(textlines[i]));

        if(lines.length() <= 1){
            return;
        }
        if(gutters.length()){
            sort_boxes_by_x0(gutters);
            extend_lines(lines, gutters, charstats.img_width);
        }

        // Determine reading order
        narray<bool> lines_dag; // Directed acyclic graph of lines
        lines_dag.resize( lines.length(), lines.length() );
        fill(lines_dag,false);

        construct_graph(lines, lines_dag);
        depthFirstSearch(lines_dag);
        int size = ro_index.length();
        textlines.clear();
        for(int i = 1; i <= size; i++){
            textlines.push(lines[ro_index[size-i]].getTextLine());
        }
    }

    void ReadingOrderByTopologicalSort::sortTextlines(narray<TextLine> &textlines,
                                                      rectarray &gutters,
                                                      rectarray &hor_rulings,
                                                      rectarray &vert_rulings,
                                                      CharStats &charstats){
        id = 0;
        val.clear();
        ro_index.clear();
        narray<line> lines;
        for(int i = 0; i<textlines.length(); i++)
            lines.push(line(textlines[i]));

        if(lines.length() <= 1){
            return;
        }

        //Since vertical rulings has the same role as whitespace gutters, just
        //add them to vertical separators list
        rectarray vert_separators;
        for(int i=0,l=vert_rulings.length(); i<l; i++){
            vert_separators.push(vert_rulings[i]);
        }
        for(int i=0,l=gutters.length(); i<l; i++){
            vert_separators.push(gutters[i]);
        }

        if(vert_separators.length()){
            sort_boxes_by_x0(vert_separators);
            extend_lines(lines, vert_separators, charstats.img_width);
        }

        // Make dummy text-lines from horizontal rulings to use them as
        // separating elements for reading order.
        for(int i=0,l=hor_rulings.length(); i<l; i++){
            line l;
            rectangle r = hor_rulings[i];
            l.c = r.ycenter();
            l.m = 0;
            l.start  = r.x0;
            l.end    = r.x1;
            l.top    = r.y0;
            l.bottom = r.y1;
            l.istart = r.x0;
            l.iend   = r.x1;

            // Set these values to -1 to identify dummy lines later
            l.d = -1;
            l.xheight = -1;

            lines.push(l);
        }

        // Determine reading order
        narray<bool> lines_dag; // Directed acyclic graph of lines
        lines_dag.resize( lines.length(), lines.length() );
        fill(lines_dag,false);

        construct_graph(lines, lines_dag);
        depthFirstSearch(lines_dag);
        int size = ro_index.length();
        textlines.clear();
        for(int i = 1; i <= size; i++){
            TextLine tl=lines[ro_index[size-i]].getTextLine();
            if(tl.xheight>0) // Remove dummy lines
                textlines.push(tl);
        }
    }

    ReadingOrderByTopologicalSort *make_ReadingOrderByTopologicalSort(){
        return new ReadingOrderByTopologicalSort();
    }

}
