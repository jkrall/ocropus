// -*- C++ -*-

// Copyright 2006-2007 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Project:
// File:
// Purpose:
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites:


// FIXME/faisal this should really work "word"-wise, centered on each word,
// otherwise it does the wrong thing for non-deskewed lines
// (it worked "word"-wise in the original version) --tmb

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include "ocropus.h"

using namespace ocropus;
using namespace iulib;
using namespace colib;

namespace {
    Logger log_main("lineseg.seg-cuts");

    void combine_segmentations(intarray &dst, intarray &src) {
        CHECK_ARG(samedims(dst, src));
        int n = max(dst) + 1;
        for(int i = 0; i < dst.length1d(); i++)
            dst.at1d(i) += src.at1d(i) * n;
        renumber_labels(dst, 1);
    }

    void local_min(floatarray &result,floatarray &data,int r) {
        int n = data.length();
        result.resize(n);
        for(int i=0;i<n;i++) {
            float lmin = data(i);
            for(int j=-r;j<=r;j++) {
                int k = i+j;
                if(unsigned(k)>=unsigned(n)) continue;
                if(data(k)>=lmin) continue;
                lmin = data(k);
            }
            result(i) = lmin;
        }
    }

    void local_minima(intarray &result,floatarray &data,int r,float threshold) {
        int n = data.length();
        result.clear();
        floatarray lmin;
        local_min(lmin,data,r);
        for(int i=1;i<n-1;i++) {
            if(data(i)<=threshold && data(i)<=lmin(i) &&
               data(i)<=data(i-1) && data(i)<data(i+1)) {
                result.push(i);
            }
        }
    }

    void line_segmentation_sort_x(intarray &segmentation) {
        CHECK_ARG(max(segmentation)<100000);
        narray<rectangle> bboxes;
        bounding_boxes(bboxes,segmentation);
        floatarray x0s;
        x0s.push(-999999);
        for(int i=1;i<bboxes.length();i++) {
            if(bboxes(i).empty()) {
                x0s.push(999999);
            } else {
                x0s.push(bboxes(i).x0);
            }
        }
        // dprint(x0s,1000); printf("\n");
        intarray permutation,rpermutation;
        quicksort(permutation,x0s);
        rpermutation.resize(permutation.length());
        for(int i=0;i<permutation.length();i++)
            rpermutation[permutation[i]] = i;
        // dprint(rpermutation,1000); printf("\n");
        for(int i=0;i<segmentation.length1d();i++) {
            if(segmentation.at1d(i)==0) continue;
            segmentation.at1d(i) = rpermutation(segmentation.at1d(i));
        }
    }

    void remove_small_components(intarray &segmentation,int r=5) {
        CHECK_ARG(max(segmentation)<100000);
        narray<rectangle> bboxes;
        bounding_boxes(bboxes,segmentation);
        for(int i=1;i<bboxes.length();i++) {
            rectangle b = bboxes(i);
            if(b.width()<r && b.height()<r) {
                for(int x=b.x0;x<b.x1;x++)
                    for(int y=b.y0;y<b.y1;y++)
                        if(segmentation(x,y)==i)
                            segmentation(x,y) = 0;
            }
        }
    }

    param_int seg_cuts_merge("seg_cuts_merge",10,"merge components smaller than this in seg-cuts");

