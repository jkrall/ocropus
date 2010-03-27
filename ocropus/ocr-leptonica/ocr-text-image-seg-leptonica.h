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
// File: ocr-text-image-seg-leptonica.h
// Purpose: Wrapper class for getting non-text mask from leptonica
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrtextimageseglept__
#define h_ocrtextimageseglept__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ocropus.h"

namespace ocropus {
    using namespace colib;

    struct TextImageSegByLeptonica : ITextImageClassification {
        ~TextImageSegByLeptonica() {}

        const char *name() {
            return "leptti";
        }
        const char *description() {
            return "Get text/image probability map\n";
        }

        void init(const char **argv) {
            // nothing to be done
        }

        // FIXME/faisal this should not be exposed in the header file --tmb
        // Get non-text mask from leptonica
        void getLeptNonTextMask(bytearray &out, bytearray &in);

        // Get text-image map from a binary image
        void textImageProbabilities(intarray &out, bytearray &in);

    };

    ITextImageClassification *make_TextImageSegByLeptonica();

}

#endif
