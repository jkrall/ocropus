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
// File: narray-io.cc
// Purpose: I/O utilities
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include <stdlib.h>
#include <ctype.h>
#include "colib/colib.h"
#include "narray-io.h"

using namespace colib;

namespace ocropus {

    void text_write_1d(FILE *f, intarray &a) {
        fprintf(f, "%d", a.length());
        for(int i = 0; i < a.length(); i++)
            fprintf(f, " %d", a[i]);
        fprintf(f, "\n");
    }

    void text_read_1d(FILE *f, intarray &a) {
        int n;
        if (fscanf(f, "%d", &n) != 1)
            throw "I/O error";
        a.resize(n);
        for(int i = 0; i < n; i++)
            fscanf(f, "%d", &a[i]);
    }


    template<class T> void text_write_narray(FILE *f,
                                             narray<T> &a,
                                             const char *format) {
        intarray dims;
        get_dims(dims, a);
        text_write_1d(f, dims);
        for(int i = 0; i < a.length1d(); i++) {
            fprintf(f, format, a.at1d(i));
            fprintf(f, "\n");
        }
    }

    template<class T> void text_read_narray(FILE *f,
                                            narray<T> &a,
                                            const char *format) {
        intarray dims;
        text_read_1d(f, dims);
        set_dims(a, dims);
        for(int i = 0; i < a.length1d(); i++)
            fscanf(f, format, &a.at1d(i));
    }


    void text_write(FILE *f, doublearray &a) {text_write_narray(f, a, "%g");}
    void text_write(FILE *f, floatarray &a) {text_write_narray(f, a, "%g");}
    void text_write(FILE *f, intarray &a)   {text_write_narray(f, a, "%d");}
    void text_write(FILE *f, bytearray &a)  {text_write_narray(f, a, "%d");}
    void text_read(FILE *f, doublearray &a)  {text_read_narray(f, a, "%lf");}
    void text_read(FILE *f, floatarray &a)  {text_read_narray(f, a, "%f");}
    void text_read(FILE *f, intarray &a)    {text_read_narray(f, a, "%d");}

    void text_read(FILE *f, bytearray &a) {
        intarray tmp;
        text_read(f, tmp);
        copy(a, tmp);
    }

    void parse_vector(doublearray &a, const char *s) {
        a.clear();
        char *p;
        do {
            double x = strtod(s, &p);
            if (!p || s == p) break;
            s = p;
            a.push(x);
        } while(*s);
    }

    void bin_write_1d(FILE *f, intarray &a) {
        write_int32(a.length(), f);
        for(int i = 0; i < a.length(); i++)
            write_int32(a[i], f);
    }
    void bin_read_1d(FILE *f, intarray &a) {
        a.resize(read_int32(f));
        for(int i = 0; i < a.length(); i++)
            a[i] = read_int32(f);
    }
    void bin_write_mnist_dims(FILE *f, intarray &a) {
        ASSERT(a.length() <= 255);
        fputc(0x8, f); // MNIST magic
        fputc(a.length(), f);
        for(int i = 0; i < a.length(); i++)
            write_int32(a[i], f);
    }
    void bin_read_mnist_dims(FILE *f, intarray &a) {
        if(fgetc(f) != 0x8)
            throw "bin_read_dims: no magic byte found";
        int len = fgetc(f);
        if(len == EOF)
            throw "bin_read_dims: premature EOF";
        a.resize(len);
        for(int i = 0; i < a.length(); i++)
            a[i] = read_int32(f);
    }

    void bin_write(FILE *f, bytearray &a) {
        intarray dims;
        get_dims(dims, a);
        bin_write_mnist_dims(f, dims);
        write_raw(f, a);
    }

    void bin_read(FILE *f, bytearray &a) {
        intarray dims;
        bin_read_mnist_dims(f, dims);
        set_dims(a, dims);
        read_raw(f, a);
    }

    static int product(intarray &a) {
        int result = 1;
        for(int i = 0; i < a.length(); i++)
            result *= a[i];
        return result;
    }

