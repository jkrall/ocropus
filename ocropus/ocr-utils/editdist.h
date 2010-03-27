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
// File: editdist.h
// Purpose: edit distance with block movements
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de

#ifndef h_editdist_
#define h_editdist_

namespace ocropus {
    float edit_distance(colib::nustring &str1, colib::nustring &str2, float del_cost=1, float ins_cost=1, float sub_cost=1);

    /// A variant of edit_distance() with a confusion matrix.
    /// The confusion matrix should be pre-initialized.
    float edit_distance(colib::intarray &confusion,
                        colib::nustring &str1,
                        colib::nustring &str2,
                        float del_cost=1,
                        float ins_cost=1,
                        float sub_cost=1);


    /// Asymmetric edit cost with block movement.
    /// \param[in] from     The string where editing starts.
    /// \param[in] to       The string where editing finishes.
    /// \param c        Cost of block movement (moving the "cursor" anywhere).
    float block_move_edit_cost(colib::nustring &from,
                               colib::nustring &to,
                               float c);
    
    /// Symmetrization of block_move_edit_cost().
    float block_move_edit_distance(colib::nustring &a,
                                   colib::nustring &b,
                                   float c);
    
    /// Same as block_move_edit_cost(), but also records all block movements
    /// (aka cursor jumps) in the form of two integer arrays.
    float block_move_edit_cost_record_jumps(colib::intarray &jumps_from,
                                            colib::intarray &jumps_to,
                                            colib::nustring &from,
                                            colib::nustring &to, float c);
    
    /// See what parts of the text were 
    /// By looking at the jumps recorded by block_move_edit_cost_record_jumps(),
    /// this function produces a map of areas in the source string
    /// covered by non-jump cursor movements.
    void analyze_jumps(colib::bytearray &area_covered_by_non_jumps,
                       colib::intarray &from,
                       colib::intarray &to,
                       int source_length);
    
    void get_text_jumped_over(colib::nustring &result, 
                              colib::bytearray &covered,
                              colib::nustring &text);

    float edit_cost_for_layout(colib::intarray &jumps_from, colib::intarray &jumps_to, colib::nustring &from, colib::nustring &to, float c);

}

#endif