    void line_segmentation_merge_small_components(intarray &segmentation,int r=10) {
        CHECK_ARG(max(segmentation)<100000);
        make_line_segmentation_black(segmentation);
        narray<rectangle> bboxes;
        bounding_boxes(bboxes,segmentation);
        bboxes(0) = rectangle();
        bool changed;
        do {
            changed = false;
            for(int i=1;i<bboxes.length();i++) {
                rectangle b = bboxes(i);
                if(b.empty()) continue;
                if(b.width()>=r || b.height()>=r) continue;
#if 0
                // merge any small component with nearby big components
                floatarray dists;
                for(int j=0;j<bboxes.length();j++) {
                    if(i==j || bboxes(j).empty())
                        dists.push(9999999);
                    else
                        dists.push(fabs(bboxes(i).xcenter()-bboxes(j).xcenter()));
                }
                int closest = argmin(dists);
#else
                // merge small components only with touching components
                int closest = 0;
                rectangle b1 = b.grow(1);
                b1.intersect(rectangle(0,0,segmentation.dim(0),segmentation.dim(1)));
                for(int x=b1.x0;x<b1.x1;x++) {
                    for(int y=b1.y0;y<b1.y1;y++) {
                        int value = segmentation(x,y);
                        if(value==0) continue;
                        if(value==i) continue;
                        closest = value;
                        break;
                    }
                }
                if(closest==0) continue;
#endif
                for(int x=b.x0;x<b.x1;x++)
                    for(int y=b.y0;y<b.y1;y++)
                        if(segmentation(x,y)==i)
                            segmentation(x,y) = closest;
                bboxes(i) = rectangle();
                changed = true;
            }
        } while(changed);
    }
}

namespace ocropus {
    struct ICurvedCutSegmenter {
        int down_cost;
        int outside_diagonal_cost;
        int inside_diagonal_cost;
        int boundary_diagonal_cost;
        int inside_weight;
        int boundary_weight;
        int outside_weight;
        int min_range;
        float min_thresh;
        //virtual void params_for_chars() = 0;
        virtual void params_for_lines() = 0;
        virtual void findAllCuts() = 0;
        virtual void findBestCuts() = 0;
        // virtual void relabel_image(bytearray &image) = 0;
        // virtual void relabel_image(intarray &image) = 0;
        virtual void setImage(bytearray &image) = 0;
        virtual ~ICurvedCutSegmenter() {}
    };

    struct CurvedCutSegmenterImpl : ICurvedCutSegmenter {
        // input
        intarray wimage;
        int where;

        // output
        intarray costs;
        intarray sources;
        int direction;
        int limit;

        intarray bestcuts;

        iucstring debug;
        intarray dimage;

        narray< narray <point> > cuts;
        floatarray cutcosts;

        CurvedCutSegmenterImpl() {
            //params_for_chars();
            params_for_lines();
            //params_from_hwrec_c();
        }

        void params_for_lines() {
            down_cost = 0;
            outside_diagonal_cost = 4;
            inside_diagonal_cost = 4;
            boundary_diagonal_cost = 0;
            outside_weight = 0;
            boundary_weight = -1;
            inside_weight = 4;
            min_range = 3;
            //min_thresh = -2.0;
            min_thresh = 10.0;
        }

        // this function calculates the actual costs
        void step(int x0,int x1,int y) {
            int w = wimage.dim(0),h = wimage.dim(1);
            Queue<point> queue(w*h);
            for(int i=x0;i<x1;i++) queue.enqueue(point(i,y));
            int low = 1;
            int high = wimage.dim(0)-1;

            while(!queue.empty()) {
                point p = queue.dequeue();
                int i = p.x, j = p.y;
                int cost = costs(i,j);
                int ncost = cost+wimage(i,j)+down_cost;
                if(costs(i,j+direction)>ncost) {
                    costs(i,j+direction) = ncost;
                    sources(i,j+direction) = i;
                    if(j+direction!=limit) queue.enqueue(point(i,j+direction));
                }
                if(i>low) {
                    if(wimage(i,j)==0)
                        ncost = cost+wimage(i,j)+outside_diagonal_cost;
                    else if(wimage(i,j)>0)
                        ncost = cost+wimage(i,j)+inside_diagonal_cost;
                    else if(wimage(i,j)<0)
                        ncost = cost+wimage(i,j)+boundary_diagonal_cost;
                    if(costs(i-1,j+direction)>ncost) {
                        costs(i-1,j+direction) = ncost;
                        sources(i-1,j+direction) = i;
                        if(j+direction!=limit) queue.enqueue(point(i-1,j+direction));
                    }
                }
                if(i<high) {
                    if(wimage(i,j)==0)
                        ncost = cost+wimage(i,j)+outside_diagonal_cost;
                    else if(wimage(i,j)>0)
                        ncost = cost+wimage(i,j)+inside_diagonal_cost;
                    else if(wimage(i,j)<0)
                        ncost = cost+wimage(i,j)+boundary_diagonal_cost;
                    if(costs(i+1,j+direction)>ncost) {
                        costs(i+1,j+direction) = ncost;
                        sources(i+1,j+direction) = i;
                        if(j+direction!=limit) queue.enqueue(point(i+1,j+direction));
                    }
                }
            }
        }

