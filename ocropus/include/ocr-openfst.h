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
// Project: ocr-openfst
// File: ocr-openfst.h
// Purpose: public API for our bindings to OpenFST
// Responsible:
// Reviewer:
// Primary Repository:
// Web Sites: www.iupr.org, www.dfki.de, www.ocropus.org

#ifndef h_ocr_openfst_
#define h_ocr_openfst_

#include "colib/colib.h"
#include "ocrinterfaces.h"

using namespace ocropus;

// This is a forward declaration for StdVectorFst.
namespace fst {
    class StdArc;
    template<class T> class Fst;
    template<class T> class VectorFst;
    typedef VectorFst<StdArc> StdVectorFst;
}

namespace ocropus {
    using namespace colib;

    // FIXME why are these still here?
    fst::StdVectorFst *fst_ignoring(intarray &a,int maxsymbol=128,int minsymbol=1);
    fst::StdVectorFst *fst_keeping(intarray &a,int maxsymbol=128,int minsymbol=1);
    fst::StdVectorFst *fst_edit_distance(float subst,float ins,float del,int maxymbol=128,int minsymbol=1);
    fst::StdVectorFst *fst_limited_edit_distance(int maxins,float ins,int maxdel,float del,int maxsymbol=128,int minsymbol=1);
    fst::StdVectorFst *fst_insdel(float ins,float del,int maxymbol=128,int minsymbol=1);
    fst::StdVectorFst *fst_size_range(int minsize,int maxsize,int maxsymbol=128,int minsymbol=1);

    // FIXME make these components
    struct UnigramModel {
        virtual void clear() = 0;
        virtual void addSymbol(int input,int output,float cost=0.0) = 0;
        virtual fst::StdVectorFst *take() = 0;
        virtual ~UnigramModel() {}
    };
    UnigramModel *make_UnigramModel();

    struct DictionaryModel {
        virtual void clear() = 0;
        virtual void addWord(intarray &w,float cost=0.0) = 0;
        virtual void addWordSymbol(intarray &w,int output,float cost=0.0) = 0;
        virtual void addWordTranscription(intarray &input,colib::intarray &output,float cost=0.0) = 0;
        virtual void addWord(const char *w,float cost=0.0) = 0;
        virtual void addWordSymbol(const char *w,int output,float cost=0.0) = 0;
        virtual void addWordTranscription(const char *input,const char *output,float cost=0.0) = 0;
        virtual void minimize() = 0;
        virtual fst::StdVectorFst *take() = 0;
        virtual ~DictionaryModel() {}
    };
    DictionaryModel *make_DictionaryModel();

    struct NgramModel {
        // ngrams are in reading order, with the last element conditioned on the previous ones
        virtual void addNgram(intarray &ngram,float cost) = 0;
        virtual void addNgram(const char *ngram,float cost) = 0;
        virtual fst::StdVectorFst *take() = 0;
        virtual ~NgramModel() {}
    };
    NgramModel *make_NgramModel();

    void fst_prune_arcs(fst::StdVectorFst &result,fst::StdVectorFst &fst,int maxarcs,float maxratio,bool keep_eps);


    double bestpath(colib::nustring &result, colib::floatarray &costs, colib::intarray &ids,fst::Fst<fst::StdArc> &fst,bool copy_eps=false);
    double bestpath(colib::nustring &result,fst::Fst<fst::StdArc> &fst,bool copy_eps=false);
    double bestpath2(colib::nustring &result, colib::floatarray &costs, colib::intarray &ids,fst::StdVectorFst &fst,fst::StdVectorFst &fst2,bool copy_eps=false);
    double bestpath2(colib::nustring &result,fst::StdVectorFst &fst,fst::StdVectorFst &fst2,bool copy_eps=false);

    fst::StdVectorFst *as_fst(const char *s,float cost=0.0,float skip_cost=9999,float junk_cost=9999);
    fst::StdVectorFst *as_fst(colib::intarray &a,float cost=0.0,float skip_cost=9999,float junk_cost=9999);
    fst::StdVectorFst *as_fst(colib::nustring &s,float cost=0.0,float skip_cost=9999,float junk_cost=9999);

    double score(fst::StdVectorFst &fst,colib::intarray &in);
    double score(fst::StdVectorFst &fst,const char *in);
    double score(colib::intarray &out,fst::StdVectorFst &fst,colib::intarray &in);
    double score(const char *out,fst::StdVectorFst &fst,const char *in);
    double translate(colib::intarray &out,fst::StdVectorFst &fst,colib::intarray &in);
    const char *translate(fst::StdVectorFst &fst,const char *in);
    double reverse_translate(colib::intarray &out,fst::StdVectorFst &fst,colib::intarray &in);
    const char *reverse_translate(fst::StdVectorFst &fst,const char *in);
    double sample(colib::intarray &out,fst::StdVectorFst &fst);

    double score(fst::StdVectorFst &fst,const char *s);
    const char *translate(fst::StdVectorFst &fst,colib::intarray &in);
    const char *reverse_translate(fst::StdVectorFst &fst,colib::intarray &in);

    fst::StdVectorFst *compose(fst::StdVectorFst &a,fst::StdVectorFst &b);
    fst::StdVectorFst *compose(fst::StdVectorFst &a,fst::StdVectorFst &b,bool rmeps,bool det,bool min);
    fst::StdVectorFst *determinize(fst::StdVectorFst &a);
    fst::StdVectorFst *difference(fst::StdVectorFst &a,fst::StdVectorFst &b);
    fst::StdVectorFst *intersect(fst::StdVectorFst &a,fst::StdVectorFst &b);
    fst::StdVectorFst *reverse(fst::StdVectorFst &a);

    void fst_add_ascii_symbols(fst::StdVectorFst &a,bool input,bool output);
    void fst_add_to_each_transition(fst::StdVectorFst &fst,int ilabel,int olabel,float cost,bool eps_too);

    struct FstBuilder : IGenericFst {
        virtual fst::StdVectorFst *take() = 0;
    };
    FstBuilder *make_FstBuilder(fst::StdVectorFst *fst = NULL);
}


#endif
