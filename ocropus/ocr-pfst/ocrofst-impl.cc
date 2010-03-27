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
// Project:
// File: ocrofst-impl.h
// Purpose: OcroFST class implementation
// Responsible: mezhirov
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org

#include "ocr-pfst.h"
#include "fst-io.h"
#include "a-star.h"

namespace ocropus {

    struct OcroFSTImpl : OcroFST {
        objlist<intarray> m_targets;
        objlist<intarray> m_inputs;
        objlist<intarray> m_outputs;
        objlist<floatarray> m_costs;
        floatarray m_heuristics;
        floatarray accept_costs;
        int start;

        virtual intarray &targets(int vertex) {
            return m_targets[vertex];
        }
        virtual intarray &inputs(int vertex) {
            return m_inputs[vertex];
        }
        virtual intarray &outputs(int vertex) {
            return m_outputs[vertex];
        }
        virtual floatarray &costs(int vertex) {
            return m_costs[vertex];
        }
        
        virtual float acceptCost(int vertex) {
            return accept_costs[vertex];
        }
        
        virtual void setAcceptCost(int vertex, float new_value) {
            accept_costs[vertex] = new_value;
        }


        virtual const char *description() {
            return "Lattice";
        }

        // reading
        virtual int nStates() {
            return accept_costs.length();
        }
        virtual int getStart() {
            return start;
        }
        virtual float getAcceptCost(int node) {
            return accept_costs[node];
        }

        virtual void arcs(intarray &out_inputs,
                          intarray &out_targets,
                          intarray &out_outputs,
                          floatarray &out_costs,
                          int from) {
            copy(out_inputs, m_inputs[from]);
            copy(out_targets, m_targets[from]);
            copy(out_outputs, m_outputs[from]);
            copy(out_costs, m_costs[from]);
        }

        virtual void clear() {
            start = 0;
            m_targets.clear();
            m_inputs.clear();
            m_outputs.clear();
            m_costs.clear();
            accept_costs.clear();
        }

        // writing
        virtual int newState() {
            accept_costs.push() = 1e38;
            m_targets.push();
            m_inputs.push();
            m_outputs.push();
            m_costs.push();
            return accept_costs.length() - 1;
        }
        virtual void addTransition(int from,int to,int output,float cost,int input) {
            m_targets[from].push(to);
            m_outputs[from].push(output);
            m_inputs[from].push(input);
            m_costs[from].push(cost);
        }
        
        virtual void rescore(int from,int to,int output,float cost,int input) {
            intarray &t = m_targets[from];
            intarray &i = m_inputs[from];
            intarray &o = m_outputs[from];
            for(int j = 0; j < t.length(); j++) {
                if(t[j] == to
                && i[j] == input
                && o[j] == output) {
                    m_costs[from][j] = cost;
                    break;
                }
            }
        }

        virtual void setStart(int node) {
            start = node;
        }
        virtual void setAccept(int node,float cost=0.0) {
            accept_costs[node] = cost;
        }
        virtual int special(const char *s) {
            return 0;
        }
        virtual void bestpath(colib::nustring &result) {
            a_star(result, *this);
        }
        virtual void save(const char *path) {
            fst_write(path, *this);
        }
        virtual void load(const char *path) {
            fst_read(*this, path);
        }

    private:
        int flags;
        void achieve(int flag) {
            CHECK_ARG(flag == SORTED_BY_INPUT
                   || flag == SORTED_BY_OUTPUT
                   || flag == HAS_HEURISTICS);

            if(flags & flag)
                return;

            if(flag == HAS_HEURISTICS) {
                a_star_backwards(m_heuristics, *this);
                return;
            }

            for(int node = 0; node < nStates(); node++) {
                intarray permutation;
                if(flag == OcroFST::SORTED_BY_INPUT)
                    quicksort(permutation, m_inputs[node]);
                else
                    quicksort(permutation, m_outputs[node]);
                permute(m_inputs[node], permutation);
                permute(m_outputs[node], permutation);
                permute(m_targets[node], permutation);
                permute(m_costs[node], permutation);
            }
            flags |= flag;
        }
    public:
        virtual void sortByInput() {
            achieve(SORTED_BY_INPUT);
        }

        virtual void sortByOutput() {
            achieve(SORTED_BY_OUTPUT);
        }

        virtual bool hasFlag(int flag) {
            return flags & flag;
        }
        

        virtual floatarray &heuristics() {
            return m_heuristics;
        }
        
        virtual void calculateHeuristics() {
            achieve(HAS_HEURISTICS);
        }

        OcroFSTImpl(int max_size=0) : 
            m_targets(max_size),
            m_inputs(max_size),
            m_outputs(max_size),
            m_costs(max_size),
            accept_costs(max_size),
            start(0), flags(0) {
        }
        
        virtual void clearFlags() {
            flags = 0;
        }

    };

    OcroFST *make_OcroFST() {
        return new OcroFSTImpl();
    };

}
