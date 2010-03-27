/*
  $Date: 1999/10/15 12:40:27 $
  $Revision: 1.1.1.1 $
  $Author: kise $
  main.c
  メインプログラム
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "defs.h"
#include "const.h"
#include "function.h"

namespace voronoi{
#define LINE_C  192 // blue color in range 0-255
#define WIDTH   5

    BlackPixel 	*bpx;		/* 黒画素の座標とそのラベル */
    Neighborhood	*neighbor;	/* 隣接連結成分間の特徴量 */
    LineSegment	*lineseg;	/* 始点・終点の座標とラベル */
    HashTable	*hashtable[M1+M2];
    /* 隣接連結成分のラベルに対す
       るハッシュ表 */
    EndPoint	*endp;		/* 線分の端点 */

    NumPixel	BPnbr;		/* 黒画素の数 */
    Label	        LABELnbr;	/* 連結成分数 */
    unsigned int	NEIGHnbr;	/* 隣接する連結成分の組の数 */
    unsigned int	LINEnbr;	/* 除去前のエリアボロノイ辺の
                                           線分の数 */
    unsigned int	Enbr;		/* ボロノイ辺が除去される連結
                                           成分の組の数 */
    long		SiteMax;	/* ボロノイ点の番号の最大値 */

    int		noise_max = NOISE_MAX;	   /* 除去する連結成分の画素数 */
    int		sample_rate = SAMPLE_RATE; /* 境界線追跡でサンプリングす */
					   /* る割合 */
    float		freq_rate = FREQ_RATE;
    int             Ta = Ta_CONST;
    unsigned int	sample_pix;	/* サンプリングして得られた画 */
    /* 素数 */
    unsigned int	point_edge;	/* 点ボロノイ辺の本数 */
    unsigned int	edge_nbr;	/* 除去後のエリアボロノイ辺の */
    /* 線分の本数 */
    int             *area;       /* ラベルln のついた連結成分の面積 */

    // Modification by Faisal Shafait
    // keep track of noise components to remove them
    // from the output image
    bool *noise_comp;
    unsigned int nconcomp_inc=50;
    unsigned int nconcomp_size=0;
    // End of Modification
    
#ifdef TIME
    float    b_s_time=0;
    float    v_time=0;
    float    e_time=0;
    float    o_time=0;
    clock_t		start, end;
