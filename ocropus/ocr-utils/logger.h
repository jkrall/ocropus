// -*- C++ -*-
// Copyright 2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// File: logger.h
// Purpose:
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org

/// \file logger.h
/// \brief The logging facility for debugging.

#ifndef h_logger_
#define h_logger_

#include "ocropus.h"

namespace ocropus {

    /// \brief The Logger class provides the centralized logging mechanism.
    ///
    /// Just look at the example:
    /// \code
    /// #include "logger.h"
    ///
    /// Logger log_stuff("car.engine.debug");
    /// ...
    ///     log_stuff("ignition");
    /// ...
    ///     log_stuff("the front view", image);
    /// ...
    ///     log_stuff.format("ran %d km so far", km);
    /// ...
    /// \endcode
    ///
    /// By default, all the loggers are disabled. Run the program like this
    /// to see the log:
    /// \code
    /// mkdir log
    /// ocrologdir=log ocrolog=car.engine <program>
    /// firefox log/index.html &
    /// \endcode
    ///
    /// Note that enabling a logger X on the command line will also enable any
    /// loggers having the form X.Y, X.Y.Z etc.
    ///
    /// To enable several loggers with the "ocrolog" variable,
    /// separate them with colons or semicolons.
    ///
    /// To enable messages related to the logging itself, enable the logger
    /// named "logger".
    ///
    /// Note: you can use logger.log() methods along with logger() operators.
    /// (This is the only way under Lua)
    class Logger {
        colib::iucstring name;
        colib::stdio logImage(const char *description, int w, int h);
        void html(colib::bytearray &img);
        void html(colib::nustring &val);
        colib::stdio logImageHtml();
        colib::stdio logImageHtmlBorder();
        colib::stdio logText(const char *description);

    public:
        /// FIXME these violate the naming conventions for methods
        bool turned_on(const char *name);
        void init_logging();
        void start_logging();

        bool enabled;

        void putIndent();

        /// \brief
        /// Construct a logger with a given name
        /// and decide whether it's enabled or not.
        Logger(const char *name);

        /// A printf-like method.
        void format(const char *format, ...);

        /// Recolor a segmentation and log it.
        void recolor(const char *description, colib::intarray &, float zoom = 100.);

        /// Just log the message.
        void operator()(const char *message);
        void log(const char *message){(*this)(message);}

        /// Log a boolean value.
        void operator()(const char *message, bool);
        void log(const char *message, bool value){(*this)(message, value);}

        /// Log an integer value.
        void operator()(const char *message, int);
        void log(const char *message, int value){(*this)(message, value);}

        /// Log a double value.
        void operator()(const char *message, double);
        void log(const char *message, double value){(*this)(message, value);}

        /// Log a string.
        void operator()(const char *description, const char *);
        void log(const char *message, const char *str){(*this)(message, str);}

        /// \brief Log a grayscale image.
        ///
        /// If the image is not 2-dimensional,
        /// it will be written as text
        /// (and the description text will become a link to it).
        void operator()(const char *description, colib::bytearray &, float zoom=100);
        void log(const char *descr, colib::bytearray &a, float zoom=100) {(*this)(descr, a, zoom);}

        /// \brief Log a color image.
        ///
        /// If the image is not 2-dimensional,
        /// it will be written as text.
        void operator()(const char *description, colib::intarray &, float zoom=100);
        void log(const char *descr, colib::intarray &a, float zoom=100) {(*this)(descr, a, zoom);}

        /// \brief Log an array of floats.
        void operator()(const char *description, colib::floatarray &);
        void log(const char *descr, colib::floatarray &a){(*this)(descr, a);}

        /// Log a nuchar value.
        void operator()(const char *description, colib::nuchar);
        void log(const char *descr, colib::nuchar c){(*this)(descr, c);}

        /// Log a nustring value, decoding it to UTF-8.
        void operator()(const char *description, colib::nustring &);
        void log(const char *descr, colib::nustring &s){(*this)(descr, s);}

        /// Log a rectangle.
        void operator()(const char *description, colib::rectangle &);
        void log(const char *descr, colib::rectangle &r){(*this)(descr, r);}

        /// Log the value of a pointer (not quite useful).
        void operator()(const char *description, void *);
        void log(const char *descr, void *ptr){(*this)(descr, ptr);}

        /// Get a bestpath() and log it.
        void operator()(const char *description, IGenericFst &l);
        void log(const char *descr, IGenericFst &L){(*this)(descr, L);}

        /// Increase indentation level in the log.
        void indent();

        /// Decrease indentation level in the log.
        void dedent();

        /// Put a border around an image
        void html_border(colib::bytearray &img);

        /// Directly add text in html source
        void html(const char*);

#if 0
        // FIXME/rangoni get this out of the logger class; some of these should
        // just become top-level functions, others should go into
        // their respective packages --tmb

        void display_one_line_graph(
                        int debug_display_transitions, int size, int indent,
                        colib::intarray &cuts, colib::floatarray &cost_t,
                        colib::intarray &id_t, colib::intarray &prev_t,
                        colib::intarray &trans_t, colib::intarray &charcode_t,
                        colib::objlist<colib::bytearray> &subimages);

        /// Draw 4 lines on the line image and log it.
        void operator()(const char *description, colib::bytearray &line_image,
             int baseline_y, int xheight_y, int ascender_y, int descender_y);
        void operator()(const char *description, colib::intarray &line_image,
             int baseline_y, int xheight_y, int ascender_y, int descender_y);
        }

        /// Display transitions, n-best costs, pictures and labels
        /// for a NewGroupingLineOCR
        /// Display is list-like
        void transitions_in_list(int maxcomp, int debug_display_transitions,
            intarray &cuts, floatarray &cost_t, intarray &id_t, intarray &prev_t,
            intarray &trans_t, intarray &charcode_t, objlist<bytearray> &subimages);

        /// Display transitions, n-best costs, pictures and labels
        /// for a NewGroupingLineOCR
        /// Display is graph-like, simulated with a html table
        void transitions_in_graph(int maxcomp, int debug_display_transitions,
            intarray &cuts, floatarray &cost_t, intarray &id_t, intarray &prev_t,
            intarray &trans_t, intarray &charcode_t,
            objlist<bytearray> &subimages);
#endif

    };

    /// Switch the logger directory. All the logs will continue to the new file.
    void set_logger_directory(const char *);
    const char *get_logger_directory();
    int get_image_counter();
    void set_image_counter(int);
};

#endif
