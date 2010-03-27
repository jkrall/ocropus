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

#define __warn_unused_result__ __far__

#include <cctype>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ocropus.h"
#include "glinerec.h"
#include "glutils.h"

using namespace iulib;
using namespace colib;
using namespace ocropus;
using namespace narray_ops;
using namespace narray_io;
using namespace glinerec;

namespace {
    Logger logger("glr");

    void push_unary(floatarray &v,float value,float lo,float hi,
                    int steps,float weight=1.0) {
        float delta = (hi-lo)/steps;
        for(int i=0;i<steps;i++) {
            float thresh = i * delta + lo;
            if(value>=thresh) v.push(weight);
            else v.push(0.0);
        }
    }
}

namespace glinerec {

    void centroid(float &x,float &y,bytearray &image) {
        float total = 0;
        for(int i=0;i<image.dim(0);i++) {
            for(int j=0;j<image.dim(1);j++) {
                float value = image(i,j);
                total += value;
                x += i*value;
                y += j*value;
            }
        }
        x /= total;
        y /= total;
    }

    // boundary feature extractor

    void check_sorted(intarray &a) {
        for(int i=1;i<a.length();i++)
            CHECK_ARG(a(i)>=a(i-1));
    }

    bool equals(intarray &a,intarray &b) {
        if(a.length()!=b.length()) return 0;
        for(int i=0;i<a.length();i++)
            if(a[i]!=b[i]) return 0;
        return 1;
    }

    void segmentation_correspondences(objlist<intarray> &segments,intarray &seg,intarray &cseg) {
        CHECK_ARG(max(seg)<10000);
        CHECK_ARG(max(cseg)<10000);
        int nseg = max(seg)+1;
        int ncseg = max(cseg)+1;
        intarray overlaps(nseg,ncseg);
        overlaps = 0;
        CHECK_ARG(seg.length()==cseg.length());
        for(int i=0;i<seg.length();i++)
            overlaps(seg[i],cseg[i])++;
        segments.clear();
        segments.resize(ncseg);
        for(int i=0;i<nseg;i++) {
            int j = rowargmax(overlaps,i);
            ASSERT(j>=0 && j<ncseg);
            segments(j).push(i);
        }
    }

    struct LinerecExtracted : IRecognizeLine {
        enum { reject_class = '~' };
        autodel<ISegmentLine> segmenter;
        autodel<IGrouper> grouper;
        autodel<IModel> classifier;
        autodel<IFeatureMap> featuremap;
        int ntrained;

        LinerecExtracted() {
            pdef("classifier","latin","character classifier");
            pdef("cpreload","none","classifier to be loaded prior to training");
            pdef("verbose",0,"verbose output from glinerec");
            pdef("mode","centered-scaled","line recognition mode");
            pdef("use_props",1,"use character properties (aspect ratio, etc.)");
            pdef("use_reject",1,"use a reject class (use posteriors only and train on junk chars)");
            pdef("njitter",1,"#repeat presentations (use with jitter in the feature map)");
            pdef("csize",40,"target character size after rescaling");
            pdef("maxrange",5,"maximum number of components that are grouped together");
            pdef("context",1.5,"how much context to include (1.0=no context)");
            pdef("mdilate",2,"dilate the extraction mask by this much");
            pdef("correct_lineinfo",1,"try to correct the result from get_extended_line_info");
            pdef("fmap","sfmap","feature map to be used for recognition");
            pdef("cnorm","center","character normalization (center, abs, baseline)");
            pdef("abs_xhmul",1.5,"ascender multiplier for absolute rescaling");
            pdef("abs_truncate",0,"truncate character that are too big");
            pdef("space_fractile",0.5,"fractile for space estimation");
            pdef("space_multiplier",2,"multipler for space estimation");
            pdef("space_min",0.2,"minimum space threshold (in xheight)");
            pdef("space_max",1.1,"maximum space threshold (in xheight)");
            pdef("maxheight",300,"maximum height of input line");
            pdef("maxaspect",0.5,"maximum height/width ratio of input line");
            segmenter = make_DpSegmenter();
            grouper = make_SimpleGrouper();
            featuremap = dynamic_cast<IFeatureMap*>(component_construct(pget("fmap")));
            classifier = make_model("float8buffer");
            classifier->setModel(make_model(pget("classifier")),0);
            if(!classifier) throw "construct_model didn't yield an IModel";
            ntrained = 0;
        }

