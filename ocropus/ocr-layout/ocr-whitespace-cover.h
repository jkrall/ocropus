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
// File: ocr-whitespce-cover.h
// Purpose: Header file declaring data structures used in whitespace cover
//          computation
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrwhitespacecover__
#define h_ocrwhitespacecover__

#include "ocropus.h"
#include "ocr-layout.h"
#include "colib/iarith.h"

namespace ocropus {
    using namespace colib;
    /////////////////////////////////////////////////////////////////////
    ///
    /// \struct WhitespaceCover
    /// Purpose: Whitespace Cover finding algorithm.
    ///
    //////////////////////////////////////////////////////////////////////
    enum qfunc {width, height, area};

    class WhitespaceCover {
    private:
        int   verbose;
        int   max_results;
        float min_weight;
        float max_overlap;
        float min_aspect;
        float max_aspect;
        float min_width;
        float min_height;
        float logmin_aspect;
        bool  greedy;
        colib::rectangle  bounds;
        qfunc quality_func;

        typedef colib::shortarray Matches;
        typedef counted<Matches> CMatches;
        /////////////////////////////////////////////////////////////////////
        ///
        /// \struct WState
        /// Purpose: Current state of the Whitespace Cover finding algorithm.
        ///
        //////////////////////////////////////////////////////////////////////

        struct WState {
            int current_nrects;
            float weight;
            short top,left,bottom,right;
            colib::rectangle bounds;
            CMatches matches;

            bool isDone(WhitespaceCover *env);
            void update(WhitespaceCover *env);
            int maxCentricity(WhitespaceCover *env);

        };

        typedef counted<WState> CState;

        colib::rectarray rects;
        int initial_nrects;
        heap<CState> queue;
        colib::narray<CState> results;
        void compute();
        bool goodDimensions(CState &result);
        void generateChildStates(CState &state, colib::rectangle &pivot);
    public:
        WhitespaceCover();
        WhitespaceCover(colib::rectangle image_boundary);
        ~WhitespaceCover() {}
        void init();
        const char *description();
        void compute(colib::rectarray &whitespaces, colib::rectarray &obstacles);
        void addRect(colib::rectangle r) {
            rects.push(r);
        }
        void setMaxResults(int value) {
            max_results = value;
        }
        void setMinWeight(float value) {
            min_weight = value;
        }
        void setMinWidth(float value) {
            min_width = value;
        }
        void setMinHeight(float value) {
            min_height = value;
        }
        void setBounds(int x0,int y0,int x1,int y1) {
            bounds = colib::rectangle(x0,y0,x1,y1);
        }
        void setVerbose(int value) {
            verbose = value;
        }
        void setGreedy(bool value) {
            greedy = value;
        }
        void setMaxOverlap(float value) {
            max_overlap = value;
        }
        void setAspectRange(float min,float max) {
            min_aspect = min;
            max_aspect = max;
        }
        void setLogminAspect(float m) {
            logmin_aspect = m;
        }
        void setQfunc(qfunc t) {
            quality_func = t;
        }
        // Fit the bounds tightly to include all rectangles in the stack 'rects'.
        void snugBounds() {
            bounds = colib::rectangle();
            for(int i=0;i<rects.length();i++) {
                bounds.include(rects[i]);
            }
        }
        int nSolutions() {
            return results.length();
        }
        void solution(int index,int &x0,int &y0,int &x1,int &y1) {
            colib::rectangle &b = results[index]->bounds;
            x0 = b.x0;
            y0 = b.y0;
            x1 = b.x1;
            y1 = b.y1;
        }
    };
    WhitespaceCover *make_WhitespaceCover(colib::rectangle &r);
    WhitespaceCover *make_WhitespaceCover(int x0, int y0, int x1, int y1);

}

#endif
