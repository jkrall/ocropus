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
// File: ocr-classify-zones.cc
// Purpose: Document zone classification using run-lengths and
//          connected components based features and logistic regression
//          classifier as described in:
//          D. Keysers, F. Shafait, T.M. Breuel. "Document Image Zone Classification -
//          A Simple High-Performance Approach",  VISAPP 2007, pages 44-51.
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"
#include "log-reg-data.h"

using namespace ocropus;
using namespace iulib;
using namespace colib;

namespace ocropus {

    void ZoneFeatures::compressHist(intarray &histogram){
        intarray histogram_comp;
        histogram_comp.resize(MAX_LEN);
        fill(histogram_comp,0);
        // COMPRESSION
        int i = 0;
        int j = 0;
        int m = COMP_START;
        int k = COMP_LEN_START;
        while(i < MAX_LEN) {
            histogram_comp(j) = histogram(i);
            i++;
            if(i > COMP_START){
                m = min(MAX_LEN, m+k);
                while(i < m){
                    histogram_comp(j)+= histogram(i);
                    i++;
                }
                k = COMP_LEN_INC_A * k + COMP_LEN_INC_B;
            }
            j++;
        }
        histogram.clear();
        for(i = 0; i < j; i++)
            histogram.push(histogram_comp(i));
    }

    void ZoneFeatures::compress2DHist(intarray &histogram_2D){

        int steps[2];

        intarray histogram_2D_comp;
        histogram_2D_comp.resize(MAX_LEN,MAX_LEN);
        fill(histogram_2D_comp,0);

        int i, j, k, m, n;

        for(n = 0; n < MAX_LEN; n++) {
            i = 0;
            j = 0;
            m = COMP_START;
            k = COMP_LEN_START;
            while(i < MAX_LEN) {
                histogram_2D_comp(n,j) = histogram_2D(n,i);
                i++;
                if(i > COMP_START) {
                    m = min(MAX_LEN, m+k);
                    while(i < m) {
                        histogram_2D_comp(n,j)+= histogram_2D(n,i);
                        i++;
                    }
                    k = COMP_LEN_INC_A * k + COMP_LEN_INC_B;
                }
                j++;
            }
        }
        steps[0] = j;

        for(n = 0; n < steps[0]; n++) {
            i = 0;
            j = 0;
            m = COMP_START;
            k = COMP_LEN_START;
            while(i < MAX_LEN) {
                histogram_2D_comp(j,n) = histogram_2D_comp(i,n);
                i++;
                if(i > COMP_START) {
                    m = min(MAX_LEN, m+k);
                    while(i < m) {
                        histogram_2D_comp(j,n)+= histogram_2D_comp(i,n);
                        i++;
                    }
                    k = COMP_LEN_INC_A * k + COMP_LEN_INC_B;
                }
                j++;
            }
        }
        steps[1] = j;

        //        move(histogram_2D, histogram_2D_comp);
        histogram_2D.clear();
        histogram_2D.resize(steps[0], steps[1]);
        for(i = 0; i < steps[0]; i++){
            for(j = 0; j < steps[1]; j++){
                histogram_2D(i,j) = histogram_2D_comp(i,j);
            }
        }
    }



