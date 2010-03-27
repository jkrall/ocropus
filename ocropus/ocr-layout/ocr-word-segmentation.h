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
// File: ocr-word-segmentation.h
// Purpose: Word segmentation using smearing of bounding boxes
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrwordseg__
#define h_ocrwordseg__

#include "ocropus.h"
#include "ocr-layout.h"

namespace ocropus {

    ISegmentPage *make_SegmentWords();
    bool segment_words_by_projection(colib::intarray &seg,
                                     colib::bytearray &in, int nwords);

}

#endif
