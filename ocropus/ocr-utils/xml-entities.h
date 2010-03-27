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
// Project:
// File: xml-entities.cc
// Purpose: routines for parsing XML predefined and numeric entities
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef xml_entities_h__
#define xml_entities_h__

#include "ocropus.h"

namespace ocropus {
    /// Converts XML entities into Unicode symbols, also doing UTF-8 decoding.
    /// Only predefined XML entities (amp, quot, apos, lt, gt) and the numerics
    /// (both decimal and hexadecimal) are supported.
    //
    // proposed for colib
    void xml_unescape(colib::nustring &dest, const char *src);
};

#endif
