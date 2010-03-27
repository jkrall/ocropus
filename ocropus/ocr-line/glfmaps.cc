#define __warn_unused_result__ __far__

#include <cctype>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <math.h>
#include "ocropus.h"
#include "glinerec.h"
#include "glfmaps.h"

using namespace iulib;
using namespace colib;
using namespace ocropus;
using namespace narray_ops;
using namespace narray_io;
using namespace glinerec;

namespace {
    float H(float x) {
        return x<0?0:x;
    }
    double uniform(double lo,double hi) {
        return drand48()*(hi-lo)+lo;
    }
    double loguniform(double lo,double hi) {
        return exp(drand48()*(log(hi)-log(lo))+log(lo));
    }

    int count_neighbors(bytearray &bi,int x,int y) {
        int nn=0;
        if (bi(x+1,y)) nn++;
        if (bi(x+1,y+1)) nn++;
        if (bi(x,y+1)) nn++;
        if (bi(x-1,y+1)) nn++;
        if (bi(x-1,y)) nn++;
        if (bi(x-1,y-1)) nn++;
        if (bi(x,y-1)) nn++;
        if (bi(x+1,y-1)) nn++;
        return nn;
    }

    void skeletal_features(bytearray &endpoints,
                           bytearray &junctions,
                           bytearray &image,
                           float presmooth=0.0,
                           float skelsmooth=0.0) {
        bytearray temp;
        temp.copy(image);
        greater(temp,128,0,255);
        if(presmooth>0) {
            gauss2d(temp,presmooth,presmooth);
            greater(temp,128,0,255);
        }
        thin(temp);
        dshow(temp,"yYy");
        makelike(junctions,temp);
        junctions = 0;
        makelike(endpoints,temp);
        endpoints = 0;
        for(int i=1;i<temp.dim(0)-1;i++) {
            for(int j=1;j<temp.dim(1)-1;j++) {
                if(!temp(i,j)) continue;
                int n = count_neighbors(temp,i,j);
                if(n==1) endpoints(i,j) = 255;
                if(n>2) junctions(i,j) = 255;
            }
        }
        binary_dilate_circle(junctions,int(skelsmooth+0.5));
        binary_dilate_circle(endpoints,int(skelsmooth+0.5));
    }

    float normorient(float x) {
        while(x>=M_PI/2) x -= M_PI;
        while(x<-M_PI/2) x += M_PI;
        return x;
    }
    float normadiff(float x) {
        while(x>=M_PI) x -= 2*M_PI;
        while(x<-M_PI) x += 2*M_PI;
        return x;
    }
    float normorientplus(float x) {
        while(x>=M_PI) x -= M_PI;
        while(x<0) x += M_PI;
        return x;
    }

    void checknan(floatarray &v) {
        int n = v.length1d();
        for(int i=0;i<n;i++)
            CHECK(!isnan(v.unsafe_at1d(i)));
    }

    inline float floordiv(float x,float y) {
        float n = floor(x/y);
        return x - n*y;
    }

    void ridgemap(narray<floatarray> &maps,bytearray &binarized,
                  float rsmooth=1.0,float asigma=0.7,float mpower=0.5,
                  float rpsmooth=1.0) {
        floatarray smoothed;
        smoothed = binarized;
        sub(max(smoothed),smoothed);
        brushfire_2(smoothed);
        gauss2d(smoothed,rsmooth,rsmooth);
        floatarray zero;
        zero = smoothed;
        floatarray strength;
        floatarray angle;
        horn_riley_ridges(smoothed,zero,strength,angle);
        // dshown(smoothed,"yyY");
        // dshown(zero,"yYy");
        for(int i=0;i<zero.length();i++) {
            if(zero[i]) continue;
            strength[i] = 0;
            angle[i] = 0;
            smoothed[i] = 0;
        }
        dshown(smoothed,"yYY");
        dshown(angle,"Yyy");

        for(int m=0;m<maps.length();m++) {
            float center = m*M_PI/maps.length();
            maps(m) = smoothed;
            for(int i=0;i<smoothed.length();i++) {
                if(!smoothed[i]) continue;
                float a = angle[i];
                float d = a-center;
                float dn = normorient(d);
                float v = exp(-sqr(dn/(2*asigma)));
                maps(m)[i] = pow(maps(m)[i],mpower)*v;
            }
            gauss2d(maps(m),rpsmooth,rpsmooth);
        }
    }

