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

#ifndef arraypaint_h_
#define arraypaint_h_

namespace ocropus {
    // TODO/tmb implement these using function templates --tmb
    // TODO/tmb create a bunch of narray drawing functions in iulib/utils --tmb
    /// Draw a rectangle on an image
    void paint_box(intarray &image, rectangle r,
                   int color, bool inverted=false);
    void paint_box(bytearray &image, rectangle r,
                   byte color, bool inverted=false);
    void paint_box_border(intarray &image, rectangle r,
                          int color, bool inverted=false);
    void paint_box_border(bytearray &image, rectangle r,
                          byte color, bool inverted=false);

    // Draw an array of rectangle boundaries into a new image
    void draw_rects(intarray &out, bytearray &in,
                    narray<rectangle> &rects,
                    int downsample_factor=1, int color=0x00ff0000);

    // Draw an array of filled rectangles into a new image
    void draw_filled_rects(intarray &out, bytearray &in,
                           narray<rectangle> &rects,
                           int downsample_factor=1,
                           int color=0x00ffff00,
                           int border_color=0x0000ff00);

    void paint_rectangles(intarray &image,rectarray &rectangles);

}

#endif
