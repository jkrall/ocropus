#ifndef gsl_h__
#define gsl_h__

#include <gsl/gsl_linalg.h>

namespace gsl {
    // FIXME clean this up with C++0x rvalues
    struct vector {
        gsl_vector *p;
        gsl_vector_view view;
        vector(int n=1) {
            p = gsl_vector_alloc(n);
            view = gsl_vector_subvector_with_stride(p,0,p->stride,p->size);
        }
        ~vector() {
            gsl_vector_free(p);
            p = 0;
        }
        int length() {
            return p->size;
        }
        int dim(int i) {
            if(i==0) return p->size;
            throw "oops";
        }
        void resize(int n) {
            gsl_vector_free(p);
            p = gsl_vector_alloc(n);
        }
        void operator=(double value) {
            gsl_vector_set_all(p,value);
        }
        double &operator()(int i) {
            CHECK(unsigned(i)<=unsigned(p->size));
            return p->data[i*p->stride];
        }
        operator gsl_vector_view *() {
            return &view;
        }
        operator gsl_vector *() {
            return p;
        }
        template <class T>
        void get(T &other) {
            resize(other.dim(0));
            for(int i=0;i<other.dim(0);i++)
                operator()(i) = other(i);
        }
        template <class T>
        void put(T &other) {
            other.resize(dim(0));
            for(int i=0;i<dim(0);i++)
                other(i) = operator()(i);
        }
    };

    struct matrix {
        gsl_matrix *p;
        gsl_matrix_view view;
        matrix(int n=1,int m=1) {
            p = gsl_matrix_alloc(n,m);
            view = gsl_matrix_view_array_with_tda(p->data,p->size1,p->size2,p->tda);
        }
        ~matrix() {
            gsl_matrix_free(p);
            p = 0;
        }
        int dim(int i) {
            if(i==0) return p->size1;
            if(i==1) return p->size2;
            throw "oops";
        }
        void resize(int n,int m) {
            gsl_matrix_free(p);
            p = gsl_matrix_alloc(n,m);
            view = gsl_matrix_view_array_with_tda(p->data,p->size1,p->size2,p->tda);
        }
        void operator=(double value) {
            gsl_matrix_set_all(p,value);
        }
        double &operator()(int i,int j) {
            CHECK(unsigned(i)<=unsigned(p->size1));
            CHECK(unsigned(j)<=unsigned(p->size2));
            return p->data[i*p->tda+j];
        }
        operator gsl_matrix_view *() {
            return &view;
        }
        operator gsl_matrix *() {
            return p;
        }
        template <class T>
        void get(T &other) {
            resize(other.dim(0),other.dim(1));
            for(int i=0;i<other.dim(0);i++)
                for(int j=0;j<other.dim(1);j++)
                    operator()(i,j) = other(i,j);
        }
        template <class T>
        void put(T &other) {
            other.resize(dim(0),dim(1));
            for(int i=0;i<dim(0);i++)
                for(int j=0;j<dim(1);j++)
                    other(i,j) = operator()(i,j);
        }
    };
};

#endif
