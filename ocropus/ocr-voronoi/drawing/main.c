static char version_string[]="$Id: main.c,v 1.1.1.1 1999/10/15 12:26:21 kise Exp $";
/*
   $Date: 1999/10/15 12:26:21 $
   $Revision: 1.1.1.1 $
   $Author: kise $

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "color.h"
#include "read_image.h"
#include "function.h"

int main(int argc,char *argv[]){

  FILE	*lfp,*ofp;
  char	ifname[NAMELEN];
  char  lfname[NAMELEN];
  char  pfname[NAMELEN];
  char  rfname[NAMELEN];
  char	ofname[NAMELEN];
  char	lcolor[CLEN], tmp_str[CLEN];
	
  ImageData	in_imgd, out_imgd;
	
  int	c_rgb;
  int	i;
  int	noborder=YES;
  int   noimage=NO;
  int	is, ie, js, je;
  int   width = WIDTH;
  int   pi, pj;
  int   ifn=NO, lfn=NO, ofn=NO, pfn=NO, rfn=NO, default_w=YES;

  /* 引数の解析 */
  i=1;
  while( argv[i] != NULL ){
    if(strcmp(argv[i],"-border")==0) {
      noborder=NO;
    }
    else if(strcmp(argv[i],"-noimage")==0) {
      noimage=YES;
    }
    else if(strcmp(argv[i],"-c")==0) {
      i++;
      if(argv[i] == NULL) usage();
      strcpy(lcolor, argv[i]);
    }
    else if(strcmp(argv[i],"-i")==0) {
      i++;
      if(argv[i] == NULL) usage();
      strcpy(ifname, argv[i]);
      ifn = YES;
    }
    else if(strcmp(argv[i],"-o")==0) {
      i++;
      if(argv[i] == NULL) usage();
      strcpy(ofname, argv[i]);
      ofn = YES;
    }
    else if(strcmp(argv[i],"-l")==0) {
      i++;
      if(argv[i] == NULL) usage();
      strcpy(lfname, argv[i]);
      lfn = YES;
    }
    else if(strcmp(argv[i],"-p")==0) {
      i++;
      if(argv[i] == NULL) usage();
      strcpy(pfname, argv[i]);
      pfn = YES;
    }
    else if(strcmp(argv[i],"-r")==0) {
      i++;
      if(argv[i] == NULL) usage();
      strcpy(rfname, argv[i]);
      rfn = YES;
    }
    else if(strcmp(argv[i],"-w")==0) {
      i++;
      if(argv[i] == NULL) usage();
      width = atoi(argv[i]);
      default_w = NO;
    }
    else {
      usage();
    }
    i++;
  }

  /* checking mandatory arguments */
  if(ifn!=YES || ofn!=YES || (lfn!=YES && pfn!=YES && rfn!=YES) ){
    usage();
  }

  /* changing default width for points */
  if(pfn==YES && default_w==YES){
    width=PWIDTH;
  }
  if(rfn==YES && default_w==YES){
    width=RWIDTH;
  }

  fprintf(stderr, "input        : %s\n",ifname);
  if(lfn==YES){
    fprintf(stderr, "line segment : %s\n",lfname);
  }
  else if(pfn == YES){
    fprintf(stderr, "point        : %s\n",pfname);
  }
  else{
    fprintf(stderr, "rectangle    : %s\n",rfname);
  }
  fprintf(stderr, "output       : %s\n",ofname);
  fprintf(stderr, "width        : %d\n",width);
    
  strcpy( tmp_str, lcolor );
  tmp_str[2] = '\0';

  /* 線の色の指定を読み取る */
  if( (strcmp(lcolor, "r")==0) || (strcmp(lcolor, "red")==0) ){
    c_rgb = Red;
  }
  else if( (strcmp(lcolor, "g")==0) || (strcmp(lcolor, "green")==0) ){
    c_rgb = Green;
  }
  else if( strcmp(lcolor, "black")==0 ){
    c_rgb = Black;
  }
  else if( (strcmp(lcolor, "b")==0) || (strcmp(lcolor, "blue")==0) ){
    c_rgb = Blue;
  }
  else if( strcmp(lcolor, "black")==0 ){
    c_rgb = Black;
  }
  else if(strcmp(tmp_str, "0x")==0){
    c_rgb = strtol(lcolor, NULL, 16);
  }
  else{
    c_rgb = Red;
  }

  if( lfn == YES && (lfp=fopen(lfname,"r")) == NULL ){
    fprintf(stderr,"FILE %s not found\n",lfname);
    exit(-1);
  }
  if( pfn == YES && (lfp=fopen(pfname,"r")) == NULL ){
    fprintf(stderr,"FILE %s not found\n",lfname);
    exit(-1);
  }
  if( rfn == YES && (lfp=fopen(rfname,"r")) == NULL ){
    fprintf(stderr,"FILE %s not found\n",lfname);
    exit(-1);
  }
  if( (ofp=fopen(ofname,"w")) == NULL ){
    fprintf(stderr,"FILE %s not found\n",ofname);
    exit(-1);
  }

  /* reading image data */
  read_image(ifname,&in_imgd);

  /* setting image size */
  out_imgd.imax=in_imgd.imax;
  out_imgd.jmax=in_imgd.jmax;

  fprintf(stderr,"image size: (%d X %d)\n",in_imgd.imax,in_imgd.jmax);
	
  /* 出力画像の領域確保 */
  if(NULL == (out_imgd.image =
	      (char *)malloc(out_imgd.imax*out_imgd.jmax))){
    fprintf(stderr,"No enough memory! for org\n" );
    exit(-1);
  }

  /* 入力画像(1bit)を出力画像(8bits)にコピー 
     copying the input image (binary;1bit/pixel) to
     the output image (8bits/pixel)
     */
  bit_to_byte( &in_imgd, &out_imgd, noimage );
    
  /* 画像の枠を記述 */
  if(noborder==NO)
    frame(&out_imgd,1,LINE_C);
    
  i=0;
  if(lfn==YES){
    fprintf(stderr,"drawing lines...");
    /* 線セグメントデータの読み込み と 描画 */
    while(fscanf(lfp,"%d %d %d %d",&is, &ie, &js, &je)!=EOF){
      draw_line(&out_imgd, is, js, ie, je, LINE_C, width);
      i++;
    }
    fclose(lfp);
    fprintf(stderr,"done\n");
    fprintf(stderr,"# of line segments : %d\n",i);
  }
  else if(pfn==YES){
    fprintf(stderr,"drawing points...");
    /* 点データの読み込み と 描画 */
    while(fscanf(lfp,"%d %d",&pi, &pj)!=EOF){
      draw_point(&out_imgd, pi, pj, LINE_C, width);
      i++;
    }
    fclose(lfp);
    fprintf(stderr,"done\n");
    fprintf(stderr,"# of line points : %d\n",i);
  }
  else{
    fprintf(stderr,"drawing rectangless...");
    /* 矩形データの読み込み と 描画 */
    while(fscanf(lfp,"%d %d %d %d",&is, &ie, &js, &je)!=EOF){
      draw_rect(&out_imgd, is, js, ie, je, LINE_C, width);
      i++;
    }
    fclose(lfp);
    fprintf(stderr,"done\n");
    fprintf(stderr,"# of rectangles : %d\n",i);
  }
    
  /* 画像の出力 */
  write_image(ofname, &out_imgd, c_rgb, DEPTH);

  return 0;
}

void usage()
{
  fprintf(stderr,"usage: dl (990818)\n");
  fprintf(stderr,"Mandatory arguments :\n");
  fprintf(stderr,"\t-i <input file>\n");
  fprintf(stderr,"\t-l <line segment> OR -p <point> OR -r <rectangle>\n");
  fprintf(stderr,"\t-o <output file> \n");
  fprintf(stderr,"Optional arguments :\n");
  fprintf(stderr,"\t-c <lines' or points' color> r(ed), b(lue), g(reen) or RGB value \n");
  fprintf(stderr,"\t-w <line width> odd integer\n");
  fprintf(stderr,"\t-border (with frame)\n");
  fprintf(stderr,"\t-noimage (without image)\n");
  exit(-1);
}
