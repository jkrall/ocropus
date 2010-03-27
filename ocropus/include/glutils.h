// -*- C++ -*-

#ifndef classutils_h_
#define classutils_h_

#include <unistd.h>
#include <sys/stat.h>
#include <cctype>
#include "glinerec.h"

#ifndef _OPENMP
#define THREAD 0
#define NTHREADS 1
#else
#include <omp.h>
#define THREAD omp_get_thread_num()
#define NTHREADS omp_get_num_threads()
#endif

namespace {
    using namespace iulib;
    using namespace narray_ops;
    using namespace narray_io;
    using namespace ocropus;

    // remove trailing newline

    void chomp(char *p) {
        while(*p) {
            if(*p=='\n') { *p = 0; return; }
            p++;
        }
    }

    void remove_spaces(char *p) {
        char *q = p;
        while(*p) {
            if(!std::isspace(*p)) { *q++ = *p; }
            p++;
        }
        *q = 0;
    }

    inline float rnormal() {
        float x,y,s;
        do {
            x = 2*drand48()-1;
            y = 2*drand48()-1;
            s = x*x+y*y;
        } while(s>1.0);
        return x*sqrt(-log(s)/s);
    }

    inline float rnormal(float mu,float sigma) {
        return rnormal()*sigma+mu;
    }

    inline float rlognormal(float x,float r) {
        CHECK(r>1.0);
        float n = rnormal(log(x),log(r));
        float result = exp(n);
        CHECK(!isnan(result));
        return result;
    }

    void fill_random(floatarray &v,float lo,float hi) {
        for(int i=0;i<v.length1d();i++)
            v.at1d(i) = (hi-lo)*drand48()+lo;
    }

    // report statistics on some variable

    struct Report {
        const char *name;
        double count,sum,sum2,lo,hi,last_count;
        Report(const char *s) {
            name = s;
            count = 0;
            last_count = count;
            sum = 0;
            sum2 = 0;
            lo = 1e300;
            hi = -1e300;
        }
        ~Report() {
            report();
        }
        void report() {
            if(count>last_count) {
                fprintf(stderr,"REPORT %s %g mean %g stddev %g lo %g hi %g\n",name,
                        count,sum/count,sqrt(sum2/count-sqr(sum/count)),lo,hi);
                last_count = count;
            }
        }
        template <class T>
        T operator+=(T value) {
            count++;
            sum += value;
            sum2 += sqr(value);
            if(value<lo) lo = value;
            if(value>hi) hi = value;
            return value;
        }
    };

    // check for absence of NaNs

    inline bool valid(floatarray &v) {
        for(int i=0;i<v.length1d();i++)
            if(isnan(v.at1d(i)))
                return 0;
        return 1;
    }

    // indent a line by n steps

    void indent(int n) {
        for(int i=0;i<n;i++) printf("   ");
    }

    template <class T,class S>
    void get_rectangle(narray<T> &out,narray<S> &in,rectangle b) {
        out.resize(b.width(),b.height());
        for(int i=0;i<out.dim(0);i++)
            for(int j=0;j<out.dim(1);j++)
                out(i,j) = bat(in,b.x0+i,b.y0+j,0);
    }

    // estimate the nearest neighbor classification error for
    // a particular classification problem

    double nn_error(floatarray &data,intarray &classes) {
        int total = 0;
        int n = data.dim(0);
        floatarray dists(n);
        for(int i=0;i<n;i++) {
            floatarray u;
            rowget(u,data,i);
#pragma omp parallel for
            for(int j=0;j<n;j++) {
                if(i==j) { dists(j) = 1e30; continue; }
                floatarray v;
                rowget(v,data,j);
                dists(j) = dist2squared(u,v);
            }
            int index = argmin(dists);
            if(classes(index)!=classes(i)) total++;
        }
        return total/double(data.dim(0));
    }

    // given a set of classes and distances, estimate the posterior
    // probability for the k nearest neighbors (by counting)

    void knn_posterior(floatarray &posterior,intarray &classes,floatarray &costs,int k) {
        NBest nbest(k);
        for(int i=0;i<costs.length();i++)
            nbest.add(i,-costs(i));
        posterior.resize(max(classes)+1);
        posterior.fill(0);
        for(int i=0;i<nbest.length();i++) {
            int index = nbest[i];
            posterior(classes(index))++;
        }
        posterior /= sum(posterior);
    }

    // perform k nearest neighbor classification