        void findAllCuts() {
            int w = wimage.dim(0), h = wimage.dim(1);
            // initialize dimensions of cuts, costs etc
            cuts.resize(w);
            cutcosts.resize(w);
            costs.resize(w,h);
            sources.resize(w,h);

            fill(costs, 1000000000);
            for(int i=0;i<w;i++) costs(i,0) = 0;
            fill(sources, -1);
            limit = where;
            direction = 1;
            step(0,w,0);

            for(int x=0;x<w;x++) {
                cutcosts(x) = costs(x,where);
                cuts(x).clear();
                // bottom should probably be initialized with 2*where instead of
                // h, because where cannot be assumed to be h/2. In the most extreme
                // case, the cut could go through 2 pixels in each row
                narray<point> bottom;
                int i = x, j = where;
                while(j>=0) {
                    bottom.push(point(i,j));
                    i = sources(i,j);
                    j--;
                }
                //cuts(x).resize(h);
                for(i=bottom.length()-1;i>=0;i--) cuts(x).push(bottom(i));
            }

            fill(costs, 1000000000);
            for(int i=0;i<w;i++) costs(i,h-1) = 0;
            fill(sources, -1);
            limit = where;
            direction = -1;
            step(0,w,h-1);

            for(int x=0;x<w;x++) {
                cutcosts(x) += costs(x,where);
                // top should probably be initialized with 2*(h-where) instead of
                // h, because where cannot be assumed to be h/2. In the most extreme
                // case, the cut could go through 2 pixels in each row
                narray<point> top;
                int i = x, j = where;
                while(j<h) {
                    if(j>where) top.push(point(i,j));
                    i = sources(i,j);
                    j++;
                }
                for(i=0;i<top.length();i++) cuts(x).push(top(i));
            }

            // add costs for line "where"
            for(int x=0;x<w;x++) {
                cutcosts(x) += wimage(x,where);
            }

        }

        void findBestCuts() {
            for(int i=0;i<cutcosts.length();i++) ext(dimage,i,int(cutcosts(i)+10)) = 0xff0000;
            for(int i=0;i<cutcosts.length();i++) ext(dimage,i,int(min_thresh+10)) = 0x800000;
            floatarray temp;
            gauss1d(temp,cutcosts,3.0);
            cutcosts.move(temp);
            local_minima(bestcuts,cutcosts,min_range,min_thresh);
            for(int i=0;i<bestcuts.length();i++) {
                narray<point> &cut = cuts(bestcuts(i));
                for(int j=0;j<cut.length();j++) {
                    point p = cut(j);
                    ext(dimage,p.x,p.y) = 0x00ff00;
                }
            }
            if(debug) write_image_packed(debug,dimage);
            // dshow1d(cutcosts,"Y");
            //dshow(dimage,"Y");
        }

        void setImage(bytearray &image) {
            copy(dimage,image);
            int w = image.dim(0), h = image.dim(1);
            wimage.resize(w,h);
            fill(wimage, 0);
            float s1 = 0.0, sy = 0.0;
            for(int i=1;i<w;i++) for(int j=0;j<h;j++) {
                    if(image(i,j)) { s1++; sy += j; }
                    if(!image(i-1,j) && image(i,j)) wimage(i,j) = boundary_weight;
                    else if(image(i,j)) wimage(i,j) = inside_weight;
                    else wimage(i,j) = outside_weight;
                }
            where = int(sy/s1);
            for(int i=0;i<dimage.dim(0);i++) dimage(i,where) = 0x008000;
        }
    };

