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
// Project:
// File: lattice.cc
// Purpose:
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org


#include "ocr-pfst.h"
#include "lattice.h"

using namespace colib;
using namespace ocropus;

namespace {
    /// FIXME/mezhirov presort -- ???
    struct CompositionFstImpl : CompositionFst {
        autodel<IGenericFst> l1, l2;
        int override_start;
        int override_finish;
        virtual const char *description() {return "CompositionLattice";}
        CompositionFstImpl(IGenericFst *l1, IGenericFst *l2,
                               int o_s, int o_f) :
            override_start(o_s), override_finish(o_f) {
            CHECK_ARG(l1->nStates() > 0);
            CHECK_ARG(l2->nStates() > 0);

            // this should be here, not in the initializers.
            // (otherwise if CHECKs throw an exception, bad things happen)
            this->l1 = l1;
            this->l2 = l2;
        }

        IGenericFst *move1() {return l1.move();}
        IGenericFst *move2() {return l2.move();}

        void checkOwnsNothing() {
            ALWAYS_ASSERT(!l1);
            ALWAYS_ASSERT(!l2);
        }

        virtual int nStates() {
            return l1->nStates() * l2->nStates();
        }
        int combine(int i1, int i2) {
            return i1 * l2->nStates() + i2;
        }
        virtual int getStart() {
            int s1 = override_start >= 0 ? override_start : l1->getStart();
            return combine(s1, l2->getStart());
        }
        virtual void splitIndex(int &result1, int &result2, int index) {
            int k = l2->nStates();
            result1 = index / k;
            result2 = index % k;
        }
        virtual void splitIndices(colib::intarray &result1,
                                  colib::intarray &result2,
                                  colib::intarray &indices) {
            makelike(result1, indices);
            makelike(result2, indices);
            int k = l2->nStates();
            for(int i = 0; i < indices.length(); i++) {
                result1[i] = indices[i] / k;
                result2[i] = indices[i] % k;
            }
        }

        virtual float getAcceptCost(int node) {
            int i1 = node / l2->nStates();
            int i2 = node % l2->nStates();
            double cost1;
            if(override_finish >= 0)
                cost1 = i1 == override_finish ? 0 : 1e38;
            else
                cost1 = l1->getAcceptCost(i1);
            return cost1 + l2->getAcceptCost(i2);
        }
        virtual void load(const char *) {
            // Saving compositions can't be implemented because we need to
            // delegate save() to the operands, but they need to accept streams,
            // not file names. Fortunately, we don't need it. --IM
            throw "CompositionFstImpl::load unimplemented";
        }
        virtual void save(const char *) {
            throw "CompositionFstImpl::save unimplemented";
        }
        virtual void arcs(intarray &ids,
                          intarray &targets,
                          intarray &outputs,
                          floatarray &costs,
                          int node) {
            int n1 = node / l2->nStates();
            int n2 = node % l2->nStates();
            intarray ids1, ids2;
            intarray t1, t2;
            intarray o1, o2;
            floatarray c1, c2;
            l1->arcs(ids1, t1, o1, c1, n1);
            l2->arcs(ids2, t2, o2, c2, n2);

            // sort & permute
            intarray p1, p2;

            quicksort(p1, o1);
            permute(ids1, p1);
            permute(t1, p1);
            permute(o1, p1);
            permute(c1, p1);

            quicksort(p2, ids2);
            permute(ids2, p2);
            permute(t2, p2);
            permute(o2, p2);
            permute(c2, p2);

            int k1, k2;
            // l1 epsilon moves
            for(k1 = 0; k1 < o1.length() && !o1[k1]; k1++) {
                ids.push(ids1[k1]);
                targets.push(combine(t1[k1], n2));
                outputs.push(0);
                costs.push(c1[k1]);
            }
            // l2 epsilon moves
            for(k2 = 0; k2 < o2.length() && !ids2[k2]; k2++) {
                ids.push(0);
                targets.push(combine(n1, t2[k2]));
                outputs.push(o2[k2]);
                costs.push(c2[k2]);
            }
            // non-epsilon moves
            while(k1 < o1.length() && k2 < ids2.length()) {
                while(k1 < o1.length() && o1[k1] < ids2[k2]) k1++;
                if(k1 >= o1.length()) break;
                while(k2 < ids2.length() && o1[k1] > ids2[k2]) k2++;
                while(k1 < o1.length() && k2 < ids2.length() && o1[k1] == ids2[k2]){
                    for(int j = k2; j < ids2.length() && o1[k1] == ids2[j]; j++) {
                        ids.push(ids1[k1]);
                        targets.push(combine(t1[k1], t2[j]));
                        outputs.push(o2[j]);
                        costs.push(c1[k1] + c2[j]);
                    }
                    k1++;
                }
            }
        }

