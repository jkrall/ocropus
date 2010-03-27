/*
  $Id: extern.h,v 1.1.1.1 1999/10/15 12:40:27 kise Exp $
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
*/

#include<stdio.h>
#include "defs.h"

namespace voronoi{
    extern float	xmin, xmax, ymin, ymax, deltax, deltay;

    extern struct	Site		*sites;
    extern int			nsites;
    extern int			siteidx;
    extern int			sqrt_nsites;
    extern int			nvertices;
    extern struct 	Freelist 	sfl;
    extern struct	Site		*bottomsite;

    extern int 			nedges;
    extern struct	Freelist 	efl;

    extern struct   Freelist	hfl;
    extern struct	Halfedge	*ELleftend, *ELrightend;
    extern int 			ELhashsize;
    extern struct	Halfedge	**ELhash;

    extern int 			PQhashsize;
    extern struct	Halfedge 	*PQhash;
    extern int 			PQcount;
    extern int 			PQmin;

    /* extern Component                *component; */
    extern BlackPixel               *bpx;
    extern Neighborhood	        *neighbor;
    extern LineSegment	        *lineseg;
    extern EndPoint		        *endp;
    extern HashTable	        *hashtable[M1+M2];

    extern NumPixel                 BPnbr;
    extern Label                    LABELnbr;
    extern unsigned int	        NEIGHnbr;
    extern unsigned int	        LINEnbr;
    extern unsigned int             sample_pix;
    extern unsigned int             point_edge;
    extern unsigned int             edge_nbr;
    extern long		        SiteMax;

    extern int		        sample_rate;
    extern int		        noise_max;
    extern float                    freq_rate;
    extern int                      smwind;
    extern int                      Ta;

    extern char                     output_points;
    extern char                     output_pvor;
    extern char                     output_avor;
    extern char                     display_parameters;
    
    // Modification by Faisal Shafait
    // keep track of noise components to remove them
    // from the output image
    extern bool *noise_comp;
    extern unsigned int nconcomp_inc;
    extern unsigned int nconcomp_size;
    // End of Modification

#ifdef TIME
    extern float    b_s_time;
    extern float    r_time;
    extern float    v_time;
    extern float    e_time;
    extern float    o_time;
#endif /* TIME */
}
