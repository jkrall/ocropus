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
// File: ocr-char-stats.h
// Purpose: Header file declaring data structures for computing document
//          statistics
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_nucharstatsx__
#define h_nucharstatsx__

#include "ocropus.h"
#include "ocr-layout.h"

namespace ocropus {
    using namespace colib;

    void sort_boxes_by_x0(rectarray &boxes);
    void sort_boxes_by_y0(rectarray &boxes);
    int  calc_xheight(rectarray &bboxes);

    //////////////////////////////////////////////////////////////////////////
    ///
    /// \struct CharStats
    /// Purpose: Character bounding boxes and statistics extracted from them
    ///
    //////////////////////////////////////////////////////////////////////////

    struct CharStats {
        int    img_height;
        int    img_width;
        int    xheight;
        int    char_spacing;
        int    word_spacing;
        int    line_spacing;
        rectarray concomps;
        rectarray char_boxes;
        rectarray dot_boxes;
        rectarray large_boxes;

        CharStats();
        CharStats(CharStats &c);
        ~CharStats();
        void print();
        void getCharBoxes(rectarray &concomps);
        void calcCharStats();
        void calcCharStats(rectarray &cboxes);
        void calcCharStatsForOneLine();
        void calcCharStatsForOneLine(rectarray &cboxes);
    };
    CharStats *make_CharStats();
    CharStats *make_CharStats(CharStats &c);

}

#endif
