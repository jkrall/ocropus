#ifndef glcuts_h__
#define glcuts_h__

#include "ocropus.h"

namespace glinerec {
    using namespace ocropus;
    struct IDpSegmenter : ISegmentLine {
        int down_cost;
        int outside_diagonal_cost;
        int inside_diagonal_cost;
        int boundary_diagonal_cost;
        int inside_weight;
        int boundary_weight;
        int outside_weight;
        int min_range;
        float cost_smooth;
        float min_thresh;
        intarray dimage;
    };
    IDpSegmenter *make_DpSegmenter();
}

#endif
