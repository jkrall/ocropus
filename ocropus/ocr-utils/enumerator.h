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
// Project: segeval - Color based evaluation of page segmentation
// File: enumerator.h
// Purpose: Hash table based enumerator data structure
// Responsible: Faisal Shafait (faisal.shafait@dfki.de)
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de


#ifndef h_enumerator__
#define h_enumerator__

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "ocropus.h"

template <class T>
inline void memswap(T &a,T &b) {
    char buf[sizeof (T)];
    memcpy(buf,(char *)&b,sizeof (T));
    memcpy((char *)&b,(char *)&a,sizeof (T));
    memcpy((char *)&a,buf,sizeof (T));
}


template <class K,class V>
struct EnumHash {
    struct KVP {
        K key;
        V value;
        KVP() {}
        KVP(K key):key(key) {}
        KVP(K key,V value):key(key),value(value) {}
    };
    int nentries;
    colib::narray<KVP*> entries;
    EnumHash(int buckets=10) {
        entries.resize(buckets);
        fill(entries,(KVP*) NULL);
        nentries = 0;
    }
    V &operator[](const K &key) {
        if(nentries>=entries.length()/2) grow();
        int index = find_index(key);
        if(!entries(index)) {
            entries(index) = new KVP(key);
            nentries++;
        }
        return entries(index)->value;
    }
    V &get(const K &key,const V &dflt) {
        if(nentries>=entries.length()/2) grow();
        int index = find_index(key);
        if(!entries(index)) {
            entries(index) = new KVP(key,dflt);
            nentries++;
        }
        return entries(index)->value;
    }
//     bool hasKey(const K &key) {
//      int index = find_index(key);

//     }
    int find_index(const K &key) {
        int index = abs(Hash_hash(key)) % entries.length();
        for(;;) {
            KVP *entry = entries(index);
            if(!entry) return index;
            if(entry->key==key) return index;
            index = (index+1)%entries.length();
        }
    }
    void grow() {
        EnumHash<K,V> temp(2*entries.length());
        for(int i=0;i<entries.length();i++) {
            KVP *p = entries(i);
            if(p) temp[p->key] = p->value;
        }
        memswap(entries,temp.entries);
        memswap(nentries,temp.nentries);
    }
    void put(KVP *kvp) {
        (*this)[kvp->key] = kvp->value;
    }
};

inline int Hash_hash(int x) {
    return x;
}

struct Enumerator {
    int n;
    EnumHash<int,int> translation;
    Enumerator(int start=0) {
        n = start;
    }
    int operator[](int i) {
        int result = translation.get(i,n);
        if(result==n) n++;
        return result;
    }
    int total() {
        return n;
    }
};

#endif
