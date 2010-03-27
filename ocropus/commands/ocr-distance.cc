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
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites:


#include <stdio.h>
#include "colib/colib.h"
#include "ocropus.h"

using namespace ocropus;
using namespace colib;

namespace {
    template<class T>
    void swap_(narray<T> &a, narray<T> &b) {
        narray<T> c;
        move(c,a);
        move(a,b);
        move(b,c);
    }
}

float dk_edit_cost(bytearray &from, bytearray &to, float c) {
    floatarray upper, row;
    row.resize(from.length() + 1);
    upper.resize(from.length() + 1);
    for(int j=0;j<=from.length();j++)
        row[j] = min(c, j);
    for(int i=1;i<=to.length();i++) {
        swap_(row, upper);

        // calculate costs of a sequence of actions
        row[0] = upper[0] + 1;
        for(int j=1;j<=from.length();j++)
            row[j] = min(min(upper[j] + 1, upper[j-1] + (to[i-1] == from[j-1] ? 0 : 1)),
                               row[j-1] + 1);

        // find the minimum in the current row
        float best = row[0];
        for(int j=1;j<=from.length();j++) {
            if (row[j] < best)
                best = row[j];
        }

        // now a sequence of actions
        float upper_bound = best + c;

        for(int j=0;j<=from.length();j++) {
            if (row[j] > upper_bound)
                row[j] = upper_bound;
        }
    }

    //printf_narray("%f", table);
    return row[from.length()];
}

#if 0

// FIXME/mezhirov dead code -- remove this if it's not working/needed anymore --tmb

float dk_edit_cost_record_jumps(intarray &jumps_from, intarray &jumps_to, bytearray &from, bytearray &to, float c) {
    floatarray upper(from.length() + 1);
    floatarray row(from.length() + 1);
    narray<intarray> jumps(from.length() + 1);
    for(int j=0;j<=from.length();j++)
        row[j] = min(c, j);
    for(int i=1;i<=to.length();i++) {
        swap(row, upper);
        intarray jumps_tmp; // contains what was in jumps[j-1] at previous iteration

        // calculate costs of a sequence of actions
        row[0] = upper[0] + 1;
        for(int j=1;j<=from.length();j++) {
            float go_up_cost = upper[j] + 1;
            float go_diag_cost = upper[j-1] + (to[i-1] == from[j-1] ? 0 : 1);
            float go_left_cost = row[j-1] + 1;

            if (go_up_cost < go_diag_cost && go_up_cost < go_left_cost) {
                row[j] = go_up_cost;
                copy(jumps_tmp, jumps[j]);
            } else if (go_left_cost < go_diag_cost) {
                row[j] = go_left_cost;
                move(jumps_tmp, jumps[j]);
                copy(jumps[j], jumps[j-1]);
            } else {
                row[j] = go_diag_cost;
                swap(jumps_tmp, jumps[j]);
            }
        }

        // find the minimum in the current row
        float best = row[0];
        int best_index = 0;
        for(int j=1;j<=from.length();j++) {
            if (row[j] < best) {
                best = row[j];
                best_index = j;
            }
        }

        // now a sequence of actions
        float upper_bound = best + c;

        for(int j=0;j<=from.length();j++) {
            if (row[j] > upper_bound) {
                // we jump from best_index to j
                row[j] = upper_bound;
                copy(jumps[j], jumps[best_index]);
                jumps[j].push(best_index);
                jumps[j].push(j);
            }
        }
    }

    //printf_narray("%f", table);
    intarray &jumplist = jumps[from.length()];
    jumps_from.resize(jumplist.length() / 2);
    makelike(jumps_to, jumps_from);
    for (int i = 0; i < jumps_from.length(); i++) {
        jumps_from[i] = jumplist[2 * i];
        jumps_to[i] = jumplist[2 * i + 1];
    }
    return row[from.length()];
}
#endif

float dk_edit_dist(bytearray &a, bytearray &b, float c) {
    return (dk_edit_cost(a, b, c)
          + dk_edit_cost(b, a, c)) / 2;
}

void load_entire_file(bytearray &result, const char *path) {
    int c;
    result.clear();
    stdio f(path, "r");
    while ((c = fgetc(f)) != EOF)
        result.push(c);
}

void show_usage_and_exit(const char *we) {
    printf("usage: %s <source> <result> <jump penalty>\n", we);
    exit(1);
}

template<class T, class S>
void fill_slice(narray<T> &array, S item, int from, int upto_excl) {
    for (int i = from; i < upto_excl; i++)
        array[i] = item;
}


void analyze_jumps(bytearray &area_covered_by_non_jumps, intarray &from, intarray &to, int source_length) {
    area_covered_by_non_jumps.resize(source_length);
    int njumps = from.length();
    ASSERT(njumps == to.length());
    if (!njumps) {
        fill(area_covered_by_non_jumps, 1);
        return;
    }
    fill(area_covered_by_non_jumps, 0);
    fill_slice(area_covered_by_non_jumps, 1, 0, from[0]);
    for (int i = 0; i < njumps - 1; i++) {
        fill_slice(area_covered_by_non_jumps, 1, to[i], from[i+1]);
    }
    fill_slice(area_covered_by_non_jumps, 1, to[njumps-1], source_length);
}

void print_uncovered(bytearray &covered, bytearray &text) {
    ASSERT(covered.length() == text.length());
    for (int i = 0; i < text.length(); i++) {
        if (!covered[i])
            putchar(text[i]);
    }
    putchar('\n');
}

int main(int argc, char **argv) {
    bytearray a, b;
    intarray from, to;
    if (argc != 4)
        show_usage_and_exit(argv[0]);
    load_entire_file(a, argv[1]);
    load_entire_file(b, argv[2]);
    double c = atof(argv[3]);
    printf("%f\n", 100. * dk_edit_dist(a, b, c) / a.length());
    //printf("Edit cost: %f\n", dk_edit_cost_record_jumps(from, to, a, b, c));
    //bytearray covered;
    //analyze_jumps(covered, from, to, a.length());
    //printf("Text present in %s but missing from %s follows\n", argv[1], argv[2]);
    //print_uncovered(covered, a);
    exit(0);
}