    int knn_classify(intarray &classes,floatarray &costs,int k) {
        floatarray p;
        knn_posterior(p,classes,costs,k);
        return argmax(p);
    }

    // debugging printout of an array

    void dprint(intarray &a,int n) {
        if(a.rank()==1) {
            printf("[");
            int i;
            for(i=0;i<min(n,a.dim(0));i++) {
                if(i>0) printf(" ");
                printf("%d",a(i));
            }
            if(i<a.dim(0)) printf("...");
            printf("]");
        } else if(a.rank()==2) {
            for(int i=0;i<min(n,a.dim(0));i++) {
                printf("%4d [",i);
                int j;
                for(j=0;i<min(n,a.dim(j));i++) {
                    if(j>0) printf(" ");
                    printf("%d",a(i,j));
                }
                if(j<a.dim(1)) printf("...");
                printf("]");
            }
        } else throw "unimplemented";
    }

    void dprint(floatarray &a,int n) {
        if(a.rank()==1) {
            printf("[");
            int i;
            for(i=0;i<min(n,a.dim(0));i++) {
                if(i>0) printf(" ");
                printf("%g",a(i));
            }
            if(i<a.dim(0)) printf("...");
            printf("]");
        } else if(a.rank()==2) {
            for(int i=0;i<min(n,a.dim(0));i++) {
                printf("%4d [",i);
                int j;
                for(j=0;i<min(n,a.dim(j));i++) {
                    if(j>0) printf(" ");
                    printf("%g",a(i,j));
                }
                if(j<a.dim(1)) printf("...");
                printf("]");
            }
        } else throw "unimplemented";
    }

    void dprint(const char *where,const char *label,intarray &a,int n) {
        // FIXME
        printf("[%s] %s ",where,label);
        dprint(a,n);
        printf("\n");
    }

    void dprint(const char *where,const char *label,floatarray &a,int n) {
        // FIXME
        printf("[%s] %s ",where,label);
        dprint(a,n);
        printf("\n");
    }
    // compute, then print a histogram

    void print_hist(intarray &a) {
        intarray hist(max(a)+1);
        hist.fill(0);
        for(int i=0;i<a.length();i++)
            hist(a(i))++;
        dprint(hist,hist.length());
    }

    // compute, then print a histogram of characters

    void print_charhist(intarray &a) {
        intarray hist(max(a)+1);
        hist.fill(0);
        for(int i=0;i<a.length();i++)
            hist(a(i))++;
        printf("[charhist");
        for(int i=0;i<hist.length();i++) {
            if(hist(i)==0) continue;
            if(i<=32)
                printf(" %d:%d",i,hist(i));
            else
                printf(" '%c':%d",i,hist(i));
        }
        printf("]\n");
    }

    // randomly split data (e.g., into a test set and training set)

    void split_data(floatarray &vs1,intarray &cs1,
                    floatarray &vs2,intarray &cs2,
                    floatarray &vs,intarray &cs,
                    float frac) {
        int n = vs.dim(0), d = vs.dim(1);
        intarray selection;
        rpermutation(selection,n);
        int split = int(n*frac);
        vs1.resize(split,d);
        cs1.resize(split);
        for(int i=0;i<split;i++) {
            for(int j=0;j<d;j++) vs1(i,j) = vs(i,j);
            cs1(i) = cs(i);
        }
        vs2.resize(n-split,d);
        cs2.resize(n-split);
        for(int i=0;i<n-split;i++) {
            for(int j=0;j<d;j++) vs2(i,j) = vs(i+split,j);
            cs2(i) = cs(i+split);
        }
    }

    void split_classes(intarray &cs1,intarray &cs2,intarray &cs,float frac) {
        CHECK(frac>1e-6 && frac<1.0);
        int n = cs.length();
        intarray selection;
        cs1.copy(cs);
        cs2.copy(cs);
        int split = int(n*frac);
        rpermutation(selection,n);
        for(int i=0;i<split;i++)
            cs2(selection(i)) = -1;
        for(int i=split;i<n;i++)
            cs1(selection(i)) = -1;
    }

    void split_samples(intarray &training,intarray &testing,intarray &samples,float frac=0.1) {
        intarray a;
        a = samples;
        shuffle(a);
        training.clear();
        testing.clear();
        int split = int(frac*a.length());
        for(int i=0;i<split;i++)
            testing.push(samples(i));
        for(int i=split;i<a.length();i++)
            training.push(samples(i));
        ASSERT(training.length()+testing.length()==samples.length());
    }

    // given a rank 3 array (e.g. MNIST list of images), extract
    // a rank two array data(which,:,:)

