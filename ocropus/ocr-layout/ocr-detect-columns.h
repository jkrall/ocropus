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
// File: ocr-detect-columns.h
// Purpose: Group textlines into text columns 
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrdetectcolumns__
#define h_ocrdetectcolumns__

#include "ocropus.h"
#include "ocr-layout-internal.h"

namespace ocropus {

    // Get text columns from an array of text-line bounding boxes and an array
    // of whitespace column separators or vertical rulings
    void get_text_columns(colib::rectarray &textcolumns,
                          colib::rectarray &textlines, 
                          colib::rectarray &gutters);

    // Get text columns from an array of paragraphs or zones using their alignment
    void get_text_columns(colib::rectarray &columns,
                          colib::rectarray &paragraphs);
}

#endif
