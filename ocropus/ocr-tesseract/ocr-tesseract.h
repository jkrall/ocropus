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
// Project: tesseract-interface
// File: tesseract.h
// Purpose: interfaces to Tesseract
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

/// \file tesseract.h
/// \brief interfaces to Tesseract

#ifndef h_tesseract_
#define h_tesseract_

#include "ocropus.h"

namespace ocropus {

    /// \brief Make a Tesseract-based line recognizer.
    ///
    /// Tesseract is not very good in recognizing single lines,
    /// and blockwise recognition looks better.
    ///
    /// Note: Tesseract uses a lot of global variables,
    /// so different instances of this IRecognizeLine will train each other.
    IRecognizeLine *make_TesseractRecognizeLine();

    void tesseract_recognize_blockwise_and_dump(colib::bytearray &gray,
                                                colib::intarray &pageseg);

    /// \brief Recognize an image with Tesseract, returning detailed results.
    ///
    /// \param[out] text    An array of nustrings, one per line.
    /// \param[out] bboxes  Character bounding boxes, an array per line,
    ///                     corresponding to text.
    /// \param[out] costs   Costs (badness rating) for characters.
    ///                     The exact semantics is Tesseract-specific.
    /// \param[in] gray     Grayscale input image.
    /// \param[in] pseg     Page segmentation.
    void tesseract_recognize_blockwise_and_split_to_lines(
            colib::narray<colib::nustring> &text,
            colib::narray<colib::narray<colib::rectangle> > &bboxes,
            colib::narray<colib::floatarray> &costs,
            colib::bytearray &gray,
            colib::intarray &pseg);

    /// \brief A transport object responsible for keeping the Tesseract results.
    ///
    /// If Lua would be able to handle stuff like narray<narray<rectangle>>,
    /// we would use tesseract_recognize_blockwise_and_split_to_lines() instead
    /// of this.
    class RecognizedPage {
        colib::narray<colib::nustring>   line_texts;
        colib::narray<colib::floatarray> line_costs;
        colib::narray<colib::rectarray> line_bboxes;
        colib::iucstring desc;
        int w, h;
        colib::iucstring time_report;
    public:
        /// Get page width
        int width() {
            return w;
        }
        /// Get page height
        int height() {
            return h;
        }
        /// Set page width
        void setWidth(int width) {
            w = width;
        }
        /// Set page height
        void setHeight(int height) {
            h = height;
        }
        /// Get page description (like a filename)
        const char *description() {
            return desc;
        }
        /// Set page description
        void setDescription(const char *s) {
            desc = s;
        }
        /// Get time report
        const char *timeReport() {
            return time_report;
        }
        /// Set time report
        void setTimeReport(const char *s) {
            time_report = s;
        }
        /// Set the number of lines
        void setLinesCount(int n) {
            line_texts.resize(n);
            line_costs.resize(n);
            line_bboxes.resize(n);
        }
        /// Get the number of lines
        int linesCount() {
            return line_texts.length();
        }
        /// Set the text of the given line
        void setText(colib::nustring &s, int index) {
            colib::copy(line_texts[index], s);
        }
        /// Get the text of the given line
        void text(colib::nustring &s, int index) {
            colib::copy(s, line_texts[index]);
        }
        /// Set the cost array of the given line
        void setCosts(colib::floatarray &c, int index) {
            colib::copy(line_costs[index], c);
        }
        /// Get the cost array of the given line
        void costs(colib::floatarray &c, int index) {
            colib::copy(c, line_costs[index]);
        }
        /// Get the bounding box of a given line
        colib::rectangle bbox(int index);

        /// Get the bounding boxes for all the characters in the line
        void bboxes(colib::rectarray &result, int index) {
            copy(result, line_bboxes[index]);
        }
        /// Set the bounding box of a given line
        void setBboxes(colib::rectarray &bboxes, int index) {
            copy(line_bboxes[index], bboxes);
        }
    };

    /// Recognize a segmented page into a RecognizedPage struct.
    void tesseract_recognize_blockwise(RecognizedPage &result,
                                       colib::bytearray &gray,
                                       colib::intarray &pageseg);

    /// \brief Start Tesseract with the specified language.
    /// The language is specified with Tesseract's 3-letter code.
    /// similarly to -l option of Tesseract.
    /// Obviously, the corresponding data files should be installed.
    void tesseract_init_with_language(const char *language);

    /// Recognize a sub-image with Tesseract and return the text.
    char *tesseract_rectangle(colib::bytearray &image,int x0,int y0,int x1,int y1);
    /// Recognize an image with Tesseract and return the text.
    char *tesseract_block(colib::bytearray &image);

    /// Dismiss Tesseract.
    void tesseract_end();
};

#endif
