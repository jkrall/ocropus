// -*- C++ -*-

// Copyright 2006-2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Project:
// File:
// Purpose:
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites:


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "colib/colib.h"
#include "iulib/imgio.h"
#include "iulib/imglib.h"
#include "segmentation.h"

using namespace ocropus;
using namespace iulib;
using namespace colib;

namespace ocropus {
    // FIXME/faisal what is the difference between this and SegmentLineByCCS? --tmb
    class ConnectedComponentSegmenter : public ISegmentLine {
        virtual const char *description() {
            return "connected component segmenter";
        }
        const char *name() {
            return "segccs";
        }
        virtual void init(const char **argv=0) {
        }

        virtual void charseg(intarray &segmentation,bytearray &image) {
                bytearray temp_image;
                copy(temp_image, image);
                binary_autoinvert(temp_image);
            copy(segmentation,temp_image);
            label_components(segmentation);
        }
    };
}

ISegmentLine *ocropus::make_ConnectedComponentSegmenter() {
    return new ConnectedComponentSegmenter();
}
