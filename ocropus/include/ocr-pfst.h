// Copyright 2007-2008 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Project: ocr-pfst
// File: ocr-pfst.h
// Purpose: PFST public API
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org

#ifndef h_ocr_pfst_
#define h_ocr_pfst_

/// \file ocr-pfst.h

#include "colib/colib.h"
#include "ocrinterfaces.h"

namespace ocropus {
    using namespace colib;

    struct OcroFST : IGenericFst {
        virtual intarray &targets(int vertex) = 0;
        virtual intarray &inputs(int vertex) = 0;
        virtual intarray &outputs(int vertex) = 0;
        virtual floatarray &costs(int vertex) = 0;
        virtual float acceptCost(int vertex) = 0;
        virtual void setAcceptCost(int vertex, float new_value) = 0;
        virtual floatarray &heuristics() = 0;
        
        enum {
            SORTED_BY_INPUT = 1,
            SORTED_BY_OUTPUT = 2,
            HAS_HEURISTICS = 4
        };
        virtual bool hasFlag(int flag) = 0;
        virtual void clearFlags() = 0;

        virtual void sortByInput() = 0;
        virtual void sortByOutput() = 0;
        virtual void calculateHeuristics() = 0;
    };

    OcroFST *make_OcroFST();

    /// \brief Copy one FST to another.
    ///
    /// \param[out]     dst     The destination. Will be cleared before copying.
    /// \param[in]      src     The FST to copy.
    void fst_copy(IGenericFst &dst, IGenericFst &src);

    /// \brief Copy one FST to another, preserving only lowest-cost arcs.
    /// This is useful for visualization.
    ///
    /// \param[out]     dst     The destination. Will be cleared before copying.
    /// \param[in]      src     The FST to copy.
    void fst_copy_best_arcs_only(IGenericFst &dst, IGenericFst &src);

    /// \brief Compose two FSTs.
    ///
    /// This function copies the composition of two given FSTs.
    /// That causes expansion (storing all arcs explicitly).
    void fst_expand_composition(IGenericFst &out, OcroFST &, OcroFST &);


    /// Randomly sample an FST, assuming any input.
    ///
    /// \param[out] result  The array of output labels (incl. 0) along the path.
    /// \param[in]  fst     The FST.
    /// \param      max     The maximum length of the result.
    /// \returns total cost
    double fst_sample(intarray &result, IGenericFst &fst, int max=1000);

    /// Randomly sample an FST, assuming any input. Removes epsilons.
    /// \param[out] result  The array of output symbols, excluding epsilons.
    /// \param[in]  fst     The FST.
    /// \param      max     The maximum length of the result.
    /// \returns total cost
    double fst_sample(nustring &result, IGenericFst &fst, int max=1000);

    /// Remove epsilons (zeros) and converts integers to nuchars.
    void remove_epsilons(nustring &s, intarray &a);
    
    /// Make an in-place Kleene closure of the FST.
    void fst_star(IGenericFst &fst);

    /// Make a Kleene closure.
    void fst_star(IGenericFst &result, IGenericFst &fst);

    void fst_line(IGenericFst &fst, nustring &s);
    void get_alphabet(intarray &alphabet, objlist<nustring> &dict);

    /// \brief Simplified interface for a_star().
    ///
    /// \param[out] result      FST output with epsilons removed,
    ///                         converted to a nustring.
    /// \param[in]  fst         the FST to search
    /// \returns total cost
    double a_star(nustring &result, OcroFST &fst);

    /// \brief Simplified interface for a_star_in_composition().
    ///
    /// \param[out] result      FST output with epsilons removed,
    ///                         converted to a nustring.
    /// \param[in]  fst         the FST to search
    /// \returns total cost
    double a_star(nustring &result, OcroFST &fst1, OcroFST &fst2);


    // TODO/mezhirov document return value
    /// \brief Search for the best path through the composition of 2 FSTs
    ///        using A* algorithm.
    ///
    /// The interface is analogous to a_star(),
    /// but returns 2 arrays of vertices.
    ///
    /// This function uses A* on reversed FSTs separately,
    /// then uses the cost sum as a heuristic.
    /// No attempt to cache the heuristic is made.
    bool a_star_in_composition(intarray &inputs,
                               intarray &vertices1,
                               intarray &vertices2,
                               intarray &outputs,
                               floatarray &costs,
                               OcroFST &fst1,
                               OcroFST &fst2);

    bool a_star_in_composition(intarray &inputs,
                               intarray &vertices1,
                               intarray &vertices2,
                               intarray &outputs,
                               floatarray &costs,
                               OcroFST &fst1,
                               floatarray &g1,
                               OcroFST &fst2,
                               floatarray &g2);


    // TODO/mezhirov document return value
    /// \brief Search for the best path through the composition of 3 FSTs
    ///        using A* algorithm.
    /// The interface is analogous to a_star(),
    /// but returns 3 arrays of vertices.
    ///
    /// This function uses A* on reversed FSTs separately,
    /// then uses the cost sum as a heuristic.
    /// No attempt to cache the heuristic is made.
    /*bool a_star_in_composition(intarray &inputs,
                               intarray &vertices1,
                               intarray &vertices2,
                               intarray &vertices3,
                               intarray &outputs,
                               floatarray &costs,
                               OcroFST &fst1,
                               OcroFST &fst2,
                               OcroFST &fst3);
    */
    
    void beam_search(intarray &vertices1,
                     intarray &vertices2,
                     intarray &inputs,
                     intarray &outputs,
                     floatarray &costs,
                     OcroFST &fst1,
                     OcroFST &fst2,
                     int beam_width=1000);

    double beam_search(nustring &result, OcroFST &fst1, OcroFST &fst2,
                       int beam_width=1000);

};

#endif
