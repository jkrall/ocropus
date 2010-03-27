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
// Project:
// File:
// Purpose:
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_pages__
#define h_pages__

#include "ocropus.h"

namespace ocropus {
    struct Pages {
        colib::autodel<IBinarize> binarizer;
        colib::narray<colib::iucstring> files;
        bool want_gray;
        bool want_color;

        int current_index;
        colib::iucstring current_file;
        bool has_gray;
        bool has_color;
        bool autoinv;
        colib::bytearray binary;
        colib::bytearray gray;
        colib::intarray color;

        Pages() {
            current_index = -1;
            autoinv = 1;
        }
        void clear() {
            files.clear();
        }
        void addFile(const char *file) {
            files.push() = file;
        }
        void parseSpec(const char *spec) {
            current_index = -1;
            clear();
            if(spec[0]=='@') {
                char buf[9999];
                colib::stdio stream(spec+1,"r");
                for(;;) {
                    if(!fgets(buf,sizeof buf,stream)) break;
                    int n = strlen(buf);
                    if(n>0) buf[n-1] = 0;
                    addFile(buf);
                }
            } else {
                addFile(spec);
            }
        }
        void wantGray(bool flag) {
            want_gray = flag;
        }
        void wantColor(bool flag) {
            want_color =flag;
        }
        void setAutoInvert(bool flag) {
            autoinv = flag;
        }
        void setBinarizer(IBinarize *arg) {
            binarizer = arg;
        }
        int length() {
            return files.length();
        }
        void getPage(int index) {
            current_index = index;
            current_file = files[index];
            loadImage();
        }
        bool nextPage() {
            ++current_index;
            if(current_index>=files.length()) return false;
            getPage(current_index);
            return true;
        }
        void rewind() {
            current_index = -1;
        }
        void loadImage() {
            has_gray = false;
            has_color = false;
            binary.clear();
            gray.clear();
            color.clear();
            iulib::read_image_gray(gray,current_file);
            if(autoinv) {
                iulib::make_page_black(gray);
                invert(gray);
            }
            if(!binarizer) {
                float v0 = min(gray);
                float v1 = max(gray);
                float threshold = (v1+v0)/2;
                makelike(binary,gray);
                for(int i=0;i<gray.length1d();i++)
                    binary.at1d(i) = (gray.at1d(i) > threshold) ? 255:0;
            } else {
                colib::floatarray temp;
                copy(temp,gray);
                binarizer->binarize(binary,temp);
            }
        }
        const char *getFileName() {
            return (const char *)current_file;
        }
        bool hasGray() {
            return true;
        }
        bool hasColor() {
            return false;
        }
        colib::bytearray &getBinary() {
            return binary;
        }
        colib::bytearray &getGray() {
            return gray;
        }
        colib::bytearray &getColor() {
            throw "unimplemented";
        }
        void getBinary(colib::bytearray &dst) {
            copy(dst,binary);
        }
        void getGray(colib::bytearray &dst) {
            copy(dst,gray);
        }
        void getColor(colib::intarray &dst) {
            copy(dst,color);
        }
    private:
        void invert(bytearray &a) {
            int n = a.length1d();
            for (int i = 0; i < n; i++) {
                a.at1d(i) = 255 - a.at1d(i);
            }
        }
    };
}

#endif
