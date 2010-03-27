// -*- C++ -*-

// Copyright 2006-2008 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz 
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
// Project: OCRopus
// File: ocr-color-encode-layout.h
// Purpose: Color encode output of RAST layout analysis as per OCRopus
//          color coding conventions:
//          http://docs.google.com/Doc?id=dfxcv4vc_92c8xxp7
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_ocrcolorencodelayout__
#define h_ocrcolorencodelayout__

#include "ocropus.h"

namespace ocropus {

    const int GUTTER_COLOR   = 0x00ffff80;
    const int IMAGE_COLOR    = 0x0000ff00;
    const int GRAPHICS_COLOR = 0x0000fe00;
    const int RULING_COLOR   = 0x0000fa00;

    const int MULTI_COLUMN_ELEMENT_COLOR = 0x00fe0000;

    struct ColorEncodeLayout{
        colib::intarray outputImage;
        colib::bytearray inputImage;
        colib::rectarray textlines;
        colib::rectarray paragraphs;
        colib::rectarray textcolumns;
        colib::rectarray gutters;
        colib::rectarray rulings;
        colib::rectarray sidebar;
        colib::rectarray caption;
        colib::rectarray table;
        colib::rectarray graphics;
        colib::rectarray images;
        colib::rectarray pageNumber;
        colib::rectarray header;
        colib::rectarray footer;
        colib::rectarray noise;

        void encode();

    private:
        void encode_textlines();
        void encode_gutters();

        void encode_zones(colib::rectarray &zones, int zone_color);
        void encode_images();
        void encode_graphics();
        void encode_rulings();
    };

    ColorEncodeLayout *make_ColorEncodeLayout();
    
}

#endif
