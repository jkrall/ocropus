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
// File: ocr-pageframe-rast.cc
// Purpose: Page Frame Detection using RAST
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace colib;
using namespace iulib;

namespace ocropus {
    param_bool debug_pf("debug_pf",0,"save intermediate images");

    // Remove noise regions from an input image
    static void remove_noise_boxes(rectarray &bboxes, bytearray &image){

        int border_margin=25;
        rectarray out_bboxes,noise_boxes;
        int x0, y0, x1, y1;
        int image_width   = image.dim(0);
        int image_height  = image.dim(1);
        for (int i = 0; i < bboxes.length(); i++){
            if(!bboxes[i].area() || bboxes[i].area()>=image_width*image_height)
                continue;
            //bboxes[i].println();
            x0 = ( bboxes[i].x0 > 0 ) ? bboxes[i].x0 : 0;
            y0 = ( bboxes[i].y0 > 0 ) ? bboxes[i].y0 : 0;
            x1 = ( bboxes[i].x1 < image_width)  ? bboxes[i].x1 : image_width-1;
            y1 = ( bboxes[i].y1 < image_height) ? bboxes[i].y1 : image_height-1;
            if(x1<=x0 || y1<=y0)
                continue;

            if(x0<border_margin || y0<border_margin ||
               x1>image.dim(0)-border_margin ||
               y1>image.dim(1)-border_margin){
                noise_boxes.push(bboxes[i]);
                continue;
            }
            out_bboxes.push(bboxes[i]);
        }
        bboxes.clear();
        copy(bboxes,out_bboxes);
        if(debug_pf){
            intarray noiseimage;
            draw_filled_rects(noiseimage,image,noise_boxes);
            write_image_packed("noise-zones.png",noiseimage);
        }
    }

    void PageFrameRAST::setDefaultParameters() {
        generation = 0;
        lsq        = 1;
        epsilon    = 150.0;
        maxsplits  = 1;
        delta      = 2.0;

        min_overlap = 0.8;
        min_q       = 3.0;
        min_count   = 6;
        max_results = 1000;

        splitscale[0] = 1.0;
        splitscale[1] = 1.0;
        splitscale[2] = 1.0;
        splitscale[3] = 1.0;
        all_params[0] = interval(0,800);
        all_params[1] = interval(2,3);
        all_params[2] = interval(100,2500);
        all_params[3] = interval(3300,3301);
        empty_parameters[0] = interval(1,0);
        empty_parameters[1] = interval(1,0);
        empty_parameters[2] = interval(1,0);
        empty_parameters[3] = interval(1,0);

    }
    void PageFrameRAST::setSearchParameters(bytearray &in) {
        int width  = in.dim(0);
        int height = in.dim(1);

        all_params[0] = interval(0,width/3);
        all_params[1] = interval(2,3);
        all_params[2] = interval(width/3,width);
        all_params[3] = interval(height-2,height-1);
    }

    PageFrameRAST::PageFrameRAST(){
        setDefaultParameters();
    }

    PageFrameRAST::PFStateBasic::PFStateBasic() {
        depth = -(1<<14);
        quality = 0.0;
        matches.clear();
        splits = 0;
        splittable = true;
    }

    void PageFrameRAST::PFStateBasic::set(PageFrameRAST &pf,
                                          int depth,Parameters &params,
                                          Matches &candidates,int splits) {
        this->splits = splits;
        this->splittable = (splits<pf.maxsplits);
        this->depth = depth;
        this->params = params;
        rank = -1;
        matches.clear();
        update(pf,candidates);
    }

    void PageFrameRAST::PFStateBasic::reeval(PageFrameRAST &pf) {
        Matches temp,rest;
        copy(temp,matches);
        copy(matches,rest);
        //temp.swapwith(matches);
        update(pf,temp);
    }

