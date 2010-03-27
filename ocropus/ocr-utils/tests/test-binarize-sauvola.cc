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
// Responsible: mezhirov
// Reviewer: 
// Primary Repository: 
// Web Sites: 

#include "ocropus.h"

using namespace colib;
using namespace ocropus;

int main() {
    autodel<IBinarize> binarizer(make_BinarizeBySauvola());
    bytearray image;
    read_image_gray(image, "data/testimages/blob600.png");
    binarizer->description();
    bytearray out;
    floatarray in;
    copy(in, image);
    /*in.resize(2,2);
    in(0,0) = 0.1;
    in(0,1) = 200;
    in(1,0) = 0.3;
    in(1,1) = 210*/
    binarizer->binarize(out, in);
    in.resize(0,0);
    binarizer->binarize(out, in);
    return 0;
}
