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
    // A valid line segmentation may contain 0 or 0xffffff as the
    // background, and otherwise numbers components starting at 1.
    // The segmentation consists of segmented background pixels
    // (0x80xxxx) and segmented foreground pixels (0x00xxxx).  The
    // segmented foreground pixels should constitute a usable
    // binarization of the original image.

    void check_line_segmentation(intarray &cseg) {
        if(cseg.length1d()==0) return;
        CHECK_ARG(cseg.rank()==2);
        for(int i=0;i<cseg.length1d();i++) {
            int value = cseg.at1d(i);
            if(value==0) continue;
            if(value==0xffffff) continue;
            if(value&0x800000)
                CHECK_ARG((value&~0x800000)<100000);
            else
                CHECK_ARG(value<100000);
        }
    }

    // FIXME/mezhirov add comments --tmb

    void make_line_segmentation_black(intarray &a) {
        check_line_segmentation(a);
        replace_values(a, 0xFFFFFF, 0);
        for(int i = 0; i < a.length1d(); i++)
            a.at1d(i) &= 0xFFF;
    }

    // FIXME/mezhirov add comments --tmb

    void make_line_segmentation_white(intarray &a) {
        replace_values(a, 0, 0xFFFFFF);
        //for(int i = 0; i < a.length1d(); i++)
        //    a.at1d(i) = (a.at1d(i) & 0xFFF) | 0x1000;
        check_line_segmentation(a);
    }

}
