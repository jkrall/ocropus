/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
  output.c
*/

#include <stdio.h>
#include <math.h>
#include "const.h"
#include "defs.h"
#include "extern.h"
#include "function.h"


namespace voronoi{
    /* float pxmin, pxmax, pymin, pymax, cradius; */

    /*
     * (x,y)の座標が枠外であれば, 枠内の座標に変換する関数
     * 新しいボロノイ点(x,y)の番号はFRAME(-1)にする.
     */
    void in_frame(float *x, float *y, float d,
                  struct Edge *e, int lr,
                  Coordinate max_x, Coordinate max_y)
    {
        /* x の座標が0 以下の場合 */
        if(*x < 0){
            *y += (-*x)*d;
            *x = 0;
            e->ep[lr]->sitenbr = FRAME;
            if(*y < 0){
                *x += (-*y)/d;
                *y = 0;
            }
            else if(*y > max_y){
                *x += ((float)max_y - *y)/d;
                *y = (float)max_y;
            }
        }
        /* x の座標がmax_x 以上の場合 */
        else if(*x > max_x){
            *y += ((float)max_x - *x)*d;
            *x = (float)max_x;
            e->ep[lr]->sitenbr = FRAME;
            if(*y < 0){
                *x += (-*y)/d;
                *y = 0;
            }
            else if(*y > max_y){
                *x += ((float)max_y - *y)/d;
                *y = (float)max_y;
            }
        }
        /* y の座標が0 以下の場合 */
        else if(*y < 0){
            *x += (-*y)/d;
            *y = 0;
            e->ep[lr]->sitenbr = FRAME;
        }
        /* y の座標がmax_y 以上の場合 */
        else if(*y > max_y){
            *x += ((float)max_y - *y)/d;
            *y = (float)max_y;
            e->ep[lr]->sitenbr = FRAME;
        }
    }

    /* 無限遠点である始点を枠内の座標に変換する関数 */
    void s_in_frame(float *xsf, float *xef, float *ysf, float *yef,
                    struct Edge *e, Coordinate max_y)
    {
        float d;
    
        if((e->b) == 0) {	/* 傾きが∞の場合 */
            *xsf = *xef;
            *ysf = (float)max_y;
        }
        else {
            d = -(e->a)/(e->b); /* 傾き */
	    
            if(d == 0){	/* 傾きが0 の場合 */
                *xsf = 0;
                *ysf = *yef;
            }
            else if(d > 0){	/* 傾きが正の場合 */
                if(*yef > *xef*d){
                    *xsf = 0;
                    *ysf = *yef - (*xef*d);
                }
                else {
                    *xsf = *xef - (*yef/d);
                    *ysf = 0;
                }
            }
            else {		/* 傾きが負の場合 */
                if(((float)max_y - *yef) > ((-*xef) * d)){
                    *xsf = 0;
                    *ysf = *yef + ((-*xef) * d);
                }
                else {
                    *xsf = *xef - (-((float)max_y - *yef)) / d;
                    *ysf = (float)max_y;
                }
            }
        }
    }

    /* 無限遠点である終点を枠内の座標に変換する関数 */
    void e_in_frame(float *xsf, float *xef, float *ysf, float *yef,
                    struct Edge *e, Coordinate max_x, Coordinate max_y)
    {
        float d;

        if((e->b) ==0) {	/* 傾きが∞の場合 */
            *xef = *xsf;
            *yef = 0;
        }
        else {
            d = -(e->a)/(e->b); /* 傾き */

            if(d == 0){	/* 傾きが0 の場合 */
                *xef = (float)max_x;
                *yef = *ysf;
            }
            else if(d > 0){	/* 傾きが正の場合 */
                if(((float)max_y - *ysf) > (((float)max_x - *xsf) * d)){
                    *xef = (float)max_x;
                    *yef = *ysf + ((float)max_x - *xsf) * d;
                }
                else {
                    *xef = *xsf + ((float)max_y - *ysf) / d;
                    *yef = (float)max_y;
                }
            }
            else {		/* 傾きが負の場合 */
                if(*ysf > (- ((float)max_x - *xsf) * d)){
                    *xef = (float)max_x;
                    *yef = *ysf - (- ((float)max_x - *xsf) * d);
                }
                else {
                    *xef = *xsf + (-*ysf) / d;
                    *yef =0;
                }
            }
        }
    }

