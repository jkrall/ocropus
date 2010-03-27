// -*- C++ -*-

// Copyright 2006-2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Project:
// File: ocr-make-garbage.cc
// Purpose: produce garbage grid from a correctly segmented line
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "iulib/iulib.h"
#include "ocropus.h"

using namespace colib;
using namespace iulib;
using namespace ocropus;


int main(int argc, char **argv) {
    if(argc != 3) {
        fprintf(stderr,
                "Usage: %s <input GT segm. (PNG)> <output grid (PNM)>\n",
                argv[0]);
        exit(2);
    }
    narray<rectangle> bboxes;
    narray<bytearray> garbage;
    intarray image;
    read_png_rgb(image, stdio(argv[1], "rb"));
    make_garbage(bboxes, garbage, image);


/*    float intercept;
    float slope;
    float xheight;
    float descender_sink;
    float ascender_rise;

    if(!get_extended_line_info(intercept,slope,xheight,descender_sink,
                               ascender_rise, oversegmented_line)) {
        intercept = 0;
        slope = 0;
        xheight = 0;
        descender_sink = 0;
        ascender_rise = 0;
    }
    xheight = estimate_xheight(oversegmented_line, slope);

    //float baseline = slope * (bbox.x0 + bbox.x1) / 2 + intercept;
  */

    Grid g;
    g.create();
    for(int i = 0; i < garbage.length(); i++)
        g.add(garbage[i]);
    g.save(argv[2]);
}
