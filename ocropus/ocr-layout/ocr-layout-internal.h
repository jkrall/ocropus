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
// File: ocr-layout.h
// Purpose: Internal header file containing data structures that are frquently
//          used by different modules in layout analysis.
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef ocr_layoutinternal_h__
#define ocr_layoutinternal_h__

#include "ocr-layout-structs.h"
#include "ocr-char-stats.h"
#include "ocr-whitespace-cover.h"
#include "ocr-extract-gutters.h"
#include "ocr-extract-rulings.h"
#include "ocr-ctextline-rast-extended.h"
#include "ocr-ctextline-rast.h"
#include "ocr-color-encode-layout.h"
#include "ocr-detect-columns.h"
#include "ocr-detect-paragraphs.h"
#include "ocr-reading-order.h"
#include "ocr-visualize-layout-rast.h"
#include "ocr-layout-rast.h"
#include "line-info.h"

#include "ocr-deskew-rast.h"

#include "ocr-noisefilter.h"
#include "ocr-doc-clean.h"
#include "ocr-doc-clean-concomp.h"
#include "ocr-pageframe-rast.h"

#include "ocr-pageseg-wcuts.h"
#include "ocr-pageseg-xycut.h"
#include "ocr-word-segmentation.h"

#include "ocr-segmentations.h"

#include "log-reg-data.h"
#include "ocr-classify-zones.h"
#include "ocr-text-image-seg.h"

#endif