#endif /* TIME */

    float	xmin, xmax, ymin, ymax, deltax, deltay;

    struct Site		*sites;
    int			nsites;
    int			siteidx;
    int			sqrt_nsites;
    int			nvertices;
    struct Freelist 	sfl;
    struct Site		*bottomsite;

    int 		nedges;
    struct Freelist 	efl;

    struct Freelist	hfl;
    struct	Halfedge	*ELleftend, *ELrightend;
    int 			ELhashsize;
    struct	Halfedge	**ELhash;

    int 			PQhashsize;
    struct	Halfedge 	*PQhash;
    int 			PQcount;
    int 			PQmin;

    /* 追加機能用 */
    int    smwind = SMWIND;

    /* 追加分 */
    char     output_points = NO;
    char     output_pvor = NO;
    char     output_avor = NO;
    char     display_parameters = NO;


    void voronoi_pageseg(LineSegment **mlineseg, 
                         unsigned int *nlines,
                         ImageData *imgd1) {
        int 		i;

        point_edge = 0;
        edge_nbr = 0;

        BPnbr = LABELnbr = NEIGHnbr = LINEnbr = Enbr = SiteMax = 0;

        /* 処理パラメータの表示
           displaying parameters */
        if(display_parameters == YES)
            dparam();

        /* 画像の周囲1 画素分を0 にセットする. */
        frame(imgd1,1,0);

        /* 黒画素bpx の領域確保 */
        bpx=(BlackPixel *)myalloc(sizeof(BlackPixel)* INITPIXEL);

        /* Site 型sites の領域確保 */
        sites = (struct Site *) myalloc(SITE_BOX*sizeof *sites);
    
        /* 入力画像をSite 型に変換 */
    
        //fprintf(stderr,"Transforming Image to Site...");
#ifdef TIME
        start = clock();
#endif
        img_to_site(imgd1);
#ifdef TIME
        end = clock();
        b_s_time = (float)((end-start)/((float)CLOCKS_PER_SEC));
#endif
        //fprintf(stderr,"done\n");

        /* area[ln] の領域確保 */
        area=(int *)myalloc(sizeof(int)*LABELnbr);

        /* area[ln] の値を初期化 */
        for(i=0;i<LABELnbr;i++) area[i]=0;

        /* area[ln] の値をセット */
        for(i=0;i<BPnbr;i++) area[bpx[i].label]++;

        /* bpx の領域解放 */
        //        free(bpx);
    
        /* 隣接連結成分間の特徴量neighbor の領域確保 */
        neighbor = (Neighborhood *)myalloc(sizeof(Neighborhood)* INITNEIGHBOR);

        /* 線分lineseg の領域確保 */
        lineseg = (LineSegment *)myalloc(sizeof(LineSegment)* INITLINE);

        /* ハッシュ表を初期化
           initialization of hash tables */
        init_hash();

        /* エリアVoronoi 図作成 
           constructing the area Voronoi diagram */
    
        //fprintf(stderr,"Constructing area Voronoi diagram...");
#ifdef TIME
        start = clock();
#endif
        voronoi(imgd1->imax, imgd1->jmax);
#ifdef TIME
        end = clock();
        v_time = (float)((end-start)/((float)CLOCKS_PER_SEC));
#endif
        //fprintf(stderr,"done\n");

        /* ボロノイ辺の端点endp の領域確保 */
        SiteMax+=1;
        endp = (EndPoint *)myalloc(sizeof(EndPoint) * SiteMax);
    
        /* ボロノイ辺除去 */
        //fprintf(stderr,"Erasing Voronoi edges...");
#ifdef TIME
        start = clock();
#endif
        erase();
#ifdef TIME
        end = clock();
        e_time = (float)((end-start)/((float)CLOCKS_PER_SEC));
#endif
        //fprintf(stderr,"done\n");

        /* neighbor の領域解放 */
        free(neighbor);
        
        /* ボロノイ辺出力 */
#ifdef TIME
        start = clock();
#endif
        *nlines = LINEnbr;
        *mlineseg = (LineSegment *)malloc(LINEnbr*sizeof(LineSegment));
        for(i=0;i<LINEnbr;i++) {
            (*mlineseg)[i] = lineseg[i];
            if(lineseg[i].yn == OUTPUT &&
               (lineseg[i].xs != lineseg[i].xe
                || lineseg[i].ys != lineseg[i].ye)) {
                edge_nbr++;
            }
        }

#ifdef TIME
        end = clock();
        o_time = (float)((end-start)/((float)CLOCKS_PER_SEC));
#endif
        // Output debug information 
        //dnumber(imgd1->imax, imgd1->jmax);
        /* 領域解放 */
        free(area);
        free(sites);
        free(lineseg);
        EndPoint point;
        for(i=0;i<SiteMax;i++){
            while(endp[i].next){
                point=*endp[i].next;
                free(endp[i].next);
                endp[i].next=point.next;
            }
        }
        free(endp);
        freelist_destroy(&hfl);
        freelist_destroy(&efl);
        freelist_destroy(&sfl);
    }

    void set_param(int nm, int sr, float fr, int ta){
        if(nm>=0)
            noise_max = nm;
        if(sr>=0)
            sample_rate = sr;
        if(fr>=0)
            freq_rate = FREQ_RATE;
        if(ta>=0)
            Ta = ta;
    }

    void voronoi_colorseg(ImageData *out_img,
                          ImageData *in_img,
                          bool remove_noise) {
    
        unsigned int nlines=0;
        LineSegment	 *mlineseg;
        voronoi_pageseg(&mlineseg,&nlines,in_img);

        /* setting image size */
        out_img->imax=in_img->imax;
        out_img->jmax=in_img->jmax;
        if((out_img->image=(char *)malloc(in_img->imax*in_img->jmax))==NULL){
            fprintf(stderr,"voronoi_colorseg: not enough memory for image\n");
            exit(1);
        }
        bool noimage = false;
        bit_to_byte(in_img,out_img,noimage);

        if(remove_noise){
            for(int i=0;i<BPnbr; i++){
                int index = bpx[i].xax+(bpx[i].yax*out_img->imax);
                if(noise_comp[bpx[i].label] && index<out_img->imax*out_img->jmax)
                    out_img->image[index] = WHITE;
            }
        }

        for(int i=0;i<nlines;i++){
            if(mlineseg[i].yn == OUTPUT &&
               (mlineseg[i].xs != mlineseg[i].xe
                || mlineseg[i].ys != mlineseg[i].ye)) {
                draw_line(out_img, mlineseg[i].xs, mlineseg[i].ys, 
                          mlineseg[i].xe, mlineseg[i].ye, LINE_C, WIDTH);
                //             fprintf(stderr,"%d %d %d %d\n",
                // 		    mlineseg[i].xs,mlineseg[i].xe,
                // 		    mlineseg[i].ys,mlineseg[i].ye);
            }
        }
        free(bpx);
        free(noise_comp);
        free(mlineseg);
    }
}