    void compute_troughs(floatarray &troughs,bytearray &binarized,
                         float rsmooth=1.0) {
        floatarray smoothed;
        smoothed = binarized;
        brushfire_2(smoothed);
        gauss2d(smoothed,rsmooth,rsmooth);
        floatarray zero;
        zero = smoothed;
        floatarray strength;
        floatarray angle;
        horn_riley_ridges(smoothed,zero,strength,angle);
        // dshown(smoothed,"yyY");
        // dshown(zero,"yYy");
        for(int i=0;i<zero.length();i++) {
            if(zero[i]) continue;
            strength[i] = 0;
            angle[i] = 0;
            smoothed[i] = 0;
        }
        abs(strength);
        troughs = strength;
        troughs /= max(troughs);
    }

    void extract_holes(bytearray &holes,bytearray &binarized) {
        intarray temp;
        temp.copy(binarized);
        sub(255,temp);
        label_components(temp);
        int background = -1;
        for(int i=0;i<temp.dim(0);i++) {
            if(temp(i,0)!=0) {
                background = temp(i,0);
                break;
            }
        }
        makelike(holes,temp);
        holes = 0;
        CHECK(background>0);
        for(int i=0;i<temp.dim(0);i++) {
            for(int j=0;j<temp.dim(1);j++) {
                if(temp(i,j)>0 && temp(i,j)!=background)
                    holes(i,j) = 255;
            }
        }
        //dshowr(temp,"Yyy");
        dshow(holes,"YyY");
    }

    template <class T>
    inline void push_array(floatarray &out,narray<T> &in) {
        for(int i=0;i<in.length();i++)
            out.push() = in[i];
    }

    inline double sigmoid(double x) {
        if(x<-10) return 0;
        if(x>10) return 1;
        return 1/(1+exp(-x));
    }
    inline double ssigmoid(double x) {
        if(x<-10) return -1;
        if(x>10) return 1;
        return -1+2/(1+exp(-x));
    }
    void ssigmoid(floatarray &dt) {
        for(int i=0;i<dt.length();i++)
            dt[i] = ssigmoid(dt[i]);
    }
    void heaviside(floatarray &dt) {
        for(int i=0;i<dt.length();i++)
            if(dt[i]<0) dt[i] = 0;
    }
    void negheaviside(floatarray &dt) {
        for(int i=0;i<dt.length();i++) {
            if(dt[i]>0) dt[i] = 0;
            else dt[i] = -dt[i];
        }
    }
    float absfractile_nz(floatarray &dt,float f=0.9) {
        floatarray temp;
        for(int i=0;i<dt.length();i++) {
            float value = fabs(dt[i]);
            if(value<1e-6) continue;
            temp.push() = value;
        }
        return fractile(temp,f);
    }
    float absmean_nz(floatarray &dt) {
        float temp;
        int count;
        for(int i=0;i<dt.length();i++) {
            float value = fabs(dt[i]);
            if(value>1e-6) {
                temp += value;
                count++;
            }
        }
        return temp/count;
    }
    void apow(floatarray &dt,float p) {
        for(int i=0;i<dt.length();i++) {
            float value = dt[i];
            value = (value<0?-1:1) * pow(fabs(value),p);
            dt[i] = value;
        }
    }
}

namespace glinerec {
    struct SimpleFeatureMap : IFeatureMap {
        bytearray line;
        bytearray binarized;
        bytearray holes;
        bytearray junctions;
        bytearray endpoints;
        narray<floatarray> maps;
        floatarray troughs;
        floatarray dt;
        floatarray dt_x,dt_y;
        narray<floatarray> dt_maps;
        int pad;

