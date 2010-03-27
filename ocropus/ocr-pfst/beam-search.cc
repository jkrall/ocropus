// Copyright 2008-2009 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Project: ocrofst
// File: beam-search.cc
// Purpose: beam search
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org

#include "ocr-pfst.h"
#include "fst-heap.h"

using namespace colib;
using namespace ocropus;

namespace {

    /// A SearchTree contains all vertices that were ever touched during the
    /// search, and can produce a prehistory for every ID.
    struct SearchTree {
        intarray parents;
        intarray inputs;
        intarray outputs;
        intarray v1; // vertices from FST 1
        intarray v2; // vertices from FST 2
        floatarray costs;

        void clear() {
            parents.clear();
            inputs.clear();
            outputs.clear();
            v1.clear();
            v2.clear();
            costs.clear();
        }
        
        void get(intarray &r_vertices1,
                 intarray &r_vertices2,
                 intarray &r_inputs,
                 intarray &r_outputs,
                 floatarray &r_costs,
                 int id) {
            intarray t_v1; // vertices
            intarray t_v2; // vertices
            intarray t_i; // inputs
            intarray t_o; // outputs
            floatarray t_c; // costs
            int current = id;
            while(current != -1) {
                t_v1.push(v1[current]);
                t_v2.push(v2[current]);
                t_i.push(inputs[current]);
                t_o.push(outputs[current]);
                t_c.push(costs[current]);
                current = parents[current];
            }
            
            reverse(r_vertices1, t_v1);
            reverse(r_vertices2, t_v2);
            reverse(r_inputs, t_i);
            reverse(r_outputs, t_o);
            reverse(r_costs, t_c);
        }

        int add(int parent, int vertex1, int vertex2,
                   int input, int output, float cost) {
            int n = parents.length();
            //logger.format("stree: [%d]: parent %d, v1 %d, v2 %d, cost %f",
            //               n, parent, vertex1, vertex2, cost);
            parents.push(parent);
            v1.push(vertex1);
            v2.push(vertex2);
            inputs.push(input);
            outputs.push(output);
            costs.push(cost);
            return n;
        }
    };
    
    struct BeamSearch {
        OcroFST &fst1;
        OcroFST &fst2;
        SearchTree stree;

        intarray beam; // indices into stree
        floatarray beamcost; // global cost, corresponds to the beam

        PriorityQueue nbest;
        intarray all_inputs;
        intarray all_targets1;
        intarray all_targets2;
        intarray all_outputs;
        floatarray all_costs;
        intarray parent_trails; // indices into the beam
        int beam_width;
        int accepted_from1;
        int accepted_from2;
        float g_accept;   // best cost for accept so far
        int best_so_far;  // ID into stree (-1 for start)
        float best_cost_so_far;

        BeamSearch(OcroFST &fst1, OcroFST &fst2, int beam_width): 
                fst1(fst1),
                fst2(fst2),
                nbest(beam_width),
                beam_width(beam_width),
                accepted_from1(-1),
                accepted_from2(-1) {
        }

        void clear() {
            nbest.clear();
            all_targets1.clear();
            all_targets2.clear();
            all_inputs.clear();
            all_outputs.clear();
            all_costs.clear();
            parent_trails.clear();
        }

        void relax(int f1, int f2, int t1, int t2, double cost,
                   int arc_id1, int arc_id2,
                   int input, int intermediate, int output,
                   double base_cost, int trail_index) {
            //logger.format("relaxing %d %d -> %d %d (bcost %f, cost %f)", f1, f2, t1, t2, base_cost, cost);
            
            if(!nbest.add_replacing_id(t1 * fst2.nStates() + t2,
                                       all_costs.length(),
                                       - base_cost - cost))
                return;
            
            //logger.format("nbest changed");
            //nbest.log(logger);

            if(input) {
                // The candidate for the next beam is stored in all_XX arrays.
                // (can we store it in the stree instead?)
                all_inputs.push(input);
                all_targets1.push(t1);
                all_targets2.push(t2);
                all_outputs.push(output);
                all_costs.push(cost);
                parent_trails.push(trail_index);
            } else {
                // Beam control hack
                // -----------------
                // if a node is important (changes nbest) AND its input is 0,
                // then it's added to the CURRENT beam.
                
                //logger.format("pushing control point from trail %d to %d, %d",
                              //trail_index, t1, t2);
                int new_node = stree.add(beam[trail_index], t1, t2, input, output, cost);
                beam.push(new_node);
                beamcost.push(base_cost + cost);

                // This is a stub entry indicating that the node should not
                // be added to the next generation beam.
                all_inputs.push(0);
                all_targets1.push(-1);
                all_targets2.push(-1);
                all_outputs.push(0);
                all_costs.push(0);
                parent_trails.push(-1);
            }
        }

