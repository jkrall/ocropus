#ifndef glfmaps_h__
#define glfmaps_h__

#include "ocropus.h"

namespace glinerec {
    using namespace colib;
    using namespace ocropus;

    struct IFeatureMap : IComponent {
        virtual void setLine(bytearray &image) = 0;
        virtual void extractFeatures(floatarray &v,
                                     rectangle b,
                                     bytearray &mask) = 0;
    };

    IFeatureMap *make_SimpleFeatureMap();
    IFeatureMap *make_RidgeFeatureMap();
    void init_glfmaps();
}

#endif