    void get_img(floatarray &image,floatarray &data,int which) {
        int h = data.dim(2);
        image.resize(data.dim(2),data.dim(1));
        for(int i=0;i<image.dim(0);i++)
            for(int j=0;j<image.dim(1);j++)
                image(i,j) = data(which,h-j-1,i);
    }

    void get_img(bytearray &image,bytearray &data,int which) {
        int h = data.dim(2);
        image.resize(data.dim(2),data.dim(1));
        for(int i=0;i<image.dim(0);i++)
            for(int j=0;j<image.dim(1);j++)
                image(i,j) = data(which,h-j-1,i);
    }

    void get_vec(floatarray &v,floatarray &data,int which) {
        int h = data.dim(2);
        CHECK(data.rank()==3);
        v.resize(data.dim(2)*data.dim(1));
        for(int i=0;i<data.dim(1);i++)
            for(int j=0;j<data.dim(2);j++)
                v(i*h+j) = data(which,h-j-1,i);
    }

    inline float norm(floatarray &v) {
        double total = 0.0;
        for(int i=0;i<v.length1d();i++)
            total += v.at1d(i);
        return total;
    }

    void rowmean(floatarray &mean,floatarray &data) {
        mean.resize(data.dim(1));
        mean.fill(0);
        for(int i=0;i<data.dim(0);i++) {
            for(int j=0;j<data.dim(1);j++)
                mean(j) += data(i,j);
        }
        mean /= data.dim(0);
    }

    void rownormalize1(floatarray &data) {
        for(int i=0;i<data.dim(0);i++) {
            double total = 0.0;
            for(int j=0;j<data.dim(1);j++)
                total += data(i,j);
            for(int j=0;j<data.dim(1);j++)
                data(i,j) /= total;
        }
    }

    void rownormalize2(floatarray &data) {
        for(int i=0;i<data.dim(0);i++) {
            double total = 0.0;
            for(int j=0;j<data.dim(1);j++)
                total += sqr(data(i,j));
            total = sqrt(total);
            for(int j=0;j<data.dim(1);j++)
                data(i,j) /= total;
        }
    }

    struct GaussModel {
        double count,sx,sx2;
        GaussModel() {
            count = 0;
            sx = 0;
            sx2 = 0;
        }
        void operator+=(double x) {
            count++;
            sx += x;
            sx2 += x*x;
        }
        double cost(double x) {
            double mean = sx/count;
            double var = sx2/count - sqr(mean);
            return sqr(x-mean)/var;
        }
    };

    struct GaussDiagModel {
        float count;
        floatarray sx,sx2;
        GaussDiagModel() {
        }
        void clear(int n) {
            count = 0;
            sx.resize(n);
            sx.fill(0);
            sx2.resize(n);
            sx2.fill(0);
        }
        void operator+=(floatarray &v) {
            if(sx.length()==0) clear(v.length());
            count++;
            sx += v;
            for(int i=0;i<v.length();i++)
                sx2(i) += sqr(v(i));
        }
        float cost(floatarray &v) {
            if(sx.length()==0)  return 1e30;
            float total = 0.0;
            for(int i=0;i<v.length();i++) {
                double mean = sx(i)/count;
                double var = sx2(i)/count - sqr(mean);
                if(var<1e-6) var = mean;
                if(var<1e-6) var = 1.0;
                total += sqr(v(i)-mean)/var;
            }
            CHECK(total>=0);
            return total;
        }
        void save(FILE *stream) {
            scalar_write(stream,count);
            narray_write(stream,sx);
            narray_write(stream,sx2);
        }
        void load(FILE *stream) {
            scalar_read(stream,count);
            narray_read(stream,sx);
            narray_read(stream,sx2);
        }
    };
    inline int fclamp(float x,int lo,int hi) {
        int result = int(x);
        if(result<0) return lo;
        if(result>=hi) return hi-1;
        return result;
    }

    template <class T>
    rectangle bbox(narray<T> &in) {
        int w = in.dim(0), h = in.dim(1);
        int x0=w,y0=h,x1=0,y1=0;
        int threshold = min(in);
        for(int i=0;i<w;i++) {
            for(int j=0;j<h;j++) {
                if(in(i,j)<=threshold) continue;
                if(i<x0) x0 = i;
                if(i>x1) x1 = i;
                if(j<y0) y0 = j;
                if(j>y1) y1 = j;
            }
        }
        return rectangle(x0,y0,x1+1,y1+1);
    }