    void ZoneFeatures::horizontalRunLengths(floatarray &resulthist,
                                              floatarray &resultstats,
                                              const bytearray &image){

        int imwidth  = image.dim(0);
        int imheight = image.dim(1);

        intarray histogram_fg;
        histogram_fg.resize(MAX_LEN);
        fill(histogram_fg,0);
        intarray histogram_bg;
        histogram_bg.resize(MAX_LEN);
        fill(histogram_bg,0);

        int current_run_length_bg = 0;
        int current_run_length_fg = 0;
        int run_length_count_fg = 0;
        int run_length_count_bg = 0;
        bool end_fg = false;
        bool end_bg = false;
        // indicator that the end of the running segment occurs
        float mean_fg = 0, variance_fg = 0;
        float mean_bg = 0, variance_bg = 0;
        for(int j = 0; j < imheight; j++){
            for(int i = 0; i < imwidth; i++){
                if(image(i,j) == 0){
                    current_run_length_fg++;
                    end_fg = false;
                }
                else
                    end_fg = true;

                if((current_run_length_fg > 0) && (end_fg || (i == imwidth-1))){
                    current_run_length_fg = min(current_run_length_fg, MAX_LEN);
                    histogram_fg(current_run_length_fg-1)++;
                    run_length_count_fg++;
                    mean_fg += current_run_length_fg;
                    variance_fg += current_run_length_fg * current_run_length_fg;
                    current_run_length_fg = 0;
                    end_fg = false;
                }

                if(image(i,j) == 255){
                    current_run_length_bg++;
                    end_bg = false;
                }
                else
                    end_bg = true;

                if((current_run_length_bg > 0) && (end_bg || (i == imwidth-1))){
                    current_run_length_bg = min(current_run_length_bg, MAX_LEN);
                    histogram_bg(current_run_length_bg-1)++;
                    run_length_count_bg++;
                    mean_bg += current_run_length_bg;
                    variance_bg += current_run_length_bg * current_run_length_bg;
                    current_run_length_bg = 0;
                    end_bg = false;
                }

            }
        }
        compressHist(histogram_fg);
        compressHist(histogram_bg);
        for(int i=0, l=histogram_fg.length(); i<l; i++)
            resulthist.push(histogram_fg[i]);
        for(int i=0, l=histogram_bg.length(); i<l; i++)
            resulthist.push(histogram_bg[i]);

        if(run_length_count_fg){
            mean_fg/= run_length_count_fg;
            variance_fg = variance_fg/run_length_count_fg - mean_fg*mean_fg;
        }
        else{
            mean_fg=0; variance_fg=0;
        }
        resultstats.push(run_length_count_fg);
        resultstats.push(mean_fg);
        resultstats.push(variance_fg);

        if(run_length_count_bg){
            mean_bg/= run_length_count_bg;
            variance_bg = variance_bg/run_length_count_bg - mean_bg*mean_bg;
        }
        else{
            mean_bg=0; variance_bg=0;
        }
        resultstats.push(run_length_count_bg);
        resultstats.push(mean_bg);
        resultstats.push(variance_bg);

    }

    void ZoneFeatures::verticalRunLengths(floatarray &resulthist,
                                            floatarray &resultstats,
                                            const bytearray &image){

        int imwidth  = image.dim(0);
        int imheight = image.dim(1);

        intarray histogram_fg;
        histogram_fg.resize(MAX_LEN);
        fill(histogram_fg,0);
        intarray histogram_bg;
        histogram_bg.resize(MAX_LEN);
        fill(histogram_bg,0);

        int current_run_length_bg = 0;
        int current_run_length_fg = 0;
        int run_length_count_fg = 0;
        int run_length_count_bg = 0;
        bool end_fg = false;
        bool end_bg = false;
        // indicator that the end of the running segment occurs
        float mean_fg = 0, variance_fg = 0;
        float mean_bg = 0, variance_bg = 0;
        for(int i = 0; i < imwidth; i++){
            for(int j = 0; j < imheight; j++){
                if(image(i,j) == 0){
                    current_run_length_fg++;
                    end_fg = false;
                }
                else
                    end_fg = true;

                if((current_run_length_fg > 0) && (end_fg || (i == imheight-1))){
                    current_run_length_fg = min(current_run_length_fg, MAX_LEN);
                    histogram_fg(current_run_length_fg-1)++;
                    run_length_count_fg++;
                    mean_fg += current_run_length_fg;
                    variance_fg += current_run_length_fg * current_run_length_fg;
                    current_run_length_fg = 0;
                    end_fg = false;
                }

                if(image(i,j) == 255){
                    current_run_length_bg++;
                    end_bg = false;
                }
                else
                    end_bg = true;

                if((current_run_length_bg > 0) && (end_bg || (i == imheight-1))){
                    current_run_length_bg = min(current_run_length_bg, MAX_LEN);
                    histogram_bg(current_run_length_bg-1)++;
                    run_length_count_bg++;
                    mean_bg += current_run_length_bg;
                    variance_bg += current_run_length_bg * current_run_length_bg;
                    current_run_length_bg = 0;
                    end_bg = false;
                }

            }
        }
        compressHist(histogram_fg);
        compressHist(histogram_bg);
        for(int i=0, l=histogram_fg.length(); i<l; i++)
            resulthist.push(histogram_fg[i]);
        for(int i=0, l=histogram_bg.length(); i<l; i++)
            resulthist.push(histogram_bg[i]);

        if(run_length_count_fg){
            mean_fg/= run_length_count_fg;
            variance_fg = variance_fg/run_length_count_fg - mean_fg*mean_fg;
        }
        else{
            mean_fg=0; variance_fg=0;
        }
        resultstats.push(run_length_count_fg);
        resultstats.push(mean_fg);
        resultstats.push(variance_fg);

        if(run_length_count_bg){
            mean_bg/= run_length_count_bg;
            variance_bg = variance_bg/run_length_count_bg - mean_bg*mean_bg;
        }
        else{
            mean_bg=0; variance_bg=0;
        }
        resultstats.push(run_length_count_bg);
        resultstats.push(mean_bg);
        resultstats.push(variance_bg);

    }

