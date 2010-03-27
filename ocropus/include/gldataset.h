// -*- C++ -*-

#ifndef gldataset_h__
#define gldataset_h__

#include "narray-binio.h"

namespace glinerec {
    using namespace narray_io;

    struct IDataset : IComponent {
        virtual int nsamples() = 0;
        virtual int nclasses() = 0;
        virtual int nfeatures() = 0;
        virtual void input(floatarray &v,int i) = 0;
        virtual int cls(int i) = 0;
        virtual int id(int i) = 0;
    };

    struct float8 {
        signed char val;
        float8() {
        }
        float8(double v) {
            if(v<-1.2||v>1.2) throw "float8: value out of range";
            val = (int)(v*100);
        }
        float8(float v) {
            if(v<-1.2||v>1.2) throw "float8: value out of range";
            val = (int)(v*100);
        }
        operator float() {
            return val/100.0;
        }
    };

    template <class T>
    struct Dataset : IDataset {
        narray<T> &data;
        intarray &classes;
        int nc;
        int nf;
        Dataset(narray<T> &data,intarray &classes)
            :data(data),classes(classes) {
            if(classes.length()>0) {
                nc = max(classes)+1;
                nf = data.dim(1);
                CHECK(min(data)>-100 && max(data)<100);
                CHECK(min(classes)>=-1 && max(classes)<10000);
            } else {
                nc = 0;
                nf = -1;
            }
        }
        const char *name() {
            return "dataset";
        }
        void save(FILE *stream) {
            magic_write(stream,"dataset");
            int t = sizeof (T);
            scalar_write(stream,t);
            narray_write(stream,data);
            narray_write(stream,classes);
            scalar_write(stream,nc);
            scalar_write(stream,nf);
        }
        void load(FILE *stream) {
            magic_read(stream,"dataset");
            int t;
            scalar_read(stream,t);
            CHECK(t==sizeof (T));
            narray_read(stream,data);
            narray_read(stream,classes);
            scalar_read(stream,nc);
            scalar_read(stream,nf);
        }
        int nsamples() {
            return data.dim(0);
        }
        int nclasses() {
            return nc;
        }
        int nfeatures() {
            return nf;
        }
        int cls(int i) {
            return classes(i);
        }
        void input(floatarray &v,int i) {
            v.resize(data.dim(1));
            for(int j=0;j<v.dim(0);j++)
                v(j) = data(i,j);
        }
        int id(int i) {
            return i;
        }
        void add(floatarray &v,int c) {
            CHECK(min(v)>-100 && max(v)<100);
            CHECK(c>=-1 && c<1000000);
            if(c>=nc) nc = c+1;
            if(nf<0) nf = v.length();
            rowpush(data,v);
            classes.push(c);
        }
        void clear() {
            data.clear();
            classes.clear();
            nc = 0;
            nf = -1;
        }
    };

    template <class T>
    struct RowDataset : IDataset {
        objlist< narray<T> > data;
        intarray classes;
        int nc;
        int nf;
        RowDataset() {
            nc = -1;
            nf = -1;
        }
        RowDataset(narray<T> &ds,intarray &cs) {
            add(ds,cs);
            recompute();
        }
        const char *name() {
            return "rowdataset";
        }
        void save(FILE *stream) {
            magic_write(stream,"dataset");
            int t = sizeof (T);
            scalar_write(stream,t);
            scalar_write(stream,int(data.dim(0)));
            scalar_write(stream,nc);
            scalar_write(stream,nf);
            for(int i=0;i<data.dim(0);i++) {
                narray_write(stream,data(i));
            }
            narray_write(stream,classes);
        }
        void load(FILE *stream) {
            magic_read(stream,"dataset");
            int t,nsamples;
            scalar_read(stream,t);
            CHECK(t==sizeof (T));
            scalar_read(stream,nsamples);
            scalar_read(stream,nc);
            scalar_read(stream,nf);
            for(int i=0;i<nsamples;i++) {
                narray_read(stream,data.push());
            }
            narray_read(stream,classes);
            CHECK(nf>0&&nf<1000000);
            CHECK(nc>0&&nc<1000000);
        }
        int nsamples() {
            return data.dim(0);
        }
        int nclasses() {
            return nc;
        }
        int nfeatures() {
            return nf;
        }
        int cls(int i) {
            return classes(i);
        }
        void input(floatarray &v,int i) {
            v.copy(data(i));
        }
        int id(int i) {
            return i;
        }
        void add(floatarray &v,int c) {
            CHECK(min(v)>-100 && max(v)<100);
            CHECK(c>=-1 && c<1000000);
            if(c>=nc) nc = c+1;
            if(nf<0) nf = v.length();
            data.push().copy(v);
            classes.push() = c;
            CHECK(nc>0);
            CHECK(nf>0);
        }
        void add(narray<T> &ds,intarray &cs) {
            for(int i=0;i<ds.dim(0);i++) {
                rowget(data.push(),ds,i);
                classes.push() = cs(i);
            }
        }
        void recompute() {
            nc = max(classes)+1;
            nf = data(0).dim(0);
            CHECK(min(data)>-100 && max(data)<100);
            CHECK(min(classes)>=-1 && max(classes)<10000);
            CHECK(nc>0);
            CHECK(nf>0);
        }
        void clear() {
            data.clear();
            classes.clear();
            nc = 0;
            nf = -1;
        }
    };

    struct MappedDataset : IDataset {
        IDataset &ds;
        intarray &classes;
        MappedDataset(IDataset &ds,intarray &classes)
            :ds(ds),classes(classes) {
            CHECK(ds.nsamples()==classes.length());
        }
        const char *name() {
            return "mappedds";
        }
        int nsamples() {
            return ds.nsamples();
        }
        int nclasses() {
            return max(classes)+1;
        }
        int nfeatures() {
            return ds.nfeatures();
        }
        int cls(int i) {
            return classes(i);
        }
        void input(floatarray &v,int i) {
            ds.input(v,i);
        }
        int id(int i) {
            return ds.id(i);
        }
    };

    struct Datasubset : IDataset {
        IDataset &ds;
        intarray &samples;
        Datasubset(IDataset &ds,intarray &samples)
            :ds(ds),samples(samples) {}
        const char *name() {
            return "datasubset";
        }
        int nsamples() {
            return samples.length();
        }
        int nclasses() {
            return ds.nclasses();
        }
        int nfeatures() {
            return ds.nfeatures();
        }
        int cls(int i) {
            return ds.cls(samples(i));
        }
        void input(floatarray &v,int i) {
            ds.input(v,samples(i));
        }
        int id(int i) {
            return ds.id(samples(i));
        }
    };

    struct AugmentedDataset : IDataset {
        IDataset &ds;
        narray<floatarray> augments;
        AugmentedDataset(IDataset &ds):ds(ds) {
            augments.resize(ds.nsamples());
        }
        const char *name() {
            return "augmenteddataset";
        }
        int nsamples() {
            return ds.nsamples();
        }
        int nclasses() {
            return ds.nclasses();
        }
        int nfeatures() {
            return ds.nfeatures() + augments(0).length();
        }
        int cls(int i) {
            return ds.cls(i);
        }
        void input(floatarray &v,int i) {
            ds.input(v,i);
            floatarray &a = augments(i);
            for(int j=0;j<a.dim(0);j++)
                v.push() = a(j);
        }
        int id(int i) {
            return ds.id(i);
        }
        void augment(int i,floatarray &p) {
            for(int j=0;j<p.dim(0);j++)
                augments(i).push() = p(j);
        }
    };
}

#endif
