// -*- C++ -*-

// Copyright 2006 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
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
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites:


#ifndef h_queue_
#define h_queue_

#include "ocropus.h"

namespace ocropus {

template <class T>
struct Queue {
    int qmax,qfront,qback;
    T *data;
    Queue() {
        data = 0;
    }
    Queue(int n) {
        data = 0;
        resize(n);
    }
    ~Queue() {
        delete [] data;
        qmax = qfront = qback = 0;
    }
    void resize(int n) {
        if(data) delete [] data;
        qmax = n;
        data = new T[n];
        qfront = 0;
        qback = 0;
    }
    bool empty() {
        return qfront==qback;
    }
    void enqueue(T value) {
        data[qfront] = value;
        qfront++;
        if(qfront>=qmax) qfront = 0;
        ASSERT(qfront!=qback);
    }
    T dequeue() {
        ASSERT(qfront!=qback);
        T result = data[qback];
        qback++;
        if(qback>=qmax) qback = 0;
        return result;
    }
private:
    Queue(const Queue<T> &);
    Queue(Queue<T> &);
    void operator=(const Queue<T> &);
    void operator=(Queue<T> &);
};

}

#endif