        /// Call relax() for each arc going out of the given node.
        void traverse(int n1, int n2, double cost, int trail_index) {
            //logger.format("traversing %d %d", n1, n2);
            intarray &o1 = fst1.outputs(n1);
            intarray &i1 = fst1.inputs(n1);
            intarray &t1 = fst1.targets(n1);
            floatarray &c1 = fst1.costs(n1);

            intarray &o2 = fst2.outputs(n2);
            intarray &i2 = fst2.inputs(n2);
            intarray &t2 = fst2.targets(n2);
            floatarray &c2 = fst2.costs(n2);

            // for optimization
            int *O1 = o1.data;
            int *O2 = o2.data;
            int *I1 = i1.data;
            int *I2 = i2.data;
            int *T1 = t1.data;
            int *T2 = t2.data;
            float *C1 = c1.data;
            float *C2 = c2.data;

            // Relax outbound arcs in the composition
            int k1, k2;
            // relaxing fst1 epsilon moves
            for(k1 = 0; k1 < o1.length() && !O1[k1]; k1++) {
                relax(n1, n2, T1[k1], n2, C1[k1], k1, -1, 
                      I1[k1], 0, 0, cost, trail_index);
            }
            // relaxing fst2 epsilon moves
            for(k2 = 0; k2 < o2.length() && !I2[k2]; k2++) {
                relax(n1, n2, n1, T2[k2], C2[k2], -1, k2, 0,
                      0, O2[k2], cost, trail_index);
            }
            
            // relaxing non-epsilon moves
            while(k1 < o1.length() && k2 < i2.length()) {
                while(k1 < o1.length() && O1[k1] < I2[k2]) k1++;
                if(k1 >= o1.length()) break;
                while(k2 < i2.length() && O1[k1] > I2[k2]) k2++;
                while(k1 < o1.length() && k2 < i2.length() && O1[k1] == I2[k2]){
                    for(int j = k2; j < i2.length() && O1[k1] == I2[j]; j++)
                        relax(n1, n2, T1[k1], T2[j], C1[k1] + C2[j],
                              k1, j, I1[k1], O1[k1], O2[j], cost, trail_index);
                    k1++;
                }
            }
        }

        // The main loop iteration.
        void radiate() {
            clear();
            
            //logger("beam", beam);
            //logger("beamcost", beamcost);

            int control_beam_start = beam.length();
            for(int i = 0; i < control_beam_start; i++)
                try_accept(i);

            // in this loop, traversal may add "control nodes" to the beam
            for(int i = 0; i < beam.length(); i++) {
                traverse(stree.v1[beam[i]], stree.v2[beam[i]],
                         beamcost[i], i);
            }

            // try accepts from control beam nodes 
            // (they're not going to the next beam)
            for(int i = control_beam_start; i < beam.length(); i++)
                try_accept(i);


            intarray new_beam;
            floatarray new_beamcost;
            for(int i = 0; i < nbest.length(); i++) {
                int k = nbest.tag(i);
                if(parent_trails[k] < 0) // skip the control beam nodes
                    continue;
                new_beam.push(stree.add(beam[parent_trails[k]],
                                        all_targets1[k], all_targets2[k],
                                        all_inputs[k], all_outputs[k],
                                        all_costs[k]));
                new_beamcost.push(beamcost[parent_trails[k]] + all_costs[k]);
                //logger.format("to new beam: trail index %d, stree %d, target %d,%d",
                        //k, new_beam[new_beam.length() - 1], all_targets1[k], all_targets2[k]);
            }
            move(beam, new_beam);
            move(beamcost, new_beamcost);
        }

        // Relax the accept arc from the beam node number i.
        void try_accept(int i) {
            float a_cost1 = fst1.getAcceptCost(stree.v1[beam[i]]);
            float a_cost2 = fst2.getAcceptCost(stree.v2[beam[i]]);
            float candidate = beamcost[i] + a_cost1 + a_cost2;
            if(candidate < best_cost_so_far) {
                //logger.format("accept from beam #%d (stree %d), cost %f",
                //              i, beam[i], candidate);
                best_so_far = beam[i];
                best_cost_so_far = candidate;
            }
        }

        void bestpath(intarray &v1, intarray &v2, intarray &inputs, 
                      intarray &outputs, floatarray &costs) {
            stree.clear();

            beam.resize(1);
            beamcost.resize(1);
            beam[0] = stree.add(-1, fst1.getStart(), fst2.getStart(), 0, 0, 0);
            beamcost[0] = 0;

            best_so_far = 0;
            best_cost_so_far = fst1.getAcceptCost(fst1.getStart()) + 
                               fst2.getAcceptCost(fst1.getStart());

            while(beam.length())
                radiate();

            stree.get(v1, v2, inputs, outputs, costs, best_so_far);
            costs.push(fst1.getAcceptCost(stree.v1[best_so_far]) + 
                       fst2.getAcceptCost(stree.v2[best_so_far]));

            //logger("costs", costs);
        }
    };

};

namespace ocropus {
    void beam_search(intarray &vertices1,
                     intarray &vertices2,
                     intarray &inputs,
                     intarray &outputs,
                     floatarray &costs,
                     OcroFST &fst1, 
                     OcroFST &fst2,
                     int beam_width) {
        BeamSearch b(fst1, fst2, beam_width);
        fst1.sortByOutput();
        fst2.sortByInput();
        b.bestpath(vertices1, vertices2, inputs, outputs, costs);
    }

    double beam_search(nustring &result, OcroFST &fst1, OcroFST &fst2,
                       int beam_width) {
        intarray v1;
        intarray v2;
        intarray i;
        intarray o;
        floatarray c;
        beam_search(v1, v2, i, o, c, fst1, fst2, beam_width);
        remove_epsilons(result, o);
        return sum(c);
    }
}
