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
// File: ocr-noisefilter.h
// Purpose: Docuemnt image cleanup using projection profiles and 
//          connected component filtering.
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrnoisefilter__
#define h_ocrnoisefilter__

#include "ocropus.h"
#include "ocr-layout.h"

namespace ocropus {

    using namespace colib;

    const int white = 0xff;
    const int black = 0x00;
    bool  is_line_image(bytearray &in);
    struct NoiseFilter {
        int   xstep;
        int   ystep;
        int   xwidth;
        int   yheight;
        int   max_width;
        int   max_height;
        int   border_margin;
        float bfthreshold;
        float wfthreshold;
        
        NoiseFilter();
        ~NoiseFilter();
        void blackFilter(bytearray &out,bytearray &in);
        void whiteFilter(bytearray &out,bytearray &in);
        void blackFilter(bytearray &out,bytearray &in,
                         float threshold,int xstep,int ystep,int xwidth,int yheight);
        void whiteFilter(bytearray &out,bytearray &original,
                         float threshold,int xstep,int ystep, int xwidth,int yheight);
        void remove(int x0,int y0,int x1,int y1,bytearray &image);
        void ccanalysis(bytearray &out,bytearray &in, 
                        rectarray &bboxes);
        float blackRatio(int x0,int y0,int x1, int y1,bytearray &image);
        float whiteRatio(int x0,int y0, int x1, int y1, bytearray &image);
    };
    NoiseFilter *make_NoiseFilter();
    
}

#endif
