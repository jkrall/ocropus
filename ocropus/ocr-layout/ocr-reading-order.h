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
// File: ocr-reading-order.h
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

#ifndef h_ocrreadingorder__
#define h_ocrreadingorder__

#include "ocropus.h"
#include "ocr-layout-internal.h"

namespace ocropus {

    //Extend textlines to the nearest whitespace gutter or image start/end
    void extend_lines(colib::narray<line> &lines,
                      colib::rectarray &wboxes,
                      int image_width);

    struct ReadingOrderByTopologicalSort {
        ReadingOrderByTopologicalSort();
        ~ReadingOrderByTopologicalSort() {}

        // Extract reading order using whitespace gutters as separators
        void sortTextlines(colib::narray<TextLine> &textlines,
                           colib::rectarray &gutters,
                           CharStats &charstats);

        // Extract reading order using whitespace gutters and
        // vertical/horizontal rulings as separators
        void sortTextlines(colib::narray<TextLine> &textlines,
                           colib::rectarray &gutters,
                           colib::rectarray &hor_rulings,
                           colib::rectarray &vert_rulings,
                           CharStats &charstats);
    private:
        int  id;
        colib::narray<int> val;
        colib::narray<int> ro_index;

        void visit(int k, colib::narray<bool> &lines_dag);
        void depthFirstSearch(colib::narray<bool> &lines_dag);
    };

    ReadingOrderByTopologicalSort *make_ReadingOrderByTopologicalSort();
}

#endif