    void ZoneFeatures::mainDiagRunLengths(floatarray &resulthist,
                                             floatarray &resultstats,
                                             const bytearray &image){

        int imwidth  = image.dim(0);
        int imheight = image.dim(1);

        intarray histogram_fg;
        histogram_fg.resize(MAX_LEN);
        fill(histogram_fg,0);
        intarray histogram_bg;
        histogram_bg.resize(MAX_LEN);
        fill(histogram_bg,0);

        int current_run_length_bg = 0;
        int current_run_length_fg = 0;
        int run_length_count_fg = 0;
        int run_length_count_bg = 0;
        bool end_fg = false;
        bool end_bg = false;
        // indicator that the end of the running segment occurs
        float mean_fg = 0, variance_fg = 0;
        float mean_bg = 0, variance_bg = 0;

        int pix = 0;
        for(int i = 0; i < imwidth + imheight; i++){
            for(int j = 0; j < min(imwidth, imheight); j++){
                if(i < imwidth){
                    if(j < i+1)
                        pix = image(i-j, j);
                    else
                        j = imwidth*imheight;
                }
                else{
                    if(j < imwidth-1+imheight-i)
                        pix = image(imwidth - j - 1, i - (imwidth-1) + j);
                    else
                        j = imwidth*imheight;
                }

                if((pix == 0) && (j != imwidth*imheight)){
                    current_run_length_fg++;
                    end_fg = false;
                }
                else
                    end_fg = true;

                if( (current_run_length_fg > 0) &&
                    (end_fg || (j == imwidth*imheight))){
                    current_run_length_fg = min(current_run_length_fg, MAX_LEN);
                    histogram_fg(current_run_length_fg-1)++;
                    run_length_count_fg++;
                    mean_fg += current_run_length_fg;
                    variance_fg += current_run_length_fg * current_run_length_fg;
                    current_run_length_fg = 0;
                    end_fg = false;
                }

                if((pix == 255) && (j != imwidth*imheight)){
                    current_run_length_bg++;
                    end_bg = false;
                }
                else
                    end_bg = true;

                if( (current_run_length_bg > 0) &&
                    (end_bg || (j == imwidth*imheight))){
                    current_run_length_bg = min(current_run_length_bg, MAX_LEN);
                    histogram_bg(current_run_length_bg-1)++;
                    run_length_count_bg++;
                    mean_bg += current_run_length_bg;
                    variance_bg += current_run_length_bg * current_run_length_bg;
                    current_run_length_bg = 0;
                    end_bg = false;
                }
            }
        }


        compressHist(histogram_fg);
        compressHist(histogram_bg);
        for(int i=0, l=histogram_fg.length(); i<l; i++)
            resulthist.push(histogram_fg[i]);
        for(int i=0, l=histogram_bg.length(); i<l; i++)
            resulthist.push(histogram_bg[i]);

        if(run_length_count_fg){
            mean_fg/= run_length_count_fg;
            variance_fg = variance_fg/run_length_count_fg - mean_fg*mean_fg;
        }
        else{
            mean_fg=0; variance_fg=0;
        }
        resultstats.push(run_length_count_fg);
        resultstats.push(mean_fg);
        resultstats.push(variance_fg);

        if(run_length_count_bg){
            mean_bg/= run_length_count_bg;
            variance_bg = variance_bg/run_length_count_bg - mean_bg*mean_bg;
        }
        else{
            mean_bg=0; variance_bg=0;
        }
        resultstats.push(run_length_count_bg);
        resultstats.push(mean_bg);
        resultstats.push(variance_bg);

    }

