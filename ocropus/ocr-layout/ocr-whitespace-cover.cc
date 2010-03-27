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
// File: ocr-whitespace-cover.cc
// Purpose: Compute the whitespace cover of the page background
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace colib;

namespace ocropus {

    // Done when the bounds rectangle fits along the borders of the rectangles
    // in stack 'matches'
    bool WhitespaceCover::WState::isDone(WhitespaceCover *env) {
        for(int i=0;i<matches->length();i++) {
            rectangle &r = env->rects[matches->at(i)];
            if(r.x1!=bounds.x0 &&
               r.y1!=bounds.y0 &&
               r.x0!=bounds.x1 &&
               r.y0!=bounds.y1) return false;
        }
        return true;
    }
    
    void WhitespaceCover::WState::update(WhitespaceCover *env) {
        // assume that "bounds" has been set to the new bounds
        rectangle nbounds;
        CMatches nmatches;
        for(int i=0;i<matches->length();i++) {
            int index = matches->at(i);
            rectangle &r = env->rects[index];
            if(!bounds.overlaps(r)) continue;
            nmatches->push(index);
            if(nmatches->length()==1) {
                nbounds.x0 = r.x1;
                nbounds.y0 = r.y1;
                nbounds.x1 = r.x0;
                nbounds.y1 = r.y0;
            } else {
                nbounds.x0 = min(nbounds.x0,r.x1); //<?= r.x1;
                nbounds.y0 = min(nbounds.y0,r.y1); //<?= r.y1;
                nbounds.x1 = max(nbounds.x1,r.x0); //>?= r.x0;
                nbounds.y1 = max(nbounds.y1,r.y0); //>?= r.y0;
            }
        }
        bounds = nbounds;
        matches = nmatches;
        switch(env->quality_func){
        case area:    weight = bounds.area(); break;
        case width:   weight = bounds.area()*bounds.width()*bounds.width(); break;
        case height:  weight = bounds.area()*bounds.height()*bounds.height(); break;
        default:      weight = bounds.area(); break;
        }
    }
    
    int WhitespaceCover::WState::maxCentricity(WhitespaceCover *env) {
        int mi = -1;
        float mv = -1.0;
        for(int i=0;i<matches->length();i++) {
            int index = matches->at(i);
            rectangle &r = env->rects[index];
            float v = bounds.centricity(r);
            if(v<0) throw "oops";
            if(v<mv) continue;
            mv = v;
            mi = i;
        }
        return matches->at(mi);
    }
    
    WhitespaceCover::WhitespaceCover() {
        verbose       = -1;
        bounds        = rectangle(0,0,3000,4000);
        max_results   = 300;
        min_weight    = 100.0;
        max_overlap   = 0.95;
        greedy        = false;
        min_aspect    = 0.0;
        max_aspect    = 1e38;
        min_width     = 5.0;
        min_height    = 5.0;
        logmin_aspect = 0.0;
        quality_func  = area;
    }

    WhitespaceCover::WhitespaceCover(rectangle image_boundary) {
        verbose       = -1;
        bounds        = image_boundary;
        max_results   = 300;
        min_weight    = 100.0;
        max_overlap   = 0.95;
        greedy        = false;
        min_aspect    = 0.0;
        max_aspect    = 1e38;
        min_width     = 5.0;
        min_height    = 5.0;
        logmin_aspect = 0.001;
        quality_func  = area;
    }

    const char * WhitespaceCover::description() {
        return "Compute the whitespace cover of the page background\n";
    }
    
    void WhitespaceCover::init() {
        // nothing to be done
    }

    void WhitespaceCover::compute(rectarray &whitespaces,
                                  rectarray &obstacles) {
        
        for(int i=0; i<obstacles.length(); i++) {
            if(obstacles[i].area())
                addRect(rectangle(obstacles[i]));
        }
        //snug_bounds();
        compute();
        for(int i=0;i<nSolutions();i++) {
            int x0,y0,x1,y1;
            solution(i,x0,y0,x1,y1);
            if(verbose==0)
                printf("%d %d %d %d\n",x0,y0,x1,y1);
            whitespaces.push(rectangle(x0,y0,x1,y1));
        }
    }

