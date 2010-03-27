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
// File: ocr-doc-clean-concom.cc
// Purpose: Document image cleanup by filtering large connected components.
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout.h"

// FIXME/faisal this dependency violates coding conventions --tmb
#include "ocr-char-stats.h"

using namespace iulib;
using namespace colib;

namespace ocropus {

    struct DocCleanConComp : ICleanupBinary {
        ~DocCleanConComp() {}

        const char *description() {
            return "Remove large components from the image\n";
        }

        void init(const char **argv) {
            // nothing to be done
        }

        const char *name() {
            return "doccleancon";
        }


        void cleanup(bytearray &out,bytearray &in) {

            int page_width = in.dim(0);
            int page_height = in.dim(1);

            intarray charimage;
            makelike(charimage,in);
            for(int i=0,l=in.length1d(); i<l; i++)
                charimage.at1d(i) = !in.at1d(i);
            label_components(charimage,false);

            // Clean non-text and noisy boxes and get character statistics
            rectarray bboxes;
            bounding_boxes(bboxes,charimage);
            ASSERT(bboxes.length()!=0);

            // get char stats
            autodel<CharStats> charstats(make_CharStats());
            charstats->getCharBoxes(bboxes);

            int dotlength = charstats->dot_boxes.length();
            int charlength = charstats->char_boxes.length();

            makelike(out,in);
            fill(out,255);
            for(int i=0; i<dotlength; i++){
                rectangle b = charstats->dot_boxes[i];
                b.x1=min(b.x1,page_width-1);
                b.y1=min(b.y1,page_height-1);
                for(int x=b.x0; x<=b.x1; x++)
                    for(int y=b.y0; y<=b.y1; y++)
                        out(x,y) = in(x,y);
            }

            for(int i=0; i<charlength; i++){
                rectangle b = charstats->char_boxes[i];
                b.x1=min(b.x1,page_width-1);
                b.y1=min(b.y1,page_height-1);
                for(int x=b.x0; x<=b.x1; x++)
                    for(int y=b.y0; y<=b.y1; y++)
                        out(x,y) = in(x,y);
            }
        }
    };

    ICleanupBinary *make_DocCleanConComp() {
        return new DocCleanConComp();
    }
}

