// Copyright 2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// File: didegrade.cc
// Purpose: 
// Responsible: mezhirov
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org


#include "iulib/imglib.h"
#include "didegrade.h"
#include "logger.h"

using namespace colib;
using namespace iulib;
using namespace ocropus;

namespace {
    Logger logger("degrade");    

    double rand_uniform() {
        return double(rand())/double(RAND_MAX);
    }

    // Definitely a reimplementation of something
    // generate a number approx N(mean,sigma) using the Box-Muller Transform
    float rand_gauss(float mean, float sigma) {
        float r,phi;
        if(sigma == 0) 
            return 0;
        do {
            r = rand_uniform();
            phi = rand_uniform();
        } while(r == 0.0 || phi == 0.0);
        return mean + sigma*cos(2*M_PI*phi)*sqrt(-2*log(r));
    }

    void elastic_transform_map(floatarray &result, int w, int h,
                               float alpha, float sigma) {
        result.resize(w, h);
        for(int i = 0; i < result.length1d(); i++)
            result.at1d(i) = alpha * (2 * rand_uniform() - 1);
        gauss2d(result, sigma, sigma);
    }    

    void elastic_transform(floatarray &out, floatarray &in,
                           float alpha = 6, float sigma = 4) {
        floatarray dx, dy;
        elastic_transform_map(dx, in.dim(0), in.dim(1), alpha, sigma);
        elastic_transform_map(dy, in.dim(0), in.dim(1), alpha, sigma);
        makelike(out, in);
        for(int x = 0; x < in.dim(0); x++) {
            for(int y = 0; y < in.dim(1); y++) {
                float nx = x + dx(x,y);
                float ny = y + dy(x,y);
                int x0 = min(max(int(floor(nx)), 0), in.dim(0) - 1);
                int y0 = min(max(int(floor(ny)), 0), in.dim(1) - 1);
                int x1 = min(x0 + 1, in.dim(0) - 1);
                int y1 = min(y0 + 1, in.dim(1) - 1);
                float xx = nx - x0;
                float yy = ny - y0;
                float co_00 = (1 - xx) * (1 - yy);
                float co_01 = (1 - xx) * yy;
                float co_10 = xx * (1 - yy);
                float co_11 = xx * yy;
                float in_00 = in(x0,y0);
                float in_01 = in(x0,y1);
                float in_10 = in(x1,y0);
                float in_11 = in(x1,y1);
                out(x,y)=co_00*in_00 + co_01*in_01 + co_10*in_10 + co_11*in_11; 
            }
        }
        if(logger.enabled) {
            bytearray tmp; copy(tmp, out);
            logger("after elastic transform", tmp);
        }
    }

    void jitter(floatarray &out, floatarray &in, float mean, float sigma) {
        float delta_x=0.0;
        float delta_y=0.0;
        int x1,x2,y1,y2;
        float co_11,co_22,co_12,co_21;
        float val_11,val_22,val_12,val_21;
        
        makelike(out, in);
        fill(out, 255);

        for(int i=2;i<out.dim(0)-2;i++) {
            for(int j=2;j<out.dim(1)-2;j++) {
                delta_x=rand_gauss(mean,sigma);
                delta_y=rand_gauss(mean,sigma);
                x1=i;
                y1=j;
                x2=(delta_x>0)?i+1:i-1;
                y2=(delta_y>0)?j+1:j-1;

    //              x2=int((delta_x>0)?ceil(delta_x):floor(delta_x))+i;
    //              y2=int((delta_y>0)?ceil(delta_y):floor(delta_y))+j;

                co_11 = (1-delta_x)*(1-delta_y);
                co_12 = delta_x*(1-delta_y);
                co_21 = (1-delta_x)*delta_y;
                co_22 = delta_x*delta_y;

                val_11 = in(x1,y1); 
                val_12 = in(x1,y2);
                val_21 = in(x2,y1);
                val_22 = in(x2,y2);

                out(i,j)=co_11*val_11 + co_21*val_21 + co_12*val_12 + co_22*val_22;     
            }
        }
    }
    
    void adjust_sensitivity(floatarray &a, double mean, double sigma) {
        for(int i = 0; i < a.length1d(); i++)
            a.at1d(i) -= 255 * rand_gauss(mean,sigma);
    }

    void threshold(floatarray &a, double mean, double sigma) {
        double threshold;
        for(int i = 0; i < a.length1d(); i++) {
            threshold = 255 - 255 * rand_gauss(mean,sigma);
            a.at1d(i) = (a.at1d(i) <= threshold  ?  0  : 255);
        }
    }

}

namespace ocropus {

    void degrade(bytearray &image,
                 double jitter_mean,
                 double jitter_sigma,
                 double sensitivity_mean,
                 double sensitivity_sigma,
                 double threshold_mean,
                 double threshold_sigma) {
        floatarray a;
        copy(a, image);
        floatarray elastic;
        elastic_transform(elastic, a);
        jitter(a, elastic,jitter_mean,jitter_sigma);
        adjust_sensitivity(a, sensitivity_mean, sensitivity_sigma);
        if(threshold_mean)
            threshold(a, threshold_mean, threshold_sigma);
        copy(image, a);
    }

}
