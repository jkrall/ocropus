/*
  $Date: 1999/10/18 08:59:27 $
  $Revion: $
  $Author: kise $
  erase.c
  ボロノイ辺除去プログラム

  1つのボロノイ辺に対応する連結成分間の距離が,
  閾値以下の場合, そのボロノイ辺を除去.
  その結果, できる単純端点を持つボロノイ辺も除去

  endp	: この配列のエントリはボロノイ辺の端点の番号
  先頭の値(line)は, このボロノイ点を端点として持つ
  線分(ボロノイ辺)の数
  その後に確保された領域の値(line)は, このボロノイ点を
  端点として持つ線分(ボロノイ辺)の番号
  lineseg : ボロノイ辺の端点(ボロノイ点)の番号と,
  ボロノイ辺の始点, 終点の座標,
  出力許可情報を記憶
*/

#include <stdio.h>
#include <math.h>
#include "const.h"
#include "defs.h"
#include "extern.h"
#include "function.h"

namespace voronoi{
    extern int              *area;

    float			Td2=0.0;	/* 判別式の係数 */
    unsigned int		Td1=0;		/* 距離の閾値 */

    unsigned int            Dmax;

    int start_pos (int pos)
    {
        int cpos = pos - smwind;
        if (cpos < 0) {
            return 0;
        }
        else {
            return cpos;
        }
    }

    int end_pos (int pos)
    {
        int cpos = pos + smwind;
        if (cpos >= Dmax){
            return Dmax-1;
        }
        else {
            return cpos;
        }
    }

    unsigned int Dh_ave(unsigned int *Dh, int pos)
    {
        int i;
        unsigned int ave=0;

        int start = start_pos(pos);
        int end = end_pos(pos);

        for(i=start;i<=end;i++){
            ave=ave+Dh[i];
        }
        return ( (unsigned int) (ave / (end - start + 1)) );
    }

    /*
      距離, 黒画素数の比(差), 平均黒ラン長の差のヒストグラムを作る関数
    */

    void hist()
    {
        int i,j;
        unsigned int *Dh, *Dh_ref, max1, max2;
        float freq,freq1,freq2;

        Dmax = 0;
    
        /* 距離, 黒画素の比(差), 平均黒ラン長の差の最大値を求める */
        for(i=0;i<NEIGHnbr;i++) {
            if(Dmax < neighbor[i].dist)
                Dmax = (unsigned int)(neighbor[i].dist+0.5);
        }
        Dmax++;
    
        /* 距離のヒストグラム用配列の領域確保 */
        Dh = (unsigned int *)myalloc(sizeof(unsigned int)* Dmax);
        Dh_ref = (unsigned int *)myalloc(sizeof(unsigned int)* Dmax);

        /* 配列を初期化 */
        for(i=0;i<Dmax;i++) {
            init_u_int(&Dh[i]);
            init_u_int(&Dh_ref[i]);
        }
    
        /* ヒストグラム作成 */
        for(i=0;i<NEIGHnbr;i++) {
            Dh[(int)(neighbor[i].dist+0.5)]++;
        }

        /* ヒストグラムのスムージング */
        for(i=0;i<Dmax;i++){
            Dh_ref[i]=Dh[i];
        }
        for(i=0;i<Dmax;i++){
            Dh[i]=Dh_ave(Dh_ref,i);
        }

        /* ヒストグラムを調べる */

        /* 距離に関する定数値Td2 の値を決める */
        max1 = max2  = 0;
        for(i=1;i<Dmax-1;i++) {
	
            /* i が極大点の場合 */
            if(Dh[i-1] < Dh[i] && Dh[i] > Dh[i+1]){
                if(Dh[max1] < Dh[i]) {
                    max2 = max1;
                    max1 = i;
                }
                else if(Dh[max2] < Dh[i])
                    max2 = i;
            }
            else if(Dh[i-1] == Dh[i] && Dh[i] > Dh[i+1]) {
                for(j=i-2;j>=0;j--) {
                    if(Dh[j] < Dh[i]) {
                        if(Dh[max1] < Dh[i]) {
                            max2 = max1;
                            max1 = i;
                        }
                        else if(Dh[max2] < Dh[i])
                            max2 = i;

                        break;
                    }
                    else if(Dh[j] > Dh[i])
                        break;
                }
            }
        }
        if(max1 > max2) {
            i = max2;
            max2 = max1;
            max1 = i;
        }
    
        /*
          (i,Dh[i])と(i+1,Dh[i+1])の間を線形補完
        */
        freq=(float)Dh[max2]*freq_rate;
        for(i=max2 ; i<Dmax-1 ; i++) {
            freq1=(float)Dh[i];
            freq2=(float)Dh[i+1];
            if(freq1 >= freq && freq >= freq2){
                if(freq1 != freq2){
                    Td2=(freq1*(float)(i+1)-freq2*(float)i-freq)/(freq1-freq2);
                }
                else{
                    for(j=i+1;j<Dmax;j++){
                        if(Dh[j]!=freq){
                            Td2=(float)(i+j-1)/2.0;
                            /* 平行の場合は中点 */
                            break;
                        }
                    }
                }
                break;
            }
        }

        Td1 = max1;

        //fprintf(stderr,
        //        "\n\tdist\tmax1 %d  max2 %d : Td1 %d  Td2 %.1f  Ta %d\n",
        //        max1,max2,Td1,Td2,Ta);

        if(Td2 == 0.0) {
            fprintf(stderr,"係数Td2の値が0です\n");
            exit(1);
        }

        /* 領域解放 */
        free(Dh);
        free(Dh_ref);
    }