        virtual void bestpath(nustring &s) {
            throw "NIY";
        }
    };
}

namespace ocropus {
    CompositionFst *make_CompositionFst(OcroFST *l1,
                                        OcroFST *l2,
                                        int override_start,
                                        int override_finish) {
        return new CompositionFstImpl(l1, l2,
                                      override_start, override_finish);
    }

    void rescore_path(IGenericFst &fst,
                      colib::intarray &inputs,
                      colib::intarray &vertices,
                      colib::intarray &outputs,
                      colib::floatarray &new_costs,
                      int override_start) {
        CHECK_ARG(vertices.length() == inputs.length());
        CHECK_ARG(vertices.length() == outputs.length());
        CHECK_ARG(vertices.length() == new_costs.length());
        CHECK_ARG(vertices.length() > 0);
        //rescore_path_log("inputs", inputs);
        //rescore_path_log("vertices", vertices);
        //rescore_path_log("outputs", outputs);
        //rescore_path_log("new_costs", new_costs);
        //rescore_path_log("override_start", override_start);
        int start;
        if(override_start != -1)
            start = override_start;
        else
            start = fst.getStart();
        fst.rescore(start, vertices[0], outputs[0], new_costs[0], inputs[0]);
        for(int i = 0; i < vertices.length() - 1; i++) {
            fst.rescore(vertices[i], vertices[i + 1],
                        outputs[i], new_costs[i], inputs[i]);
        }
    }

    void fst_copy(IGenericFst &dst, IGenericFst &src) {
        dst.clear();
        int n = src.nStates();
        for(int i = 0; i < n; i++)
            dst.newState();
        dst.setStart(src.getStart());
        for(int i = 0; i < n; i++) {
            dst.setAccept(i, src.getAcceptCost(i));
            intarray targets, outputs, inputs;
            floatarray costs;
            src.arcs(inputs, targets, outputs, costs, i);
            ASSERT(inputs.length() == targets.length());
            ASSERT(inputs.length() == outputs.length());
            ASSERT(inputs.length() == costs.length());
            for(int j = 0; j < inputs.length(); j++)
                dst.addTransition(i, targets[j], outputs[j], costs[j], inputs[j]);
        }
    }

    void fst_copy_reverse(IGenericFst &dst, IGenericFst &src, bool no_accept) {
        dst.clear();
        int n = src.nStates();
        for(int i = 0; i <= n; i++)
            dst.newState();
        if(!no_accept)
            dst.setAccept(src.getStart());
        dst.setStart(n);
        for(int i = 0; i < n; i++) {
            dst.addTransition(n, i, 0, src.getAcceptCost(i), 0);
            intarray targets, outputs, inputs;
            floatarray costs;
            src.arcs(inputs, targets, outputs, costs, i);
            ASSERT(inputs.length() == targets.length());
            ASSERT(inputs.length() == outputs.length());
            ASSERT(inputs.length() == costs.length());
            for(int j = 0; j < inputs.length(); j++)
                dst.addTransition(targets[j], i, outputs[j], costs[j], inputs[j]);
        }
    }

    void fst_copy_best_arcs_only(IGenericFst &dst, IGenericFst &src) {
        dst.clear();
        int n = src.nStates();
        for(int i = 0; i < n; i++)
            dst.newState();
        dst.setStart(src.getStart());
        for(int i = 0; i < n; i++) {
            dst.setAccept(i, src.getAcceptCost(i));
            intarray targets, outputs, inputs;
            floatarray costs;
            src.arcs(inputs, targets, outputs, costs, i);
            int n = inputs.length();
            ASSERT(n == targets.length());
            ASSERT(n == outputs.length());
            ASSERT(n == costs.length());
            inthash< Integer<-1> > hash;
            for(int j = 0; j < n; j++) {
                int t = targets[j];
                int best_so_far = hash(t);
                if(best_so_far == -1 || costs[j] < costs[best_so_far])
                    hash(t) = j;
            }
            intarray keys;
            hash.keys(keys);
            for(int k = 0; k < keys.length(); k++) {
                int j = hash(keys[k]);
                dst.addTransition(i, targets[j], outputs[j], costs[j], inputs[j]);
            }
        }
    }

    void fst_expand_composition(IGenericFst &out,
                                OcroFST &f1, OcroFST &f2) {
        autodel<CompositionFst> composition(make_CompositionFst(&f1, &f2));
        try {
            fst_copy(out, *composition);
        } catch(...) {
            composition->move1();
            composition->move2();
            throw;
        }
        composition->move1();
        composition->move2();
    }
};
