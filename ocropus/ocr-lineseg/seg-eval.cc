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
// Project: segeval - Color based evaluation of page segmentation
// File: seg-eval.cc
// Purpose: Color based segmentation evaluation using graph matching
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: Yves Rangoni (rangoni@iupr.org)
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "seg-eval.h"

namespace ocropus {

    int normalize_image(intarray &image, int &noiseval, bool allow_dont_care) {
        Enumerator enumerator(1);
        for(int i=0;i<image.length1d();i++) {
            unsigned int value = image.at1d(i);
            if(value == BACKGROUND) {
                image.at1d(i)=0; 
                continue;
            }
            if(value == DONTCARE && allow_dont_care)
                image.at1d(i) = -1; 
            else
                image.at1d(i) = enumerator[value];
            if(value == NOISE)
                noiseval = image.at1d(i);
        }
        return enumerator.total();
    }
    
    void Evaluator::clear(int nbuckets) {
        threshold = 0.1;
        absthresh = 300;
        mhist.resize(nbuckets);
        fill(mhist,0);
        mover = 0;
        mcount = 0;
        movercount = 0;
        ihist.resize(nbuckets);
        fill(ihist,0);
        iover = 0;
        icount = 0;
        iovercount = 0;
        miss = 0;
        falarm = 0;
        correct = 0;
    }
    
    void Evaluator::add(intarray &model,intarray &image) {
        ASSERT(min(image)>=0);
        ASSERT(max(model)<1000);
        ASSERT(max(image)<1000);
        ASSERT(model.length1d()==image.length1d());
        int mmodel = max(model)+1;
        int mimage = max(image)+1;
        //printf("Model length= %d, Image length = %d\n",mmodel,mimage);
        table.resize(mmodel,mimage);
        fill(table,0);
        for(int i=0;i<model.length1d();i++) {
            int mval = model.at1d(i);
            if(mval<0) continue;
            int ival = image.at1d(i);
            table(mval,ival)++;
        }
        add_hist();
        calc_correct();
        calc_accuracy();
    }
    
    void Evaluator::add_hist() {
        int n = ihist.length();
        ASSERT(n==mhist.length());
        for(int i=1;i<table.dim(0);i++){
            if(i == mnoiseval) continue;
            int total = 0;
            for(int j=1;j<table.dim(1);j++){
                if(j==inoiseval)
                    continue;
                total += table(i,j);
                }
            if(total==0){
                //if(table(i,inoiseval)!=0){
                mcount++; mover++; miss++;
                imissed.push(i);
                //}
                continue;
            }
            mcount++;
            int splits = 0;
            for(int j=1;j<table.dim(1);j++) {
                if(table(i,j)==0) continue;
                double frac = table(i,j)*1.0/total;
                mhist(min(n-1,int(frac*n)))++;
                if(frac>threshold || table(i,j)>=absthresh) {
                    mover++; splits++;
                }
            }
            if(splits>1) {movercount++; ioverseg.push(i);}
        }
        for(int j=1;j<table.dim(1);j++) {
            if(j == inoiseval) continue;
            int total = 0;
            for(int i=1;i<table.dim(0);i++){
                if(i==mnoiseval)
                    continue;
                total += table(i,j);
            }
            if(total==0){
                //if(table(mnoiseval,j)!=0){
                icount++; iover++; falarm++;
                ifalarm.push(j);
                //}
                continue;
            }
            icount++;
            int splits = 0;
            for(int i=1;i<table.dim(0);i++) {
                if(table(i,j)==0) continue;
                if(i == mnoiseval) continue;
                double frac = table(i,j)*(1.0/total);
                ihist(min(n-1,int(frac*n)))++;
                if(frac>threshold || table(i,j)>=absthresh) {
                    iover++; splits++; 
                }
            }
            if(splits>1) {iovercount++; iunderseg.push(j); }
        }
    }
    void Evaluator::calc_correct() {
        intarray rowsum,colsum;
        int ncols = table.dim(0);
        int nrows = table.dim(1);
        colsum.resize(ncols);
        rowsum.resize(nrows);
        fill(colsum,0);
        fill(rowsum,0);
        
        for(int i=1;i<ncols;i++){
            if(i == mnoiseval) continue;
            for(int j=1;j<nrows;j++) {
                if(j == inoiseval) continue;
                colsum[i] += table(i,j);
            }
        }
        for(int j=1;j<nrows;j++) {
            if(j == inoiseval) continue;
            for(int i=1;i<ncols;i++){
                if(i == mnoiseval) continue;
                rowsum[j] += table(i,j);
            }
        }
        
        for(int i=1;i<ncols;i++){
            if(i == mnoiseval) continue;
            for(int j=1;j<nrows;j++) {
                if(j == inoiseval) continue;
                bool extraedges = false;
                if(//Edge incident to the model is significant
                   (table(i,j)>=(1-threshold)*colsum[i]) &&
                   //Edge incident to the image is significant
                   (table(i,j)>=(1-threshold)*rowsum[j]) &&
                   (table(i,j)>=absthresh)){
                    //Model has no other significant edge
                    for(int n=1;n<ncols;n++){
                        if(n == mnoiseval) continue;
                        if(n == i) continue;
                        if(table(n,j)>=absthresh) 
                            extraedges = true;
                    }
                    //Image has no other significant edge
                    for(int n=1;n<nrows;n++){
                        if(n == inoiseval) continue;
                        if(n == j) continue;
                        if(table(i,n)>=absthresh) 
                            extraedges = true;
                    }
                    //Image has no other significant edge
                    if(!extraedges)
                        correct ++;
                }
            }
        }
    }
    
