// -*- C++ -*-

// Copyright 2006-2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org

#include <stdio.h>
#include "ocropus.h"

namespace ocropus {
    using namespace iulib;
    using namespace colib;

    namespace {
        template <class T>
        int find(T &object,narray<T> &list) {
            for(int i=0;i<list.length();i++) {
                if(list[i]==object) return i;
            }
            return -1;
        }

        void check_approximately_sorted(intarray &labels) {
            for(int i=0;i<labels.length1d();i++)
                if(labels.at1d(i)>100000)
                    throw "labels out of range";
            narray<rectangle> rboxes;
            bounding_boxes(rboxes,labels);
#if 0
            // TODO/tmb disabling check until the overseg issue is fixed --tmb
            for(int i=1;i<rboxes.length();i++) {
                if(rboxes[i].x1<rboxes[i-1].x0) {
                    errors_log("bad segmentation", labels);
                    errors_log.recolor("bad segmentation (recolored)", labels);
                    throw_fmt("boxes aren't approximately sorted: "
                              "box %d is to the left from box %d", i, i-1);
                }
            }
#endif
        }
    }

    void sort_by_xcenter(intarray &labels) {
        make_line_segmentation_black(labels);
        floatarray centers;
        intarray counts;
        int n = max(labels)+1;
        ASSERT(n<10000);
        centers.resize(n);
        counts.resize(n);
        fill(centers,0);
        fill(counts,0);

        for(int i=0;i<labels.dim(0);i++) for(int j=0;j<labels.dim(1);j++) {
                int label = labels(i,j);
                centers(label) += i;
                counts(label)++;
            }

        counts(0) = 0;

        for(int i=0;i<centers.length();i++) {
            if(counts(i)>0) centers(i) /= counts(i);
            else centers(i) = 999999;
        }

        intarray permutation;
        quicksort(permutation,centers);
        intarray rpermutation(permutation.length());
        for(int i=0;i<permutation.length();i++) rpermutation(permutation(i)) = i;

        for(int i=0;i<labels.dim(0);i++) for(int j=0;j<labels.dim(1);j++) {
                int label = labels(i,j);
                if(counts(label)==0)
                    labels(i,j) = 0;
                else
                    labels(i,j) = rpermutation(label)+1;
            }
    }

    struct SimpleGrouper : IGrouper {
        int maxrange;
        int maxdist;
        intarray labels;
        narray<rectangle> boxes;
        objlist<intarray> segments;
        narray<rectangle> rboxes;
        floatarray classifications;
        floatarray spaces;
        bool fullheight;

        SimpleGrouper() {
            maxrange = 4;
            maxdist = 2;
            fullheight = false;
        }

        void set(const char *s,double v) {
            if(!strcmp(s,"maxrange")) maxrange = int(v);
            else if(!strcmp(s,"maxdist")) maxdist = int(v);
            else if(!strcmp(s,"fullheight")) fullheight = int(v);
            else throw "unknown parameter";
        }

        const char *name() {
            return "simplegrouper";
        }

        const char *description() {
            return "SimpleGrouper";
        }

        // Set a segmentation.

        void setSegmentation(intarray &segmentation) {
            copy(labels,segmentation);
            make_line_segmentation_black(labels);
            check_approximately_sorted(labels);
            boxes.dealloc();
            segments.dealloc();
            classifications.dealloc();
            spaces.dealloc();
            computeGroups();
        }

        // Set a character segmentation.

        void setCSegmentation(intarray &segmentation) {
            maxrange = 1;
            maxdist = 2;
            copy(labels,segmentation);
            make_line_segmentation_black(labels);
            check_approximately_sorted(labels);
            boxes.dealloc();
            segments.dealloc();
            classifications.dealloc();
            spaces.dealloc();
            computeGroups();
        }

        // Compute the groups for a segmentation (internal method).

        void computeGroups() {
            rboxes.clear();
            bounding_boxes(rboxes,labels);
            int n = rboxes.length();
            // NB: we start with i=1 because i=0 is the background
            for(int i=1;i<n;i++) {
                for(int range=1;range<=maxrange;range++) {
                    if(i+range>n) continue;
                    rectangle box = rboxes[i];
                    intarray seg;
                    bool bad = 0;
                    for(int j=i;j<i+range;j++) {
                        if(j>i && rboxes[j].x0-rboxes[j-1].x1>maxdist) {
                            bad = 1;
                            break;
                        }
                        box.include(rboxes[j]);
                        seg.push(j);
                    }
                    if(bad) continue;
                    boxes.push(box);
                    move(segments.push(),seg);
                }
            }
        }

