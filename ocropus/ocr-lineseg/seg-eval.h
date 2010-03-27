// -*- C++ -*-

// Copyright 2006 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Project: segeval - Color based evaluation of page segmentation
// File: seg-eval.h
// Purpose: Data structures for color-based segmentation evaluation
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: Yves Rangoni (rangoni@iupr.org)
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_segeval__
#define h_segeval__

#include "ocropus.h"

using namespace colib;

namespace ocropus {

#define DONTCARE   0xffffffff
#define NOISE      0x00000000
#define BACKGROUND 0x00ffffff

    int normalize_image(colib::intarray &image, int &noiseval, bool allow_dont_care=false);

    struct Evaluator {
        double threshold;
        intarray ihist,mhist;
        int mnoiseval;
        int inoiseval;
        int mover;
        int mcount;
        int iover;
        int icount;
        int movercount;
        int iovercount;
        int miss;
        int falarm;
        int correct;
        float accuracy;
        int absthresh;
        intarray ioverseg, iunderseg, imissed, ifalarm;

        Evaluator() { clear(); }

        void clear(int nbuckets = 50);

        // table of correspondences
        // rows are model component numbers
        // cols are image component numbers
        intarray table;

        void add(colib::intarray &model,colib::intarray &image);

        void add_hist();

        void calc_correct();

        void calc_accuracy();

        void print_raw() {
            for(int i=0;i<table.dim(0);i++) {
                for(int j=0;j<table.dim(1);j++) {
                    int count = table(i,j);
                    if(count>0) printf("%6d %6d %10d\n",i,j,count);
                }
            }
        }
        void print_over() {
            printf("mcount %d mover %d icount %d iover %d\n",mcount,mover,icount,iover);
            printf("mratio %g iratio %g\n",mover/(double)mcount,iover/(double)icount);
        }

        void print_hist() {
            int n = ihist.length();
            //CHECK(n==mhist.length());
            for(int i=0;i<ihist.length();i++) {
                printf("%4d %4d %4d\n",i*100/n,mhist(i),ihist(i));
            }
        }

        void set_noiseval(int mval, int ival){
            mnoiseval = mval;
            inoiseval = ival;
        }

        void set_thresh(int at, float rt){
            absthresh = at;
            threshold = rt;
        }
    };

    int segeval_full(   intarray &model, intarray &image,
                        Evaluator &eval, intarray &ground_err, intarray &seg_err,
                        int &num_over,int &num_under,int &num_miss,int &num_falarm,
                        int athresh = 300, float rthresh = 0.1);

}

#endif

