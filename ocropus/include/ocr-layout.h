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
// File: ocr-layout.h
// Purpose: Top level public header file for including layout analysis
//          functionality
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef ocr_layout_h__
#define ocr_layout_h__

namespace ocropus {
    using namespace colib;

    // Document cleanup by removing border noise
    ICleanupBinary *make_DocClean();
    ICleanupBinary *make_PageFrameRAST();

    // Deskew page by RAST
    ICleanupBinary *make_DeskewPageByRAST();
    ICleanupGray *make_DeskewGrayPageByRAST();

    // Get skew angle of the page using RAST
    double estimate_skew_by_rast(bytearray &in);

    // Text/Image segmentation
    ICleanupBinary *make_RemoveImageRegionsBinary();
    ICleanupGray   *make_RemoveImageRegionsGray();
    ITextImageClassification *make_TextImageSegByLogReg();
    ITextImageClassification *make_TextImageSegByLeptonica();

    // Remove a masked region from an input image
    void remove_masked_region(bytearray &out,
                              bytearray &mask,
                              bytearray &in);

    // Remove rectangular regions from an input image
    void remove_rectangular_region(bytearray &out,
                                   rectarray &boxes,
                                   bytearray &in);

    // get a binary mask image for non-text regions from a text/image
    // probability map
    void get_nontext_mask(bytearray &out, intarray &in);

    // get non-text rectangles from a text/image probability map
    void get_nontext_boxes(rectarray &nontext_boxes,
                           intarray &text_img_map);

    // Page segmentation and layout analysis
    ISegmentPage *make_SegmentPageByMorphTrivial();
    ISegmentPage *make_SegmentPageBy1CP();

    /// Segment page using RAST.
    ///
    /// @param max_results maximum number of lines to be extracted
    /// @param gap_factor distance between consecutive characters at which a
    ///                   textline should be split: gap_factor*xheight
    /// @param use_four_line_model use four line model for line finding
    ISegmentPage *make_SegmentPageByRAST();

    ISegmentPage *make_SegmentPageByVORONOI();
    ISegmentPage *make_SegmentPageByXYCUTS();
    ISegmentPage *make_SegmentWords();

    // Textline extraction using RAST
    
    void visualize_segmentation_by_RAST(intarray &result, 
                                        bytearray &in_not_inverted);
    void visualize_segmentation_by_RAST(intarray &result, 
                                        bytearray &in_not_inverted,
                                        rectarray &extra_obstacles);
    void ocr_bboxes_to_charseg(intarray &cseg,
                               rectarray &bboxes,
                               intarray &segmentation);

}
#endif
