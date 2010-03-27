/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
  voronoi.c
*/

#include <stdio.h>
#include "const.h"
#include "defs.h"
#include "extern.h"
#include "function.h"

/* implicit parameters: nsites, sqrt_nsites, xmin, xmax, ymin, ymax,
   deltax, deltay (can all be estimates).
   Performance suffers if they are wrong; better to make nsites,
   deltax, and deltay too big than too small.  (?) */

namespace voronoi{
    void voronoi( Coordinate imax, Coordinate jmax)
    {
        struct Site *newsite, *bot, *top, *temp, *p;
        struct Site *v = 0;
        struct Point newintstar = {0.0,0.0};
        int pm;
        struct Halfedge *lbnd, *rbnd, *llbnd, *rrbnd, *bisector;
        struct Edge *e;

        freeinit(&sfl, sizeof *sites);
        siteidx = 0;
        geominit();
        PQinitialize();
        bottomsite = (*nextsite)();
        ELinitialize();

        newsite = (*nextsite)();

        while(1) {
            if(!PQempty()) newintstar = PQ_min();

            if (newsite != (struct Site *)NULL
                && (PQempty()
                    || newsite -> coord.y < newintstar.y
                    || (newsite->coord.y == newintstar.y
                        && newsite->coord.x < newintstar.x))) {
                /* new site is smallest */
                lbnd = ELleftbnd(&(newsite->coord));
                rbnd = ELright(lbnd);
                bot = rightreg(lbnd);
                e = bisect(bot, newsite);
                bisector = HEcreate(e, LE);
                ELinsert(lbnd, bisector);
                if ((p = intersect(lbnd, bisector)) != (struct Site *) NULL) {
                    PQdelete(lbnd);
                    PQinsert(lbnd, p, dist(p,newsite));
                }
                lbnd = bisector;
                bisector = HEcreate(e, RE);
                ELinsert(lbnd, bisector);
                if ((p = intersect(bisector, rbnd)) != (struct Site *) NULL) {
                    PQinsert(bisector, p, dist(p,newsite));
                }
                newsite = (*nextsite)();
            }
            else if (!PQempty()) {
                /* intersection is smallest */
                lbnd = PQextractmin();
                llbnd = ELleft(lbnd);
                rbnd = ELright(lbnd);
                rrbnd = ELright(rbnd);
                bot = leftreg(lbnd);
                top = rightreg(rbnd);

                v = lbnd->vertex;
                makevertex(v);
                endpoint(lbnd->ELedge,lbnd->ELpm,v,imax,jmax);
                endpoint(rbnd->ELedge,rbnd->ELpm,v,imax,jmax);
                ELdelete(lbnd);
                PQdelete(rbnd);
                ELdelete(rbnd);
                pm = LE;
                if (bot->coord.y > top->coord.y) {
                    temp = bot;
                    bot = top;
                    top = temp;
                    pm = RE;
                }
                e = bisect(bot, top);
                bisector = HEcreate(e, pm);
                ELinsert(llbnd, bisector);
                endpoint(e, RE-pm, v, imax, jmax);
                deref(v);
                if((p = intersect(llbnd, bisector)) != (struct Site *) NULL){
                    PQdelete(llbnd);
                    PQinsert(llbnd, p, dist(p,bot));
                }
                if ((p = intersect(bisector, rrbnd)) != (struct Site *) NULL){
                    PQinsert(bisector, p, dist(p,bot));
                }
            }
            else break;
        }

        for(lbnd=ELright(ELleftend); lbnd != ELrightend; lbnd=ELright(lbnd)) {
            e = lbnd -> ELedge;
            out_ep2(e,v,imax,jmax); /* Voronoi  ’§Ú¿∏¿Æ */
        }
    }
}