    /*
      2つの連結成分間の距離, 黒画素数の差, (平均黒ラン長の差)から,
      ボロノイ辺がとりあえず出力可能か判別する関数
    */   
    int distinction(Label lab1, Label lab2, int j)
    {
        float dist,dxy,xy1,xy2,n;
            
        /* 連結成分間の距離 */
        dist = neighbor[j].dist;
    
        if(dist <= Td1) return(NO_OUTPUT);	/* 出力しない(除去する) */

        /* 2つの連結成分の黒画素数の数 */
        xy1 = (float)area[lab1];
        xy2 = (float)area[lab2];
        if(xy1 > xy2)
            dxy = xy1 / xy2;
        else
            dxy = xy2 / xy1;
        n = dist / Td2 + dxy / (float)Ta;
        if(n > 1){
            return(OUTPUT);		/* 出力(除去しない) */
        }
        else{
            return(NO_OUTPUT);	/* 出力しない(除去する) */
        }
    }

    /* 単純端点を持つボロノイ辺を除去する関数 */
    void erase_endp(int j)
    {
        EndPoint *point;

        while(j != FRAME) {
            endp[j].line--;
            /* ボロノイ辺の数が1 の場合 */
            if(endp[j].line == 1) {
                point = endp[j].next;
                /* ボロノイ辺の数を0 にする */
                endp[j].line = 0;
                while(1) {
                    /* まだ出力するとなっているボロノイ辺のもう一方の端点 */
                    if(lineseg[point->line].yn == OUTPUT) {
                        if(j == lineseg[point->line].sp){
                            j = lineseg[point->line].ep;
                        }
                        else{
                            j = lineseg[point->line].sp;
                        }
                        /* 出力しなくする  */
                        lineseg[point->line].yn = NO_OUTPUT;
                        /* 次の点j を除去出来れば除去 */
                        break;
                    }
                    /* ボロノイ辺のリストを全部見終わった場合 */
                    else if(point->next == NULL){
                        return;
                    }
                    /* 次を呼び出す */
                    else point = point->next;
                }
                continue;
            }
            else return;
        }
    }

    void erase_aux()
    {
        int i,j;
        EndPoint *point;

        /* endp を初期化 */
        for(i=0;i<SiteMax;i++){
            init_int(&endp[i].line);
            endp[i].next = NULL;
        }

        /*
         * 連結成分間の距離が, 閾値(THRESHOLD)以上の
         * ボロノイ辺を調べる
         */
        for(i=0;i<LINEnbr;i++) {
            j = search(lineseg[i].lab1,lineseg[i].lab2);

            /* ボロノイ辺が判別式から, とりあえず出力可なら */
            if(distinction(lineseg[i].lab1,lineseg[i].lab2,j)
               == OUTPUT) {

                /* 始点が画像の縁でない場合 */
                if(lineseg[i].sp != FRAME) {
                    point = (EndPoint *)myalloc(sizeof(EndPoint)* 1);
	    
                    /* 確保した領域をendp[] につなげる */
                    point->next = endp[lineseg[i].sp].next;
                    endp[lineseg[i].sp].next = point;
                    point->line = i;
                    endp[lineseg[i].sp].line ++;
                }

                /* 終点が画像の縁でない場合 */
                if(lineseg[i].ep != FRAME) {
                    point = (EndPoint *)myalloc(sizeof(EndPoint)* 1);
	   
                    /* 確保した領域をendp[] につなげる */		
                    point->next = endp[lineseg[i].ep].next;
                    endp[lineseg[i].ep].next = point;
                    point->line = i;
                    endp[lineseg[i].ep].line ++;
                }

                lineseg[i].yn = OUTPUT;	/* とりあえず出力してよいと */
					/* する */
            }

            /* 出力しない */
            else {
                lineseg[i].yn = NO_OUTPUT;
            }
        }

        /* 端点除去 */
        for(i=0;i<SiteMax;i++) {
            /* 1つのボロノイ辺にしか属さない端点のとき */
            if(endp[i].line == 1) {
                point = endp[i].next;

                endp[i].line = 0;	/* ボロノイ辺の数を0 にする */

                lineseg[point->line].yn = NO_OUTPUT; /* 出力しなくする */
	    
                /* ボロノイ辺のもう一方の端点を求める */
                if(i == lineseg[point->line].sp){
                    j = lineseg[point->line].ep;
                }
                else if(i == lineseg[point->line].ep) {
                    j = lineseg[point->line].sp;
                }
                else {
                    fprintf(stderr,"erase() error(1) !!\n");
                    exit(1);
                }
	    	    
                erase_endp(j);	/* 点j が除去できるか調べ, 出来れば除
				   去する */
            }
        }
    }

    /* ボロノイ辺除去関数 */
    void erase()
    {
        /* ヒストグラムを求め, 判別式の閾値計算 */
        hist();
        erase_aux();
    }

    /* unsigned int 型を初期化 */
    void init_u_int(unsigned int *data)
    {
        *data = 0;
    }

    /* int 型を初期化 */
    void init_int(int *data)
    {
        *data = 0;
    }
}
