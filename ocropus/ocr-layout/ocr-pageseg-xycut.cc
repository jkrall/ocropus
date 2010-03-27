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
// This version of the xycut algorithm has been implemented according to
// the algorithm described in "Empirical Performance Evaluation Methodology and
// Its Application to Page Segmentation Algorithms" by Mao and Kanungo(Figure 3)
// 
// Project: OCRopus
// File: ocr-pageseg-xycuts.cc
// Purpose: Page segmentation using XYCuts 
// Responsible: Joost van Beusekom (joost@iupr.net)
// Reviewer: Faisal Shafait (faisal.shafait@dfki.de)
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de

#include "ocropus.h"
#include "ocr-layout-internal.h"

using namespace iulib;
using namespace colib;

namespace ocropus {
    
    param_int debug_xycut_intermediate("debug",0,"print intermediate results"
        "to stdout");
    
    // compute integral image for BINARY images
    static void compute_integral_image(bytearray& img, 
                                       intarray& int_img_hor, 
                                       intarray& int_img_ver){
        int_img_ver.resize(img.dim(0), img.dim(1)) ;
        int_img_hor.resize(img.dim(0), img.dim(1)) ;
        for (int x = 0; x < int_img_ver.dim(0); x++) {
            for (int y = 0; y < int_img_ver.dim(1); y++) {
                int_img_ver(x,y) = 0 ;
                int_img_hor(x,y) = 0 ;
            }
        }
        for (int x = 0; x < int_img_ver.dim(0); x++) {
            int count = 0 ;
            for (int y = 0; y < int_img_ver.dim(1); y++) {
                if (img(x,y) == 0) {
                    count++ ;
                }
                int_img_ver(x,y) = count ;
            }
        }
        
        for (int y = 0; y < int_img_hor.dim(1); y++) {
            int count = 0 ;
            for (int x = 0; x < int_img_hor.dim(0); x++) {
                if (img(x,y) == 0) {
                    count++ ;
                }
                int_img_hor(x,y) = count ;
            }
        }
    }

    // Compute horizontal and vertical projection profiles for a given rectangle
    // r out of integral image
    static void get_projection_profiles(intarray& proj_on_yaxis, 
                                        intarray& proj_on_xaxis,
                                        rectangle& r, 
                                        intarray& int_img_hor, 
                                        intarray& int_img_ver) {
        // Mao. Fig.3. 2.a)
        proj_on_yaxis.resize((int)r.y1 - (int)r.y0) ;
        fill(proj_on_yaxis,0);

        proj_on_xaxis.resize((int)r.x1 - (int)r.x0) ;
        fill(proj_on_xaxis,0);

        // JvB: y<= (int)...: all pixels inside the rectangle have to be included.
        for (int y = (int)r.y0; y < (int)r.y1; y++){
            int m=0;
            if(r.x0-1>=0)
                m=int_img_hor((int)r.x0-1, y);
            proj_on_yaxis(y - (int)r.y0) = int_img_hor((int)r.x1-1, y) - m;
        }
        for (int x = (int)r.x0; x < (int)r.x1; x++){
            int m=0;
            if(r.y0-1>=0)
                m=int_img_ver(x,(int)r.y0-1);
            proj_on_xaxis(x - (int)r.x0) = int_img_ver(x, (int)r.y1-1) - m;
        }
    }


