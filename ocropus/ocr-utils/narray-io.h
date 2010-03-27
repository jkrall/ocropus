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
// Project: ocropus
// File: narray-io.h
// Purpose: I/O utilities
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

/// \file narray-io.h
/// \brief I/O utilities for narrays

#ifndef h_narray_io_
#define h_narray_io_

#include <stdio.h>
#include <stdlib.h>
#include "ocropus.h"

namespace ocropus {

// ____________________   printing narrays for humans   _______________________

    /// Print a 2D array formatted with to the given format string.
    template<class T>
    void printf_matrix(const char *format, colib::narray<T> &a) {
        ASSERT(a.rank() == 2);
        for(int i = 0; i < a.dim(0); i++) {
            for(int j = 0; j < a.dim(1); j++) {
                if(j)
                    putchar(' ');
                printf(format, a(i,j));
            }
            putchar('\n');
        }
    }

    /// Print any array as 1D vector formatted with the given format string.
    template<class T>
    void printf_vector(const char *format, colib::narray<T> &a) {
        for(int i = 0; i < a.length1d(); i++) {
            if(i)
                putchar(' ');
            printf(format, a.at1d(i));
        }
        putchar('\n');
    }

    /// Print any 1D or 2D array formatted with the given format string.
    template<class T>
    void printf_narray(const char *format, colib::narray<T> &a) {
        if(a.rank() == 2)
            printf_matrix(format, a);
        else
            printf_vector(format, a);
    }

    /// Print a 1D or 2D doublearray to stdout.
    inline void print(colib::doublearray &a) {printf_narray("%g", a);}
    /// Print a 1D or 2D floatarray to stdout.
    inline void print(colib::floatarray &a) {printf_narray("%g", a);}
    /// Print a 1D or 2D intarray to stdout.
    inline void print(colib::intarray &a) {printf_narray("%d", a);}


    // FIXME/tmb there is another, binary narray-io facility in glinerec; that
    // needs to move to ocropus, and we need to give them different names. --tmb

    // ________________________   text I/O for narrays  _______________________

    /// Write a 1D intarray in a simple format: length and items.
    void text_write_1d(FILE *, colib::intarray &);
    /// Read a 1D intarray in a simple format: length and items.
    void text_read_1d(FILE *, colib::intarray &);

    // FIXME/tmb rename these to write_narray_text (verb first!) --tmb
    //\{
    /// Write the array (with dimensions) into a text file.
    void text_write(FILE *, colib::floatarray &);
    inline void text_write(colib::floatarray &a, FILE *f) {text_write(f, a);}
    void text_write(FILE *, colib::doublearray &);
    inline void text_write(colib::doublearray &a, FILE *f) {text_write(f, a);}
    void text_write(FILE *, colib::intarray &);
    inline void text_write(colib::intarray &a, FILE *f) {text_write(f, a);}
    void text_write(FILE *, colib::bytearray &);
    inline void text_write(colib::bytearray &a, FILE *f) {text_write(f, a);}
    //\}

    //\{
    /// Read the array (with dimensions) from a text file.
    void text_read(FILE *, colib::floatarray &);
    inline void text_read(colib::floatarray &a, FILE *f) {text_read(f, a);}
    void text_read(FILE *, colib::doublearray &);
    inline void text_read(colib::doublearray &a, FILE *f) {text_read(f, a);}
    void text_read(FILE *, colib::intarray &);
    inline void text_read(colib::intarray &a, FILE *f) {text_read(f, a);}
    void text_read(FILE *, colib::bytearray &);
    inline void text_read(colib::bytearray &a, FILE *f) {text_read(f, a);}
    //\}

// ______________________   binary I/O for primitives   _______________________

    //\{
    /// Write a 32-bit integer into the file in a binary form, MSB first.
    inline void write_int32(int32_t n, FILE *f) {
        fputc(n >> 24, f);
        fputc(n >> 16, f);
        fputc(n >>  8, f);
        fputc(n,       f);
    }
    inline void write_int32(FILE *f, int32_t n) {write_int32(n,f);}
    //\}

    /// \brief Read a 32-bit integer from the file in a binary form, MSB first.
    /// Doesn't check for EOF.
    inline int32_t read_int32(FILE *f) {
        int i = fgetc(f);
        i = (i << 8) | fgetc(f);
        i = (i << 8) | fgetc(f);
        i = (i << 8) | fgetc(f);
        return i;
    }

