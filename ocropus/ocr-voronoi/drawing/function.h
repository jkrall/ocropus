/*
   $Id: function.h,v 1.1.1.1 1999/10/15 12:26:21 kise Exp $
   $Date: 1999/10/15 12:26:21 $
   $Revision: 1.1.1.1 $
   $Author: kise $
*/
#include "color.h"
#include "read_image.h"

void write_image( char *, ImageData *, int, int );
int bit_get( ImageData *, register int, register int, int );
void make_mask();
void bit_to_byte( ImageData *, ImageData *, int );
void frame( ImageData *, int, int );
void read_image( char *, ImageData * );
int tiff2imgd( char *, ImageData * );
int tiff_p( char * );
int ras2imgd( char *, ImageData * );
void swab_rashead( struct rasterfile * );
void make_map ( unsigned short *, unsigned short *, unsigned short *, int );
void usage();
void draw_point( ImageData *, int, int, int, int );
void draw_line( ImageData *, int, int, int, int, int, int );