    class CurvedCutSegmenter : public ISegmentLine {
    public:
        autoref<CurvedCutSegmenterImpl> segmenter;
        int small_merge_threshold;

        CurvedCutSegmenter() {
            small_merge_threshold = 1;
        }

        const char *name() {
            return "curvedcut";
        }

        virtual const char *description() {
            return "curved cut segmenter";
        }

        virtual void set(const char *key,const char *value) {
            log_main.format("set parameter %s to sf", key, value);
            if(!strcmp(key,"debug"))
                segmenter->debug = value;
            else
                throw "unknown key";
        }

        virtual void set(const char *key,double value) {
            log_main.format("set parameter %s to %f", key, value);
            if(!strcmp(key,"down_cost"))
                segmenter->down_cost = (int)value;
            else if(!strcmp(key,"small_merge_threshold"))
                small_merge_threshold = (int)value;
            else if(!strcmp(key,"outside_diagonal_cost"))
                segmenter->outside_diagonal_cost = (int)value;
            else if(!strcmp(key,"inside_diagonal_cost"))
                segmenter->inside_diagonal_cost = (int)value;
            else if(!strcmp(key,"boundary_diagonal_cost"))
                segmenter->boundary_diagonal_cost = (int)value;
            else if(!strcmp(key,"outside_weight"))
                segmenter->outside_weight = (int)value;
            else if(!strcmp(key,"boundary_weight"))
                segmenter->boundary_weight = (int)value;
            else if(!strcmp(key,"inside_weight"))
                segmenter->inside_weight = (int)value;
            else if(!strcmp(key,"min_range"))
                segmenter->min_range = (int)value;
            else if(!strcmp(key,"min_thresh"))
                segmenter->min_thresh = value;
            else
                throw "unknown key";
        }

        virtual void charseg(intarray &segmentation,bytearray &raw) {
            log_main("segmenting", raw);
            enum {PADDING = 3};
            optional_check_background_is_lighter(raw);
            bytearray image;
            image.copy(raw);
            binarize_simple(image);
            invert(image);

            segmenter->setImage(image);
            segmenter->findAllCuts();
            segmenter->findBestCuts();

            intarray seg;
            seg.copy(image);

            for(int r=0;r<segmenter->bestcuts.length();r++) {
                int w = seg.dim(0);
                int c = segmenter->bestcuts(r);
                narray<point> &cut = segmenter->cuts(c);
                for(int y=0;y<image.dim(1);y++) {
                    for(int i=-1;i<=1;i++) {
                        int x = cut(y).x;
                        if(x<1||x>=w-1) continue;
                        seg(x+i,y) = 0;
                    }
                }
            }
            label_components(seg);
            // dshowr(seg,"YY"); dwait();
            segmentation.copy(image);
            propagate_labels_to(segmentation,seg);

            line_segmentation_merge_small_components(segmentation,small_merge_threshold);
            line_segmentation_sort_x(segmentation);

            make_line_segmentation_white(segmentation);
            // set_line_number(segmentation, 1);
            log_main("resulting segmentation", segmentation);
        }
    };

    ISegmentLine *make_CurvedCutSegmenter1() {
        return new CurvedCutSegmenter();
    }

// FIXME/faisal Messy implementation--get rid of this. --tmb

    class CurvedCutSegmenterToISegmentLineAdapter : public ISegmentLine {
    public:
        autoref<CurvedCutSegmenterImpl> segmenter;
        int small_merge_threshold;

        CurvedCutSegmenterToISegmentLineAdapter() {
            small_merge_threshold = 0;
        }

        const char *name() {
            return "curvedcut";
        }

        virtual const char *description() {
            return "curved cut segmenter";
        }