        SimpleFeatureMap() {
            // parameters affecting all features
            pdef("ftypes","bejh","which feature types to extract (bgxyhejrt)");
            pdef("csize",40,"target character size after rescaling");
            pdef("context",1.3,"how much context to include");
            pdef("scontext",0.3,"value to multiply context pixels with (e.g., -1, 0, 1, 0.5)");
            pdef("aa",0.5,"amount of anti aliasing (-1 = use other algorithm)");
            pdef("maxheight",300,"maximum height for feature extraction");

            // parameters specific to individual feature maps
            pdef("skel_pre_smooth",0.0,"smooth by this amount prior to skeletal extraction");
            pdef("skel_post_dilate",3.0,"how much to smooth after skeletal extraction");
            pdef("grad_pre_smooth",2.0,"how much to smooth before gradient extraction");
            pdef("grad_post_smooth",0.0,"how much to smooth after gradient extraction");
            pdef("ridge_pre_smooth",1.0,"how much to smooth before skeletal extraction (about 0.5 to 3.0)");
            pdef("ridge_post_smooth",1.0,"how much to smooth after skeletal extraction (about 0.5 to 3.0)");
            pdef("ridge_asigma",0.6,"angle orientation bin overlap (about 0.5 to 1.5)");
            pdef("ridge_mpower",0.5,"gamma for ridge orientation map (about 0.2-2.0)");
            pdef("ridge_nmaps",4,"number of feature maps for ridge extraction (should be 4)");
            pdef("dt_power",1.0,"power to which to raise the distance transform");
            pdef("dt_asigma",0.7,"power to which to raise the distance transform");
            pdef("dt_which","inside","inside, outside, or both");
            pdef("dt_grad_smooth",1.0,"smoothing of the distance transform before gradient computation");
            pad = 10;
        }

        const char *name() {
            return "sfmap";
        }

        void save(FILE *stream) {
            magic_write(stream,"sfmap");
            psave(stream);
        }

        void load(FILE *stream) {
            magic_read(stream,"sfmap");
            pload(stream);
        }

        virtual void setLine(bytearray &image_) {
            const char *ftypes = pget("ftypes");
            maps.resize(int(pgetf("ridge_nmaps")));
            line = image_;
            dsection("setline");
            dclear(0);
            dshow(line,"yyy");
            pad_by(line,pad,pad,max(line));

            // compute a simple binarized version and segment
            binarize_simple(binarized,line);
            sub(255,binarized);
            remove_small_components(binarized,3,3);
            dshow(binarized,"yyy");

            // skeletal features
            if(strchr(ftypes,'j') || strchr(ftypes,'e')) {
                float presmooth = pgetf("skel_pre_smooth");
                int skelsmooth = pgetf("skel_post_dilate");
                skeletal_features(endpoints,junctions,binarized,presmooth,skelsmooth);
                dshow(junctions,"yYY");
                dshow(endpoints,"Yyy");
            }

            // computing holes
            if(strchr(ftypes,'h')) {
                extract_holes(holes,binarized);
                dshown(holes,"yYY");
            }

            // compute ridge orientations
            if(strchr(ftypes,'r')) {
                float rsmooth = pgetf("ridge_pre_smooth");
                float rpsmooth = pgetf("ridge_post_smooth");
                float asigma = pgetf("ridge_asigma");
                float mpower = pgetf("ridge_mpower");
                ridgemap(maps,binarized,rsmooth,asigma,mpower,rpsmooth);
                dshown(maps(0),"Yyy");
                dshown(maps(1),"YyY");
                dshown(maps(2),"YYy");
                dshown(maps(3),"YYY");
            }

            // compute troughs
            if(strchr(ftypes,'t')) {
                float rsmooth = pgetf("ridge_pre_smooth");
                compute_troughs(troughs,binarized,rsmooth);
                dshown(troughs,"yYY");
            }

            // compute different distance transforms
            if(strchr(ftypes,'D') || strchr(ftypes,'G') || strchr(ftypes,'M')) {
                const char *which = pget("dt_which");
                if(!strcmp(which,"outside")) {
                    dt = binarized;
                    brushfire_2(dt);
                    dt /= absfractile_nz(dt);
                    ssigmoid(dt);
                } else if(!strcmp(which,"inside")) {
                    dt = binarized;
                    sub(max(dt),dt);
                    brushfire_2(dt);
                    dt /= absfractile_nz(dt);
                    ssigmoid(dt);
                } else if(!strcmp(which,"both")) {
                    dt = binarized;
                    sub(max(dt),dt);
                    brushfire_2(dt);
                    // pick scale according to inside only
                    float scale = absfractile_nz(dt);
                    floatarray mdt;
                    mdt = binarized;
                    brushfire_2(mdt);
                    dt -= mdt;
                    dt /= scale;
                    ssigmoid(dt);
                }
                debugf("sfmaprange","dt %g %g\n",min(dt),max(dt));
                if(strchr(ftypes,'G') || strchr(ftypes,'M')) {
                    floatarray smoothed;
                    smoothed = dt;
                    float s = pgetf("dt_grad_smooth");
                    gauss2d(smoothed,s,s);
                    makelike(dt_x,smoothed);
                    makelike(dt_y,smoothed);
                    dt_x = 0;
                    dt_y = 0;
                    for(int i=1;i<smoothed.dim(0)-1;i++) {
                        for(int j=1;j<smoothed.dim(1)-1;j++) {
                            float dx = smoothed(i,j) - smoothed(i-1,j);
                            float dy = smoothed(i,j) - smoothed(i,j-1);
                            dt_x(i,j) = dx;
                            dt_y(i,j) = dy;
                        }
                    }
                    float n = max(1e-5,max(max(dt_x),max(dt_y)));
                    dt_x /= n;
                    dt_y /= n;
                    ssigmoid(dt_x);
                    ssigmoid(dt_y);
                    checknan(dt_x);
                    checknan(dt_y);
                    dshown(dt_x,"YyY");
                    dshown(dt_y,"YYy");
                    debugf("sfmaprange","dt_x %g %g\n",min(dt),max(dt_x));
                    debugf("sfmaprange","dt_y %g %g\n",min(dt),max(dt_y));
                }
                // split the distance transform gradient into separate
                // maps for positive and negative
                apow(dt,pgetf("dt_power"));
                dshown(dt,"Yyy");
                if(strchr(ftypes,'M')) {
                    dt_maps.resize(4);
                    dt_maps(0) = dt_x;
                    heaviside(dt_maps(0));
                    dt_maps(1) = dt_x;
                    negheaviside(dt_maps(1));
                    dt_maps(2) = dt_y;
                    heaviside(dt_maps(2));
                    dt_maps(3) = dt_y;
                    negheaviside(dt_maps(3));
                    dshown(dt_maps(0),"Yyy");
                    dshown(dt_maps(1),"YyY");
                    dshown(dt_maps(2),"YYy");
                    dshown(dt_maps(3),"YYY");
                }
            }

            dwait();
        }