        const char *name() {
            return "linerec";
        }

        void info(int depth,FILE *stream) {
            iprintf(stream,depth,"LinerecExtracted\n");
            pprint(stream,depth);
            iprintf(stream,depth,"segmenter: %s\n",!segmenter?"null":segmenter->description());
            iprintf(stream,depth,"grouper: %s\n",!grouper?"null":grouper->description());
            featuremap->info(depth,stream);
            classifier->info(depth,stream);
        }

        const char *description() {
            return "Linerec";
        }
        const char *command(const char *argv[]) {
            const char *key = argv[0];
            const char *value = argv[1];
            if(key && value && !strcmp(key,"save_ds8")) {
                classifier->saveData(stdio(value,"w"));
                return "ok";
            } else if(key && value && !strcmp(key,"load_ds8")) {
                classifier->loadData(stdio(value,"r"));
                current_recognizer_ = this;
                return "ok";
            } else {
                return classifier->command(argv);
            }
        }
        void set(const char *key,double value) {
            throw "unknown parameter";
        }
        void set(const char *key,const char *value) {
            const char *argv[] = { key, value, 0 };
            command(argv);
        }
        void save(FILE *stream) {
            magic_write(stream,"linerec");
            psave(stream);
            // FIXME save grouper and segmenter here
            save_component(stream,featuremap.ptr());
            save_component(stream,classifier.ptr());
        }
        void load(FILE *stream) {
            magic_read(stream,"linerec");
            pload(stream);
            // FIXME load grouper and segmenter here
            featuremap = dynamic_cast<IFeatureMap*>(load_component(stream));
            classifier = dynamic_cast<IModel*>(load_component(stream));
        }

        void startTraining(const char *) {
            const char *preload = pget("cpreload");
            if(strcmp(preload,"none")) {
                stdio stream(preload,"r");
                load(stream);
                debugf("info","preloaded classifier %s\n");
                classifier->info();
            }
        }

        void finishTraining() {
            classifier->updateModel();
        }

        nustring transcript;
        bytearray line;
        intarray segmentation;

        float intercept,slope,xheight,descender_sink,ascender_rise;

        void correct_extended_line_info(float &intercept,float &slope,float &xheight,
                                        float &descender_sink,float &ascender_rise,
                                        intarray &segmentation) {
            int lc = 0;
            int uc = 0;
            narray<rectangle> boxes;
            bounding_boxes(boxes,segmentation);
            for(int i=1;i<boxes.length();i++) {
                int x = boxes(i).xcenter();
                int y_base = intercept+x*slope;
                int y_xheight = y_base+xheight;
                int y_ascender = y_base+ascender_rise;
                CHECK(y_ascender>=y_xheight);
                int y_thresh = (y_xheight+y_ascender)/2;
                if(boxes(i).y1<y_thresh) lc++;
                else uc++;
            }
            debugf("linemod","xheight=%g descender_sink=%g ascender_rise=%g\n",
                   xheight,descender_sink,ascender_rise);
            debugf("linemod","line_info %d total %d uc %d lc\n",
                   boxes.length(),uc,lc);
            if(uc<1) {
                ascender_rise = xheight;
                xheight = 0.7 * ascender_rise;
            } else if(lc<1) {
                xheight = 0.7 * ascender_rise;
            } else if(fabs(xheight-ascender_rise)<2.0) {
                xheight = 0.7 * ascender_rise;
            }
        }

