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
// This version of the xycut algorithm has been implemented according to
// the algorithm described in "Empirical Performance Evaluation Methodology and
// Its Application to Page Segmentation Algorithms" by Mao and Kanungo(Figure 3)
//
// Project: OCRopus
// File: ocr-pageseg-xycuts.h
// Purpose: Page segmentation using XYCuts
// Responsible: Joost van Beusekom (joost@iupr.net)
// Reviewer: Faisal Shafait (faisal.shafait@dfki.de)
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrpagesegxycuts__
#define h_ocrpagesegxycuts__

#include "ocropus.h"
#include "ocr-layout.h"

namespace ocropus {

    enum {HORIZONTAL_CUT, VERTICAL_CUT};

    struct SegmentPageByXYCUTS : ISegmentPage {
    private:
        unsigned int tnx; // noise threshold on projection on x-axis
        unsigned int tny; // noise threshold on projection on y-axis
        unsigned int tcx; // min gap size on x-axis projection
        unsigned int tcy; // min gap size on y-axis projection
    public:
        SegmentPageByXYCUTS();
        SegmentPageByXYCUTS(unsigned int itnx,
                            unsigned int itny,
                            unsigned int itcx,
                            unsigned int itcy);

        ~SegmentPageByXYCUTS() {}

        const char *name() {
            return "segxy";
        }

        const char *description() {
            return "segment page by XY-Cut algorithm\n"
                "Default parameters: \n"
                "\ttnx=78, tny=32, tcx=35, tcy=54\n"
                "\ttnx,tny: cleaning trhesholds\n"
                "\ttcx,tcy = min gap size hor. and ver.\n" ;
        }

        void setParameters(unsigned int itnx, unsigned int itny, unsigned int itcx, unsigned int itcy);

        void segment(colib::intarray &image,colib::bytearray &in);
    };

    ISegmentPage *make_SegmentPageByXYCUTS(unsigned int itnx,
                                           unsigned int itny,
                                           unsigned int itcx,
                                           unsigned int itcy);

}

#endif
