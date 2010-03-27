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
// File: ocr-text-image-seg-leptonica.cc
// Purpose: Wrapper class for getting non-text mask from leptonica
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "ocrinterfaces.h"

using namespace colib;

#ifndef HAVE_LEPTONICA

namespace ocropus {
    ITextImageClassification *make_TextImageSegByLeptonica(){
        throw "Leptonica is disabled, please compile with it or don't use it!";
    }
}

#else

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "iulib/imgio.h"
#include "iulib/imglib.h"
#include "allheaders.h" // leptonica image class

#include "ocr-text-image-seg-leptonica.h"

using namespace iulib;

namespace ocropus {

    void TextImageSegByLeptonica::getLeptNonTextMask(bytearray &out,
                                                     bytearray &in){
        makelike(out,in);
        fill(out,255);
        PIX *pix;
        int w = in.dim(0);
        int h = in.dim(1);
        int d = 8;

        if ((pix = pixCreate(w, h, d)) == NULL){
            fprintf(stderr,"PIX structure could not be made.\n");
            exit(1);
        }

        unsigned int *data = pixGetData(pix);
        int wpl = pixGetWpl(pix);
        int bpl = (d * w + 7) / 8;
        unsigned int *line;
        unsigned char val8;

        for(int y=0; y<h; y++) {
            line = data + y * wpl;
            for(int x=0; x<bpl; x++) {
                val8 = in(x,h-y-1);
                SET_DATA_BYTE(line,x,val8);
            }
        }

        PIX *pixb, *pixseed4, *pixmask4, *pixsf4, *pixd4, *pixd;
        int thresh = 128;
        pixb = pixThresholdToBinary(pix, thresh);

        /* Mask at 4x reduction */
        static const char *mask_sequence = "r11";
        /* Seed at 4x reduction, formed by doing a 16x reduction,
         * an opening, and finally a 4x replicative expansion. */
        static const char *seed_sequence = "r1143 + o5.5+ x4";
        /* Simple dilation */
        static const char *dilation_sequence = "d3.3";

        /* Make seed and mask, and fill seed into mask */
        pixseed4 = pixMorphSequence(pixb, seed_sequence, 0);
        pixmask4 = pixMorphSequence(pixb, mask_sequence, 0);
        pixsf4 = pixSeedfillBinary(NULL, pixseed4, pixmask4, 8);
        pixd4 = pixMorphSequence(pixsf4, dilation_sequence, 0);

        /* Mask at full resolution */
        pixd = pixExpandBinaryPower2(pixd4, 4);

        /* Write pix back to bytearray*/
        pixGetDimensions(pixd, &w, &h, &d);
        unsigned int *datad, *lined;
        datad = pixGetData(pixd);
        int wpld = pixGetWpl(pixd);
        //fprintf(stderr,"bpl = %d wpld = %d\n",bpl,wpld);

        for(int y=0; y<h; y++) {
            lined = datad + y * wpld;
            for(int x=0; x<w; x++)
                out(x,h-y-1) = 255*!GET_DATA_BIT(lined,x);
        }


        //pixWriteStreamPnm(fopen("maske.pgm","w"),pixd);

        pixDestroy(&pix);
        pixDestroy(&pixb);
        pixDestroy(&pixseed4);
        pixDestroy(&pixmask4);
        pixDestroy(&pixsf4);
        pixDestroy(&pixd4);
        pixDestroy(&pixd);

    }

    // Get text-image map from a binary image
    void TextImageSegByLeptonica::textImageProbabilities(intarray &out,
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

        bytearray nontext_mask;
        getLeptNonTextMask(nontext_mask,in_binary);
        //write_image_gray("nontext-mask.png",nontext_mask);
        ASSERT(in_binary.length1d()==nontext_mask.length1d());
        makelike(out,in_binary);
        int w = in_binary.dim(0);
        int h = in_binary.dim(1);
        for(int x=0; x<w; x++){
            for(int y=0; y<h; y++){
                if(!nontext_mask(x,y))
                    out(x,y) = 0x0000ff00;
                else if(!in_binary(x,y))
                    out(x,y) = 0x00ff0000;
                else
                    out(x,y) = 0x00ffffff;
            }
        }
    }

    ITextImageClassification *make_TextImageSegByLeptonica(){
        return new TextImageSegByLeptonica();
    }

}

#endif
