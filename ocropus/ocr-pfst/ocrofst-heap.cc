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
// File: fst-heap.cc
// Purpose: heaps for searches
// Responsible: mezhirov
// Reviewer: 
// Primary Repository: 
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org


#include "fst-heap.h"

namespace {
    int left(int i) {  return 2 * i + 1; }
    int right(int i) {  return 2 * i + 2; }
    int parent(int i) {  return (i - 1) / 2; }
}

namespace ocropus {
    void PriorityQueue::clear() {
        fill = 0;
    }

    /*void PriorityQueue::log(Logger &logger) {
        logger("fill", fill);
        logger("ids", ids);
        logger("values", values);
    }*/

    void PriorityQueue::move_value(int id, int tag, float value, int start, int end) {
        int i = start;
        while(i>end) {
            if(values[i-1]>=value) break;
            values[i] = values[i-1];
            tags[i] = tags[i-1];
            ids[i] = ids[i-1];
            i--;
        }
        values[i] = value;
        tags[i] = tag;
        ids[i] = id;
    }



    bool PriorityQueue::add(int id, int tag, float value) {
        if(fill == n) {
            if(values[n-1]>=value) return false;
            move_value(id, tag, value, n-1, 0);
        } else if(fill == 0) {
            values[0] = value;
            ids[0] = id;
            tags[0] = tag;
            fill++;
        } else {
            move_value(id, tag, value, fill, 0);
            fill++;
        }
        return true;
    }

    int PriorityQueue::find_id(int id) {
        for(int i = 0; i < fill; i++) {
            if(ids[i] == id)
                return i;
        }
        return -1;
    }


    bool PriorityQueue::add_replacing_id(int id, int tag, float value) {
        int former = find_id(id);
        if(former == -1)
            return add(id, tag, value);
        if(values[former]>=value)
            return false;
        move_value(id, tag, value, former, 0);
        return true;
    }

    int Heap::rotate(int i) {
        int size = heap.length();
        int j = left(i);
        int k = right(i);
        if(k < size && costs[k] < costs[j]) {
            if(costs[k] < costs[i]) {
                heapswap(k, i);
                return k;
            }
        } else if(j < size) {
            if(costs[j] < costs[i]) {
                heapswap(j, i);
                return j;
            }
        }
        return i;
    }

    void Heap::heapswap(int i, int j) {
        int t = heap[i];
        heap[i] = heap[j];
        heap[j] = t;

        float c = costs[i];
        costs[i] = costs[j];
        costs[j] = c;

        heapback[heap[i]] = i;
        heapback[heap[j]] = j;
    }

    void Heap::heapify_down(int i) {
        while(1) {
            int k = rotate(i);
            if(i == k)
                return;
            i = k;
        }
    }

    void Heap::heapify_up(int i) {
        while(i) {
            int j = parent(i);
            if(costs(i) < costs(j)) {
                heapswap(i, j);
                i = j;
            } else {
                return;
            }
        }
    }

    int Heap::pop() {
        heapswap(0, heap.length() - 1);
        int result = heap.pop();
        costs.pop();
        heapify_down(0);
        heapback[result] = -1;
        return result;
    }

    bool Heap::push(int node, float cost) {
        int i = heapback[node];
        if(i != -1) {
            if(cost < costs[i]) {
                costs[i] = cost;
                heapify_up(i);
                return true;
            }
            return false;
        } else {
            heap.push(node);
            costs.push(cost);
            heapback[node] = heap.length() - 1;
            heapify_up(heap.length() - 1);
            return true;
        }
    }

    bool Heap::remove(int node) {
        int i = heapback[node];
        if(i == -1)
            return false;

        heapswap(i, heap.length() - 1);
        heap.pop();
        costs.pop();
        heapify_down(i);
        heapback[node] = -1;
        return true;
    }
}