    void bin_append(FILE *f, bytearray &a) {
        long begin_pos = ftell(f);
        intarray dims;
        bin_read_mnist_dims(f, dims);
        intarray dims_a;

        // check that it makes sense to append
        get_dims(dims_a, a);
        dims_a[0] = dims[0];
        CHECK_CONDITION(equal(dims_a, dims));

        fseek(f, begin_pos + 4, SEEK_SET); // seek to the first dim
        write_int32(dims[0] + a.dim(0), f);// and fix it
        fseek(f, dims.length() * 4 - 4 + product(dims), SEEK_CUR);
        write_raw(a, f);
    }


    void bin_write_nustring(FILE *f, nustring &a) {
        intarray dims;
        get_dims(dims, a);
        bin_write_1d(f, dims);
        for(int i = 0; i < a.length1d(); i++)
            write_int32(a.at1d(i).ord(), f);
    }

    void bin_read_nustring(FILE *f, nustring &a) {
        intarray dims;
        bin_read_1d(f, dims);
        set_dims(a, dims);
        for(int i = 0; i < a.length1d(); i++)
            a.at1d(i) = nuchar(read_int32(f));
    }

    void write_utf8(FILE *f, nustring &a) {
        char *p = a.newUtf8Encode();
        fprintf(f, "%s\n", p);
        delete[] p;
    }

    // adapted from old code, might be too pointer-happy, but should work
    char *read_line_malloc(FILE *f) {
        CHECK_ARG(f);

        unsigned line_size = 2; // should also work if it's 2 (for valgrind)
        char *line = (char *) malloc(line_size);
        char *end = line + line_size - 1; // last char in the buffer
        char *start = line; // where to put coming bytes

        while(1) {
            *end = 'X'; // to know if we've reached the end of the buffer

            char *p = fgets(start, end - start + 1, f);

            if(!p) { // EOF
                if (start == line)
                    return NULL; // we haven't read anything at all
                else
                    return line; // we've read something before, so return it
            }

            if (*end)
                return line; // the buffer was enough (our 'X' is untouched)

            if (end[-1] == '\n' || end[-1] == '\r')
                return line; // the buffer is full, the line had just fit

            // Bytes are still coming, we need to call fgets() again.
            int new_offset = line_size - 1; // point to the last '\0'
            // (we store offsets rather than pointers
            //  since the line is about to move)

            // grab more memory
            line_size <<= 1;
            line = (char *) realloc(line, line_size);
            start = line + new_offset;
            end = line + line_size - 1;
        } // while(1)
    }

    static void chomp_last_char_if_newline(char *str, int &len) {
        if(!len) return;
        if(str[len - 1] == '\r' || str[len - 1] == '\n') {
            len--;
            str[len] = '\0';
        }
    }

    bool read_utf8_line(nustring &a, FILE *f) {
        char *line = read_line_malloc(f);
        if(!line)
            return false;
        int n = strlen(line);
        chomp_last_char_if_newline(line, n);
        chomp_last_char_if_newline(line, n); // for fans of \r\n
        a.utf8Decode(line, n);
        free(line);
        return true;
    }

    /// This function name is deprecated because it suggests reading of the
    /// whole file.
    bool read_utf8(nustring &a, FILE *f) {
        return read_utf8_line(a, f);
    }

    void read_checkpoint(FILE *f, const char *checkpoint) {
        int c;
        while((c = fgetc(f)) != EOF) if(!isspace(c)) break; // skip spaces
        while(*checkpoint) {
            if(c != *checkpoint) {
                throw_fmt("error reading from file: %s expected at pos %ld",
                          checkpoint, ftell(f));
            }
            c = fgetc(f);
            checkpoint++;
        }
        if(!isspace(c))
            throw_fmt("error reading from file: space expected at pos %ld");
    }

    void write_checkpoint(FILE *f, const char *checkpoint) {
        fputs(checkpoint, f);
        fputc('\n', f);
    }
};
