// Copyright 2008 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// File: a-star.cc
// Purpose: A* search
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org

#include "ocr-pfst.h"
#include "fst-heap.h"
#include "lattice.h"

using namespace colib;
using namespace ocropus;

namespace {
    class AStarSearch {
        IGenericFst &fst;

        intarray came_from; // the previous node in the best path; 
                            // -1 for unseen, self for the start
        int accepted_from;
        float g_accept;     // best cost for accept so far
        int n;              // the number of nodes; also the virtual accept index
        Heap heap;

    public:
        floatarray g;       // the cost of the best path from the start to here
    public:

        virtual double heuristic(int index) {
            return 0;
        }

        AStarSearch(IGenericFst &fst): fst(fst),
                                            accepted_from(-1),
                                            heap(fst.nStates() + 1) {
            n = fst.nStates();
            came_from.resize(n);
            fill(came_from, -1);
            g.resize(n);

            // insert the start node
            int s = fst.getStart();
            g[s] = 0;
            came_from(s) = s;
            heap.push(s, heuristic(s));
        }
        virtual ~AStarSearch() {}

        bool step() {
            int node = heap.pop();
            if(node == n)
                return true;  // accept has popped up

            // get outbound arcs
            intarray inputs;
            intarray targets;
            intarray outputs;
            floatarray costs;
            fst.arcs(inputs, targets, outputs, costs, node);
            for(int i = 0; i < targets.length(); i++) {
                int t = targets[i];
                if(came_from[t] == -1 || g[node] + costs[i] < g[t]) {
                    // relax the edge
                    came_from[t] = node;
                    g[t] = g[node] + costs[i];
                    heap.push(t, g[t] + heuristic(t));
                }
            }
            if(accepted_from == -1
            || g[node] + fst.getAcceptCost(node) < g_accept) {
                // relax the accept edge
                accepted_from = node;
                g_accept = g[node] + fst.getAcceptCost(node);
                heap.push(n, g_accept);
            }
            return false;
        }

        bool loop() {
            while(heap.length()) {
                if(step())
                    return true;
            }
            return false;
        }

        bool reconstruct_vertices(intarray &result_vertices) {
            intarray vertices;
            if(accepted_from == -1)
                return false;
            vertices.push(accepted_from);
            int last = accepted_from;
            int next;
            while((next = came_from[last]) != last) {
                vertices.push(next);
                last = next;
            }
            reverse(result_vertices, vertices);
            return true;
        }

        void reconstruct_edges(intarray &inputs,
                               intarray &outputs,
                               floatarray &costs,
                               intarray &vertices) {
            int n = vertices.length();
            inputs.resize(n);
            outputs.resize(n);
            costs.resize(n);
            for(int i = 0; i < n - 1; i++) {
                int source = vertices[i];
                int target = vertices[i + 1];
                intarray out_ins;
                intarray out_targets;
                intarray out_outs;
                floatarray out_costs;
                fst.arcs(out_ins, out_targets, out_outs, out_costs, source);

                costs[i] = 1e38;

                // find the best arc
                for(int j = 0; j < out_targets.length(); j++) {
                    if(out_targets[j] != target) continue;
                    if(out_costs[j] < costs[i]) {
                        inputs[i] = out_ins[j];
                        outputs[i] = out_outs[j];
                        costs[i] = out_costs[j];
                    }
                }
            }
            inputs[n - 1] = 0;
            outputs[n - 1] = 0;
            costs[n - 1] = fst.getAcceptCost(vertices[n - 1]);
        }
    };

    struct AStarCompositionSearch : AStarSearch {
        floatarray &g1, &g2; // well, that's against our convention,
        CompositionFst &c;   // but I let it go since it's so local.

        virtual double heuristic(int index) {
            int i1, i2;
            c.splitIndex(i1, i2, index);
            return g1[i1] + g2[i2];
        }

        AStarCompositionSearch(floatarray &g1, floatarray &g2, CompositionFst &c) : AStarSearch(c), g1(g1), g2(g2), c(c) {
        }
    };

    /*struct AStarN : AStarSearch {
        narray<floatarray> &g;
        narray< autodel<CompositionFst> > &c;
        int n;

        virtual double heuristic(int index) {
            double result = 0;
            for(int i = 0; i < n - 1; i++) {
                int i1, i2;
                c[i]->splitIndex(i1, i2, index);
                result += g[i][i1];
                index = i2;
            }
            result += g[n - 1][index];
            return result;
        }

        AStarN(narray<floatarray> &g, narray< autodel<CompositionFst> > &c)
        : AStarSearch(*c[0]), g(g), c(c), n(g.length()) {
            CHECK_ARG(c.length() == n - 1);
        }
    };*/

    bool a_star2_internal(intarray &inputs,
                          intarray &vertices1,
                          intarray &vertices2,
                          intarray &outputs,
                          floatarray &costs,
                          IGenericFst &fst1,
                          IGenericFst &fst2,
                          floatarray &g1,
                          floatarray &g2,
                          CompositionFst& composition) {
        intarray vertices;
        AStarCompositionSearch a(g1, g2, composition);
        if(!a.loop())
            return false;
        if(!a.reconstruct_vertices(vertices))
            return false;
        a.reconstruct_edges(inputs, outputs, costs, vertices);
        composition.splitIndices(vertices1, vertices2, vertices);
        return true;
    }

