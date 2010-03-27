#ifndef didegrade_h__
#define didegrade_h__
// Copyright 2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// File: didegrade.h
// Purpose: document image degradation
// Responsible: mezhirov (original code by Daniel Wright)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org

/// \file didegrade.h
/// \brief Document image degradation

#include "ocropus.h"

namespace ocropus {

    /// Degrade a grayscale text image by applying Baird's degradation model.
    void degrade(colib::bytearray &image,
                 double jitter_mean = .2,
                 double jitter_sigma = .1,
                 double sensitivity_mean = .125,
                 double sensitivity_sigma = .04,
                 double threshold_mean = .4,
                 double threshold_sigma = .04);
};
#endif
