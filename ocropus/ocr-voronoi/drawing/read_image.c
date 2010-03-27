static char version_string[]="$Id: read_image.c,v 1.1.1.1 1999/10/15 12:26:21 kise Exp $";
/*
   $Date: 1999/10/15 12:26:21 $
   $Revision: 1.1.1.1 $
   $Author: kise $
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <tiffio.h>
#include "color.h"
#include "read_image.h"
#include "function.h"

/* reading image whose format is either sunraster or tiff */
void read_image(char *fname, ImageData *imgd)
{
  if(ras2imgd(fname,imgd)!=YES &&
     tiff2imgd(fname,imgd)!=YES){
    fprintf(stderr,"file is neither sunraster nor tiff\n");
    exit(1);
  }
}

/* from tiff binary image to ImageData */
int tiff2imgd(char* fname, ImageData *imgd)
{
  if(tiff_p(fname)==YES){
    TIFF* tif=TIFFOpen(fname,"r");
    if (tif) {
      uint32        w,h;
      size_t        bufsize;
      short	    bitspersample;
      short	    samplesperpixel;
      short         photometric;
      unsigned char *buf;
      int           i,j;
      char          emask=0x00;

      TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
      TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
      if (bitspersample != 1 || samplesperpixel != 1){
	fprintf(stderr, "tiff2imgd: input image is not a binary image\n");
	exit(1);
      }
      TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
      TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
      TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric);
      bufsize=TIFFScanlineSize(tif);

      if((buf = (unsigned char *) malloc(bufsize))==NULL){
	fprintf(stderr,"tiff2imgd: not enough memory for buf\n");
	exit(1);
      }

      if((imgd->image=(char *)malloc(bufsize*h))==NULL){
	fprintf(stderr,"tiff2imgd: not enough memory for image\n");
	exit(1);
      }
      /* setting dimension */
      imgd->imax=bufsize*BYTE;
      imgd->jmax=h;

      /* reading image data */
      switch (photometric) {
      case PHOTOMETRIC_MINISWHITE:
	for(j=0 ; j < h ; j++){
	  TIFFReadScanline(tif, buf, j, 0);
	  for(i=0 ; i<bufsize ; i++){
	    *((imgd->image)+i+j*bufsize)=*(buf+i);
	  }
	}
	break;
      case PHOTOMETRIC_MINISBLACK:
	for(j=0 ; j < h ; j++){
	  TIFFReadScanline(tif, buf, j, 0);
	  for(i=0 ; i<bufsize ; i++){
	    *((imgd->image)+i+j*bufsize)=~*(buf+i);
	  }
	}
      }
      TIFFClose(tif);

      /* cleaning the right edge of the image */
      for( i = 0 ; i < w%BYTE ; i++)
	emask|=(0x01 << (BYTE-i-1));
      for( j = 0 ; j < h ; j++)
	*((imgd->image)+(j+1)*bufsize-1)&=emask;
    }
    return(YES);
  }
  else{
    return(NO);
  }
}

/* predicate :
   if <fname> is tiff file, return YES;otherwise NO
*/
int tiff_p(char* fname)
{
  TIFFHeader hdr;
  int fd;

  if((fd=open(fname, O_RDONLY|O_BINARY,0))<0){
    fprintf(stderr,"tiff_p: file open error :%s\n",fname);
    exit(1);
  }
  lseek(fd,(off_t) 0,0);
  if(read(fd, (char*) &hdr, sizeof(hdr))!=sizeof(hdr)){
    fprintf(stderr,"tiff_p: header read error\n");
    exit(1);
  }
  close(fd);
  if(hdr.tiff_magic != TIFF_BIGENDIAN &&
     hdr.tiff_magic != TIFF_LITTLEENDIAN){
    return(NO);
  }
  if(hdr.tiff_version != TIFF_VERSION &&
     hdr.tiff_version != II_TIFF_VER){
    return(NO);
  }
  return(YES);
}

/* from sunraster to ImageData */
int ras2imgd(char* fname, ImageData *imgd)
{
  struct rasterfile header;  
  FILE *fp;
  int length; /* in BYTE */
  
  if((fp=fopen(fname,"r"))==NULL){
    fprintf(stderr,"ras2imgd: file open error :%s\n",fname);
    exit(1);
  }
  /* reading header */
  fread(&header, sizeof(header),1,fp);
  if(header.ras_magic != RAS_MAGIC && 
     header.ras_magic != II_RAS_MAGIC){
    return(NO);
  }
  if(header.ras_magic == II_RAS_MAGIC){
    swab_rashead(&header);
  }
  if(header.ras_depth!=1){
    fprintf(stderr,"ras2imgd: file is not binary :%s\n",fname);
    exit(1);
  }
    
  /* setting imax, jmax */

  if(header.ras_width%MULTIPLY!=0){
    imgd->imax=MULTIPLY*(header.ras_width/MULTIPLY)+MULTIPLY;
  }
  else{
    imgd->imax=header.ras_width;
  }

  if(imgd->imax != header.ras_length/header.ras_height*BYTE){
    fprintf(stderr,"ras2imgd: Warning - ras_length = %d may be wrong.\n",
	    header.ras_length);
    fprintf(stderr,"ras2imgd: Warning - width = %d is used\n",
	    imgd->imax);
  }
  /* imgd->imax = header.ras_length/header.ras_height*BYTE; */
  /* we cannot use this because ras_length is sometimes set to 0. */
  imgd->jmax = header.ras_height;
  length = imgd->imax * imgd->jmax /BYTE;

  if(((imgd->image) = (char *)malloc(length)) == NULL){
    fprintf(stderr,"ras2imgd: not enough memory for imgd\n");
    exit(1);
  }
  fread(imgd->image,sizeof(char),length,fp);
  fclose(fp);
  return(YES);
}

/* swab bytes of the header in a sunraster file
   required for reading the Intel data format */
void swab_rashead(struct rasterfile *header)
{
  int i,j;
  unsigned char tmp[sizeof(*header)];

  for(i=0 ; i<sizeof(*header) ; i++){
    tmp[i]=*((char *)header + i);
  }
  for(i=0; i<sizeof(*header)/sizeof(int); i++)
    for(j=0; j<sizeof(int); j++)
      *((int *)header +i)=
	*((int *)header +i)<<BYTE | *(tmp+i*4+j);
}
