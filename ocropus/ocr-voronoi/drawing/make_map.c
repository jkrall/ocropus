static char version_string[]="$Id: make_map.c,v 1.1.1.1 1999/10/15 12:26:21 kise Exp $";
/*
   $Date: 1999/10/15 12:26:21 $
   $Revision: 1.1.1.1 $
   $Author: kise $

   カラーマップを作るプログラム

   白: 0x00, 黒: 0x01, 線の色: 0x02 と仮定.
   
   使いたい内容に依存したプログラムなので, 他に利用したい場合は
   検討が必要！！

*/

#include <stdio.h>
#include "color.h"
#include "function.h"

void make_map (unsigned short *red,unsigned short *green, unsigned short *blue, int c_rgb )
{
	red[0] = 0xffff;
	red[1] = 0x0000;
	
	green[0] = 0xffff;
	green[1] = 0x0000;
	
	blue[0] = 0xffff;
	blue[1] = 0x0000;

	if(c_rgb == 0x0000ff){
		red[2] = 0x0000;
		green[2] = 0x0000;
		blue[2] = 0xffff;
	}
	else if(c_rgb == 0x00ff00){
		red[2] = 0x0000;
		green[2] = 0xffff;
		blue[2] = 0x0000;
	}
	else{
		red[2] = 0xffff;
		green[2] = 0x0000;
		blue[2] = 0x00000;
	}
}