    // shrink to the bounding boy of all the black pixels in the rectangle and
    // remove noise
    static void shrink_and_clean(rectangle& r, 
                                 intarray& proj_on_yaxis, 
                                 intarray& proj_on_xaxis, 
                                 double f_tnx, 
                                 double f_tny) {
        // Mao. Fig.3. 2.b)
        // shrinking
        ASSERT(r.area());
        ASSERT(proj_on_xaxis.dim(0));
        ASSERT(proj_on_xaxis.dim(0));
        ASSERT(sum(proj_on_xaxis)!=0);
        ASSERT(sum(proj_on_xaxis)==sum(proj_on_yaxis));
        int xbegin = 0, xend = proj_on_xaxis.dim(0) - 1 ;
        while (xbegin < proj_on_xaxis.length() && proj_on_xaxis(xbegin) <= 0)
            xbegin++ ;
        while (xend >= 0 && proj_on_xaxis(xend) <= 0)
            xend-- ;
        int ybegin = 0, yend = proj_on_yaxis.dim(0) - 1 ;
        while (ybegin < proj_on_yaxis.length() && proj_on_yaxis(ybegin) <= 0)
            ybegin++ ;
        while (yend >= 0 && proj_on_yaxis(yend) <= 0)
            yend-- ;

        
        int old_x0 = (int)r.x0 ;
        int old_y0 = (int)r.y0 ;
        r.x0 = old_x0 + xbegin ;
        r.x1 = old_x0 + xend+1 ;
        r.y0 = old_y0 + ybegin ;
        r.y1 = old_y0 + yend+1 ;

        // Mao. Fig.3. 2.b) & 2.c)
        // cleaning
        for (int i = 0; i < proj_on_yaxis.dim(0); i++)
            proj_on_yaxis(i) = proj_on_yaxis(i) - (int)(r.width()*f_tny) ;
        for (int i = 0; i < proj_on_xaxis.dim(0); i++)
            proj_on_xaxis(i) = proj_on_xaxis(i) - (int)(r.height()*f_tnx) ;
        
    }


    // returns the widest non-starting or ending white gape of the two
    // projection profiles
    static void get_widest_gap(int& cut_pos_y, 
                               int& gap_y,
                               int& cut_pos_x, 
                               int& gap_x,  
                               intarray& proj_on_yaxis,
                               intarray& proj_on_xaxis) {
        int gap_hor = -1, gap_ver = -1, pos_hor = -1, pos_ver = -1 ;
        int begin = -1 ;
        int end = -1 ;
        // find gap in y-axis projection
        for (int i = 1; i < proj_on_yaxis.dim(0); i++) {
            if (begin>=0
                && proj_on_yaxis(i-1) <= 0 
                && proj_on_yaxis(i) > 0)
                end = i ;
            if (proj_on_yaxis(i-1) > 0 && proj_on_yaxis(i) <= 0)
                begin = i ;
            if (begin > 0 && end > 0 && end-begin > gap_hor) {
                gap_hor = end - begin ;
                pos_hor = (end + begin) / 2 ;
                begin = -1 ;
                end = -1 ;
            }
        }
        
        begin = -1 ;
        end = -1 ;
        // find gap in x-axis projection
        for (int i = 1; i < proj_on_xaxis.dim(0); i++) {
            if (begin>=0
                && proj_on_xaxis(i-1) <= 0 
                && proj_on_xaxis(i) > 0)
                end = i ;
            if (proj_on_xaxis(i-1) > 0 && proj_on_xaxis(i) <= 0)
                begin = i ;
            if (begin > 0 && end > 0 && end-begin > gap_ver) {
                gap_ver = end - begin ;
                pos_ver = (end + begin) / 2 ;
                begin = -1 ;
                end = -1 ;
            }
        }
        
        cut_pos_y = pos_hor;
        gap_y = gap_hor;
        
        cut_pos_x = pos_ver;
        gap_x = gap_ver;
        
    }

    // Split rectangle by returning the coordinates of two new rectangles r1, r2
    static void split_rect(rectangle& r1, 
                           rectangle& r2, 
                           rectangle& r, 
                           int dir, 
                           int pos) {
        if (dir == HORIZONTAL_CUT) { // horizontal cut
            r1 = rectangle(r.x0, r.y0, r.x1, pos) ; // lower rectangle
            r2 = rectangle(r.x0, pos, r.x1, r.y1) ; // upper rectangle
        }
        if (dir == VERTICAL_CUT) { // vertical cut
            r1 = rectangle(r.x0, r.y0, pos, r.y1) ; // left rectangle
            r2 = rectangle(pos, r.y0, r.x1, r.y1) ; // rigth rectangle
        }
    }
    
