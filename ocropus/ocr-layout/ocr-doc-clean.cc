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
// File: ocr-doc-clean.cc
// Purpose: Wrapper class for docuemnt image cleanup.
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace iulib;
using namespace colib;

namespace ocropus {

    param_string debug_cleanup("debug_cleanup",0,"output cleaned document image as png");

    struct DocClean : ICleanupBinary {
        ~DocClean() {}

        const char *description() {
            return "Running black filter,white filter on the image  thus removing noise \n";
        }

        void init(const char **argv) {
            // nothing to be done
        }

        const char *name() {
            return "docclean";
        }

        void cleanup(bytearray &out,bytearray &in) {

            autodel<NoiseFilter> noisefilter(make_NoiseFilter());

            //Check if it is the complete image
            if(is_line_image(in)){
                copy(out,in);
                return ;
            }
            //run black filter first
            bytearray result_bf,result_cc,result_bf_inverted;
            makelike(result_cc,in);
            fill(result_cc,0xff);
            noisefilter->blackFilter(result_bf,in);

            // Do connected component analysis
            intarray charimage;
            copy(result_bf_inverted,result_bf);
            make_page_binary_and_black(result_bf_inverted);
            copy(charimage,result_bf_inverted);
            label_components(charimage,false);

            // Clean noisy boxes
            rectarray bboxes;
            bounding_boxes(bboxes,charimage);
            noisefilter->ccanalysis(result_cc,result_bf,bboxes);

            //run whitefilter on output of connected component analysis result
            noisefilter->whiteFilter(out,result_cc);
            if(debug_cleanup) {
                write_png(stdio(debug_cleanup,"w"), out);
            }
        }
    };

    ICleanupBinary *make_DocClean() {
        return new DocClean();
    }
}