        bytearray binarized;
        void setLine(bytearray &image) {
            CHECK_ARG(image.dim(1)<pgetf("maxheight"));
            // initialize the feature map to the line image
            featuremap->setLine(image);

            // run the segmenter
            binarize_simple(binarized,image);
            segmenter->charseg(segmentation,binarized);
            sub(255,binarized);
            make_line_segmentation_black(segmentation);
            renumber_labels(segmentation,1);

            // set up the grouper
            grouper->setSegmentation(segmentation);

            // debugging info
            IDpSegmenter *dp = dynamic_cast<IDpSegmenter*>(segmenter.ptr());
            if(dp) {
                logger.log("DpSegmenter",dp->dimage);
                dshow(dp->dimage,"YYy");
            }
            logger.recolor("segmentation",segmentation);
            dshowr(segmentation,"YYY");

            // compute line info
            get_extended_line_info(intercept,slope,xheight,
                                   descender_sink,ascender_rise,segmentation);
            if(pgetf("correct_lineinfo"))
                correct_extended_line_info(intercept,slope,xheight,
                                           descender_sink,ascender_rise,segmentation);
            debugf("detail","LineInfo %g %g %g %g %g\n",intercept,slope,xheight,descender_sink,ascender_rise);
            if(xheight<4) throw BadTextLine();
            show_baseline(slope,intercept,xheight,image,"YYY");
            bytearray baseline_image;
            debug_baseline(baseline_image,slope,intercept,xheight,image);
            logger.log("baseline\n",baseline_image);
        }

        void extractFeatures(floatarray &v,int i) {
            const char *cnorm = pget("cnorm");
            CHECK_ARG(v.dim(1)<pgetf("maxheight"));
            if(!strcmp(cnorm,"center")) extractFeaturesCenter(v,i);
            else if(!strcmp(cnorm,"abs")) extractFeaturesAbs(v,i);
            else throwf("%s: unknown cnorm",cnorm);
        }

        void extractFeaturesCenter(floatarray &v,int i) {
            dsection("featcenter");
            CHECK_ARG(v.dim(1)<pgetf("maxheight"));
            rectangle b;
            bytearray mask;
            float context = pgetf("context");
            int mdilate = pgetf("mdilate");
            grouper->getMask(b,mask,i,0);
            CHECK_ARG(b.height()<pgetf("maxheight"));
            if(mdilate>0) {
                pad_by(mask,mdilate,mdilate);
                b.pad_by(mdilate,mdilate);
                binary_dilate_circle(mask,mdilate);
            }
            if(b.width()>b.height()) {
                int dy = (b.width()-b.height())/2;
                b.y0 -= dy;
                b.y1 += dy;
                pad_by(mask,0,dy);
            } else {
                int dx = (b.height()-b.width())/2;
                b.x0 -= dx;
                b.x1 += dx;
                pad_by(mask,dx,0);
            }
            int r = int((context-1.0)*b.width());
            if(r>0) {
                pad_by(mask,r,r);
                b.x0 -= r;
                b.y0 -= r;
                b.x1 += r;
                b.y1 += r;
            }
            if(dactive()) {
                floatarray temp,mtemp;
                temp.resize(b.width(),b.height()) = 0;
                mtemp.resize(b.width(),b.height()) = 0;
                for(int i=0;i<temp.dim(0);i++)
                    for(int j=0;j<temp.dim(1);j++)
                        temp(i,j) = bat(binarized,i+b.x0,j+b.y0,0);
                temp /= max(temp);
                dshown(temp,"a");
                dshown(mask,"b");
                mtemp = mask;
                mtemp /= max(mtemp);
                mtemp *= 0.5;
                temp -= mtemp;
                dshown(temp,"c");
                dwait();
            }
            featuremap->extractFeatures(v,b,mask);
        }

        void extractFeaturesAbs(floatarray &v,int i) {
            CHECK_ARG(v.dim(1)<pgetf("maxheight"));
            rectangle b;
            bytearray mask;
            float x=0,y=0;
            dsection("featabs");
            grouper->getMask(b,mask,i,0);
            CHECK_ARG(b.height()<pgetf("maxheight"));
            dshown(mask,"b");
            centroid(x,y,mask);
            CHECK(x>=0 && y>=0);
            x += b.x0;
            y += b.y0;
            int xi = int(x);
            int yi = int(y);
            int r;
            float abs_xhmul = pgetf("abs_xhmul");
            bool abs_truncate = pgetf("abs_truncate");
            r = int(abs_xhmul*xheight/2);
            if(!abs_truncate) r = max(max(r,b.width()),b.height());
            CHECK(r>0 && r<1000);
            b.x0 = xi-r;
            b.y0 = yi-r;
            b.x1 = xi+r;
            b.y1 = yi+r;
            CHECK(b.x0>-1000 && b.x1<10000 && b.y0>-1000 && b.y1<10000);
            grouper->getMaskAt(mask,i,b);
            dshown(mask,"d");
            binary_dilate_circle(mask,3);
            featuremap->extractFeatures(v,b,mask);
        }