    /*
    // Pointers - eek... I know.
    // But we don't have a non-owning wrapper yet, do we?
    // I don't like the idea of autodels around all IGenericFsts
    // just to take them back before the destruction. --IM
    bool a_star_N_internal(colib::intarray &inputs,
                           narray<intarray> &vertices,
                           colib::intarray &outputs,
                           colib::floatarray &costs,
                           narray<IGenericFst *> &fsts,
                           narray< autodel<CompositionFst> > &compositions) {
        intarray v;
        int n = fsts.length();
        narray<floatarray> g(n);
        for(int i = 0; i < n; i++)
            a_star_backwards(g[i], *fsts[i]);
        AStarN a(g, compositions);
        if(!a.loop())
            return false;
        if(!a.reconstruct_vertices(v))
            return false;
        a.reconstruct_edges(inputs, outputs, costs, v);
        intarray t;
        for(int i = 0; i < n - 1; i++) {
            compositions[i]->splitIndices(vertices[i], t, v);
            move(v, t);
        }
        move(vertices[n - 1], v);
        return true;
    }*/

}

namespace ocropus {
    bool a_star(intarray &inputs,
                intarray &vertices,
                intarray &outputs,
                floatarray &costs,
                OcroFST &fst) {
        AStarSearch a(fst);
        if(!a.loop())
            return false;
        if(!a.reconstruct_vertices(vertices))
            return false;
        a.reconstruct_edges(inputs, outputs, costs, vertices);
        return true;
    }

    double a_star(nustring &result, OcroFST &fst) {
        intarray inputs;
        intarray vertices;
        intarray outputs;
        floatarray costs;
        if(!a_star(inputs, vertices, outputs, costs, fst))
            return 1e38;
        remove_epsilons(result, outputs);
        return sum(costs);
    }
}

namespace {
    /*bool a_star_N(colib::intarray &inputs,
                       narray<intarray> &vertices,
                       colib::intarray &outputs,
                       colib::floatarray &costs,
                       narray<OcroFST *> &fsts) {
        int n = fsts.length();
        vertices.resize(n);
        if(n == 0)
            return false;
        if(n == 1)
            return a_star(inputs, vertices[0], outputs, costs, *fsts[0]);

        narray< autodel<CompositionFst> > compositions(n - 1);
        compositions[n - 2] = make_CompositionFst(fsts[n-2], fsts[n-1]);
        for(int i = n - 3; i >= 0; i--)
           compositions[i] = make_CompositionFst(fsts[i], &*compositions[i+1]);

        bool result;
        try {
            result = a_star_N_internal(inputs, vertices, outputs, costs, fsts,
                                       compositions);
        } catch(...) {
            for(int i = 0; i < n - 1; i++) {
                compositions[i]->move1();
                compositions[i]->move2();
            }
            throw;
        }
        for(int i = 0; i < n - 1; i++) {
            compositions[i]->move1();
            compositions[i]->move2();
        }
        return result;
    }*/

}

namespace ocropus {


    bool a_star_in_composition(intarray &inputs,
                               intarray &vertices1,
                               intarray &vertices2,
                               intarray &outputs,
                               floatarray &costs,
                               OcroFST &fst1,
                               OcroFST &fst2) {
        autodel<CompositionFst> composition(make_CompositionFst(&fst1, &fst2));
        bool result;
        try {
            floatarray g1, g2;
            fst1.calculateHeuristics();
            fst2.calculateHeuristics();
            result = a_star2_internal(inputs, vertices1, vertices2, outputs,
                                      costs, fst1, fst2,
                                      fst1.heuristics(),
                                      fst2.heuristics(), *composition);
        } catch(...) {
            composition->move1();
            composition->move2();
            throw;
        }
        composition->move1();
        composition->move2();
        return result;
    }

    void a_star_backwards(floatarray &costs_for_all_nodes, IGenericFst &fst) {
        autodel<IGenericFst> reverse(make_OcroFST());
        fst_copy_reverse(*reverse, fst, true); // creates an extra vertex
        AStarSearch a(*reverse);
        a.loop();
        copy(costs_for_all_nodes, a.g);
        costs_for_all_nodes.pop(); // remove the extra vertex
    }




    bool a_star_in_composition(intarray &inputs,
                               intarray &vertices1,
                               intarray &vertices2,
                               intarray &outputs,
                               floatarray &costs,
                               OcroFST &fst1,
                               floatarray &g1,
                               OcroFST &fst2,
                               floatarray &g2) {
        autodel<CompositionFst> composition(make_CompositionFst(&fst1, &fst2));
        bool result;
        try {
            result = a_star2_internal(inputs, vertices1, vertices2, outputs,
                                      costs, fst1, fst2, g1, g2, *composition);
        } catch(...) {
            composition->move1();
            composition->move2();
            throw;
        }
        composition->move1();
        composition->move2();
        return result;
    }

    double a_star(nustring &result, OcroFST &fst1, OcroFST &fst2) {
        intarray inputs;
        intarray v1;
        intarray v2;
        intarray outputs;
        floatarray costs;
        if(!a_star_in_composition(inputs, v1, v2, outputs, costs, fst1, fst2))
            return 1e38;
        remove_epsilons(result, outputs);
        return sum(costs);
    }



/*    bool a_star_in_composition(colib::intarray &inputs,
                 colib::intarray &vertices1,
                 colib::intarray &vertices2,
                 colib::intarray &vertices3,
                 colib::intarray &outputs,
                 colib::floatarray &costs,
                 colib::IGenericFst &fst1,
                 colib::IGenericFst &fst2,
                 colib::IGenericFst &fst3) {
        narray<intarray> v;
        narray<IGenericFst *> fsts(3);
        fsts[0] = &fst1;
        fsts[1] = &fst2;
        fsts[2] = &fst3;

        bool result = a_star_N(inputs, v, outputs, costs, fsts);
        move(vertices1, v[0]);
        move(vertices2, v[1]);
        move(vertices3, v[2]);
        return result;
    }*/

};
