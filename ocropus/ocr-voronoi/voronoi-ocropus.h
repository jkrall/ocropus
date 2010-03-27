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
// File: voronoi-ocropus.h
// Purpose: Wrapper class for voronoi code
//
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de


#ifndef h_voronoi_ocropus__
#define h_voronoi_ocropus__

#include "colib/colib.h"
#include "ocropus.h"

namespace ocropus {

    struct SegmentPageByVORONOI : ISegmentPage {
        bool  remove_noise; // remove noise from output image
        int   nm;
        int   sr;
        float fr;
        int   ta;

        SegmentPageByVORONOI() {
            remove_noise=false;
            nm=-1; // use default value
            sr=-1; // use default value
            fr=-1; // use default value
            ta=-1; // use default value
        }
        ~SegmentPageByVORONOI() {}

        const char *name() {
            return "segvoronoi";
        }

        const char *description() {
            return "segment page by Voronoi algorithm\n";
        }

        void set(const char *key,double value) {
            if(!strcmp(key,"remove_noise"))
                this->remove_noise = bool(value);
            else if(!strcmp(key,"nm"))
                this->nm = int(value);
            else if(!strcmp(key,"sr"))
                this->sr = int(value);
            else if(!strcmp(key,"fr"))
                this->fr = float(value);
            else if(!strcmp(key,"ta"))
                this->ta = int(value);
            else throw "unknown parameter";
        }

        void segment(intarray &image,bytearray &in);
    };

    ISegmentPage *make_SegmentPageByVORONOI();

}


#endif
