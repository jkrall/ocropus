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
// File: ocr-cseg-projection.cc
// Purpose: segment a line of characters by thresholding the vertical projection profile
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"

using namespace ocropus;
using namespace iulib;
using namespace colib;

struct SegmentLineByProjection : ISegmentLine {
    ~SegmentLineByProjection() {}

    const char *name() {
        return "projseg";
    }

    const char *description() {
        return "segment characters by 1D projection\n";
    }

    void init(const char **argv) {
        // nothing to be done
    }

    void charseg(intarray &image,bytearray &in) {
        optional_check_background_is_lighter(in);
        param_int thigh("thigh",5,"projection threshold");
        param_int tlow("tlow",1,"projection threshold");

        copy(image,in);
        for(int i=0;i<image.length1d();i++)
            image.at1d(i) = !image.at1d(i);

        intarray projection(image.dim(0));
        for(int i=0;i<image.dim(0);i++) {
            projection(i) = 0;
            for(int j=0;j<image.dim(1);j++) {
                projection(i) += image(i,j);
            }
        }

        projection.reshape(projection.dim(0),1);
        floatarray temp;
        copy(temp,projection);
        hysteresis_thresholding(temp,tlow,thigh);
        copy(projection,temp);
        projection.reshape(projection.dim(0));

        int count = 0;
        for(int i=1;i<projection.length();i++) {
            if(projection(i-1)==0 && projection(i)==1)
                count++;
            if(projection(i))
                projection(i) = count;
        }
        for(int i=0;i<image.dim(0);i++) {
            for(int j=0;j<image.dim(1);j++) {
                if(!projection(i)) {
                    image(i,j) = 0;
                } else if(image(i,j)) {
                    image(i,j) = cseg_pixel(projection(i));
                }
            }
        }
        make_line_segmentation_white(image);
        check_line_segmentation(image);
    }
};

namespace ocropus {
    ISegmentLine *make_SegmentLineByProjection() {
        return new SegmentLineByProjection();
    }
}


