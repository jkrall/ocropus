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
// File: ocr-text-image-seg.cc
// Purpose: Wrapper class for document zone classification.
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace colib;
using namespace iulib;

namespace ocropus {

    param_bool debug_tiseg("debug_tiseg",0,"output intermediate images");

    // get a binary mask image for non-text regions from a text/image
    // probability map
    void get_nontext_mask(bytearray &out, intarray &in){
        makelike(out,in);
        for(int i=0,l=in.length1d(); i<l; i++){
            int red  = (in.at1d(i)&0x00ff0000)>>16;
            int green= (in.at1d(i)&0x0000ff00)>>8;
            int blue = (in.at1d(i)&0x000000ff);
            if( (green>red) || (blue>red))
                out.at1d(i)=0;
            else
                out.at1d(i)=255;
        }
    }

    // get non-text rectangles from a text/image probability map
    void get_nontext_boxes(rectarray &nontext_boxes, intarray &text_img_map){

        bytearray nontext_mask;
        get_nontext_mask(nontext_mask,text_img_map);
        for(int i=0,l=nontext_mask.length1d(); i<l; i++)
            nontext_mask.at1d(i)=255-nontext_mask.at1d(i);
        intarray concomps;
        copy(concomps,nontext_mask);
        label_components(concomps);
        rectarray bboxes;
        bounding_boxes(bboxes,concomps);
        for(int i=1,l=bboxes.length(); i<l; i++)
            if(bboxes[i].area())
                nontext_boxes.push(bboxes[i]);
    }


    // Remove a masked region from an input image
    void remove_masked_region(bytearray &out, bytearray &mask, bytearray &in){

        makelike(out,in);
        ASSERT(in.length1d()==mask.length1d());
        for(int i=0,l=in.length1d(); i<l; i++){
            if(mask.at1d(i))
                out.at1d(i)=in.at1d(i);
            else
                out.at1d(i)=255;
        }
    }

    // Remove rectangular regions from an input image
    void remove_rectangular_region(bytearray &out,
                                   rectarray &bboxes,
                                   bytearray &in){
        copy(out,in);
        int image_width   = in.dim(0);
        int image_height  = in.dim(1);
        int x0, y0, x1, y1, xi, yi;
        for (int i = 0; i < bboxes.length(); i++){
            if(!bboxes[i].area() || bboxes[i].area()>=image_width*image_height)
                continue;
            x0 = ( bboxes[i].x0 > 0 ) ? bboxes[i].x0 : 0;
            y0 = ( bboxes[i].y0 > 0 ) ? bboxes[i].y0 : 0;
            x1 = ( bboxes[i].x1 < image_width)  ? bboxes[i].x1 : image_width-1;
            y1 = ( bboxes[i].y1 < image_height) ? bboxes[i].y1 : image_height-1;
            if(x1<=x0 || y1<=y0)
                continue;

            for (xi = x0; xi <= x1; xi++){
                for (yi = y0; yi <= y1; yi++){
                    out(xi, yi) = 255;
                }
            }
        }
    }

    int TextImageSegByLogReg::getColor(floatarray &prob_map, int index){

        // treat class "text" and "table" as text
        //       class "math", "ruling", and "drawing" as non-text graphics
        //       class "logo", "noise", and "halftone" as images
        //

        // Each RGB triple represents three posterior probabilities:

        //  * R: probability * 255 that the pixel belongs to a printed
        //       character
        //  * G: probability * 255 that the pixel belongs to a continuous
        //       tone image
        //  * B: probability * 255 that the pixel belongs to non-text
        //       graphics
        //  * 255-R-G-B: probability that the pixel belongs to the background
        //  * 0-R-G-B: un-classified pixels, or pixels not belonging to any
        //       of the above categories


        byte red = byte(255*(max(prob_map(index,text),
                                 prob_map(index,table))));

        byte green = byte(255*(max(prob_map(index,halftone),
                                   max(prob_map(index,logo),
                                       prob_map(index,noise)))));

        byte blue = byte(255*(max(prob_map(index,drawing),
                                  max(prob_map(index,math),
                                      prob_map(index,ruling)))));

        int color = (red<<16)|(green<<8)|blue;
        return color;
    }