    void PageFrameRAST::PFStateBasic::update(PageFrameRAST &pf,
                                             Matches &candidates) {
        matches.clear();
        quality = 0.0;
        interval left   = params[0];
        interval top    = params[1];
        interval right  = params[2];
        interval bottom = params[3];
        interval qleft   = 0.0;
        interval qright  = 0.0;
        interval qtop    = 0.0;
        interval qbottom = 0.0;

        for(int i=0;i<candidates.length();i++) {
            int bi = candidates[i];
            if(pf.used[bi]) continue;

            rectangle &b = pf.lineboxes[bi];

            interval q1 = pf.influence(pf.lsq,abs(b.x0-left),pf.epsilon);
            interval q1neg = pf.influence(pf.lsq,abs(b.x1-left),pf.epsilon*2.0);
            interval q2 = 0.0;//pf.influence(pf.lsq,abs(b.y0-top),pf.epsilon);
            interval q3 = pf.influence(pf.lsq,abs(b.x1-right),pf.epsilon);
            interval q3neg = pf.influence(pf.lsq,abs(b.x0-right),pf.epsilon*2.0);
            interval q4 = 0.0;//pf.influence(pf.lsq,abs(b.y1-bottom),pf.epsilon);

            qleft   = qleft   + q1;
            qtop    = qtop    + q2;
            qright  = qright  + q3;
            qbottom = qbottom + q4;

            interval q1dif  = q1 - q1neg;
            interval q3dif  = q3 - q3neg;
            interval q      = q1dif + q3dif;
            if(abs(q).hi > 0.0){
              matches.push(bi);
            }
            quality = quality + q;
        }

        interval minq12 = min(qleft,qright);
        if(minq12 < pf.min_q)
            quality = 0;

        // check for total length; we can do that because the cboxes
        // are in sorted order
        if(matches.length() >= 2) {
            int start = matches[0];
            int end = matches[matches.length()-1];
            float length = pf.lineboxes[end].x1-pf.lineboxes[start].x0;
            if(length<pf.min_length) quality = 0;
        } else {
            quality = 0;
        }

        priority = quality.hi;
        generation = pf.generation;
    }

    void PageFrameRAST::prepare() {
        all_matches.clear();
        for(int i = 0;i<lineboxes.length();i++)
            all_matches.push(i);

        used.resize(lineboxes.length());
        fill(used,false);

        CState initial;
        initial->set(*this,0,all_params,all_matches,0);

        queue.clear();
        queue.insert(initial,initial->priority);
    }

    void PageFrameRAST::makeSubStates(narray<CState> &substates,CState &state) {
        substates.clear();
        Parameters &p = state->params;

        int mi = -1;
        double mv = -1e30;
        for(int i = 0;i<npfparams;i++) {
            double v = p[i].width()*splitscale[i];
            if(v>mv) { mi = i; mv = v; }
        }

        Parameters left = state->params.split(mi,0);
        CState sleft;
        sleft->set(*this,state->depth+1,left,state->matches,state->splits);
        substates.push(sleft);

        Parameters right = state->params.split(mi,1);
        CState sright;
        sright->set(*this,state->depth+1,right,state->matches,state->splits);
        substates.push(sright);
    }

    void PageFrameRAST::search() {
        for(int iter = 0;;iter++) {
            if(results.length() >= max_results) break;
            if(queue.length()<1) break;
            CState top;
            top = queue.extractMax();
            if(top->generation != generation) {
                top->reeval(*this);
                if(top->quality.hi<min_q) continue;
                if(top->matches.length()<min_count) continue;
                queue.insert(top,top->priority);
                continue;
            }
            if(final(top->quality,top->params)) {
                pushResult(top);
                break;
            }
            narray<CState> substates;
            makeSubStates(substates,top);
            for(int i = 0;i<substates.length();i++) {
                if(substates[i]->quality.hi<min_q) continue;
                if(substates[i]->matches.length()<min_count) continue;
                queue.insert(substates[i],substates[i]->priority);
            }
        }
    }

    void PageFrameRAST::pushResult(CState &result){
        Matches &matches = result->matches;
        for(int i=0;i<matches.length();i++)
            used[matches[i]] = true;

        results.push(result);
        generation++;
    }