        virtual void set(const char *key,const char *value) {
            log_main.format("set parameter %s to sf", key, value);
            if(!strcmp(key,"debug"))
                segmenter->debug = value;
            else
                throw "unknown key";
        }

        virtual void set(const char *key,double value) {
            log_main.format("set parameter %s to %f", key, value);
            if(!strcmp(key,"down_cost"))
                segmenter->down_cost = (int)value;
            else if(!strcmp(key,"small_merge_threshold"))
                small_merge_threshold = (int)value;
            else if(!strcmp(key,"outside_diagonal_cost"))
                segmenter->outside_diagonal_cost = (int)value;
            else if(!strcmp(key,"inside_diagonal_cost"))
                segmenter->inside_diagonal_cost = (int)value;
            else if(!strcmp(key,"boundary_diagonal_cost"))
                segmenter->boundary_diagonal_cost = (int)value;
            else if(!strcmp(key,"outside_weight"))
                segmenter->outside_weight = (int)value;
            else if(!strcmp(key,"boundary_weight"))
                segmenter->boundary_weight = (int)value;
            else if(!strcmp(key,"inside_weight"))
                segmenter->inside_weight = (int)value;
            else if(!strcmp(key,"min_range"))
                segmenter->min_range = (int)value;
            else if(!strcmp(key,"min_thresh"))
                segmenter->min_thresh = value;
            else
                throw "unknown key";
        }

        virtual void charseg(intarray &result_segmentation,bytearray &orig_image) {
            log_main("segmenting", orig_image);
            enum {PADDING = 3};
            bytearray image;
            copy(image, orig_image);
            optional_check_background_is_lighter(image);
            binarize_simple(image);
            invert(image);
            pad_by(image, PADDING, PADDING);
            intarray segmentation;
            // pass image to segmenter
            segmenter->setImage(image);
            // find all cuts in the image
            segmenter->findAllCuts();
            // choose the best of all cuts
            segmenter->findBestCuts();

            segmentation.resize(image.dim(0),image.dim(1));
            for(int i=0;i<image.dim(0);i++) for(int j=0;j<image.dim(1);j++)
                                                segmentation(i,j) = image(i,j)?1:0;
            for(int r=0;r<segmenter->bestcuts.length();r++) {
                int c = segmenter->bestcuts(r);
                narray<point> &cut = segmenter->cuts(c);
                for(int y=0;y<image.dim(1);y++) {
                    for(int x=cut(y).x;x<image.dim(0);x++)
                        if(segmentation(x,y)) segmentation(x,y)++;
                }
            }
            extract_subimage(result_segmentation,segmentation,PADDING,PADDING,
                             segmentation.dim(0)-PADDING,segmentation.dim(1)-PADDING);

            if(small_merge_threshold>0) {
                line_segmentation_merge_small_components(result_segmentation,small_merge_threshold);
                line_segmentation_sort_x(result_segmentation);
            }

            make_line_segmentation_white(result_segmentation);
            // set_line_number(result_segmentation, 1);
            log_main("resulting segmentation", result_segmentation);
        }
    };



    ISegmentLine *make_CurvedCutSegmenter() {
        return new CurvedCutSegmenterToISegmentLineAdapter();
    }

    struct CurvedCutSegmenterToISegmentLineAdapterWithCc:
            CurvedCutSegmenterToISegmentLineAdapter {
        virtual void charseg(intarray &result_segmentation,bytearray &orig_image) {
            bytearray image;
            copy(image, orig_image);
            optional_check_background_is_lighter(image);
            binarize_simple(image);
            invert(image);

            intarray ccseg;
            copy(ccseg, image);
            label_components(ccseg);

            CurvedCutSegmenterToISegmentLineAdapter::charseg(result_segmentation, orig_image);
            combine_segmentations(result_segmentation, ccseg);
        }
    };

    ISegmentLine *make_CurvedCutWithCcSegmenter() {
        return new CurvedCutSegmenterToISegmentLineAdapterWithCc();
    }
}