    void ZoneFeatures::sideDiagRunLengths(floatarray &resulthist,
                                             floatarray &resultstats,
                                             const bytearray &image){

        int imwidth  = image.dim(0);
        int imheight = image.dim(1);

        intarray histogram_fg;
        histogram_fg.resize(MAX_LEN);
        fill(histogram_fg,0);
        intarray histogram_bg;
        histogram_bg.resize(MAX_LEN);
        fill(histogram_bg,0);

        int current_run_length_bg = 0;
        int current_run_length_fg = 0;
        int run_length_count_fg = 0;
        int run_length_count_bg = 0;
        bool end_fg = false;
        bool end_bg = false;
        // indicator that the end of the running segment occurs
        float mean_fg = 0, variance_fg = 0;
        float mean_bg = 0, variance_bg = 0;

        int pix = 0;
        for(int i = 0; i < imwidth + imheight; i++){
            for(int j = 0; j < min(imwidth, imheight); j++){
                if(i < imheight){
                    if(j < i+1)
                        pix = image(j, (imheight-1) - i + j);
                    else
                        j = imwidth*imheight;
                }
                else{
                    if(j < imwidth-1+imheight-i)
                        pix = image(i - (imheight-1) + j, j);
                    else
                        j = imwidth*imheight;
                }

                if((pix == 0) && (j != imwidth*imheight)){
                    current_run_length_fg++;
                    end_fg = false;
                }
                else
                    end_fg = true;

                if( (current_run_length_fg > 0) &&
                    (end_fg || (j == imwidth*imheight))){
                    current_run_length_fg = min(current_run_length_fg, MAX_LEN);
                    histogram_fg(current_run_length_fg-1)++;
                    run_length_count_fg++;
                    mean_fg += current_run_length_fg;
                    variance_fg += current_run_length_fg * current_run_length_fg;
                    current_run_length_fg = 0;
                    end_fg = false;
                }

                if((pix == 255) && (j != imwidth*imheight)){
                    current_run_length_bg++;
                    end_bg = false;
                }
                else
                    end_bg = true;

                if( (current_run_length_bg > 0) &&
                    (end_bg || (j == imwidth*imheight))){
                    current_run_length_bg = min(current_run_length_bg, MAX_LEN);
                    histogram_bg(current_run_length_bg-1)++;
                    run_length_count_bg++;
                    mean_bg += current_run_length_bg;
                    variance_bg += current_run_length_bg * current_run_length_bg;
                    current_run_length_bg = 0;
                    end_bg = false;
                }
            }
        }


        compressHist(histogram_fg);
        compressHist(histogram_bg);
        for(int i=0, l=histogram_fg.length(); i<l; i++)
            resulthist.push(histogram_fg[i]);
        for(int i=0, l=histogram_bg.length(); i<l; i++)
            resulthist.push(histogram_bg[i]);

        if(run_length_count_fg){
            mean_fg/= run_length_count_fg;
            variance_fg = variance_fg/run_length_count_fg - mean_fg*mean_fg;
        }
        else{
            mean_fg=0; variance_fg=0;
        }
        resultstats.push(run_length_count_fg);
        resultstats.push(mean_fg);
        resultstats.push(variance_fg);

        if(run_length_count_bg){
            mean_bg/= run_length_count_bg;
            variance_bg = variance_bg/run_length_count_bg - mean_bg*mean_bg;
        }
        else{
            mean_bg=0; variance_bg=0;
        }
        resultstats.push(run_length_count_bg);
        resultstats.push(mean_bg);
        resultstats.push(variance_bg);

    }

    void ZoneFeatures::concompHist(floatarray &result,
                                    rectarray &concomps){

        intarray histogram_width;
        histogram_width.resize(MAX_LEN);
        fill(histogram_width,0);

        intarray histogram_height;
        histogram_height.resize(MAX_LEN);
        fill(histogram_height,0);

        intarray histogram_2D;
        histogram_2D.resize(MAX_LEN,MAX_LEN);
        fill(histogram_2D,0);

        int width, height;
        for(int i=0, l=concomps.length(); i<l; i++){
            if(concomps[i].width() <=0 || concomps[i].height() <=0)
                continue;
            width  = concomps[i].width();
            height = concomps[i].height();
            histogram_width(min(width, MAX_LEN)-1)++;
            histogram_height(min(height, MAX_LEN)-1)++;
            histogram_2D(min(width, MAX_LEN)-1,min(height, MAX_LEN)-1)++;
        }

        compressHist(histogram_width);
        compressHist(histogram_height);
        compress2DHist(histogram_2D);

        for(int i=0, l=histogram_width.length(); i<l; i++)
            result.push(histogram_width[i]);
        for(int i=0, l=histogram_height.length(); i<l; i++)
            result.push(histogram_height[i]);
        for(int i=0, l=histogram_2D.dim(0); i<l; i++)
            for(int j=0, k=histogram_2D.dim(1); j<k; j++)
                result.push(histogram_2D(i,j));

    }

