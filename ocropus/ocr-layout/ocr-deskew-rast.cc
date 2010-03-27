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
// File: ocr-deskew-rast.cc
// Purpose: perform skew correction using RAST
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace ocropus;
using namespace iulib;
using namespace colib;

namespace ocropus {

#define RAD_TO_DEG 57.3

    param_string debug_deskew("debug_deskew", 0,
                              "output deskewed document image as png");

    double estimate_skew_by_rast(colib::bytearray &in){
        autodel<DeskewPageByRAST> deskewer(new DeskewPageByRAST());
        return deskewer->getSkewAngle(in);
    }

    double DeskewPageByRAST::getSkewAngle(bytearray &in) {
        // Binarize if input image is grayscale
        bytearray binarized;
        makelike(binarized, in);
        autodel<IBinarize> binarizer(make_BinarizeBySauvola());
        if(contains_only(in, byte(0), byte(255))) {
            copy(binarized, in);
        } else {
            binarizer->binarize(binarized,in);
        }

        // Do connected component analysis
        intarray charimage;
        copy(charimage, binarized);
        make_page_binary_and_black(charimage);
        label_components(charimage, false);
        rectarray bboxes;
        bounding_boxes(bboxes, charimage);
        return getSkewAngle(bboxes);
    }

    double DeskewPageByRAST::getSkewAngle(rectarray &bboxes) {
        // Clean non-text and noisy boxes and get character statistics
        autodel<CharStats> charstats(make_CharStats());
        charstats->getCharBoxes(bboxes);
        charstats->calcCharStats();

        // Extract textlines
        autodel<CTextlineRAST> ctextline(make_CTextlineRAST());
        narray<TextLine> textlines;
        ctextline->max_results=1;
        ctextline->min_gap = int(charstats->word_spacing*1.5);
        ctextline->setMaxSlope(0.5);
        ctextline->extract(textlines, charstats);
        if(textlines.length())
            return atan(textlines[0].m);
        else{
            fprintf(stderr,"Warning: no textlines found. ");
            fprintf(stderr,"Skipping deskewing ...\n");
            return 0;
        }
    }

    void DeskewPageByRAST::cleanup(bytearray &image, bytearray &in) {
        makelike(image, in);
        float angle = (float) getSkewAngle(in);
        float cx = image.dim(0)/2.0;
        float cy = image.dim(1)/2.0;
        if(contains_only(in, byte(0), byte(255)))
            rotate_direct_sample(image, in, angle, cx, cy);
        else
            rotate_direct_interpolate(image, in, angle, cx, cy);
        if(debug_deskew) {
            fprintf(stderr, "Skew angle found = %.3f degrees\n", angle*RAD_TO_DEG);
            write_png(stdio(debug_deskew, "w"), image);
        }

    }

    ICleanupBinary *make_DeskewPageByRAST() {
        return new DeskewPageByRAST();
    }
    ICleanupGray *make_DeskewGrayPageByRAST() {
        return new DeskewPageByRAST();
    }

}