    bool WhitespaceCover::goodDimensions(CState &result){
        float aspect = result->bounds.aspect();
        if(!aspect) return false;
        if(aspect<min_aspect||aspect>max_aspect) return false;
        double alar = (double)log(aspect)/(double)log(2.0) ;
        alar = (alar<=0) ? -alar : alar;
        if(alar<logmin_aspect) return false;
        float width = result->bounds.width();
        if(width < min_width) return false;
        float height = result->bounds.height();
        if(height < min_height) return false;

        return true;
    }
    
    void WhitespaceCover::generateChildStates(CState &state, 
                                                colib::rectangle &pivot){

        for(int i=0;i<4;i++) {
            CState child;
            child->current_nrects = state->current_nrects;
            child->matches = state->matches;
            child->bounds = state->bounds;
            rectangle &b = child->bounds;
            switch(i) {
            case 0: b.x0 = pivot.x1; break;
            case 1: b.x1 = pivot.x0; break;
            case 2: b.y0 = pivot.y1; break;
            case 3: b.y1 = pivot.y0; break;
            }
            child->update(this);
            queue.insert(child,child->weight);
        }
    }

    void WhitespaceCover::compute() {
        rectangle &b = bounds;
        initial_nrects = rects.length();
        // Push four single row/column rects around the boundary 
        // of the rect. 'bounds'. These rectangles serve the purpose
        // of obstacles for finding the maximal whitespace rectangles
        // by separating the fringe area from the contents.
        rects.push(rectangle(b.x0-1,b.y0,b.x0,b.y1));
        rects.push(rectangle(b.x1,b.y0,b.x1+1,b.y1));
        rects.push(rectangle(b.x0,b.y0-1,b.x1,b.y0));
        rects.push(rectangle(b.x0,b.y1,b.x1,b.y1+1));
        CState initial;
        initial->bounds = bounds;
        int lr = rects.length();
        for(int i=0;i<lr;i++) initial->matches->push(i);
        initial->current_nrects = rects.length();
        initial->update(this);
        queue.insert(initial,initial->weight);
            
        for(int iter=0;;iter++) {
            if(queue.length()<1) break;
            CState state;
            state = queue.extractMax(); //Extract 'bounds' rect. with max. area
            if(state->weight<min_weight) break; //Break if area is below min_weight

            //If a new solution has been added to obstacles, recompute 
            // the quality of the node and re-enqueue it.
            if(state->current_nrects!=rects.length()) {
                for(int i=state->current_nrects;i<rects.length();i++)
                    state->matches->push(i);
                state->current_nrects = rects.length();
                state->update(this);
                queue.insert(state,state->weight);
                continue;
            }
                
            if(state->isDone(this)) { 
                if(!goodDimensions(state))
                    continue;
                if(greedy) { //If greedy = 1, push bounds as a rectangle
                    rects.push(state->bounds);
                } else {
                    rectangle nb = state->bounds;
                    bool good = true;
                    for(int i=0;i<results.length();i++) {
                        float covered = nb.fraction_covered_by(results[i]->bounds);
                        if(covered>max_overlap) {
                            good = false;
                            break;
                        }
                    }
                    if(!good) continue;
                }
                results.push(state);
                if(results.length()>=max_results) break;
            }

            int index = state->maxCentricity(this);
            rectangle &r = rects[index];
            generateChildStates(state, r);
        }
    }
    WhitespaceCover *make_WhitespaceCover(rectangle &r) {
        return new WhitespaceCover(r);
    }
    WhitespaceCover *make_WhitespaceCover(int x0, int y0, int x1, int y1) {
        return new WhitespaceCover(rectangle(x0,y0,x1,y1));
    }
}

