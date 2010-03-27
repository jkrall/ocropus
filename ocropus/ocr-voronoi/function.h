/*
  $Id: function.h,v 1.1.1.1 1999/10/15 12:40:27 kise Exp $
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
*/
#ifndef FUNCTION_H_INCLUDED_
#define FUNCTION_H_INCLUDED_
#include <stdio.h>
#include <stdlib.h>
#include "read_image.h"

namespace voronoi {
    /* bit_func.c */
    int bit_get( ImageData *, Coordinate, Coordinate );
    void bit_set( ImageData *, Coordinate, Coordinate, int );
    int byte_pos ( char, int );
    void frame( ImageData *, int, int );

    /* cline.c */
    void analyze_cline( char **, int *, int * );

    /* dinfo.c */
    void dparam();
    void dnumber( int, int );
#ifdef TIME
    void dtime();
#endif

    /* edgelist.c */
    void ELinitialize();
    struct Halfedge *HEcreate( struct Edge *, int );
    void ELinsert( struct Halfedge *, struct Halfedge * );
    struct Halfedge *ELgethash( int );
    struct Halfedge *ELleftbnd( struct Point * );
    void ELdelete( struct Halfedge * );
    struct Halfedge *ELright( struct Halfedge * );
    struct Halfedge *ELleft( struct Halfedge * );
    struct Site *leftreg( struct Halfedge * );
    struct Site *rightreg( struct Halfedge * );

    /* erase.c */
    int start_pos ( int );
    int end_pos ( int );
    unsigned int Dh_ave( unsigned int *, int );
    void hist();
    int distinction( Label, Label, int );
    void erase_endp( int );
    void erase_aux();
    void erase();
    void init_u_int( unsigned int * );
    void init_int( int * );

    /* geometry.c */
    void geominit();
    struct Edge *bisect( struct Site *, struct Site * );
    struct Site *intersect( struct Halfedge *, struct Halfedge * );
    int right_of( struct Halfedge *, struct Point * );
    void endpoint( struct Edge *, int, struct Site *, Coordinate, Coordinate );
    float dist( struct Site *, struct Site * );
    void makevertex( struct Site * );
    void deref( struct Site * );
    void ref( struct Site * );

    /* hash.c */
    HashVal hash1( Key );
    HashVal hash2( Key );
    void init_hash();
    Key key( Label, Label );
    int search( Label, Label );
    void enter( Label, Label, unsigned int );

    /* heap.c */
    void PQinsert( struct Halfedge *, struct Site *, float );
    void PQdelete( struct Halfedge * );
    int PQbucket( struct Halfedge * );
    int PQempty();
    struct Point PQ_min();
    struct Halfedge *PQextractmin();
    void PQinitialize();

    /* img_to_site.c */
    void img_to_site( ImageData * );
    void bf_edgelab_smpl( ImageData *, Coordinate, Coordinate, Label);
    Vector next_point( ImageData *, Coordinate *, Coordinate *,
                       Vector *, Vector, Label );
    Vector first_d( Vector );
    Vector rot_d( Vector );
    void edge_lab( Vector, Vector, Coordinate, Coordinate, Label );
    void bpxset( Coordinate, Coordinate, Label );
    int scomp( const void *, const void * );

    /* label_func.c */
    void lab_format( ImageData * );
    Label lab_get( Coordinate, Coordinate );
    void lab_set( Coordinate, Coordinate, Label );
    void free_limg();

    /* memory.c */
    void freeinit( struct Freelist *, int );
    void freelist_destroy( struct Freelist * );
    char *getfree( struct Freelist * );
    void makefree( struct Freenode *, struct Freelist * );
    char *myalloc( unsigned );
    char *myrealloc( void *, unsigned, unsigned int, size_t);

    /* output.c */
    void in_frame( float *, float *, float, struct Edge *, int,
                   Coordinate, Coordinate );
    void s_in_frame( float *, float *, float *, float *,
                     struct Edge *, Coordinate );
    void e_in_frame( float *, float *, float *, float *,
                     struct Edge *, Coordinate, Coordinate );
    void frameout( float *, float *, float *, float *,
                   int *, int *, struct Edge *, Coordinate, Coordinate );
    void out_ep2( struct Edge *, struct Site *, Coordinate, Coordinate );

    /* read_image.c */
    void read_image( char *, ImageData * );
    int tiff2imgd( char *, ImageData * );
    int tiff_p( char * );
    int ras2imgd( char *, ImageData * );
    void swab_rashead( struct rasterfile * );

    /* sites.c */
    struct Site *nextsite();

    /* usage.c */
    void usage();

    /* voronoi.c */
    void voronoi( Coordinate, Coordinate );

    /* voronoi-pageseg.c */
    void voronoi_pageseg(LineSegment ** ,unsigned int *, ImageData *);
    void set_param(int nm, int sr, float fr, int ta);
    void voronoi_colorseg(ImageData *, ImageData *, bool);

    /* draw_line.c */
    void make_mask(void);
    int draw_bit_get( ImageData *imgd, register int i, register int j ,int odd);
    void bit_to_byte( ImageData *in_imgd, ImageData *out_imgd, int noimage);
    void draw_line(ImageData *imgd, int is, int js, int ie, int je, int color, int width);

}
#endif /* FUNCTION_H_INCLUDED_ */
