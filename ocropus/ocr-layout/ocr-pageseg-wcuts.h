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
// File: ocr-pageseg-wcuts.h
// Purpose: Page segmentation using whitespace cuts
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrpagesegwcuts__
#define h_ocrpagesegwcuts__

#include "ocropus.h"
#include "ocr-layout.h"
#include "ocropus.h"

namespace ocropus {

    struct WhitespaceCuts : public ColSeparators{
        float max_aspect;

        WhitespaceCuts();

        void get_vborder(colib::rectarray &columns,
                         CharStats &charstats);

        void find_hspaces(colib::rectarray &hspaces,
                          colib::rectarray &whitespaceboxes,
                          CharStats &charstats);
        void filter_hspaces(colib::rectarray &hspaces,
                            CharStats &charstats);
        void filter_hanging_hspaces(colib::rectarray &hspaces,
                                    colib::rectarray &columns,
                                    CharStats &charstats);
        void get_whitespace_cuts(colib::rectarray &wcuts,
                                 CharStats &charstats);
    };

    WhitespaceCuts *make_WhitespaceCuts();

    struct SegmentPageByWCUTS : ISegmentPage {
        ~SegmentPageByWCUTS() {}

        const char *name() {
            return "segwcuts";
        }
        const char *description() {
            return "segment characters by RAST\n";
        }

        void init(const char **argv) {
            // nothing to be done
        }

        void segment(colib::intarray &image,colib::bytearray &in_not_inverted);
    };

    ISegmentPage *make_SegmentPageByWCUTS();

}

#endif
