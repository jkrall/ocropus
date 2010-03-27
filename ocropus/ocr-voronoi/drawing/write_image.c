static char version_string[]="$Id: write_image.c,v 1.1.1.1 1999/10/15 12:26:21 kise Exp $";
/*
   $Date: 1999/10/15 12:26:21 $
   $Revision: 1.1.1.1 $
   $Author: kise $
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <tiffio.h>
#include "color.h"
#include "function.h"

void write_image(char* fname, ImageData* imgd, int c_rgb, int depth)
{
  TIFF *tif;

  int width, height;
  int row;
  int scan;
  int rowsperstrip;
  int bpsl;
  short samplesperpixel;
  short bitspersample;
  short photometric;
  unsigned short red[3],
    green[3],
    blue[3];
  /* struct tm *ct; */
  /* struct timeval tv; */
  size_t bufsize;
  unsigned char *buf;
  static char *version = "dl 1.0";
  static char *datetime = "1990:01:01 12:00:00";

  /*	gettimeofday(&tv, (struct timezone *) NULL);
	ct = localtime(&tv.tv_sec);
	sprintf(datetime, "19%02d:%02d:%02d %02d:%02d:%02d",
	ct->tm_year, ct->tm_mon + 1, ct->tm_mday,
	ct->tm_hour, ct->tm_min, ct->tm_sec);*/

  tif = TIFFOpen(fname, "w");
  if(tif==NULL){
    fprintf(stderr,"can't open TIFF file %s.\n",fname);
    exit(1);
  }

  width = imgd->imax;
  height = imgd->jmax;

  switch (depth) {
  case 1:
    samplesperpixel = 1;
    bitspersample = 1;
    photometric = PHOTOMETRIC_MINISBLACK;
    break;
  case 8:
    samplesperpixel = 1;
    bitspersample = 8;
    photometric = PHOTOMETRIC_PALETTE;
    break;
  case 24:
    samplesperpixel = 3;
    bitspersample = 8;
    photometric = PHOTOMETRIC_RGB;
    break;
  case 32:
    samplesperpixel = 4;
    bitspersample = 8;
    photometric = PHOTOMETRIC_RGB;
    break;
  default:
    fprintf(stderr,"bogus depth: %d\n", depth);
    exit(1);
  }
	
  bpsl = ((depth * width + 15) >> 3) & ~1;
  rowsperstrip = (8 * 1024) / bpsl;

  TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);
  TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);
  TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, bitspersample);
  TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
  TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_LZW);
  TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, photometric);
  TIFFSetField(tif, TIFFTAG_DOCUMENTNAME, fname);
  TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, "converted Sun rasterfile");
  TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, samplesperpixel);
  TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
  /*	TIFFSetField(tif, TIFFTAG_STRIPBYTECOUNTS, height / rowsperstrip);*/
  TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
  TIFFSetField(tif, TIFFTAG_SOFTWARE, version);
  TIFFSetField(tif, TIFFTAG_DATETIME, datetime);

  memset(red, 0, sizeof(red));
  memset(green, 0, sizeof(green));
  memset(blue, 0, sizeof(blue));

  make_map(red, green, blue, c_rgb);
	
  if(depth ==8){
    TIFFSetField(tif, TIFFTAG_COLORMAP, red, green, blue);
  }

  bufsize = TIFFScanlineSize(tif);
  if((buf = (unsigned char *) malloc(bufsize))==NULL){
    fprintf(stderr,"tifdline: not enough memory for buf\n");
    exit(1);
  }

  for(row = 0; row < height; row++){
    for(scan = 0; scan < bufsize; scan++){
      buf[scan] = imgd->image[scan+row*imgd->imax];
    }
    if(TIFFWriteScanline(tif, buf, row, 0)<0){
      fprintf(stderr,"failed a scanline write (%d)\n", row);
      break;
    }
  }

  TIFFFlushData(tif);
  TIFFClose(tif);
	
  /*	exit(0); */
}
