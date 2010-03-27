// -*- C++ -*-

// Copyright 2006 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites:

/// \file segmentation.h
/// \brief Some make_Xxx() functions for line segmenters

#ifndef h_segmentation_
#define h_segmentation_

#include "ocropus.h"

namespace ocropus {
    /// Extract the set of pixels with the given value and return it
    /// as a black-on-white image.
    inline void extract_segment(bytearray &result,
                                intarray &image,
                                int n) {
        makelike(result, image);
        fill(result, 255);
        for(int i = 0; i < image.length1d(); i++) {
            if(image.at1d(i) == n)
                result.at1d(i) = 0;
        }
    }

}

#endif