    //\{
    /// Read a rectangle in a binary format.
    inline void bin_read(FILE *f, colib::rectangle &r) {
        r.x0 = read_int32(f);
        r.y0 = read_int32(f);
        r.x1 = read_int32(f);
        r.y1 = read_int32(f);
    }

    inline void bin_read(colib::rectangle &r, FILE *f) {bin_read(f,r);}
    //\}

    //\{
    /// Write a rectangle in a binary format.
    inline void bin_write(FILE *f, colib::rectangle &r) {
        write_int32(f, r.x0);
        write_int32(f, r.y0);
        write_int32(f, r.x1);
        write_int32(f, r.y1);
    }

    inline void bin_write(colib::rectangle &r, FILE *f) {bin_write(f,r);}
    //\}

    //\{
    /// \brief Read a bytearray as fread() does.
    /// The number of bytes is determined by the array length.
    inline int read_raw(FILE *f, colib::bytearray &a) {
        return fread(a.data, 1, a.length1d(), f);
    }
    inline int read_raw(colib::bytearray &a, FILE *f) {return read_raw(f, a);}
    //\}
    //\{
    /// \brief Write a bytearray as fwrite() does.
    inline int write_raw(FILE *f, colib::bytearray &a) {
        return fwrite(a.data, 1, a.length1d(), f);
    }
    inline int write_raw(colib::bytearray &a, FILE *f) {return write_raw(f, a);}
    //\}

// ______________________________   misc   ____________________________________

    /// Create a 1D doublearray given a string with space-separated numbers.
    void parse_vector(colib::doublearray &,const char *);
    /// Create a 1D intarray given a string with space-separated numbers.
    inline void parse_vector(colib::intarray &result, const char *s) {
        colib::doublearray tmp;
        parse_vector(tmp, s);
        colib::copy(result, tmp);
    }

    // FIXME/tmb rename these to write_narray_bin etc. --tmb
    void bin_write_1d(FILE *, colib::intarray &);
    void bin_read_1d(FILE *, colib::intarray &);
    void bin_write_mnist_dims(FILE *, colib::intarray &);
    void bin_read_mnist_dims(FILE *, colib::intarray &);
    void bin_write(FILE *, colib::bytearray &);
    inline void bin_write(colib::bytearray &a, FILE *f) {bin_write(f, a);}
    void bin_read(FILE *, colib::bytearray &);
    inline void bin_read(colib::bytearray &a, FILE *f) {bin_read(f, a);}
    void bin_append(FILE *, colib::bytearray &);
    inline void bin_append(colib::bytearray &a, FILE *f) {bin_append(f, a);}

    inline void bin_read(const char *path, colib::bytearray &a) {
        bin_read(colib::stdio(path, "rb"), a);
    }
    inline void bin_read(colib::bytearray &a, const char *path) {
        bin_read(path, a);
    }
    inline void bin_write(const char *path, colib::bytearray &a) {
        bin_write(colib::stdio(path, "wb"), a);
    }
    inline void bin_write(colib::bytearray &a, const char *path) {
        bin_write(path, a);
    }
    inline void bin_append(const char *path, colib::bytearray &a) {
        FILE *test = fopen(path, "rb");
        fclose(test);
        if(test) {
            colib::stdio f(path, "r+b");
            bin_append(f, a);
        } else {
            colib::stdio f(path, "wb");
            bin_write(f, a);
        }
    }
    inline void bin_append(colib::bytearray &a, const char *path) {
        bin_append(path, a);
    }

    void bin_write_nustring(FILE *f, colib::nustring &a);
    void bin_read_nustring(FILE *f, colib::nustring &a);

    /// Read a line from the file, allocating memory with malloc().
    char *read_line_malloc(FILE *f);
    /// Write utf8-encoded string followed by a newline.
    void write_utf8(FILE *f, colib::nustring &a);
    /// Read a line and decode it from utf-8.
    bool read_utf8_line(colib::nustring &a, FILE *f);
    bool read_utf8(colib::nustring &a, FILE *f) WARN_DEPRECATED;

    /// \brief Write a `checkpoint' to the file.
    /// A checkpoint is just some text lying around for 4 purposes:
    /// - after fscanf() it's unwise to use binary I/O,
    ///     but after a checkpoint it's OK;
    /// - a checkpoint makes it easier to find stuff in the saved file
    /// - extra check when loading
    /// - versioning info can be stored in a checkpoint
    void write_checkpoint(FILE *f, const char *);

    /// \brief Read a `checkpoint'.
    /// The given identifier should exactly match the one in the file.
    void read_checkpoint(FILE *f, const char *);
};

#endif