    static void insertRectToTree(int index, 
                                 rectarray& tree, 
                                 rectangle rL, 
                                 rectangle rR) {
        int left = index * 2 + 1 ;
        int right = index * 2 + 2 ;
        while (right+1 > tree.length()) // increased size of tree if necessary
            tree.push(rectangle(-1, -1, -1, -1)) ;
        tree[left] = rL ;
        tree[right] = rR ;
    }

    static void xycut(rectarray& blocks, 
                      rectarray& tree, 
                      intarray& int_img_hor,
                      intarray& int_img_ver, 
                      int tnx, int tny, int tcx, int tcy) {
        // arrays to save the projection profile
        intarray proj_on_yaxis ; // his_x
        intarray proj_on_xaxis ; // his_y
        
        // Mao. Fig.3. 2.0)
        // adding whole page as root node
        rectangle page = rectangle(0, 0, int_img_ver.dim(0),
                        int_img_ver.dim(1)) ;
        tree.push(page) ; // push root rectangle to tree
        
        // compute factor that is needed for noise removal
        if (int_img_hor.dim(0) <= 0 || 
            int_img_ver.dim(1) <= 0 || 
            int_img_hor.length1d() != int_img_ver.length1d()) {
            fprintf(stderr, "ocr-pageseg-xycut: xycut: Error in image dim!\n");
            exit(0) ;
        }
        double factor_tnx = (double)tnx / (double)int_img_hor.dim(1) ;
        double factor_tny = (double)tny / (double)int_img_ver.dim(0) ;
        
        int i = 0 ;
        rectangle r = tree.at(0) ;
        while (i < tree.length()) {
            // take node in queue and process it
            
            if (tree[i].x0 >= 0 
                && tree[i].y0 >= 0 
                && tree[i].x1 >= 0 
                && tree[i].y1 >= 0) {
                // compute projection profile
                get_projection_profiles(proj_on_yaxis, proj_on_xaxis, 
                                        r, int_img_hor, int_img_ver) ;
                
                
            // Mao. Fig.3. 2.b) & 2.c)
            // save old starting points of rectangle, before shrinking
                int old_x0 = (int)r.x0 ;
                int old_y0 = (int)r.y0 ;
            // remove noise and shrink to effectively used area
                shrink_and_clean(r, proj_on_yaxis, proj_on_xaxis, factor_tnx, factor_tny);
            // changes the values of r!!!
        
            // Mao. Fig.3. 2.d)
//                 int dir = -1 ;
                int cut_pos_y = -1 ;
                int gap_y = -1 ;
                int cut_pos_x = -1 ;
                int gap_x = -1 ;

                
            // find widest gap and its middle and the direction of the cut to be
            // done (0 = hor, 1 = ver)
                get_widest_gap(cut_pos_y, gap_y, 
                               cut_pos_x, gap_x, 
                               proj_on_yaxis, proj_on_xaxis) ;
            
                // split in the right direction
                if (gap_y >= gap_x && gap_y > tcy) {
                    rectangle rB, rT ; // rectangleTop rectangleBottom
                    split_rect(rB, rT, r, HORIZONTAL_CUT, old_y0 + cut_pos_y) ;
                    insertRectToTree(i, tree, rB, rT) ;
                }
                else if (gap_y >= gap_x && gap_y <= tcy && gap_x > tcx) {
                    rectangle rL, rR ; // rectangleTop rectangleBottom
                    split_rect(rL, rR, r, VERTICAL_CUT, old_x0 + cut_pos_x) ;
                    insertRectToTree(i, tree, rL, rR) ;
                }
                else if (gap_x > gap_y && gap_x > tcx) {
                    rectangle rL, rR ; // rectangleTop rectangleBottom
                    split_rect(rL, rR, r, VERTICAL_CUT, old_x0 + cut_pos_x) ;
                    insertRectToTree(i, tree, rL, rR) ;
                }
                else if (gap_x > gap_y && gap_x <= tcx && gap_y > tcy) {
                    rectangle rL, rR ; // rectangleTop rectangleBottom
                    split_rect(rL, rR, r, HORIZONTAL_CUT, old_y0 + cut_pos_y) ;
                    insertRectToTree(i, tree, rL, rR) ;
                }
                
                else {
                //insert dummy rects
                    rectangle rL = rectangle(-1, -1, -1, -1) ;
                    rectangle rR = rectangle(-1, -1, -1, -1) ;
                // insert leaf rect to result blocks
                    rectangle ret_rect = rectangle(r.x0, r.y0, r.x1, r.y1) ;
                    blocks.push(ret_rect) ;
                }
            }
            i++ ;
            if(i<tree.length())
                r = tree[i] ;
        }
    }

    
    void SegmentPageByXYCUTS::setParameters(unsigned int itnx, 
                                            unsigned int itny, 
                                            unsigned int itcx, 
                                            unsigned int itcy) {
        tnx=itnx ;
        tny=itny ;
        tcx=itcx ;
        tcy=itcy ;
        if (tcx <= 0) {
            fprintf(stderr, "ocr-pageseg-xycut:tcx value too low; tcx = 1\n");
            tcx = 1 ;
        }
        if (tcy <= 0) {
            fprintf(stderr, "ocr-pageseg-xycut:tcy value too low; tcy = 1\n");
            tcy = 1 ;
        }
    }
    
