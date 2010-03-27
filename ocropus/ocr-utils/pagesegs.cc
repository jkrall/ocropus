// Copyright 2006 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
// or its licensors, as applicable.
// Copyright 1995-2005 by Thomas M. Breuel
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
// Project: iulib -- image understanding library
// File: 
// Purpose: 
// Responsible: 
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"

namespace ocropus {
    using namespace colib;
    using namespace iulib;

    // FIXME/mezhirov add comments --tmb

    void check_page_segmentation(intarray &pseg) {
        bool allow_zero = true;
        narray<bool> used(5000);
        fill(used,false);
        int nused = 0;
        int mused = 0;
        for(int i=0;i<pseg.length1d();i++) {
            unsigned pixel = (unsigned)pseg.at1d(i);
            CHECK_ARG(allow_zero || pixel!=0);
            if(pixel==0 || pixel==0xffffff) continue;
            int column = 0xff & (pixel >> 16);
            int paragraph = 0xff & (pixel >> 8);
            int line = 0xff & pixel;
            CHECK_ARG((column > 0 && column < 32) || column == 254 || column == 255);
            CHECK_ARG((paragraph >= 0 && paragraph < 64) || (paragraph >=250 && paragraph <= 255));
            if (column < 32) {
                if(!used(line)) nused++;
                used(line) = true;
                if(line>mused) mused = line;
            }
        }
        // character segments need to be numbered sequentially (no gaps)
        // (gaps usually happen when someone passes a binary image instead of a segmentation)
        CHECK_ARG(nused==mused || nused==mused+1);
    }

    // FIXME/mezhirov add comments --tmb

    void make_page_segmentation_black(intarray &a) {
        check_page_segmentation(a);
        replace_values(a, 0xFFFFFF, 0);
    }

    // FIXME/mezhirov add comments --tmb

    void make_page_segmentation_white(intarray &a) {
        replace_values(a, 0, 0xFFFFFF);
        check_page_segmentation(a);
    }


}