    void fit_into_box(bytearray &out,bytearray &in,rectangle b,int background=0) {
        int w = in.dim(0), h = in.dim(1);
        float xscale = float(b.width())/out.dim(0);
        float yscale = float(b.height())/out.dim(1);
        float scale = max(xscale,yscale);
        float dx = (w-scale*out.dim(0))/2;
        float dy = (h-scale*out.dim(1))/2;
        out.fill(background);
        for(int i=0;i<out.dim(0);i++) {
            for(int j=0;j<out.dim(1);j++) {
                float x = scale*i+dx;
                float y = scale*j+dy;
                if(x<0||x>w) continue;
                if(y<0||y>h) continue;
                out(i,j) = bilin(in,x,y);
            }
        }
    }

    void extract_box(bytearray &out,bytearray &in,rectangle b,int background=0) {
        int w=b.width(),h=b.height();
        out.resize(w,h);
        out.fill(background);
        for(int i=0;i<w;i++) {
            for(int j=0;j<w;j++) {
                int x = i+b.x0;
                int y = j+b.y0;
                if(x<0||x>=w) continue;
                if(y<0||y>=h) continue;
                out(i,j) = in(x,y);
            }
        }
    }

    template <class T>
    int count(narray<T> &v,float eps=1e-6) {
        int count = 0;
        for(int i=0;i<v.length1d();i++)
            if(fabs(v.at1d(i))>eps)
                count++;
        return count;
    }

    point centroid(bytearray &a) {
        double sx=0,sy=0,w=0;
        for(int i=0;i<a.dim(0);i++) {
            for(int j=0;j<a.dim(1);j++) {
                double v = a(i,j);
                sx += v * i;
                sy += v * j;
                w += v;
            }
        }
        return point(sx/w,sy/w);
    }

    void center(bytearray &out,bytearray &in,int size=32,int background=0) {
        rectangle b = bbox(in);
        point c = centroid(in);
        // in(c.x,c.y) = 96;
        int rx = max(c.x-b.x0,b.x1-c.x+1);
        int ry = max(c.y-b.y0,b.y1-c.y+1);
        int r = max(rx,ry);
        float scale = float(r)/(size/2);
        scale = max(scale,1.0);
        // should do some anti-aliasing on the input here
        out.resize(size,size);
        out.fill(background);
        for(int i=0;i<out.dim(0);i++) {
            for(int j=0;j<out.dim(1);j++) {
                // vector from center of output
                float ox = i-size/2;
                float oy = j-size/2;
                // vector from center of input
                float ix = scale*ox;
                float iy = scale*oy;
                // converted back to input coordinates
                int x = int(ix+c.x+0.5);
                int y = int(iy+c.y+0.5);
                if(x<0 || x>=in.dim(0)) continue;
                if(y<0 || y>=in.dim(1)) continue;
                out(i,j) = in(x,y);
            }
        }
    }

    void center(floatarray &out,bytearray &bin,int size=32,int background=0,int pad=5) {
        rectangle b = bbox(bin);
        point c = centroid(bin);
        int rx = max(c.x-b.x0,b.x1-c.x+1);
        int ry = max(c.y-b.y0,b.y1-c.y+1);
        int r = max(rx,ry);
        float scale = float(r)/(size/2);
        scale = max(scale,1.0);
        floatarray in;
        in.copy(bin);
        pad_by(in,pad,pad);
        // gauss2d(in,scale,scale);
        out.resize(size,size);
        out.fill(background);
        for(int i=0;i<out.dim(0);i++) {
            for(int j=0;j<out.dim(1);j++) {
                // vector from center of output
                float ox = i-size/2;
                float oy = j-size/2;
                // vector from center of input
                float ix = scale*ox;
                float iy = scale*oy;
                // converted back to input coordinates
                float x = ix+c.x+0.5;
                float y = iy+c.y+0.5;
                out(i,j) = bilin(in,x+pad,y+pad);
            }
        }
    }

    void debug_baseline(bytearray &baseline,float slope,float intercept,float xheight,bytearray &image) {
        baseline.copy(image);
        int h = baseline.dim(1);
        for(int x=0;x<baseline.dim(0);x++) {
            int y = fclamp(x*slope+intercept,0,baseline.dim(1)-1);
            baseline(x,y) = 128;
            if(y+1<h) baseline(x,y+1) = 128;
            if(y+xheight<h) baseline(x,int(y+xheight)) = 128;
            if(y+xheight+1<h) baseline(x,int(y+xheight)+1) = 128;
        }
    }

