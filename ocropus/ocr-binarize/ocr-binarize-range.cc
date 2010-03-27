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
// File: ocr-binarize-range.cc
// Purpose: simple document binarization based on thresholding the min/max range
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "ocropus.h"

using namespace colib;

namespace ocropus {

    void binarize_by_range(bytearray &out,floatarray &in,float fraction) {
        float imin = min(in);
        float imax = max(in);
        float thresh = int(imin + (imax-imin)*fraction);
        makelike(out,in);
        for(int i=0;i<in.length1d();i++) {
            if(in.at1d(i)>thresh) out.at1d(i)=255;
            else out.at1d(i) = 0;
        }
    }

    void binarize_by_range(bytearray &image,float fraction) {
        float imin = min(image);
        float imax = max(image);
        float thresh = int(imin + (imax-imin)*fraction);
        for(int i=0;i<image.length1d();i++) {
            if(image.at1d(i)>thresh) image.at1d(i)=255;
            else image.at1d(i) = 0;
        }
    }

    struct BinarizeByRange : IBinarize {
        float fraction;

        BinarizeByRange() {
            fraction = 0.5;
        }

        ~BinarizeByRange() {}

        const char *name() {
            return "binarizerange";
        }

        const char *description() {
            return "binarize by thresholding the range between min(image) and max(image)\n";
        }

        void init(const char **argv) {
            // nothing to be done
        }

        void binarize(bytearray &out,floatarray &in) {
            binarize_by_range(out,in,fraction);
        }

    };

    IBinarize *make_BinarizeByRange() {
        return new BinarizeByRange();
    }
}

