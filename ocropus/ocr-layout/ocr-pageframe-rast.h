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
// File: ocr-pageframe-rast.h
// Purpose: Header file declaring data structures for page frame detection
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrpageframerast__
#define h_ocrpageframerast__

#include "ocropus.h"
#include "ocr-layout.h"
#include "colib/iarith.h"

namespace ocropus {

    using namespace colib;
    using namespace iulib;

    void remove_border_noise(colib::bytearray &out,
                             colib::bytearray &in,
                             colib::rectangle &pageframe);

    /////////////////////////////////////////////////////////////////////
    ///
    /// \struct PageFrameRAST
    /// Purpose: Implementation of the page frame detection
    ///          algorithm using RAST.
    ///
    //////////////////////////////////////////////////////////////////////

    // FIXME/faisal this class exposes instance variables and types that should
    // be internal to the module; this is not acceptable --tmb

    static const int npfparams = 4;
    struct PageFrameRAST : ICleanupBinary {
        PageFrameRAST();
        virtual ~PageFrameRAST(){
        }
        const char *description() {
            return "remove marginal noise using page frame detection \n";
        }

        void init(const char **argv) {
            // nothing to be done
        }

        const char *name() {
            return "rastframe";
        }

        void cleanup(colib::bytearray &out,colib::bytearray &in);

        // FIXME/faisal do not expose instance variables in header files! --tmb

        int    generation;
        bool   lsq;
        double epsilon;
        int    maxsplits;
        double delta;
        double adelta;

        float  min_overlap;
        float  min_length;
        int    min_gap;
        double min_q;
        int    min_count;
        int    max_results;
        typedef colib::vecni<npfparams> Parameters;
        double splitscale[npfparams];
        Parameters all_params;
        Parameters empty_parameters;

        colib::vec2i normalized(colib::vec2i v) {
            colib::interval a = atan2(v.y,v.x);
            return colib::vec2i(cos(a),sin(a));
        }

        inline colib::interval influence(bool lsq,colib::interval d,double epsilon) {
            if(lsq) return sqinfluence(d,epsilon);
            else return rinfluence(d,epsilon);
        }

        typedef colib::narray<int> Matches;
        colib::rectarray lineboxes;
        colib::rectarray zoneboxes;
        colib::narray<bool> used;

        bool final(colib::interval q,const Parameters &p) {
            return p[0].width()<delta &&
                p[1].width()<delta &&
                p[2].width()<delta &&
                p[3].width()<delta;
        }

        struct PFStateBasic {
            short       generation;
            short       depth;
            short       rank;
            signed char splits;
            bool        splittable;
            colib::interval    quality;
            float       priority;
            Parameters  params;
            Matches     matches;

            PFStateBasic();
            void set(PageFrameRAST &pf,int depth,Parameters &params,
                     Matches &candidates,int splits);
            void reeval(PageFrameRAST &pf);
            void update(PageFrameRAST &pf, Matches &candidates);
        };

        typedef counted<PFStateBasic> CState;
        heap<CState> queue;
        colib::narray<CState> results;
        colib::autodel<CharStats> linestats;
        Matches all_matches;

        void setDefaultParameters();
        void setSearchParameters(colib::bytearray &in);
        void prepare();
        void makeSubStates(colib::narray<CState> &substates,CState &state);
        void search();
        void pushResult(CState &result);
    };

}

#endif
