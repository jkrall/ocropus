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
// Project:
// File: editdist.cc
// Purpose: edit distance with block movements
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#include <stdio.h>
#include "colib/colib.h"
#include "editdist.h"

using namespace colib;

namespace {
    template<class T, class S> void fill_slice(narray<T> &array,
                                               S item,
                                               int from_incl,
                                               int upto_excl) {
        for (int i = from_incl; i < upto_excl; i++)
            array[i] = item;
    }

    void fill_edit_distance_table(floatarray &d, nustring &str1, nustring &str2, float del_cost, float ins_cost, float sub_cost) {
        int m = str1.length();
        int n = str2.length();
        floatarray temp_del_ins_sub(3);
        d.resize(m+1,n+1);
        for(int i=0;i<=m;i++)
                d(i,0)=i;
        for(int j=0;j<=n;j++)
                d(0,j)=j;
        for(int i=1;i<=m;i++) {
            for(int j=1;j<=n;j++) {
                temp_del_ins_sub(0) = d(i-1,j) + del_cost;
                temp_del_ins_sub(1) = d(i,j-1) + ins_cost;
                temp_del_ins_sub(2) = d(i-1,j-1) + (str1(i-1) == str2(j-1) ? 0. : sub_cost);
                d(i,j) = min(temp_del_ins_sub);
            }
        }
    }

}




namespace ocropus {

    float edit_distance(nustring &str1, nustring &str2, float del_cost, float ins_cost, float sub_cost) {
        floatarray d;
        fill_edit_distance_table(d, str1, str2, del_cost, ins_cost, sub_cost);
        return d(str1.length(), str2.length());
    }

    float edit_distance(intarray &confusion,
                        nustring &str1,
                        nustring &str2,
                        float del_cost,
                        float ins_cost,
                        float sub_cost) {
        floatarray d;
        fill_edit_distance_table(d, str1, str2, del_cost, ins_cost, sub_cost);

        /// backtrack the journey until we touch a border
        int i = str1.length();
        int j = str2.length();
        while(i && j) {
            floatarray temp_del_ins_sub(3);
            temp_del_ins_sub(0) = d(i-1,j) + del_cost;
            temp_del_ins_sub(1) = d(i,j-1) + ins_cost;
            temp_del_ins_sub(2) = d(i-1,j-1) + (str1[i-1] == str2[j-1] ? 0. : sub_cost);
            switch(argmin(temp_del_ins_sub)) {
                case 0:
                    i--;
                    confusion(str1[i].ord(), 0)++;
                break;
                case 1:
                    j--;
                    confusion(0, str2[j].ord())++;
                break;
                case 2:
                    i--; j--;
                    confusion(str1[i].ord(), str2[j].ord())++;
            }
        }

        /// we've touched the border, now we have just one way to go
        while(i--)
            confusion(str1[i].ord(), 0)++;
        while(j--)
            confusion(0, str2[j].ord())++;

        return d(str1.length(), str2.length());
    }

    float block_move_edit_cost(nustring &from, nustring &to, float c) {
        floatarray upper, row;
        row.resize(from.length() + 1);
        upper.resize(from.length() + 1);
        for(int j=0;j<=from.length();j++)
            row[j] = min(c, j);
        for(int i=1;i<=to.length();i++) {
            swap(row, upper);

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

    /// Same as block_move_edit_cost(), but also records all block movements
    /// (aka cursor jumps) in the form of two integer arrays.
    float block_move_edit_cost_record_jumps(intarray &jumps_from, intarray &jumps_to, nustring &from, nustring &to, float c) {
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


    float edit_cost_for_layout(intarray &jumps_from, intarray &jumps_to, nustring &from, nustring &to, float c) {
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

            bool can_jump_everywhere = (to[i-1] == nuchar('\n'));
            // find the minimum in the current row
            // (one for all, another ("absolute") for permitted only)
            float best = row[0];
            int best_index = 0;
            float abs_best = row[0];
            int abs_best_index = 0;
            for(int j=1;j<=from.length();j++) {
                if(row[j] < abs_best) {
                    abs_best = row[j];
                    abs_best_index = j;
                }
                if(!can_jump_everywhere && from[j-1] != nuchar('\n'))
                    continue;
                if(row[j] < best) {
                    best = row[j];
                    best_index = j;
                }
            }

            // now a sequence of actions
            float upper_bound = best + c;
            float better_upper_bound = abs_best + c;

            for(int j=0;j<=from.length();j++) {
                if ((j == 0 || from[j-1] != nuchar('\n'))
                && row[j] > better_upper_bound) {
                    // we jump from best_index to j
                    row[j] = better_upper_bound;
                    copy(jumps[j], jumps[abs_best_index]);
                    jumps[j].push(abs_best_index);
                    jumps[j].push(j);
                } else if (row[j] > upper_bound) {
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


    float block_move_edit_distance(nustring &a, nustring &b, float c) {
        return (block_move_edit_cost(a, b, c)
              + block_move_edit_cost(b, a, c)) / 2;
    }

    void analyze_jumps(bytearray &area_covered_by_non_jumps,
                       intarray &from,
                       intarray &to,
                       int source_length) {
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
        fill_slice(area_covered_by_non_jumps,1, to[njumps-1], source_length);
    }

    void get_text_jumped_over(nustring &result, bytearray &covered, nustring &text) {
        CHECK_ARG(covered.length() == text.length());
        result.clear();
        for(int i = 0; i < text.length(); i++) {
            if(!covered[i])
                result.push(text[i]);
        }
    }

}
