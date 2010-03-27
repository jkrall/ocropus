// -*- C++ -*-

// Copyright 2006-2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Project: roughocr -- mock OCR system exercising the interfaces and useful for testing
// File: ocr-layout-smear.cc
// Purpose: perform layout analysis by mophological "smearing"
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout.h"

using namespace ocropus;
using namespace iulib;
using namespace colib;

struct SegmentPageByMorphTrivial : ISegmentPage {
    ~SegmentPageByMorphTrivial() {}

    const char *description() {
        return "segment characters by horizontal projection (assumes single column)\n";
    }

    void init(const char **argv) {
        // nothing to be done
    }

    const char *name() {
        return "segmorphtriv";
    }

    void segment(intarray &image,bytearray &in) {
        param_int cwidth("cwidth",1,"cleaning width");
        param_int cheight("cheight",1,"cleaning height");
        param_int swidth("swidth",100,"smearing width");
        param_int sheight("sheight",1,"smearing height");

        optional_check_background_is_lighter(in);
        bytearray temp;
        copy(temp,in);
        invert(temp);
        copy(image,temp);
        binary_open_rect(temp,cwidth,cheight);
        binary_close_rect(temp,swidth,sheight);
        intarray labels;
        copy(labels,temp);
        label_components(labels,false);
        ASSERT(samedims(labels,image));
        for(int i=0;i<image.dim(0);i++) for(int j=0;j<image.dim(1);j++) {
            if(!labels(i,j)) ASSERT(!image(i,j));
            if(!image(i,j)) continue;
            ASSERT(labels(i,j));
            image(i,j) = pseg_pixel(1,labels(i,j));
            labels(i,j) = 0;
        }
        make_page_segmentation_white(image);
        check_page_segmentation(image);
    }
};

namespace ocropus {
    ISegmentPage *make_SegmentPageByMorphTrivial() {
        return new SegmentPageByMorphTrivial();
    }
}

