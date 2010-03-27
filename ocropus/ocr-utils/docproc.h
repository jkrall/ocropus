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
// File: docproc.h
// Purpose: miscelaneous routines
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de


#ifndef docproc_h_
#define docproc_h_

#include "ocropus.h"

namespace ocropus {
    using namespace colib;

    void count_neighbors(colib::bytearray &result,colib::bytearray &image);
    void find_endpoints(colib::bytearray &result,colib::bytearray &image);
    void find_junctions(colib::bytearray &result,colib::bytearray &image);
    void remove_singular_points(colib::bytearray &image,int d);

    void invert(bytearray &a);

    void crop_masked(bytearray &result,
                     bytearray &source,
                     rectangle box,
                     bytearray &mask,
                     int default_value,
                     int padding = 0);

    int average_on_border(bytearray &a);

    inline bool background_seems_black(bytearray &a) {
        return average_on_border(a) <= (min(a) + max(a) / 2);
    }

    inline bool background_seems_white(bytearray &a) {
        return average_on_border(a) >= (min(a) + max(a) / 2);
    }

    void optional_check_background_is_darker(bytearray &a);

    void optional_check_background_is_lighter(bytearray &a);

    inline void make_background_white(bytearray &a) {
        if(!background_seems_white(a))
            invert(a);
    }
    inline void make_background_black(bytearray &a) {
        if(!background_seems_black(a))
            invert(a);
    }

    // this is already defined in narray... shift_by --tmb not quite,
    // shift_by fills the background, and this leaves it intact --IM
    /// Copy the `src' to `dest',
    /// moving it by `shift_x' to the right and by `shift_y' up.
    /// The image must fit.

    void blit2d(bytearray &dest,
                const bytearray &src,
                int shift_x = 0,
                int shift_y = 0) DEPRECATED;

    // FIXME/tmb move into narray-util --tmb
    // float median(intarray &a) DEPRECATED;

    // FIXME/tmb explain what method this uses --tmb
    /// Estimate xheight given a slope and a segmentation.
    /// (That's an algorithm formerly used together with MLP).

    float estimate_xheight(intarray &seg, float slope) DEPRECATED;

    void plot_hist(FILE *stream, floatarray &hist);

    void get_line_info(float &baseline,
                       float &xheight,
                       float &descender,
                       float &ascender,
                       intarray &seg);

    // remove small connected components (really need to add more
    // general marker code to the library)

    template <class T>
    void remove_small_components(narray<T> &bimage,int mw,int mh);
    template <class T>
    void remove_marginal_components(narray<T> &bimage,int x0,int y0,int x1,int y1);

    /// Split a string into a list using the given array of delimiters.
    void split_string(narray<iucstring> &components,
                      const char *path,
                      const char *delimiters);
    int binarize_simple(bytearray &result, bytearray &image);
    int binarize_simple(bytearray &image);
    void binarize_with_threshold(bytearray &out, bytearray &in, int threshold);
    void binarize_with_threshold(bytearray &in, int threshold);

    void runlength_histogram(floatarray &hist,
                             bytearray &img,
                             rectangle box,
                             bool white=false,bool vert=false);

    inline void runlength_histogram(floatarray &hist,
                                    bytearray &image,
                                    bool white=false,
                                    bool vert=false) {
        runlength_histogram(hist,image,
                            rectangle(0,0,image.dim(0),image.dim(1)),
                            white,vert);
    }

    // why is this a public function? make local function somewhere. --tmb
    // this is a public function because it's used from a Lua script,
    // namely heading detector. -- IM
    int find_median_in_histogram(floatarray &);

    // this function should not be used as part of regular software; the
    // layout analysis should yield "cleans" text lines.  --tmb
    // This function is sometimes useful when you need to deal with a dataset
    // that was not put through our layout analysis. --IM

    void remove_neighbour_line_components(bytearray &line);

    template<class T>
    void rotate_90(narray<T> &out, narray<T> &in);
    template<class T>
    void rotate_180(narray<T> &out, narray<T> &in);
    template<class T>
    void rotate_270(narray<T> &out, narray<T> &in);
}

#endif
