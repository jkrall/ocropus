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
// File: ocr-deskew-rast.h
// Purpose: perform skew correction using RAST
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocr_deskew_rast_
#define h_ocr_deskew_rast_

#include "ocropus.h"
#include "ocr-layout.h"

namespace ocropus {

    using namespace colib;

    struct DeskewPageByRAST : ICleanupBinary, ICleanupGray {
        ~DeskewPageByRAST() {
        }

        const char *name() {
            return "deskewrast";
        }

        const char *description() {
            return "Deskew page image by RAST\n";
        }

        void init(const char **argv) {
            // nothing to be done
        }

        double getSkewAngle(bytearray &in);
        double getSkewAngle(rectarray &bboxes);
        void cleanup(bytearray &image, bytearray &in);
    };

}
#endif