    void show_baseline(float slope,float intercept,float xheight,bytearray &image,const char *where="") {
        bytearray baseline;
        debug_baseline(baseline,slope,intercept,xheight,image);
        dshow(baseline,where);
    }

    void segmentation_as_bitmap(bytearray &image,intarray &cseg) {
        image.makelike(cseg);
        for(int i=0;i<image.length1d();i++) {
            int value = cseg.at1d(i);
            if(value==0||value<0xffffff) image.at1d(i) = 255;
        }
    }

    void dump_fst(const char *file,IGenericFst &fst) {
        stdio stream(file,"w");
        intarray ids,targets,outputs;
        floatarray costs;
        intarray index(fst.nStates());
        floatarray mincost(fst.nStates());
        fprintf(stream,"digraph fst {\n");
        fprintf(stream,"rankdir = LR;\n");
        for(int i=0;i<fst.nStates();i++) {
            fst.arcs(ids,targets,outputs,costs,i);
            index.fill(-1);
            mincost.fill(INFINITY);
            for(int j=0;j<targets.length();j++) {
                int target = targets[j];
                if(mincost[target]>costs[j]) {
                    index[target] = j;
                    mincost[target] = costs[j];
                }
            }
            for(int j=0;j<index.length();j++) {
                char buf[100]; // costs[k]
                int k = index[j];
                if(k<0) continue;
                sprintf(buf,"%d",int(costs[k]));
                // for(int r=0;r<log(costs[k])/log(10);r++) strcat(buf,"*");
                if(outputs[k]<32||outputs[k]>=127) {
                    fprintf(stream,"   %d -> %d [label=\"%d %s\"];\n",
                            i,targets[k],outputs[k],buf);
                } else if(outputs[k]==32) {
                    fprintf(stream,"   %d -> %d [label=\"_ %s\"];\n",
                            i,targets[k],buf);
                } else {
                    fprintf(stream,"   %d -> %d [label=\"%c %s\"];\n",
                            i,targets[k],outputs[k],buf);
                }
            }
        }
        fprintf(stream,"}\n");
    }

    void fst_to_image(intarray &image,IGenericFst &fst) {
        dump_fst("/tmp/_temp_.fst",fst);
        system("dot -Tpng /tmp/_temp_.fst > /tmp/_temp_.png");
        system("convert -geometry '50%x50%' -depth 8 /tmp/_temp_.png /tmp/_temp2_.png");
        read_image_packed(image,"/tmp/_temp2_.png");
    }
    void print_fst_simple(IGenericFst &fst) {
        printf("void construct_fst(IGenericFst &fst) {\n");
        printf("    int nstates = %d;\n",fst.nStates());
        printf("    for(int i=0;i<nstates;i++) ASSERT(i==fst.newState();)\n");
        printf("    fst.setStart(%d);\n",fst.getStart());
        for(int i=0;i<fst.nStates();i++) {
            intarray ids,targets,outputs;
            floatarray costs;
            if(fst.getAcceptCost(i)<1e30)
                printf("    fst.setAccept(%d,%g);\n",
                       i,fst.getAcceptCost(i));
            fst.arcs(ids,targets,outputs,costs,i);
            for(int j=0;j<outputs.length();j++)
                printf("    fst.addTransition(%d,%d,%d/*%c*/,%g,%d);\n",
                       i,targets(j),
                       outputs(j),(outputs(j)<32?'?':outputs(j)),
                       costs(j),
                       outputs(j)); // FIXME
        }
        printf("}\n");
    }

    struct DebugFst : IGenericFst {
        int state;
        DebugFst() { state = 0; }
        const char *description() { return "DebugFst"; }
        int special(const char *) { throw "unimplemented"; }
        void load(const char *) { throw "unimplemented"; }
        void save(const char *) { throw "unimplemented"; }
        void clear() {}
        int newState() { return state++; }
        void addTransition(int from,int to,int output,float cost,int input) {
            fprintf(stderr,"%4d -> %4d [%d]'%c' %g [%d]'%c'\n",
                    from,to,
                    output,(output<32?'?':output),
                    cost,
                    input,(input<32?'?':input));
        }
        void setStart(int node) {
            fprintf(stderr,"START %d\n",node);
        }
        void setAccept(int node,float cost) {
            fprintf(stderr,"ACCEPT %d %g\n",node,cost);
        }
        void bestpath(nustring &result) {
            result.clear();
            result.utf8Decode("debug-fst-string",16);
        }
    };

}

extern void init_classutils();

#endif
