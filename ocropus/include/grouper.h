// -*- C++ -*-

// Copyright 2006-2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
// or its licensors, as applicable.
//
// You may not use this file except under the terms of the accompanying license.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You may
// obtain a copy of the License at http:  www.apache.org/licenses/LICENSE-2.0
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
// Web Sites: www.iupr.org, www.dfki.de

#ifndef grouper_h__
#define grouper_h__

#include "ocropus.h"

namespace ocropus {
    using namespace colib;

    void sort_by_xcenter(colib::intarray &);

    struct IGrouper : IComponent {
        // Set the grouper for iterating over the elements of the
        // segmentation.

        virtual void setSegmentation(colib::intarray &segmentation) = 0;

        // Set the grouper for iterating over a pre-segmented image (i.e.,
        // one group per input segment).

        virtual void setCSegmentation(colib::intarray &segmentation) = 0;

        // Number of groups generated.

        virtual int length() = 0;

        // Get the bounding rectangle and mask for group i.
        // Optionally, expand the mask by the given margin.

        virtual void getMask(colib::rectangle &r,colib::bytearray &mask,int i,int margin) = 0;

        // Get the mask around a given rectangle.

        virtual void getMaskAt(bytearray &mask,int index,rectangle &b) = 0;

        // Get the bounding box for the group i.

        virtual rectangle boundingBox(int i) = 0;

        virtual int start(int index) = 0;

        // return the last segment

        virtual int end(int index) = 0;

        // return a list of all segments

        virtual void getSegments(intarray &result,int index) = 0;

        // Extract images corresponding to group i from the source.

        virtual void extract(bytearray &out,bytearray &mask,
                             bytearray &source,int i,int grow=0) = 0;
        virtual void extract(bytearray &out,bytearray &source,
                             byte dflt,int i,int grow=0) = 0;
        virtual void extract(floatarray &out,bytearray &mask,
                             floatarray &source,int i,int grow=0) = 0;
        virtual void extract(floatarray &out,floatarray &source,
                             float dflt,int i,int grow=0) = 0;

        // slice extraction

        virtual void extractSliced(bytearray &out,bytearray &mask,
                                   bytearray &source,int index,int grow=0) = 0;
        virtual void extractSliced(bytearray &out,bytearray &source,
                                   byte dflt,int index,int grow=0) = 0;
        virtual void extractSliced(floatarray &out,bytearray &mask,
                                   floatarray &source,int index,int grow=0) = 0;
        virtual void extractSliced(floatarray &out,floatarray &source,
                                   float dflt,int index,int grow=0) = 0;

        // Set the cost for classifying group i as class cls.

        virtual void setClass(int i,int cls,float cost) = 0;

        // Space handling.  For any component, pixelSpace gets the amount
        // of subsequent space (-1 for the last component).

        virtual int pixelSpace(int i) = 0;

        // Sets the costs associated with inserting a space after the
        // character and not inserting the space.

        virtual void setSpaceCost(int index,float yes,float no) = 0;

        // Extract the lattice corresponding to the classifications
        // stored in the Grouper.

        virtual void getLattice(IGenericFst &fst) = 0;

        virtual ~IGrouper() {}
    };

    IGrouper *make_SimpleGrouper();
    IGrouper *make_StandardGrouper(); // synonymous
}


#endif /* grouper_h__ */
