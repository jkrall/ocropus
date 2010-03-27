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
// File: 
// Purpose: 
// Responsible: mezhirov
// Reviewer: 
// Primary Repository: 
// Web Sites: 


#include "ocropus.h"

using namespace colib;
using namespace ocropus;

void test_invert() {
    bytearray a;
    a.resize(2);
    a[0] = 0;
    a[1] = 10;
    invert(a);
    CHECK_CONDITION(a[0] == 255);
    CHECK_CONDITION(a[1] == 245);
}

void test_blit2d() {
    bytearray outer(2,2);
    bytearray inner(1,2);
    fill(outer, 0);
    fill(inner, 1);
    blit2d(outer, inner, 1, 0);
    ASSERT(outer(1,0) && outer(1,1));
}

void test_median() {
    int data[] = {4, 5, 1, 4, 6, 7, 0, 12, 2};
    intarray a;
    a.resize(sizeof(data)/sizeof(int));
    for(int i = 0; i < a.length(); i++)
        a[i] = data[i];
    ASSERT(median(a) == 4);
}

int main() {
    test_median();
    test_blit2d();
    test_invert();
}
