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
// File: main-voronoi-ocropus.cc
// Purpose: Wrapper class for voronoi code
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "iulib/imgio.h"
#include "iulib/imglib.h"
#include "voronoi-ocropus.h"

using namespace ocropus;
using namespace colib;
using namespace iulib;

static void invertbg(intarray &image){
    simple_recolor(image);
    for(int i=0; i<image.length1d(); i++)
        if(image.at1d(i) == 0x00800000)
            image.at1d(i) = 0x00ffffff;
    return;
}

namespace ocropus{
    param_bool colorful("colorful",0,"Recolor output for visualization");
}

int main(int argc,char **argv) {
    try {
        if(!(argc==3 || argc==2)) {
            fprintf(stderr, "Usage: ... input.png output.png\n");
            exit(1);
        }
        const char *outimage="out.png";
        if(argc==3){
            outimage=argv[2];
        }
        bytearray in ;
        intarray out;
        read_image_gray(in,stdio(argv[1],"r"));
        autodel<ISegmentPage> voronoi(make_SegmentPageByVORONOI());
        voronoi->segment(out,in);
        if(colorful)
            invertbg(out);
        write_image_packed(outimage,out);
    }
    catch(const char *oops) {
        fprintf(stderr,"oops: %s\n",oops);
    }
}

