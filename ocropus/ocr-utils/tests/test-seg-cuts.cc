// -*- C++ -*-

// Copyright 2006 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz 
// or its licensors, as applicable.
// Copyright 1995-2005 by Thomas M. Breuel
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
// Project:
// File: 
// Purpose: 
// Responsible: tmb
// Reviewer: 
// Primary Repository: 
// Web Sites: 


#include "colib/colib.h"
#include "segmentation.h"
#include "iulib/imgio.h"
#include "ocr-utils.h"

using namespace colib;
using namespace ocropus;

int main() {
    stdio filelist("test-list", "r");
    char in_path[1000];
    autodel<ISegmentLine> segline(make_CurvedCutSegmenter());
    segline->description();
    while(fscanf(filelist, "%999s", in_path) == 1) {
        bytearray image;
        intarray seg;
        read_image_gray(image, in_path);
        binarize_by_threshold(image);
        segline->charseg(seg, image);
        check_line_segmentation(seg);
        char out_path[1100];
        sprintf(out_path, "%s.cseg.png", in_path);
        write_png_packed(stdio(out_path, "wb"), seg);
    }
}
