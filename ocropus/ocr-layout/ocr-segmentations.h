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
// File: ocr-segmentations.h
// Purpose: miscelaneous routines
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

/// \file ocr-segmentations.h
/// \brief Miscelaneous routines

#ifndef h_ocr_segmentations_
#define h_ocr_segmentations_

#include "ocropus.h"
#include "ocr-layout.h"

namespace ocropus {
    using namespace colib;
    using namespace iulib;

    /// Assert that all items of this array of arrays are not empty.
    template<class T>
    void assert_all_items_nonempty(narray<narray<T> > &a) {
        for(int i=0;i<a.length();i++)
            ASSERT(a[i].length1d() > 0);
    }

    /// Remove from the segmentation those pixels which are white in gray_image.
    void binarize_in_segmentation(intarray &segmentation, /* const */ bytearray &gray_image);

    /// Set line number for all foreground pixels in a character segmentation.
    void set_line_number(intarray &a, int lnum);


    /// Unpack page segmentation into separate line masks with bounding boxes.
    void extract_lines(narray<bytearray> &lines,narray<rectangle> &rboxes,intarray &image);

    /// If the line is too small or too large, rescale it (with the mask)
    /// to a decent height (30-60 pixels).
    void rescale_if_needed(bytearray &bin_line, bytearray &gray_line);

    /// Make a binary image from a line segmentation.
    void forget_segmentation(bytearray &image, intarray &segmentation);

    /// Returns true if there's a mapping between s1's colors and s2's colors.
    bool is_oversegmentation_of(intarray &s1, intarray &s2);

    /// Return true if there are no zeros in the array.
    bool has_no_black_pixels(intarray &);

    void blit_segmentation_line(intarray &page,
                                rectangle bbox,
                                intarray &line,
                                int line_no);

    /// Blit the segmentation of src onto dst shifted by (x,y) and shifted by
    /// values by max(dst).
    void concat_segmentation(intarray &dst, intarray &src,
                             int x, int y);

    // Enlarge segmentation and AND it with line_mask.
    // Don't pass binarized grayscale image as line_mask,
    // otherwise you might get debris not from the line.
    // (that means we cannot really call this from inside LineOCR)
    void normalize_segmentation(intarray &segmentation, bytearray &line_mask);

    int max_cnum(intarray &seg);

    void get_recoloring_map(intarray &recolor, intarray &image);
    void remove_gaps_by_recoloring(intarray &image);

    void ocr_bboxes_to_charseg(intarray &cseg,narray<rectangle> &bboxes,intarray &segmentation);
    void evaluate_segmentation(int &nover,int &nunder,int &nmis,
                               intarray &model_raw,intarray &image_raw,float tolerance);
    void align_segmentation(intarray &segmentation,narray<rectangle> &bboxes);

}

#endif
