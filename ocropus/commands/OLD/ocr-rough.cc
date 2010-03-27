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
// Project: roughocr -- mock OCR system exercising the interfaces and useful for testing
// File: ocr-rough.cc
// Purpose: perform rough OCR (really just a mockup)
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "colib/colib.h"
#include "iulib/iulib.h"
#include "ocropus.h"

using namespace ocropus;
using namespace colib;

param_string debug_lines("debug_lines",0,"output each line, as found by the segmenter");
param_string debug_segm("debug_segm",0,"output segmentation file");

void extract_lines(intarray &pixels,narray<bytearray> &lines,intarray &image,narray<rectangle> &rboxes) {
    int minwidth = 10, minheight = 10;
    int padding = 3;

    inthash<rectangle> bboxes;
    for(int i=0;i<image.dim(0);i++) for(int j=0;j<image.dim(1);j++) {
        int pixel = image(i,j);
        rectangle &r = bboxes(pixel);
        r.include(i,j);
    }
    intarray keys;
    bboxes.keys(keys);
    remove_value(keys,0);
    quicksort(keys);
    pixels.resize(keys.length());
    lines.resize(keys.length());
    rboxes.resize(keys.length());
    for(int ki=0;ki<keys.length();ki++) {
        int key = keys[ki];
        rectangle box = bboxes(key);
        rboxes[ki] = box;
        if(box.width()<minwidth || box.height()<minheight) continue;
        intarray line(box.width(),box.height());
        // FIXME/tmb add greyscale extraction here --???
        for(int i=0;i<box.width();i++) for(int j=0;j<box.height();j++) {
            line(i,j) = 0xffffff * !!image(i+box.x0,j+box.y0);
        }
        pad_by(line,padding,padding,0);
        pixels[ki] = key;
        copy(lines[ki],line);
    }
}

void extract_components(objlist<bytearray> &components,intarray &image) {
    narray<rectangle> bboxes;
    bounding_boxes(bboxes,image);
    for(int i=1;i<bboxes.length();i++) {
        rectangle r = bboxes(i);
        bytearray temp;
        extract_subimage(temp,image,r.x0,r.y0,r.x1,r.y1);
        for(int i=0;i<temp.length1d();i++)
            temp.at1d(i) = 255 * !!temp.at1d(i);
        move(components.push(),temp);
    }
}

int main(int argc,char **argv) {
    narray<strbuf> files;
    files.reserve(1000);
    if(argc>1) {
        for(int i=1;i<argc;i++) {
            files.push() = argv[i];
        }
    } else {
        char buf[10000];
        while(fgets(buf,sizeof buf,stdin)) {
            files.push() = buf;
        }
    }
    printf("<html>\n");
    printf("<head>\n");
    printf("    <meta name='ocr-id' value='rocr-0.0'>\n");
    printf("    <meta name='ocr-recognized' value='lines text'>\n");
    printf("</head>\n");
    printf("<body>\n");
    try {
        autodel<IBinarize> binarizer(make_BinarizeByRange());
        autodel<ISegmentPage> pageseg(make_SegmentPageBy1CP());
        autodel<ISegmentLine> lineseg(make_SegmentLineByProjection());
        autodel<ISegmentedLineOCR> lineocr(make_LineOCRTrivial());
        autodel<ILanguageModel> langmod(make_LanguageModelTrivial());

        langmod->start_context();

        for(int pageno=0;pageno<files.length();pageno++) {
            printf("<div class='ocr_page' title='file %s'>\n",+files[pageno]);
            bytearray image;
            read_png(image,stdio(argv[1],"r"),true);

            bytearray bimage;
            floatarray temp;
            copy(temp,image);
            binarizer->binarize(bimage,temp);
            autoinvert(bimage);
            temp.dealloc();

            intarray page;
            pageseg->segment(page,bimage);

            if(debug_segm) {
                intarray temp;
                copy(temp,page);
                simple_recolor(temp);
                write_png_rgb(stdio(debug_segm,"w"),temp);
            }

            intarray rgb;
            narray<bytearray> lines;
            narray<rectangle> bboxes;
            extract_lines(rgb,lines,page,bboxes);

            objlist<bytearray> components;
            for(int line=0;line<lines.length();line++) {
                intarray segline;

                lineseg->charseg(segline,lines[line]);

                if(debug_lines) {
                    char buf[1000];
                    intarray temp;
                    copy(temp,segline);
                    simple_recolor(temp);
                    sprintf(buf,debug_lines,line);
                    write_png_rgb(stdio(buf,"w"),temp);
                }

                idmap im;
                lineocr->recognize_binary(*langmod,im,segline);
                langmod->compute(1);

                if(langmod->nresults()<1) {
                    printf("    <!-- line with not output from langmod -->\n");
                    continue;
                }
                intarray ocr;
                floatarray costs;
                intarray ids;
                intarray states;
                langmod->nbest(ocr,costs,ids,states,0);

                char bbox[1000];
                rectangle r = bboxes[line];
                sprintf(bbox,"%d %d %d %d",r.x0,bimage.dim(1)-r.y1-1,r.x1,bimage.dim(1)-r.y0-1);
                char buf[1000];
                for(int i=0;i<ocr.length();i++) buf[i] = (char)ocr[i];
                buf[ocr.length()] = 0;
                printf("    <span class='ocr_line' title='bbox %s'>%s</span>\n",bbox,buf);
            }
            printf("</div>\n");
        }
        printf("</body>\n");
        printf("</html>\n");
    }
    catch(const char *oops) {
        fprintf(stderr,"oops: %s\n",oops);
        exit(1);
    }
}