    static inline double distance(float x1, float y1, float x2, float y2) {
        return (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1);
    }

    void ZoneFeatures::concompNeighbors(floatarray &result,
                                         rectarray &concomps){
        // bounding boxes nearest neighbor distance
        int num_boxes = concomps.length();
        intarray histogram;
        histogram.resize(MAX_LEN);
        fill(histogram,0);

        float xc, yc;
        double dist_min, dist;
        for(int i=0; i<num_boxes; i++) {
            xc = concomps[i].xcenter();
            yc = concomps[i].ycenter();
            dist_min = 100000;
            for(int j=0; j<num_boxes; j++) {
                dist = distance(xc, yc, concomps[j].xcenter(), concomps[j].ycenter());
                if( dist && dist<dist_min)
                    dist_min = dist;
            }
            dist_min = sqrt(dist_min);
            if(dist_min<0 || dist_min>=MAX_LEN)
                continue;
            histogram( int (dist_min) )++;
        }

        compressHist(histogram);
        for(int i=0, l=histogram.length(); i<l; i++)
            result.push(histogram[i]);

    }

    void ZoneFeatures::extractFeatures(floatarray &feature, bytearray &image){
        if(!contains_only(image, byte(0), byte(255))){
            fprintf(stderr,"Binary image expected! ");
            fprintf(stderr,"skipping feature extraction...\n");
            return ;
        }

        // RUNNING LENGTHS
        floatarray rl_stats;
        horizontalRunLengths(feature,rl_stats,image);
        verticalRunLengths(feature,rl_stats,image);
        mainDiagRunLengths(feature,rl_stats,image);
        sideDiagRunLengths(feature,rl_stats,image);

        for(int index=0; index<rl_stats.length(); index++)
            feature.push(rl_stats[index]);

        // CONNECTED COMPONENTS
        bytearray in;
        copy(in, image);
        invert(in);

        // Do connected component analysis
        intarray charimage;
        copy(charimage,in);
        label_components(charimage,false);

        // Clean non-text and noisy boxes and get character statistics
        rectarray bboxes,boxes;
        bounding_boxes(bboxes,charimage);
        for(int i=0, l=bboxes.length(); i<l; i++)
            if(bboxes[i].area())
                boxes.push(bboxes[i]);

        concompHist(feature, boxes);
        concompNeighbors(feature, boxes);

        int z = 1;
        feature.push(z);

    }

    ZoneFeatures *make_ZoneFeatures() {
        return new ZoneFeatures();
    }

    void LogReg::loadData(){
        class_num = log_reg_class_num;
        factor = log_reg_factor;
        offset = log_reg_offset;
        feature_len = log_reg_feature_len;
        lambda.resize(class_num, feature_len);

        int index = 0;
        for(int i = 0; i < class_num; i++){
            for(int j = 0; j < feature_len; j++){
                lambda(i,j) = log_reg_data[index++];
            }
        }
    }

    zone_class LogReg::classify(floatarray &feature){

        float sum, image_probability;
        float sum_total = 0;
        float sum_max = 0;
        int   image_class = -1;

        for(int k = 0; k < class_num; k++){
            sum = 0;
            for(int j = 0; j < feature_len; j++)
                sum += lambda(k,j) * feature(j);
            sum = exp(factor * sum + feature_len * offset);
            if (sum > sum_max){
                sum_max = sum;
                image_class = k;
            }
            sum_total += sum;
        }

        image_probability = sum_max / sum_total;

        //fprintf(stderr,"%d %f \n",image_class,image_probability);
        switch(image_class){
        case 0: return math;
        case 1: return logo;
        case 2: return text;
        case 3: return table;
        case 4: return drawing;
        case 5: return halftone;
        case 6: return ruling;
        case 7: return noise;
        default: return undefined;
        }
    }

    void LogReg::getClassProbabilities(floatarray &probability,
                                       floatarray &feature){
        float sum,sum_total = 0;

        probability.resize(class_num);

        for(int k = 0; k < class_num; k++){
            sum = 0;
            for(int j = 0; j < feature_len; j++)
                sum += lambda(k,j) * feature(j);
            sum = exp(factor * sum + feature_len * offset);

            probability[k] = sum;
            sum_total += sum;
        }

        for(int k = 0; k < class_num; k++)
            probability[k] /= sum_total;

    }

    LogReg *make_LogReg() {
        return new LogReg();
    }


}
