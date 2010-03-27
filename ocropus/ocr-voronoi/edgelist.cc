/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
  edgelist.c
*/

#include <stdio.h>
#include "const.h"
#include "defs.h"
#include "extern.h"
#include "function.h"


namespace voronoi{
    int ntry, totalsearch;

    void ELinitialize()
    {
        int i;
        freeinit(&hfl, sizeof **ELhash);
        ELhashsize = 2 * sqrt_nsites;
        ELhash = (struct Halfedge **) myalloc ( sizeof *ELhash * ELhashsize);
        for(i=0; i<ELhashsize; i +=1) ELhash[i] = (struct Halfedge *)NULL;
        ELleftend = HEcreate( (struct Edge *)NULL, 0);
        ELrightend = HEcreate( (struct Edge *)NULL, 0);
        ELleftend -> ELleft = (struct Halfedge *)NULL;
        ELleftend -> ELright = ELrightend;
        ELrightend -> ELleft = ELleftend;
        ELrightend -> ELright = (struct Halfedge *)NULL;
        ELhash[0] = ELleftend;
        ELhash[ELhashsize-1] = ELrightend;
    }


    struct Halfedge *HEcreate(struct Edge *e, int pm)
    {
        struct Halfedge *answer;
        answer = (struct Halfedge *) getfree(&hfl);
        answer -> ELedge = e;
        answer -> ELpm = pm;
        answer -> PQnext = (struct Halfedge *) NULL;
        answer -> vertex = (struct Site *) NULL;
        answer -> ELrefcnt = 0;
        return(answer);
    }


    void ELinsert(struct Halfedge *lb, struct Halfedge *newedge)
    {
        newedge -> ELleft = lb;
        newedge -> ELright = lb -> ELright;
        (lb -> ELright) -> ELleft = newedge;
        lb -> ELright = newedge;
    }

    /* Get entry from hash table, pruning any deleted nodes */
    struct Halfedge *ELgethash(int b)
    {
        struct Halfedge *he;

        if(b<0 || b>=ELhashsize) return((struct Halfedge *) NULL);
        he = ELhash[b]; 
        if (he == (struct Halfedge *) NULL || 
            he -> ELedge != (struct Edge *) DELETED ) return (he);

        /* Hash table points to deleted half edge.  Patch as necessary. */
        ELhash[b] = (struct Halfedge *) NULL;
        if ((he -> ELrefcnt -= 1) == 0) makefree((struct Freenode *)he, &hfl);
        return ((struct Halfedge *) NULL);
    }	

    struct Halfedge *ELleftbnd(struct Point *p)
    {
        int i, bucket;
        struct Halfedge *he;

        /* Use hash table to get close to desired halfedge */
        bucket = (p->x - xmin)/deltax * ELhashsize;
        if(bucket<0) bucket =0;
        if(bucket>=ELhashsize) bucket = ELhashsize - 1;
        he = ELgethash(bucket);
        if(he == (struct Halfedge *) NULL)
            {   for(i=1; 1 ; i += 1)
                    {	if ((he=ELgethash(bucket-i)) != (struct Halfedge *) NULL) break;
                        if ((he=ELgethash(bucket+i)) != (struct Halfedge *) NULL) break;
                    };
                totalsearch += i;
            };
        ntry += 1;
        /* Now search linear list of halfedges for the corect one */
        if (he==ELleftend  || (he != ELrightend && right_of(he,p)))
            {do {he = he -> ELright;} while (he!=ELrightend && right_of(he,p));
                he = he -> ELleft;
            }
        else 
            do {he = he -> ELleft;} while (he!=ELleftend && !right_of(he,p));

        /* Update hash table and reference counts */
        if(bucket > 0 && bucket <ELhashsize-1)
            {	if(ELhash[bucket] != (struct Halfedge *) NULL) 
                    ELhash[bucket] -> ELrefcnt -= 1;
                ELhash[bucket] = he;
                ELhash[bucket] -> ELrefcnt += 1;
            };
        return (he);
    }

	
    /* This delete routine can't reclaim node, since pointers from hash
       table may be present.   */
    void ELdelete(struct Halfedge *he)
    {
        (he -> ELleft) -> ELright = he -> ELright;
        (he -> ELright) -> ELleft = he -> ELleft;
        he -> ELedge = (struct Edge *)DELETED;
    }


    struct Halfedge *ELright(struct Halfedge *he)
    {
        return (he -> ELright);
    }

    struct Halfedge *ELleft(struct Halfedge *he)
    {
        return (he -> ELleft);
    }


    struct Site *leftreg(struct Halfedge *he)
    {
        if(he -> ELedge == (struct Edge *)NULL) return(bottomsite);
        return( he -> ELpm == LE ? 
                he -> ELedge -> reg[LE] : he -> ELedge -> reg[RE]);
    }

    struct Site *rightreg(struct Halfedge *he)
    {
        if(he -> ELedge == (struct Edge *)NULL) return(bottomsite);
        return( he -> ELpm == LE ? 
                he -> ELedge -> reg[RE] : he -> ELedge -> reg[LE]);
    }
}
