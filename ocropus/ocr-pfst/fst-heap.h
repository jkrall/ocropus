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
// Project: ocrofst
// File: fst-heap.h
// Purpose: heaps for searches
// Responsible: mezhirov
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org

#ifndef fst_heap_h_
#define fst_heap_h_


#include "ocr-pfst.h"

namespace ocropus {    
    
    class PriorityQueue {
        int n;
        int fill;
        intarray ids;
        intarray tags;
        floatarray values;
    public:
        /// constructor for a NBest data structure of size n
        PriorityQueue(int n):n(n) {
            ids.resize(n+1);
            tags.resize(n+1);
            values.resize(n+1);
            clear();
        }

        //void log(Logger &logger);

        /// remove all elements
        void clear();

        void move_value(int id, int tag, float value, int start, int end);

        /// Add the id with the corresponding value
        /// \returns True if the queue was changed
        bool add(int id, int tag, float value);

        int find_id(int id);

        /// This function will move the existing id up
        /// instead of creating a new one.
        /// \returns True if the queue was changed
        bool add_replacing_id(int id, int tag, float value);

        /// get the value corresponding to rank i
        float value(int i) {
            if(unsigned(i)>=unsigned(fill)) throw "range error";
            return values[i];
        }
        int tag(int i) {
            if(unsigned(i)>=unsigned(fill)) throw "range error";
            return tags[i];
        }
        /// get the id corresponding to rank i
        int operator[](int i) {
            if(unsigned(i)>=unsigned(fill)) throw "range error";
            return ids[i];
        }
        /// get the number of elements in the NBest structure (between 0 and n)
        int length() {
            return fill;
        }
    };



    // We need heap reimplementation here since we need to change costs on the fly.
    // We maintain `heapback' array for that.
    class Heap {
        colib::intarray heap;       // the priority queue
        colib::intarray heapback;   // heap[heapback[node]] == node; -1 if not in the heap
        colib::floatarray costs; // the cost of the node on the heap


        int rotate(int i);

        // Swap 2 nodes on the heap, maintaining heapback.
        void heapswap(int i, int j);

        // Fix the heap invariant broken by increasing the cost of heap node i.
        void heapify_down(int i);

        // Fix the heap invariant broken by decreasing the cost of heap node i.
        void heapify_up(int i);

    public:
        /// Create a heap storing node indices from 0 to n - 1.
        inline Heap(int n) : heapback(n) { fill(heapback, -1); }

        inline int length() { return heap.length(); }

        /// Return the item with the least cost and remove it from the heap.
        int pop();

        inline float minCost() { return costs[0]; }

        /// Push the node in the heap if it's not already there, otherwise promote.
        /// @returns True if the heap was changed, false if the item was already
        ///          in the heap and with a better cost.
        bool push(int node, float cost);

        bool remove(int node);
    };
}

#endif
