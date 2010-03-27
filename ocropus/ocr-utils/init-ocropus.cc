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

#include "ocropus.h"
#include "glcuts.h"
#include "glfmaps.h"

using namespace iulib;
using namespace ocropus;
using namespace glinerec;

namespace ocropus {
    void init_ocropus_components() {
        static bool init = false;
        if(init) return;
        init = true;
        component_register("DpSegmenter",make_DpSegmenter);
        // component_register("SimpleFeatureMap",make_SimpleFeatureMap);
        // component_register("RidgeFeatureMap",make_RidgeFeatureMap);
        component_register("SimpleGrouper",make_SimpleGrouper);
        component_register("StandardGrouper",make_StandardGrouper);
        component_register("DocClean",make_DocClean);
        // component_register("DocCleanConComp",make_DocCleanConComp);
        component_register("PageFrameRAST",make_PageFrameRAST);
        component_register("DeskewPageByRAST",make_DeskewPageByRAST);
        component_register("DeskewGrayPageByRAST",make_DeskewGrayPageByRAST);
        component_register("RemoveImageRegionsBinary",make_RemoveImageRegionsBinary);
        component_register("RemoveImageRegionsGray",make_RemoveImageRegionsGray);
        component_register("TextImageSegByLogReg",make_TextImageSegByLogReg);
        component_register("TextImageSegByLeptonica",make_TextImageSegByLeptonica);
        component_register("SegmentPageByMorphTrivial",make_SegmentPageByMorphTrivial);
        component_register("SegmentPageBy1CP",make_SegmentPageBy1CP);
        component_register("SegmentPageByRAST",make_SegmentPageByRAST);
        component_register("SegmentPageByVORONOI",make_SegmentPageByVORONOI);
        component_register("SegmentPageByXYCUTS",make_SegmentPageByXYCUTS);
        component_register("SegmentWords",make_SegmentWords);
        component_register("OcroFST",make_OcroFST);
        component_register("BinarizeByRange",make_BinarizeByRange);
        component_register("BinarizeByOtsu",make_BinarizeByOtsu);
        component_register("BinarizeBySauvola",make_BinarizeBySauvola);
        component_register("SegmentLineByProjection",make_SegmentLineByProjection);
        component_register("SegmentLineByCCS",make_SegmentLineByCCS);
        component_register("ConnectedComponentSegmenter",make_ConnectedComponentSegmenter);
        component_register("CurvedCutSegmenter",make_CurvedCutSegmenter);
        component_register("CurvedCutWithCcSegmenter",make_CurvedCutWithCcSegmenter);
        component_register("SkelSegmenter",make_SkelSegmenter);

#if 0
        // FIXME remove this when it's clear that it's not needed anymore
        component_register("binarize-otsu",make_BinarizeByOtsu);
        component_register("binarize-range",make_BinarizeByRange);
        component_register("binarize-sauvola",make_BinarizeBySauvola);
        component_register("deskew-grayrast",make_DeskewGrayPageByRAST);
        component_register("deskew-rast",make_DeskewPageByRAST);
        component_register("docclean-generic",make_DocClean);
        component_register("docclean-pageframerast",make_PageFrameRAST);
        component_register("docclean-removeimage-binary",make_RemoveImageRegionsBinary);
        component_register("docclean-removeimage-gray",make_RemoveImageRegionsGray);
        component_register("recline-tess",make_TesseractRecognizeLine);
        component_register("seg-line-skel",make_SkelSegmenter);
        component_register("segline-ccs",make_ConnectedComponentSegmenter);
        component_register("segline-ccs",make_SegmentLineByCCS);
        component_register("segline-curved",make_CurvedCutSegmenter);
        component_register("segline-curvedcc",make_CurvedCutWithCcSegmenter);
        component_register("segline-proj",make_SegmentLineByProjection);
        component_register("segpage-1cp",make_SegmentPageBy1CP);
        component_register("segpage-morphtriv",make_SegmentPageByMorphTrivial);
        component_register("segpage-rast",make_SegmentPageByRAST);
        component_register("segpage-voronoi",make_SegmentPageByVORONOI);
        component_register("segpage-words",make_SegmentWords);
        component_register("segpage-xycuts",make_SegmentPageByXYCUTS);
        component_register("simple-grouper",make_SimpleGrouper);
        component_register("standard-grouper",make_StandardGrouper);
        component_register("textimage-leptonica",make_TextImageSegByLeptonica);
        component_register("textimage-logreg",make_TextImageSegByLogReg);
#endif
    }
}