    /*
     * 始点, 終点の座標が無限遠点, または, 枠外の場合,
     * 座標を修正し, そのボロノイ点の番号をFRAME(-1) にする関数
     */
    void frameout(float *xsf, float *xef, float *ysf, float *yef,
                  int *sp, int *ep, struct Edge *e,
                  Coordinate max_x, Coordinate max_y)
    {
        float d;

        /* 始点, 終点ともに無限遠点でない場合 */
        if((e->ep[LE] != (struct Site *)NULL) &&
           (e->ep[RE] != (struct Site *)NULL)){
            *xsf = e->ep[LE]->coord.x; /* 始点のx 座標 */
            *xef = e->ep[RE]->coord.x; /* 終点のx 座標 */
            *ysf = e->ep[LE]->coord.y; /* 始点のy 座標 */
            *yef = e->ep[RE]->coord.y; /* 終点のy 座標 */

            /* 傾きが∞ のとき */
            if(*xsf == *xef) {
                if(*ysf < 0) {
                    *ysf =0;
                    e->ep[LE]->sitenbr = FRAME;
                }
                else if(*ysf > max_y) {
                    *ysf = (float)max_y;
                    e->ep[LE]->sitenbr = FRAME;
                }
                if(*yef < 0) {
                    *yef = 0;
                    e->ep[RE]->sitenbr = FRAME;
                }
                else if(*yef > max_y) {
                    *yef = (float)max_y;
                    e->ep[RE]->sitenbr = FRAME;
                }
            }

            /* 傾きが有限のとき */
            else {
                d = (*yef - *ysf)/(*xef - *xsf); /* 傾き */

                /* 始点が画像の枠外の場合 */
                in_frame(xsf,ysf,d,e,LE,max_x,max_y);
                /* 終点が画像の枠外の場合 */	    
                in_frame(xef,yef,d,e,RE,max_x,max_y);
            }
	
            *sp = e->ep[LE]->sitenbr; /* 始点の番号を代入 */
            *ep = e->ep[RE]->sitenbr; /* 終点の番号を代入 */
	
        }
    
        /* 始点が無限遠点の場合 */
        else if((e->ep[LE] == (struct Site *)NULL)) {
            *xef = e->ep[RE]->coord.x; /* 終点のx 座標 */
            *yef = e->ep[RE]->coord.y; /* 終点のy 座標 */
            *sp = FRAME;	/* 始点の番号FRAME を代入 */
            *ep = e->ep[RE]->sitenbr; /* 終点の番号を代入 */

            /* 始点を枠内にする */
            s_in_frame(xsf,xef,ysf,yef,e,max_y);
        }
        
        /* 終点が無限遠点の場合 */
        else if((e->ep[RE] == (struct Site *)NULL)){
            *xsf = e->ep[LE]->coord.x; /* 始点のx 座標 */
            *ysf = e->ep[LE]->coord.y; /* 始点のy 座標 */
            *sp = e->ep[LE]->sitenbr; /* 始点の番号を代入 */
            *ep = FRAME;	/* 終点の番号FRAME を代入 */

            /* 終点を枠内にする */
            e_in_frame(xsf,xef,ysf,yef,e,max_x,max_y);
        }
    }

