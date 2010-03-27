/*
  $Id: read_image.h,v 1.1.1.1 1999/10/15 12:40:27 kise Exp $
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
*/

#ifndef READ_IMAGE_INCLUDED_
#define READ_IMAGE_INCLUDED_
#ifndef O_BINARY
#define O_BINARY        0
#endif

#define II_TIFF_VER    0x2a00
#define RAS_MAGIC      0x59a66a95
#define II_RAS_MAGIC   0x956aa659
#define MULTIPLY       16 /* the image width of sunraster 
			     must be a multiply of 16 */
namespace voronoi{
    /*
      the header of sun rasterfiles.
      see "man rasterfile"
    */
    struct rasterfile{
        int  ras_magic;
        int  ras_width;
        int  ras_height;
        int  ras_depth;
        int  ras_length;
        int  ras_type;
        int  ras_maptype;
        int  ras_maplength;
    };
}
#endif



