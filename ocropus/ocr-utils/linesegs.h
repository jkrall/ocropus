// Copyright 2006 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Project: iulib -- image understanding library
// File: 
// Purpose: 
// Responsible: 
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef linesegs_h__
#define linesegs_h__

#include "ocropus.h"

namespace ocropus {
    void check_line_segmentation(intarray &cseg);
    void make_line_segmentation_black(intarray &a);
    void make_line_segmentation_white(intarray &a);
    inline void write_line_segmentation(FILE *stream,intarray &a) {
        check_line_segmentation(a);
        make_line_segmentation_white(a);
        write_image_packed(stream,a,"png");
    }
    inline void read_line_segmentation(intarray &a,FILE *stream) {
        read_image_packed(a,stream,"png");
        check_line_segmentation(a);
        make_line_segmentation_black(a);
    }
}

#endif
