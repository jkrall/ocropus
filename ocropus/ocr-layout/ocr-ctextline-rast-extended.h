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
// File: ocr-ctextline-rast.h
// Purpose: Header file declaring data structures for constrained textline
//          extraction using RAST
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrctextlinerastextended__
#define h_ocrctextlinerastextended__

#include "ocropus.h"
#include "colib/iarith.h"

namespace ocropus {
    using namespace colib;


    /////////////////////////////////////////////////////////////////////
    ///
    /// \struct CTextlineRAST4line
    /// Purpose: 4line implementation of the constrained textline finding
    ///          algorithm using RAST. Returns parameters of text-lines in
    ///          descending order of quality.
    ///
    //////////////////////////////////////////////////////////////////////

    static const int ntl4params = 5;
    struct CTextlineRAST4line {
        CTextlineRAST4line();
        virtual ~CTextlineRAST4line(){
        }
        int    generation;
        bool   lsq;
        double epsilon;
        int    maxsplits;
        double delta;
        double adelta;

        float  min_length;
        int    min_gap;
        double min_q;
        int    min_count;
        int    max_results;
        bool   use_whitespace;

        // The parameters are:
        // all_params[0] = y-intercept of baseline
        // all_params[1] = slope
        // all_params[2] = descender distance from baseline
        // all_params[3] = xheight
        // all_params[4] = ascender distance from xheight
        typedef colib::vecni<ntl4params> Parameters;
        double splitscale[ntl4params];
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
        colib::rectarray cboxes;
        colib::rectarray wboxes;
        colib::narray<bool> used;

        bool final(colib::interval q,const Parameters &p) {
            return p[0].width()<delta &&
                p[1].width()<adelta &&
                p[2].width()<delta &&
                p[3].width()<delta &&
                p[4].width()<delta;
        }

        struct TLState4line {
            short       generation;
            short       depth;
            short       rank;
            signed char splits;
            bool        splittable;
            colib::interval    quality;
            float       priority;
            Parameters  params;
            Matches     matches;

            TLState4line();
            void set(CTextlineRAST4line &line,int depth,Parameters &params,
                     Matches &candidates,int splits);
            void reeval(CTextlineRAST4line &line);
            void update(CTextlineRAST4line &line, Matches &candidates);
            TextLineParam4line returnLineParam();
        };

        typedef counted<TLState4line> CState;
        heap<CState> queue;
        colib::narray<CState> results;
        colib::autodel<CharStats> linestats;
        Matches all_matches;

        void setDefaultParameters();
        void setMaxSlope(double max_slope);
        void setMaxYintercept(double ymin, double ymax);
        void prepare();
        void makeSubStates(colib::narray<CState> &substates,CState &state);
        int  wboxIntersection(CState &top);
        void search();
        virtual void pushResult(CState &result);
        virtual void extract(colib::narray<TextLineParam4line> &textlines,
                             colib::autodel<CharStats> &charstats);
        virtual void extract(colib::narray<TextLineParam4line> &textlines,
                             colib::rectarray &columns,
                             colib::autodel<CharStats>    &charstats);
    };
    CTextlineRAST4line *make_CTextlineRAST4line();

    /////////////////////////////////////////////////////////////////////
    ///
    /// \struct CTextlineRAST
    /// Purpose: Constrained Textline finding using RAST
    ///
    //////////////////////////////////////////////////////////////////////
    struct CTextlineRASTExtended : CTextlineRAST4line{

        CTextlineRASTExtended();
        ~CTextlineRASTExtended(){ }
        // fraction of area covered by line bounding box
        // so that char_box is included in line_box
        float  minoverlap;

        // rejection threshold for the height of a box = tr*xheight
        float  min_box_height;

        // average distance between words
        int    word_gap;

        int    min_height;
        int    assign_boxes;
        bool   aggressive;
        int    extend;
        int    pagewidth;
        int    pageheight;

        colib::rectarray cboxes_all;
        colib::narray<bool> used_all;
        colib::narray<TextLineExtended> result_lines;

        void setDefaultParameters();
        void pushResult(CState &result);
        void extract(colib::narray<TextLineExtended> &textlines,
                     colib::autodel<CharStats> &charstats);
        void extract(colib::narray<TextLineExtended> &textlines,
                     colib::rectarray &columns,
                     colib::autodel<CharStats>    &charstats);
    };
    CTextlineRASTExtended *make_CTextlineRASTExtended();

}

#endif