    void PageFrameRAST::cleanup(bytearray &out,bytearray &in){
        if(!contains_only(in,byte(0),byte(255))){
            fprintf(stderr,"Error: Page frame detection ");
            fprintf(stderr,"needs binary image as input.\n");
            exit(1);
        }

        intarray lineimage,zoneimage;
        rectarray lineboxesall,zoneboxesall;

        autodel<ISegmentPage> segmenter(make_SegmentPageByRAST());
        segmenter->segment(lineimage,in);
        replace_values(lineimage,0x00ffff00,0x00ffffff);
        bounding_boxes(lineboxesall,lineimage);
        for(int i=0,l=lineboxesall.length()-1;i<l;i++)
            if(lineboxesall[i].area()){
                lineboxes.push(lineboxesall[i]);
            }

        autodel<ISegmentPage> voronoi(make_SegmentPageByVORONOI());
        voronoi->set("remove_noise",true);
        voronoi->segment(zoneimage,in);
        bounding_boxes(zoneboxesall,zoneimage);
        for(int i=0,l=zoneboxesall.length()-1;i<l;i++){
            if(zoneboxesall[i].area()){
                zoneboxes.push(zoneboxesall[i]);
            }
        }
        remove_noise_boxes(zoneboxes,in);
        if(debug_pf){
            draw_rects(lineimage,in,lineboxes);
            draw_rects(zoneimage,in,zoneboxes);
            write_image_packed("lineimage.png",lineimage);
            write_image_packed("zoneimage.png",zoneimage);
        }
        rectangle pageframe;

        if(lineboxes.length() == 0){
            pageframe = rectangle();
            int vboxes = zoneboxes.length();
            if(vboxes){
                for(int i=0; i< vboxes; i++)
                    pageframe.include(zoneboxes[i]);
            }
            else
                pageframe = rectangle(1,1,in.dim(0)-1,in.dim(1)-1);
            remove_border_noise(out,in,pageframe);
            return;
        }

        // search for optimal page frame based on line boxes used RAST
        setSearchParameters(in);
        prepare();
        search();

        if(results.length() == 0){
            pageframe = rectangle();
            int vboxes = zoneboxes.length();
            if(vboxes){
                for(int i=0; i< vboxes; i++)
                    pageframe.include(zoneboxes[i]);
            }
            else
                pageframe = rectangle(1,1,in.dim(0)-1,in.dim(1)-1);
            remove_border_noise(out,in,pageframe);
            return;
        }

        pageframe.x0 = results[0]->params[0].center();
        pageframe.x1 = results[0]->params[2].center();
        pageframe.y0 = results[0]->params[1].center();
        pageframe.y1 = results[0]->params[3].center();
        rectangle pageframe_initial = pageframe;

        rectangle line_bb,zone_bb;
        for(int i=0, l=lineboxes.length(); i<l; i++){
            if(lineboxes[i].fraction_covered_by(pageframe) >= min_overlap){
                //pageframe.include(lineboxes[i]);
                line_bb.include(lineboxes[i]);
            }
        }
        for(int i=0, l=zoneboxes.length(); i<l; i++){
            if(zoneboxes[i].fraction_covered_by(pageframe) >= min_overlap){
                zone_bb.include(zoneboxes[i]);
            }
        }

        pageframe.x0 = min(line_bb.x0,zone_bb.x0);
        pageframe.x1 = max(line_bb.x1,zone_bb.x1);
        pageframe.y0 = min(line_bb.y0,zone_bb.y0);
        pageframe.y1 = max(line_bb.y1,zone_bb.y1);

        rectarray pfarray;
        pfarray.push(pageframe);
        if(debug_pf){
            draw_filled_rects(lineimage,in,pfarray);
            paint_box_border(lineimage,pageframe_initial,0x000000ff);
            write_image_packed("pfimage.png",lineimage);
        }
        remove_border_noise(out,in,pageframe);
    }

    ICleanupBinary *make_PageFrameRAST() {
        return new PageFrameRAST();
    }

    void remove_border_noise(bytearray &out,
                             bytearray &in,
                             rectangle &pageframe){
        makelike(out,in);
        fill(out,255);
        rectangle imagedim = rectangle(0,0,in.dim(0),in.dim(1));
        if(imagedim.contains(pageframe.x0,pageframe.y0) &&
           imagedim.contains(pageframe.x1-1,pageframe.y1-1)){
            for(int x=pageframe.x0;x<pageframe.x1;x++)
                for(int y=pageframe.y0;y<pageframe.y1;y++)
                    if(!in(x,y))
                        out(x,y)=in(x,y);
        }else{
            fprintf(stderr,"Error: page frame exceeds image dimensions.\n");
            pageframe.println(stderr);
        }
    }


}