    void Evaluator::calc_accuracy() {
        int ncols = table.dim(0);
        int nrows = table.dim(1);
        float sum = 0;
        float maxi = 0;
        float effi = 0;
        int gt_seg_counter=0;

        intarray rowsum,colsum;
        colsum.resize(ncols);
        rowsum.resize(nrows);
        fill(colsum,0);
        fill(rowsum,0);
        
        for(int i=1;i<ncols;i++){
            if(i == mnoiseval) continue;
            for(int j=1;j<nrows;j++) {
                if(j == inoiseval) continue;
                colsum[i] += table(i,j);
            }
        }
        for(int j=1;j<nrows;j++) {
            if(j == inoiseval) continue;
            for(int i=1;i<ncols;i++){
                if(i == mnoiseval) continue;
                rowsum[j] += table(i,j);
            }
        }
        
        for(int i=1;i<ncols;i++){
            if(i == mnoiseval) continue;
            gt_seg_counter++;
            //sum=0;
            for(int j=1;j<nrows;j++) {
                if(j == inoiseval) continue;
                if(table(i,j)>maxi)
                    maxi = table(i,j);
                //sum += table(i,j);
            }
            if(sum)
                effi += maxi/sum;
        }
    
        accuracy = effi / gt_seg_counter;
    }

    int segeval_full(   intarray &model, intarray &image,                        
                        Evaluator &eval, intarray &fmodel, intarray &fimage,
                        int &num_over,int &num_under,int &num_miss,int &num_falarm,
                        int athresh, float rthresh) {
        eval.clear(100);
        eval.set_thresh(athresh, rthresh);
        copy(fmodel, model);
        copy(fimage, image);
        int mnoiseval=-1, inoiseval=-1;
        eval.set_noiseval(mnoiseval, inoiseval);
        eval.add(fmodel, fimage);

        /*int mcolors = normalize_image(fmodel, mnoiseval);
        int icolors = normalize_image(fimage, inoiseval);
        printf("\nModel Colors = %d\t Image Colors = %d\n",mcolors,icolors);
        printf("\nModel Noise value = %d\t Image Noise value = %d\n",mnoiseval,inoiseval);
        printf("groundtruth-components  %d\n",eval.mcount);
        printf("segmentation-components %d\n",eval.icount-eval.falarm);
        printf("total-oversegmentation  %d\n",eval.mover-eval.mcount);
        printf("total-undersegmentation %d\n",eval.iover-eval.icount);
        printf("oversegmented-comps     %d\n",eval.movercount);
        printf("undersegmented-comps    %d\n",eval.iovercount);
        printf("missed-components       %d\n",eval.miss);
        printf("false alarms            %d\n",eval.falarm);
        printf("total correct           %d\n",eval.correct);
        printf("total accuracy          %0.3f\n",eval.accuracy);*/
     
        num_over  = eval.ioverseg.length();
        num_under = eval.iunderseg.length();
        num_miss  = eval.imissed.length();
        num_falarm= eval.ifalarm.length();
        for(int i=0;i<fmodel.length1d();i++) {
            int value = fmodel.at1d(i);
            for(int j=0;j<num_over;j++)
                if(value == eval.ioverseg[j]) {
                    fmodel.at1d(i) = 0x0000ff00; 
                    break;
                }
            for(int j=0;j<num_miss;j++)
                if(value == eval.imissed[j]) {
                    fmodel.at1d(i) = 0x000000ff;
                    break;
                }
        }
        for(int i=0;i<fimage.length1d();i++) {
            int value = fimage.at1d(i);
            for(int j=0;j<num_under;j++)
                if(value == eval.iunderseg[j]) {
                    fmodel.at1d(i) = 0x00ff003f; 
                    fimage.at1d(i) = 0x00ff003f; 
                    break;
                }
            for(int j=0;j<num_falarm;j++)
                if(value == eval.ifalarm[j]) {
                    fmodel.at1d(i) = 0x00ffff00;
                    fimage.at1d(i) = 0x00ffff00; 
                    break;
                }
        }
        return 0;
    }

}