        void extractFeatures(floatarray &v,rectangle b,bytearray &mask) {
            dsection("features");
            b.shift_by(pad,pad);

            CHECK(b.width()==mask.dim(0) && b.height()==mask.dim(1));
            const char *ftypes = pget("ftypes");
            floatarray u;
            v.clear();
            if(strchr(ftypes,'b')) {
                extractFeatures(u,b,mask,binarized);
                push_array(v,u);
                CHECK(min(v)>=-1.1 && max(v)<=1.1);
            }
            if(strchr(ftypes,'h')) {
                extractFeatures(u,b,mask,holes,false);
                push_array(v,u);
                CHECK(min(v)>=-1.1 && max(v)<=1.1);
            }
            if(strchr(ftypes,'j')) {
                extractFeatures(u,b,mask,junctions);
                push_array(v,u);
                CHECK(min(v)>=-1.1 && max(v)<=1.1);
            }
            if(strchr(ftypes,'e')) {
                extractFeatures(u,b,mask,endpoints);
                push_array(v,u);
                CHECK(min(v)>=-1.1 && max(v)<=1.1);
            }
            if(strchr(ftypes,'r')) {
                for(int i=0;i<maps.length();i++) {
                    extractFeatures(u,b,mask,maps(i));
                    push_array(v,u);
                }
                CHECK(min(v)>=-1.1 && max(v)<=1.1);
            }
            if(strchr(ftypes,'t')) {
                extractFeatures(u,b,mask,troughs);
                push_array(v,u);
                CHECK(min(v)>=-1.1 && max(v)<=1.1);
            }
            if(strchr(ftypes,'D')) {
                extractFeatures(u,b,mask,dt);
                push_array(v,u);
                CHECK(min(v)>=-1.1 && max(v)<=1.1);
            }
            if(strchr(ftypes,'G')) {
                extractFeatures(u,b,mask,dt_x);
                push_array(v,u);
                extractFeatures(u,b,mask,dt_y);
                push_array(v,u);
                CHECK(min(v)>=-1.1 && max(v)<=1.1);
            }
            if(strchr(ftypes,'M')) {
                for(int i=0;i<dt_maps.length();i++) {
                    extractFeatures(u,b,mask,dt_maps(i));
                    push_array(v,u);
                }
                CHECK(min(v)>=-1.1 && max(v)<=1.1);
            }

            if(dactive()) {
                int csize = pgetf("csize");
                floatarray temp;
                temp = v;
                temp.reshape(temp.length()/csize,csize);
                dshown(temp,"c");
                dshown(mask,"d");
                dwait();
            }
        }