        int riuniform(int hi) {
            return lrand48()%hi;
        }

        void pushProps(floatarray &v,int i) {
            rectangle b = grouper->boundingBox(i);
            float baseline = intercept + slope * b.x0;
            float bottom = (b.y0-baseline)/xheight;
            float top = (b.y1-baseline)/xheight;
            float width = b.width() / float(xheight);
            float height = b.height() / float(xheight);
            float aspect = log(b.height() / float(b.width()));
            int csize = pgetf("csize");
            push_unary(v,top,-1,4,csize);
            push_unary(v,bottom,-1,4,csize);
            push_unary(v,width,-1,4,csize);
            push_unary(v,height,-1,4,csize);
            push_unary(v,aspect,-1,4,csize);
        }

        void addTrainingLine(intarray &cseg,nustring &tr) {
            bytearray gimage;
            segmentation_as_bitmap(gimage,cseg);
            addTrainingLine(cseg,gimage,tr);
        }

        void addTrainingLine(intarray &cseg,bytearray &image,nustring &tr) {
            if(image.dim(1)>pgetf("maxheight")) 
                throwf("input line too high (%d x %d)",image.dim(0),image.dim(1));
            if(image.dim(1)*1.0/image.dim(0)>pgetf("maxaspect")) 
                throwf("input line has bad aspect ratio (%d x %d)",image.dim(0),image.dim(1));
            bool use_reject = pgetf("use_reject");
            dsection("training");
            CHECK(image.dim(0)==cseg.dim(0) && image.dim(1)==cseg.dim(1));
            current_recognizer_ = this;

            // check and set the transcript
            transcript.copy(tr);
            setLine(image);
            for(int i=0;i<transcript.length();i++)
                CHECK_ARG(transcript(i).ord()>=32);

            // compute correspondences between actual segmentation and
            // ground truth segmentation
            objlist<intarray> segments;
            segmentation_correspondences(segments,segmentation,cseg);
            dshowr(segmentation,"yy");
            dshowr(cseg,"yY");

            // now iterate through all the hypothesis segments and
            // train the classifier with them
            int total = 0;
            int junk = 0;
//#pragma omp parallel for
            for(int i=0;i<grouper->length();i++) {
                intarray segs;
                grouper->getSegments(segs,i);

                // see whether this is a ground truth segment
                int match = -1;
                for(int j=0;j<segments.length();j++) {
                    if(equals(segments(j),segs)) {
                        match = j;
                        break;
                    }
                }
                match -= 1;         // segments are numbered starting at 1
                int c = reject_class;
                if(match>=0) {
                    if(match>=transcript.length()) {
                        iucstring s;
                        s.append(transcript);
                        debugf("info","mismatch between transcript and cseg: \"%s\"\n",s.c_str());
                        continue;
                    } else {
                        c = transcript[match].ord();
                        debugf("debugmismatch","index %d position %d char %c [%d]\n",i,match,c,c);
                    }
                }

                if(c==reject_class) junk++;

                // extract the character and add it to the classifier
                int njitter = pgetf("njitter");
                bool use_props = pgetf("use_props");
                floatarray v;
                for(int k=0;k<njitter;k++) {
                    extractFeatures(v,i);
                    v.reshape(v.length());
                    if(use_props) pushProps(v,i);
#pragma omp atomic
                    total++;
#pragma omp critical
                    {
                        if(use_reject) {
                            classifier->add(v,c);
                        } else {
                            if(c!=reject_class)
                                classifier->add(v,c);
                        }
                    }
                }
#pragma omp atomic
                ntrained++;
            }
            debugf("detail","addTrainingLine trained %d chars, %d junk, %s total\n",total-junk,junk,
                    classifier->command("total"));
            dwait();
        }

        enum { high_cost = 100 };

