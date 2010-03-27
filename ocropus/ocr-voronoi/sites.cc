/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
*/
#include <stdio.h>
#include "defs.h"
#include "extern.h"
#include "function.h"

namespace voronoi{
    /* return a single in-storage site */
    struct Site *nextsite()
    {
        struct Site *s;
        if(siteidx < nsites) {
            s = &sites[siteidx];
            siteidx += 1;
            return(s);
        }
        else return( (struct Site *)NULL);
    }
}
