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
// File: ocr-cseg-ccs.cc
// Purpose: attempt to segment a line (or page) of text into characters by labeling the connected components
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "iulib/imgio.h"
#include "iulib/imglib.h"
#include "ocropus.h"

using namespace ocropus;
using namespace iulib;
using namespace colib;

struct SegmentLineByCCS : ISegmentLine {
    ~SegmentLineByCCS() {}

    const char *description() {
        return "segment characters by 1D projection\n";
    }

    const char *name() {
        return "segccs";
    }

    void init(const char **argv) {
        // nothing to be done
    }

    void charseg(intarray &image,bytearray &in) {
        param_int swidth("swidth",0,"smearing width");
        param_int sheight("sheight",10,"smearing height");
        bytearray temp;
        copy(image,in);
        copy(temp,image);
        binary_close_rect(temp,swidth,sheight);
        intarray labels;
        copy(labels,temp);
        label_components(labels);
        for(int i=0;i<image.length1d();i++)
            if(image.at1d(i))
                image.at1d(i) = cseg_pixel(labels.at1d(i));
        make_line_segmentation_white(image);
        check_line_segmentation(image);
    }
};

namespace ocropus {
    ISegmentLine *make_SegmentLineByCCS() {
        return new SegmentLineByCCS();
    }
}

