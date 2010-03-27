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
#include <time.h>
#include <map> /* CODE-OK--tmb */
#include "colib/colib.h"
#include "imgio.h"
#include "imglib.h"

using namespace ocropus;
using namespace std;
using namespace colib;

namespace ocropus {
    namespace {
        int colors[] = {
            0xff00ff,
            0x009f4f,
            0xff0000,
            0x0000ff,
            0xff0000,
            0xffff00,
            0x6f007f,
            0x00ff00,
            0x004f9f,
            0x7f9f00,
            0xaf006f,
            0x00ffff,
            0xff5f00,
        };

#define ncolors (sizeof colors/sizeof colors[0])

        struct Enumerator {
            int n;
            map<int,int> translation;
            Enumerator(int start=0) {
                n = start;
            }
            int operator[](int i) {
                map<int,int>::iterator p;
                p = translation.find(i);
                if(p!=translation.end()) {
                    return p->second;
                } else {
                    translation.insert(pair<int,int>(i,n));
                    return n++;
                }
            }
            int total() {
                return n;
            }
        };
    }
    void recolor_segmentation(intarray &image) {
        Enumerator enumerator(1);
        for(int i=0;i<image.length1d();i++) {
            if(!image.at1d(i)) continue;
            image.at1d(i) = enumerator[image.at1d(i)];
        }
        for(int i=0;i<image.length1d();i++) {
            if(!image.at1d(i)) image.at1d(i) = 0xffffff;
            else image.at1d(i) = colors[image.at1d(i)%ncolors];
        }
    }
}


int main(int argc,char **argv) {
    if(isatty(0)) {
        fprintf(stderr,"usage: %s < pnm-image > recolored-pnm-image\n",argv[0]);
        exit(1);
    }
    intarray image;
    read_ppm_packed(stdin, image);
    recolor_segmentation(image);
    write_ppm_packed(stdout, image);
}