    SegmentPageByXYCUTS::SegmentPageByXYCUTS(unsigned int itnx, 
                             unsigned int itny, 
                             unsigned int itcx, 
                             unsigned int itcy) {
        tnx=itnx ;
        tny=itny ;
        tcx=itcx ;
        tcy=itcy ;
        if (tcx <= 0) {
            fprintf(stderr, "ocr-pageseg-xycut:tcx value too low; tcx = 1\n");
            tcx = 1 ;
        }
        if (tcy <= 0) {
            fprintf(stderr, "ocr-pageseg-xycut:tcy value too low; tcy = 1\n");
            tcy = 1 ;
        }
    }
    
    SegmentPageByXYCUTS::SegmentPageByXYCUTS() {
        tnx=78 ;
        tny=32 ;
        tcx=35 ;
        tcy=54 ;
    }
    
    
    void SegmentPageByXYCUTS::segment(intarray &image,bytearray &in) {
        if(!contains_only(in,byte(0),byte(255))){
            fprintf(stderr,"X-Y Cut algorithm needs binary input image.\n");
            exit(1);
        }
        
        // Mao, Fig.3: Step 1.
        intarray int_img_hor ;
        intarray int_img_ver ;
        compute_integral_image(in, int_img_hor, int_img_ver) ;
        
        // Mao, Fig.3: Step 2.
        rectarray blocks ;
        rectarray tree ;
        
        xycut(blocks, tree, int_img_hor, int_img_ver, tnx, tny, tcx, tcy) ;
        
        int cflen = blocks.length();
        
        makelike(image,in);
        fill(image,0x00ffffff);
        for(int i=0; i<cflen; i++){
            // Do a logical OR of zone index with R=1 to assign all
            // zones to the first column
            int color = (i+1)|(0x00010000);
            rectangle r = blocks[i];

            for(int x=r.x0; x<r.x1; x++){
                for(int y=r.y0; y<r.y1; y++){
                    if(in(x,y) == 0)
                        image(x,y) = color ;
                }
            }
        }
        if(debug_xycut_intermediate){
            for (int i = 0; i < blocks.length(); i++) {
                fprintf(stderr,"blocks[%d] = %d\t%d\t%d\t%d\n", i, 
                        blocks[i].x0, blocks[i].y0, blocks[i].x1, blocks[i].y1);
            }
            write_image_packed("xycuts-cuts.png",image);
            write_image_packed("xycuts-inthor.png",int_img_hor);
            write_image_packed("xycuts-intver.png",int_img_ver);
        }
    }

    ISegmentPage *make_SegmentPageByXYCUTS() {
        return new SegmentPageByXYCUTS();
    }

    ISegmentPage *make_SegmentPageByXYCUTS(unsigned int itnx, 
                                           unsigned int itny, 
                                           unsigned int itcx, 
                                           unsigned int itcy) {
        return new SegmentPageByXYCUTS(itnx, itny, itcx, itcy);
    }
}