        void recognizeLine(IGenericFst &result,bytearray &image) {
            intarray segmentation_;
            recognizeLine(segmentation_,result,image);
        }

        float space_threshold;

        void estimateSpaceSize() {
            intarray labels;
            labels = segmentation;
            label_components(labels);
            rectarray boxes;
            bounding_boxes(boxes,labels);
            floatarray distances;
            distances.resize(boxes.length()) = 99999;
            for(int i=1;i<boxes.length();i++) {
                rectangle b = boxes[i];
                for(int j=1;j<boxes.length();j++) {
                    rectangle n = boxes[j];
                    int delta = n.x0 - b.x1;
                    if(delta<0) continue;
                    if(delta>=distances[i]) continue;
                    distances[i] = delta;
                }
            }
            float interchar = fractile(distances,pgetf("space_fractile"));
            space_threshold = interchar*pgetf("space_multiplier");;
            // impose some reasonable upper and lower bounds
            space_threshold = max(space_threshold,pgetf("space_min")*xheight);
            space_threshold = min(space_threshold,pgetf("space_max")*xheight);
        }

        void recognizeLine(intarray &segmentation_,IGenericFst &result,bytearray &image_) {
            if(image_.dim(1)>pgetf("maxheight")) 
                throwf("input line too high (%d x %d)",image_.dim(0),image_.dim(1));
            if(image_.dim(1)*1.0/image_.dim(0)>pgetf("maxaspect")) 
                throwf("input line has bad aspect ratio (%d x %d)",image_.dim(0),image_.dim(1));
            bool use_reject = pgetf("use_reject");
            bytearray image;
            image = image_;
            dsection("recognizing");
            logger.log("input\n",image);
            setLine(image);
            segmentation_ = segmentation;
            bytearray available;
            floatarray v,p,cp,ccosts,props;
            int ncomponents = grouper->length();
            rectangle b;

            estimateSpaceSize();

#pragma omp parallel for schedule(dynamic,10) private(p,v,b,props)
            for(int i=0;i<ncomponents;i++) {
#if 0
                bytearray raw;
                grouper->extract(raw,binarized,0,i);
                floatarray v;
                features->extract(v,raw);
#else
                extractFeatures(v,i);
#endif
                v.reshape(v.length());
                pushProps(v,i);
                float ccost = classifier->outputs(p,v);
#pragma omp critical
                {
                    if(use_reject) {
                        ccost = 0;
                        p /= sum(p);
                    }
                    int count = 0;
                    for(int j=0;j<p.length();j++) {
                        if(j==reject_class) continue;
                        float pcost = p(j)>1e-6?-log(p(j)):-log(1e-6);
                        debugf("dcost","%3d %10g %c\n",j,pcost+ccost,(j>32?j:'_'));
                        grouper->setClass(i,j,pcost+ccost);
                        count++;
                    }
                    if(count==0) {
                        if(b.height()<xheight/2 && b.width()<xheight/2) {
                            grouper->setClass(i,'~',high_cost/2);
                        } else {
                            grouper->setClass(i,'#',(b.width()/xheight)*high_cost);
                        }
                    }
                    if(grouper->pixelSpace(i)>space_threshold) {
                        debugf("spaces","space %d\n",grouper->pixelSpace(i));
                        grouper->setSpaceCost(i,1.0,5.0);
                    }
                    // dwait();
                }
            }
            grouper->getLattice(result);
        }

        void align(nustring &chars,intarray &seg,floatarray &costs,
                   bytearray &image,IGenericFst &transcription) {
            throw Unimplemented();
#if 0
            autodel<FstBuilder> lattice;
            recognizeLine(*lattice,image);
            intarray ids;
            bestpath2(chars,costs,ids,lattice,transcript,false);
            if(debug("info")) {
                for(int i=0;i<chars.length();i++) {
                    debugf("info","%3d %10g %6d %c\n",
                           i,costs(i),ids(i),chars(i));
                }
            }
            intarray aligned_seg;
#endif
        }


    };

    IRecognizeLine *make_Linerec() {
        return new LinerecExtracted();
    }

    void init_linerec() {
        static bool init = false;
        if(init) return;
        init = true;
        component_register<LinerecExtracted>("linerec");
    }
}