        // Return the number of character candidates found.

        int length() {
            return boxes.length();
        }

        // Return the bounding box for a character.

        rectangle boundingBox(int index) {
            return boxes[index];
        }

        // return the starting segment

        virtual int start(int index) {
            return min(segments(index));
        }

        // return the last segment

        virtual int end(int index) {
            return max(segments(index));
        }

        // return a list of all segments

        virtual void getSegments(intarray &result,int index) {
            result.copy(segments(index));
        }

        // Return the segmentation-derived mask for the character.
        // This may optionally be grown by some pixels.

        void getMask(rectangle &r,bytearray &mask,int index,int grow) {
            r = boxes[index].grow(grow);
            r.intersect(rectangle(0,0,labels.dim(0),labels.dim(1)));
            if(fullheight) {
                r.y0 = 0;
                r.y1 = labels.dim(1);
            }
            int x = r.x0, y = r.y0, w = r.width(), h = r.height();
            intarray &segs = segments[index];
            mask.resize(w,h);
            fill(mask,0);
            for(int i=0;i<w;i++) for(int j=0;j<h;j++) {
                    int label = labels(x+i,y+j);
                    if(first_index_of(segs,label)>=0) {
                        mask(i,j) = 255;
                    }
                }
            if(grow>0) binary_dilate_circle(mask,grow);
        }

        // Get a mask at a given location

        void getMaskAt(bytearray &mask,int index,rectangle &b) {
            CHECK(b.x0>-1000 && b.x1<10000 && b.y0>-1000 && b.y1<10000);
            mask.resize(b.width(),b.height());
            mask = 0;
            intarray &segs = segments[index];
            int w = b.width(), h = b.height();
            for(int i=0;i<w;i++) {
                int x = b.x0+i;
                if(unsigned(x)>=labels.dim(0)) continue;
                for(int j=0;j<h;j++) {
                    int y = b.y0+j;
                    if(unsigned(y)>=labels.dim(1)) continue;
                    int label = labels(b.x0+i,b.y0+j);
                    if(first_index_of(segs,label)>=0) {
                        mask(i,j) = 255;
                    }
                }
            }
        }

        // Extract the masked character from the source image/source
        // feature map.

        template <class T>
        void extractMasked(narray<T> &out,bytearray &mask,narray<T> &source,int index,int grow=0) {
            ASSERT(samedims(labels,source));
            rectangle r;
            getMask(r,mask,index,grow);
            int x = r.x0, y = r.y0, w = r.width(), h = r.height();
            out.resize(w,h);
            fill(out,0);
            for(int i=0;i<w;i++) for(int j=0;j<h;j++) {
                    if(mask(i,j))
                        out(i,j) = source(i+x,j+y);
                }
        }

        // Extract the character by bounding rectangle (no masking).

        template <class T>
        void extractWithBackground(narray<T> &out,narray<T> &source,T dflt,int index,int grow=0) {
            ASSERT(samedims(labels,source));
            bytearray mask;
            rectangle r;
            getMask(r,mask,index,grow);
            int x = r.x0, y = r.y0, w = r.width(), h = r.height();
            out.resize(w,h);
            fill(out,dflt);
            for(int i=0;i<w;i++) for(int j=0;j<h;j++) {
                    if(mask(i,j))
                        out(i,j) = source(i+x,j+y);
                }
        }

        // Overloaded convenience functions.

        void extract(bytearray &out,bytearray &mask,bytearray &source,int index,int grow=0) {
            extractMasked(out,mask,source,index,grow);
        }
        void extract(bytearray &out,bytearray &source,byte dflt,int index,int grow=0) {
            extractWithBackground(out,source,dflt,index,grow);
        }
        void extract(floatarray &out,bytearray &mask,floatarray &source,int index,int grow=0) {
            extractMasked(out,mask,source,index,grow);
        }
        void extract(floatarray &out,floatarray &source,float dflt,int index,int grow=0) {
            extractWithBackground(out,source,dflt,index,grow);
        }

        // Extract the masked character from the source image/source
        // feature map.