    void TextImageSegByLogReg::getProbabilityMap(floatarray &class_prob,
                                                 rectarray &bboxes,
                                                 bytearray &image){

        autodel<LogReg> logistic_regression(make_LogReg());
        autodel<ZoneFeatures> zone_features(make_ZoneFeatures());

        logistic_regression->loadData();
        class_prob.resize(bboxes.length(),logistic_regression->class_num);
        fill(class_prob,-1);

        int x0, y0, x1, y1, xi, yi;
        floatarray feature;
        int image_width   = image.dim(0);
        int image_height  = image.dim(1);
        bytearray image_tmp;
        for (int i = 0; i < bboxes.length(); i++){
            if(!bboxes[i].area() || bboxes[i].area()>=image_width*image_height)
                continue;
            //bboxes[i].println();
            x0 = ( bboxes[i].x0 > 0 ) ? bboxes[i].x0 : 0;
            y0 = ( bboxes[i].y0 > 0 ) ? bboxes[i].y0 : 0;
            x1 = ( bboxes[i].x1 < image_width)  ? bboxes[i].x1 : image_width-1;
            y1 = ( bboxes[i].y1 < image_height) ? bboxes[i].y1 : image_height-1;
            if(x1<=x0 || y1<=y0)
                continue;

            image_tmp.resize(x1 - x0 + 1, y1 - y0 + 1);
            for (xi = x0; xi <= x1; xi++){
                for (yi = y0; yi <= y1; yi++){
                    image_tmp(xi - x0, yi - y0) = image(xi, yi);
                }
            }

            feature.clear();
            zone_features->extractFeatures(feature, image_tmp);

            //logistic regression
            floatarray probs;
            logistic_regression->getClassProbabilities(probs,feature);
            ASSERT(probs.length()==class_prob.dim(1));
            for(int j=0; j<probs.length(); j++)
                class_prob(i,j) = probs[j];
        }
    }

    void TextImageSegByLogReg::textImageProbabilities(intarray &out,
                                                      intarray &in){
        // FIXME/faisal this must accept grayscale image (it can threshold them
        // internally using one of the standard methods if it likes) --tmb
        rectarray bboxes;
        bounding_boxes(bboxes,in);
        makelike(out,in);
        fill(out,0x00ffffff);

        int image_width   = in.dim(0);
        int image_height  = in.dim(1);
        bytearray image_bin;
        makelike(image_bin,in);
        for (int x=0; x<image_width; x++){
            for (int y=0; y<image_height ; y++){
                if(in(x,y) == 0x00ffffff)
                    image_bin(x,y) = 255;
                else
                    image_bin(x,y) = 0;
            }
        }

        floatarray prob_map;
        getProbabilityMap(prob_map, bboxes, image_bin);

        int x0, y0, x1, y1;
        for (int i = 0; i < bboxes.length(); i++){
            if(prob_map(i,0) == -1)
                continue;
            x0 = ( bboxes[i].x0 > 0 ) ? bboxes[i].x0 : 0;
            y0 = ( bboxes[i].y0 > 0 ) ? bboxes[i].y0 : 0;
            x1 = ( bboxes[i].x1 < image_width)  ? bboxes[i].x1 : image_width-1;
            y1 = ( bboxes[i].y1 < image_height) ? bboxes[i].y1 : image_height-1;
            if(x1<=x0 || y1<=y0)
                continue;

            int color = getColor(prob_map, i);

            for(int x=x0;x<=x1;x++){
                for(int y=y0;y<=y1;y++){
                        out(x,y)=color;
                }
            }
        }

    }

    void TextImageSegByLogReg::textImageProbabilities(intarray &out,
                                                      bytearray &in){
        // get a binary image
        bytearray in_binary;
        if(!contains_only(in,byte(0),byte(255))){
            autodel<IBinarize> binarize(make_BinarizeByOtsu());
            makelike(in_binary,in);
            binarize->binarize(in_binary,in);
        }else{
            copy(in_binary,in);
        }

        intarray image;
        autodel<ISegmentPage> segmenter(make_SegmentPageByXYCUTS(10,10,20,20));
        segmenter->segment(image,in_binary);
        textImageProbabilities(out,image);
        if(debug_tiseg){
            rectarray bboxes;
            bounding_boxes(bboxes,image);
            for (int i = 0; i < bboxes.length(); i++){
                if(!bboxes[i].area() || bboxes[i].area()>=image.dim(0)*image.dim(1))
                    continue;
                paint_box_border(image,bboxes[i],0x00ff0000);

                simple_recolor(image);
                write_image_packed("segmentation.png",image);
                write_image_packed("text-image-probs.png",out);
            }
        }

    }

    ITextImageClassification *make_TextImageSegByLogReg() {
        return new TextImageSegByLogReg();
    }

    void RemoveImageRegions::cleanup(bytearray &out, bytearray &in) {

        // get a binary image
        bytearray in_binary;
        if(!contains_only(in,byte(0),byte(255))){
            autodel<IBinarize> binarize(make_BinarizeByOtsu());
            makelike(in_binary,in);
            binarize->binarize(in_binary,in);
        }else{
            copy(in_binary,in);
        }

        // get text/image probability map
        intarray tiseg_image;
        autodel<ITextImageClassification> tiseg(make_TextImageSegByLeptonica());
        tiseg->textImageProbabilities(tiseg_image,in_binary);

        // Cleanup
        bytearray nontext_mask;
        get_nontext_mask(nontext_mask,tiseg_image);
        remove_masked_region(out,nontext_mask,in);

        if(debug_tiseg){
            write_image_gray("nontext-mask.png",nontext_mask);
        }
    }

    ICleanupBinary *make_RemoveImageRegionsBinary() {
        return new RemoveImageRegions();
    }


    ICleanupGray *make_RemoveImageRegionsGray() {
        return new RemoveImageRegions();
    }

}

