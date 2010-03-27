// -*- C++ -*-

// Copyright 2006-2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz 
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
// File: 
// Purpose: 
// Responsible: mezhirov
// Reviewer: 
// Primary Repository: 
// Web Sites: 


#include "colib/colib.h"
#include "imgio.h"
#include "imglib.h"
#include "ocr-utils.h"
#include "ocr-segmentations.h"
#include "tesseract.h"

using namespace ocropus;
using namespace colib;
using namespace iulib;


void test_ocr_result(nustring &ocr_result, const char *gt) {
    int gt_len = strlen(gt);
    for(int i = 0; i < ocr_result.length(); i++) {
        if(i >= gt_len || ocr_result[i].ord() != gt[i]) {
            fprintf(stderr, "mistaken char #%d for %lc in %s", i, ocr_result[i].ord(), gt);
        }
    }
}

static void test_line_ocr(ISimpleLineOCR &ocr) {
    stdio filelist("test-list", "r");
    char in_path[1000];
    while(fscanf(filelist, "%999s", in_path) == 1) {
        char gt_path[1000];
        fscanf(filelist, "%999s", gt_path);
        char gt[1000];
        stdio gt_file(gt_path, "r");
        fgets(gt, sizeof(gt), gt_file);
        bytearray image;
        nustring result;
        floatarray costs;
        narray<rectangle> bboxes;
        read_image_gray(image, in_path);
        ocr.recognize_gray(result, costs, bboxes, image);
        test_ocr_result(result, gt);
        binarize_by_threshold(image);
        ocr.recognize_binary(result, costs, bboxes, image);
        test_ocr_result(result, gt);
    }
}

int main(int argc, char **argv) {
    autodel<ISimpleLineOCR> ocr(make_tesseract(argv[0]));
    ocr->description();
    intarray alice;
    read_png_rgb(alice, stdio("alice.png", "rb"));
    bytearray alice_bin;
    forget_segmentation(alice_bin, alice);
    nustring alice_text(5);
    alice_text[0] = nuchar('A');
    alice_text[1] = nuchar('l');
    alice_text[2] = nuchar('i');
    alice_text[3] = nuchar('c');
    alice_text[4] = nuchar('e');
/*    CHECK_CONDITION(ocr->supports_char_training() && !ocr->supports_line_training());
    try{ocr->train_binary(alice_text, alice_bin); CHECK_CONDITION(0);} catch (...) {}
    try{ocr->train_gray  (alice_text, alice_bin); CHECK_CONDITION(0);} catch (...) {}*/
    test_line_ocr(*ocr);
/*
    ocr->train_binary_chars(alice_text, alice);
    ocr->train_gray_chars(alice_text, alice, alice_bin);
    test_line_ocr(*ocr);*/
}
