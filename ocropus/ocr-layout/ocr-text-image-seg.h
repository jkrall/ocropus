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
// File: ocr-text-image-seg.h
// Purpose: Wrapper class for document zone classification.
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrtextimageseg__
#define h_ocrtextimageseg__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ocropus.h"
#include "ocr-layout.h"

namespace ocropus {

    const int math_color     = 0x0001fa01;
    const int logo_color     = 0x0001fb01;
    const int text_color     = 0x00ff0101;
    const int table_color    = 0x0001fd01;
    const int drawing_color  = 0x0001fe01;
    const int halftone_color = 0x0001ff01;
    const int ruling_color   = 0x0001fc01;
    const int noise_color    = 0x00ffff00;

    struct TextImageSegByLogReg : ITextImageClassification {
        ~TextImageSegByLogReg() {}

        const char *description() {
            return "Get text/image probability map\n";
        }

        const char *name() {
            return "tiseglogreg";
        }

        void init(const char **argv) {
            // nothing to be done
        }

        // Get text-image map from a segmented image
        void textImageProbabilities(colib::intarray &out, colib::intarray &in);
        // Get text-image map from a binary image
        void textImageProbabilities(colib::intarray &out, colib::bytearray &in);

        void getProbabilityMap(colib::floatarray &class_prob,
                               colib::rectarray &boxes,
                               colib::bytearray &image);

        int getColor(colib::floatarray &prob_map, int index);

    };

    ITextImageClassification *make_TextImageSegByLogReg();

    struct RemoveImageRegions : virtual ICleanupBinary, ICleanupGray{
        ~RemoveImageRegions() {}

        const char *name() {
            return "removeimageregions";
        }

        const char *description() {
            return "Remove text or non-text zones\n";
        }

        void init(const char **argv) {
            // nothing to be done
        }

        void cleanup(colib::bytearray &out, colib::bytearray &in);

    };

    ICleanupBinary *make_RemoveImageRegionsBinary();
    ICleanupGray   *make_RemoveImageRegionsGray();
}

#endif