    /*
     * 連結成分間のボロノイ辺のみをlineseg に格納し,
     * 連結成分間の関係neighbor をつくる関数.
     */
    void out_ep2(struct Edge *e, struct Site *v,
                 Coordinate imax, Coordinate jmax)
    {
        int i,sp,ep;
        float xsf,xef,ysf,yef;
        //  float si,sj,ei,ej;
        Coordinate max_x=imax-1;
        Coordinate max_y=jmax-1;

        static unsigned int current_neighbor_size = INITNEIGHBOR;
        static unsigned int current_lineseg_size  = INITLINE;

        /* double i1,j1,i2,j2; */



        /* ラベルが同じ場合は出力しない */
        if(output_pvor == NO && e->lab1 == e->lab2) {
            point_edge++;
            return;
        }
        else {
            /*
             * ボロノイ点が無限遠点, または, 枠外かどうか判定し,
             * もし, そうなら枠内の座標に修正
             */
            frameout(&xsf,&xef,&ysf,&yef,&sp,&ep,e,imax-1,jmax-1);
	
            /* 上の処理によっても始点, 終点の一方が枠外の場合出力しない */
            if((xsf < 0.0) || (xsf > (float)max_x) ||
               (xef < 0.0) || (xef > (float)max_x) ||
               (ysf < 0.0) || (ysf > (float)max_y) ||
               (yef < 0.0) || (yef > (float)max_y))
                return;
        }

        // オリジナルと違うところ
        // 一度コメントアウトしてみる
        //
        //  if((e->ep[LE] != (struct Site *)NULL) &&
        //     (e->ep[RE] != (struct Site *)NULL)){
        //    /* ボロノイ辺の始点・終点ともに無限遠点でない場合 */
        //		
        //    si = e->ep[LE]->coord.x; /* 始点のx 座標 */
        //    ei = e->ep[RE]->coord.x; /* 終点のx 座標 */
        //    sj = e->ep[LE]->coord.y; /* 始点のy 座標 */
        //    ej = e->ep[RE]->coord.y; /* 終点のy 座標 */
        //		
        //    if(!((si < (float)imax)&&(ei < (float)imax)&&
        //	 (sj < (float)jmax)&&(ej < (float)jmax)
        //	 &&(si > 0)&&(ei > 0)&&(sj > 0)&&(ej > 0))){
        //      /* もし始点・終点ともに画像外にあったら */
        //      return;
        //      /* neighbor のセットを行わない */
        //    }
        //  }
        //  else{
        //    /* ボロノイ辺の始点・終点のどちらかが無限遠点である場合 */
        //    return;
        //    /* neighbor のセットを行わない */
        //  }
	
        /*
         * 連結成分の隣接関係neighbor に既に登録してあれば,
         * 登録されている距離と, 今回のボロノイ辺を構成する母点間の
         * 距離の大きさを比較し,連結成分間の距離を再定義する
         */

        i = search(e->lab1,e->lab2); /* ハッシュ表に登録されているか調べる */

        /* 未登録の場合 */

        if(i == NODATA){
            enter(e->lab1,e->lab2,NEIGHnbr); /* ハッシュ表に登録 */
            /*
              i1 = (double)(component[e->lab1].xc);
              j1 = (double)(component[e->lab1].yc);
              i2 = (double)(component[e->lab2].xc);
              j2 = (double)(component[e->lab2].yc);
            */

            /* 母点間の距離を代入 */
            neighbor[NEIGHnbr].dist = e->dist; /* 連結成分間の最小距離 */
            neighbor[NEIGHnbr].lab1 = e->lab1;
            neighbor[NEIGHnbr].lab2 = e->lab2;
	
            /* 重心間の角度を代入 */
            /*	if(i1 == i2) {
                neighbor[NEIGHnbr].angle = -RIGHTANGLE;
                }
                else {
                angle = atan2((j2-j1),(i2-i1))*2*RIGHTANGLE/M_PI;
			
                if(angle > RIGHTANGLE){
                neighbor[NEIGHnbr].angle = (float)(angle-2*RIGHTANGLE);
                }
                else if(angle <= -RIGHTANGLE){
                neighbor[NEIGHnbr].angle = (float)(angle+2*RIGHTANGLE);
                }
                else {
                neighbor[NEIGHnbr].angle = (float)angle;
                }
                }
            */
            /*	neighbor[NEIGHnbr].dist = 
                (float)sqrt((component[e->lab1].xc - component[e->lab2].xc) *
                (component[e->lab1].xc - component[e->lab2].xc) +
                (component[e->lab1].yc - component[e->lab2].yc) *
                (component[e->lab1].yc - component[e->lab2].yc));*/
            /* 連結成分の重心間の距離 */
            /*	dx = (double)(component[e->lab2].xc - component[e->lab1].xc);
                dy = (double)(component[e->lab2].yc - component[e->lab1].yc);
                neighbor[NEIGHnbr].angle = (float)atan2(dy,dx);*/
	
            NEIGHnbr++;
            if(NEIGHnbr >= current_neighbor_size) {
                neighbor=(Neighborhood *)myrealloc(neighbor,
                                                   current_neighbor_size,
                                                   INCNEIGHBOR,
                                                   sizeof(Neighborhood));
                current_neighbor_size+=INCNEIGHBOR;
            }
        }

        /* 登録済みの場合 */
        else {
            if(neighbor[i].dist > e->dist) /* 距離を比較して短い方を連結 */
                neighbor[i].dist = e->dist; /* 成分間の距離とする */
        }

        if(sp > SiteMax) SiteMax = sp;
        if(ep > SiteMax) SiteMax = ep;

        /* ボロノイ辺の情報を格納する */
        lineseg[LINEnbr].sp = sp;
        lineseg[LINEnbr].ep = ep;
        lineseg[LINEnbr].xs = (unsigned int)(xsf+0.5);
        lineseg[LINEnbr].xe = (unsigned int)(xef+0.5);
        lineseg[LINEnbr].ys = (unsigned int)(ysf+0.5);
        lineseg[LINEnbr].ye = (unsigned int)(yef+0.5);
        lineseg[LINEnbr].lab1 = e->lab1;
        lineseg[LINEnbr].lab2 = e->lab2;
        lineseg[LINEnbr].yn = OUTPUT;
        LINEnbr++;
        point_edge++;
        if(LINEnbr >= current_lineseg_size) {
            lineseg=(LineSegment *)myrealloc(lineseg,
                                             current_lineseg_size,
                                             INCLINE,
                                             sizeof(LineSegment));
            current_lineseg_size+=INCLINE;
        }
    }
}
