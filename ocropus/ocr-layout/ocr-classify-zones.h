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
// File: ocr-classify-zones.h
// Purpose: Document zone classification using run-lengths and
//          connected components based features and logistic regression
//          classifier as described in:
//          D. Keysers, F. Shafait, T.M. Breuel. "Document Image Zone Classification -
//          A Simple High-Performance Approach",  VISAPP 2007, pages 44-51.
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrclassifyzones__
#define h_ocrclassifyzones__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "ocropus.h"
#include "ocr-layout.h"

namespace ocropus {

    const int MAX_LEN = 128;          // MAX_LEN = dimension of the histogram
    const int COMP_START = 0;
    const int COMP_LEN_START = 1;
    const int COMP_LEN_INC_A = 2;
    const int COMP_LEN_INC_B = 0;

    enum zone_class {math=0, logo=1, text=2, table=3, drawing=4,
                     halftone=5, ruling=6, noise=7, undefined=-1};

    struct ZoneFeatures{

        void extractFeatures(colib::floatarray &features, colib::bytearray &image);

        void horizontalRunLengths(colib::floatarray &resulthist,
                                    colib::floatarray &resultstats,
                                    const colib::bytearray &image);
        void verticalRunLengths(colib::floatarray &resulthist,
                                    colib::floatarray &resultstats,
                                    const colib::bytearray &image);
        void mainDiagRunLengths(colib::floatarray &resulthist,
                                    colib::floatarray &resultstats,
                                    const colib::bytearray &image);
        void sideDiagRunLengths(colib::floatarray &resulthist,
                                    colib::floatarray &resultstats,
                                    const colib::bytearray &image);

        void compressHist(colib::intarray &histogram);
        void compress2DHist(colib::intarray &histogram);

        void concompHist(colib::floatarray &result,
                          colib::rectarray &concomps);

        void concompNeighbors(colib::floatarray &result,
                               colib::rectarray &concomps);

    };

    ZoneFeatures *make_ZoneFeatures();

    struct LogReg{
        int   feature_len;
        int   class_num;
        float factor;
        float offset;
        colib::floatarray lambda;

        void loadData();
        zone_class classify(colib::floatarray &feature);
        void getClassProbabilities(colib::floatarray &prob,
                                   colib::floatarray &feature);
    };

    LogReg *make_LogReg();

}

#endif