        template <class S>
        void extractFeatures(floatarray &v,rectangle b,bytearray &mask,
                             narray<S> &source,bool masked=true) {
            rectangle bp = rectangle(b.x0+pad,b.y0+pad,b.x1+pad,b.y1+pad);
            if(pgetf("aa")>=0) {
                extractFeaturesAA(v,b,mask,source,masked);
            } else {
                extractFeaturesNonAA(v,b,mask,source,masked);
            }
        }

        template <class S>
        void extractFeaturesAA(floatarray &v,rectangle b,bytearray &mask,
                             narray<S> &source,bool masked=true) {
            float scontext = pgetf("scontext");
            int csize = pgetf("csize");
            CHECK(mask.dim(0)==b.width() && mask.dim(1)==b.height());
            CHECK_ARG(v.dim(1)<pgetf("maxheight"));
            CHECK_ARG(b.height()<pgetf("maxheight"));

            floatarray sub(b.width(),b.height());
            get_rectangle(sub,source,b);
            float s = max(sub.dim(0),sub.dim(1))/float(csize);
            float sig = s * pgetf("aa");
            if(sig>0) gauss2d(sub,sig,sig);
            bytearray dmask;
            dmask = mask;
            if(int(sig)>0) binary_dilate_circle(dmask,int(sig));

            v.resize(csize,csize);
            v = 0;
            for(int i=0;i<csize;i++) {
                for(int j=0;j<csize;j++) {
                    int mval = bat(dmask,i*s,j*s,0);
                    float value = bilin(sub,i*s,j*s);
                    if(masked && !mval) value = scontext * value;
                    v(i,j) = value;
                }
            }

            float maxval = max(fabs(max(v)),fabs(min(v)));
            if(maxval>1.0) v /= maxval;
            checknan(v);
            CHECK(min(v)>=-1.1 && max(v)<=1.1);

            dsection("dfeats");
            dshown(sub,"a");
            dshown(dmask,"b");
            {floatarray temp; temp = sub; temp -= dmask; dshown(temp,"d");}
            dshown(v,"c");
            dwait();
        }

        template <class S>
        void extractFeaturesNonAA(floatarray &v,rectangle b,bytearray &mask,
                             narray<S> &source,bool masked=true) {
            // FIXME bool use_centroid = pgetf("use_centroid");
            float context = pgetf("context");
            float scontext = pgetf("scontext");
            int csize = pgetf("csize");
            float xc = b.xcenter();
            float yc = b.ycenter();
            int xm = mask.dim(0)/2;
            int ym = mask.dim(1)/2;
            int r = context*max(b.width(),b.height());
            v.resize(csize,csize);
            v = 0;
            for(int i=0;i<csize;i++) {
                for(int j=0;j<csize;j++) {
                    float x = (i*1.0/csize)-0.5;
                    float y = (j*1.0/csize)-0.5;
                    int m = bat(mask,int(x*r+xm),int(y*r+ym),0);
                    float value = bilin(source,x*r+xc,y*r+yc);
                    if(masked) {
                        if(!m) value = scontext * value;
                    }
                    v(i,j) = value;
                }
            }
            float maxval = max(fabs(max(v)),fabs(min(v)));
            if(maxval>1.0) v /= maxval;
            checknan(v);
            CHECK(min(v)>=-1.1 && max(v)<=1.1);
            dsection("dfeats");
            dshown(v,"a");
            dshown(mask,"b");
            dwait();
        }
    };

    void init_glfmaps() {
        static bool init = false;
        if(init) return;
        init = true;
        component_register<SimpleFeatureMap>("sfmap");
    }
}