        template <class T>
        void extractSlicedMasked(narray<T> &out,bytearray &mask,narray<T> &source,int index,int grow=0) {
            ASSERT(samedims(labels,source));
            rectangle r;
            getMask(r,mask,index,grow);
            int x = r.x0, y = r.y0, w = r.width(), h = r.height();
            out.resize(w,source.dim(1));
            fill(out,0);
            for(int i=0;i<w;i++) {
                for(int j=0;j<h;j++) {
                    if(mask(i,j))
                        out(i,j+y) = source(i+x,j+y);
                }
            }
        }

        // Extract the character by bounding rectangle (no masking).

        template <class T>
        void extractSlicedWithBackground(narray<T> &out,narray<T> &source,T dflt,int index,int grow=0) {
            ASSERT(samedims(labels,source));
            bytearray mask;
            rectangle r;
            getMask(r,mask,index,grow);
            int x = r.x0, y = r.y0, w = r.width(), h = r.height();
            out.resize(w,source.dim(1));
            fill(out,dflt);
            for(int i=0;i<w;i++) {
                for(int j=0;j<h;j++) {
                    if(mask(i,j))
                        out(i,j+y) = source(i+x,j+y);
                }
            }
        }

        // slice extraction

        void extractSliced(bytearray &out,bytearray &mask,bytearray &source,int index,int grow=0) {
            extractSlicedMasked(out,mask,source,index,grow);
        }
        void extractSliced(bytearray &out,bytearray &source,byte dflt,int index,int grow=0) {
            extractSlicedWithBackground(out,source,dflt,index,grow);
        }
        void extractSliced(floatarray &out,bytearray &mask,floatarray &source,int index,int grow=0) {
            extractSlicedMasked(out,mask,source,index,grow);
        }
        void extractSliced(floatarray &out,floatarray &source,float dflt,int index,int grow=0) {
            extractSlicedWithBackground(out,source,dflt,index,grow);
        }

        // FIXME/mezhirov add comment --tmb

        void maybeInit() {
            if(classifications.length1d()==0) {
                classifications.resize(boxes.length(),256); // FIXME/tmb limited size
                classifications.fill(INFINITY);
                spaces.resize(boxes.length(),2);
                spaces.fill(INFINITY);
            }
        }

        // After classification, set the class for the given character.

        void setClass(int index,int cls,float cost) {
            maybeInit();
            classifications(index,cls) = cost;
        }

        // Get spacing to the next component.

        int pixelSpace(int i) {
            int end = max(segments[i]);
            if(end>=rboxes.length()-1) return -1;
            int x0 = rboxes[end].x1;
            int x1 = rboxes[end+1].x0;
            return x1-x0;
        }

        // Set the cost for inserting a space after the given
        // character.

        void setSpaceCost(int index,float yes,float no) {
            maybeInit();
            spaces(index,0) = yes;
            spaces(index,1) = no;
        }

        // Output the segmentation into a segmentation graph.
        // Construct a state for each of the segments, then
        // add transitions between states (segments)
        // from min(segments[i]) to max(segments[i])+1.

        void getLattice(IGenericFst &fst) {
            fst.clear();

            int final = max(labels)+1;
            intarray states(final+1);

            states.fill(-1);
            for(int i=1;i<states.length();i++)
                states[i] = fst.newState();
            fst.setStart(states[1]);
            fst.setAccept(states[final]);

            for(int i=0;i<boxes.length();i++) {
                int start = min(segments[i]);
                int end = max(segments[i]);
                int id = (start << 16) + end;
                if(!segments[i].length())
                    id = 0;

                int space_state = -1;
                float yes = spaces(i,0);
                float no = spaces(i,1);
                if(yes<INFINITY) {
                    space_state = fst.newState();
                    states.push(space_state);
                }
                for(int j=0;j<classifications.dim(1);j++) {
                    if(classifications(i,j)==INFINITY) continue;
                    if(space_state<0) {
                        fst.addTransition(states[start],states[end+1],j,classifications(i,j), id);
                    } else {
                        if(no<INFINITY) {
                            fst.addTransition(states[start],states[end+1],j,classifications(i,j)+no, id);
                        }
                        fst.addTransition(states[start],space_state,j,classifications(i,j), id);
                        fst.addTransition(space_state,states[end+1],' ',yes, 0);
                    }
                    // printf("%d,%d : %d -> %d\n",i,j,start,end);
                }
            }
        }
    };

    IGrouper *make_SimpleGrouper() {
        return new SimpleGrouper();
    }
    IGrouper *make_StandardGrouper() {
        return new SimpleGrouper();
    }
}
