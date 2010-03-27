// -*- C++ -*-

// Copyright 2006 Deutsches Forschungszentrum fuer Kuenstliche Intelligenz
// or its licensors, as applicable.
//
// You may not use this file except under the terms of the accompanying license.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You may
// obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Project:
// File:
// Purpose:
// Responsible: tmb
// Reviewer:
// Primary Repository:
// Web Sites:


#ifndef h_grid_
#define h_grid_

#include "ocropus.h"

using namespace iulib;

namespace ocropus {

class Grid {
private:

    colib::bytearray data;
    // current position
    int x,y;
    // height of current line in grid
    // (i.e. the height of largest subimage added in current line)
    int hmax;
    // number of components (number of subimages in the grid)
    int ncomp;

public:
    // standard contructor just initializes members
    Grid() {
        x = 0;
        y = 1;
        hmax = 0;
        ncomp = 0;
    }

    void create(int width=4800,int height=800) {
        data.resize(width,height);
        // init data with "background", maybe we should have
        // sth like #def BG 0/1 and, of course, store in pBm
        fill(data, 255);
        x = 0;
        y = 1;
        hmax = 0;
        ncomp = 0;
    }

    /**
     * resize(): needed to add/append extra subimages to loaded grids that
     *  were cropped when saved. Adding only to the height of grid doesn't
     *  upset structure.
     */
    void resize(int height) {
        if (height < data.dim(1)) return;

        //fprintf(stderr,"%d\n",height);
        colib::bytearray tmp;
        move(tmp, data);
        //fprintf(stderr,"%d %d\n",tmp.dim(0),tmp.dim(1));
        data.resize(data.dim(0),height);
        fill(data, 255);
        for(int i=0;i<tmp.dim(0);i++)
            for(int j=0;j<tmp.dim(1);j++)
                data(i,j)=tmp(i,j);
    }

    /**
     * the next subimage will be put in the next line
     * (i.e. one above the current)
     */
    void new_line_() {
        if (hmax == 0) {
            // previously used only hmax, but if we've just loaded a grid,
            // hmax won't have been set.  */
            for(int j=data.dim(1)-1; j>=0; j--) {
                if(data(0,j)==0) {
                    hmax=j;
                    break;
                }
            }
        }
        y += hmax+1;
        x = 0;
        hmax=0;
    }

    /**
     * add a subimage to the grid
     * current error handling:
     * - if subimage is too wide -> throw exception
     * - if subimage it too high -> return false
     * the two cases should be handled the same way
     */
    bool add(colib::bytearray &image) {
        // check if new subimage is not wider than remaining space in current
        // line (width + one for start marker + one for end marker)
        if(x+image.dim(0)+2 > data.dim(0)) {
            // check if image would fit at all (-2 for start and end marker!)
            if(image.dim(0)>data.dim(0)-2) throw "image too wide";
            // go to next line and insert it there
            new_line_();
        }
        // check if new image is higher than hmax
        if (image.dim(1) > hmax) {
            hmax = image.dim(1);
            // make sure that new image height does not exceed total height
            if(y+hmax > data.dim(1)) return false;
            // adjust line height marker in very first column
            for(int i=0;i<hmax;i++) data(0,y+i) = 255;
            data(0,y+hmax) = 0;
        }

        // advance after seperator
        x++;
        // actually add new subimage
        for(int i=0;i<image.dim(0);i++) for(int j=0;j<image.dim(1);j++)
            data(x+i,y+j) = image(i,j);

        // insert start marker (one black pixel)
        ASSERT(x > 0);
        ASSERT(y > 0);
        data(x-1,y-1) = 0;
        x += image.dim(0);
        // insert end marker
        ASSERT(y > 0);
        data(x,y-1) = 0;
        // maybe we don't need ncomp...
        ncomp++;
        return true;
    }

    /**
     * store grid in a FILE stream
     */
    void save(FILE *stream,bool crop=true) {
        if(crop) {
            colib::bytearray tmp;
            int width=-1, height=-1;

            // strip superfluous whitespace from the right
            bool leave = false;
            for(int i=data.dim(0)-1;i>=0 && !leave;i--) {
                for(int j=0;j<data.dim(1);j++) {
                    if(data(i,j)!=255 && width==-1) {
                        width=i+1;
                        leave = true;
                    }
                }
            }
            // take height from height marker
            // (top black pixel of the very first column)
            for(int j=data.dim(1)-1; j>=0; j--) {
                if(data(0,j)==0) {
                    height=j+1;
                    break;
                }
            }

            tmp.resize(width,height);
            for(int i=0;i<tmp.dim(0);i++)
                for(int j=0;j<tmp.dim(1);j++)
                    tmp(i,j) = data(i,j);
            write_pgm(stream,tmp);
        } else {
            write_pgm(stream,data);
        }
    }

    /**
     * save grid to a file
     */
    void save(const char *file,bool crop=true) {
        save(colib::stdio(file,"w"),crop);
    }

    // reading grid files
    void load(FILE *stream) {
        read_pnm_gray(stream,data);
        reset();
    }

    void load(const char *file) {
        load(colib::stdio(file,"r"));
    }

    void reset() {
        x = 1;
        y = 1;
    }

    /**
     * \return true if the first image of the grid contains line information,
     * i.e. four black pixels in a one pixel wide image
     */
    bool hasLineInfo() {
        bool retval = false;
        if (count() > 0) {
            int xBak = x, yBak = y;
            reset();
            colib::bytearray lineinfo;
            next(lineinfo);
            if (lineinfo.dim(0) == 1) {
                int numBlack = 0;
                // go up the 'bar' and count exactly 4 black pixels
                for (int i = 0; i < lineinfo.dim(1); ++i) {
                    if (lineinfo(0, i) == 0) {
                        ++numBlack;
                    }
                }
                if (numBlack == 4) retval = true;
            }
            x = xBak;
            y = yBak;
        }
        return retval;
    }

    bool next(colib::bytearray &image) {
        image.resize(0,0);
        int xorig=x;
        int yorig=y;
        int x1,y1;
        for(;;) {
            y1 = y;
            while(y1<data.dim(1) && data(0,y1)!=0) y1++;
            if(y1==data.dim(1)) {
                x=xorig-1; // go back one, add() will increment.
                y=yorig;
                return false;
            }
            x1 = x;
            while(x1<data.dim(0) && data(x1,y-1)!=0) x1++;
            if(x1==data.dim(0)) {
                x = 1;
                y = y1+1;
            }
            else break;
        }
        image.resize(x1-x,y1-y);
        for(int i=x;i<x1;i++) for(int j=y;j<y1;j++)
            image(i-x,j-y) = data(i,j);
        x = x1+1;
        return true;
    }

    /**
      Count the number of subimages in a grid:

      * If no argument is given, count the total number of images in the grid,
      irrelevant of current position, i.e. regardless of how many times next() or
      add() have been called.

      * If called as "count(true)" it counts the number of subimages left in grid,
      from current position, x,y.

      this ignores grid's "ncomp", which only counts (at the moment) the number
      of objects which have been added to a grid, not how many objects are
      actually in the grid.
    */
    int count(bool count_remaining=false) {
        int x1=0,y1=0;
        int y2=0;
        int num_subimages=0;

        if(count_remaining)
            x1=x+1,y1=y;
        else
            x1=1,y1=1;

        for(;;) {

            y2=y1;
            while(y2<data.dim(1) && data(0,y2)!=0) y2++;
            if(y2==data.dim(1)) return num_subimages;

            while(x1<data.dim(0) && data(x1,y1-1)!=0) x1++;
            if(x1==data.dim(0)) {
                x1 = 1;
                y1 = y2+1;
            } else {
                x1++;
                num_subimages++;
            }
        }
    }


    int end() {
        /* search for second black pixel in first column from the top, as long
           as the grid is not empty, this pixel indicates the current row in
           grid. */
        int num=0;
        for(int j=data.dim(1); j>=0; j--) {
            if(data(0,j)==0) {
                num++;
                if(num==2) {
                    y=j;
                    break;
                }
            }
        }

        if(num<2) {
            reset();
            return 0;
        }

        // now search for position in row.
        for(int i=1;i<data.dim(0);i++) {
            if(data(i,y)==0) {
                x=i;
            }
        }
        y++;
        return 0;
    }

    bool append(colib::bytearray &image) {
        end();
        return add(image);
    }

    void getLineInformation(colib::bytearray &result) {
        int xorig = x;
        int yorig = y;
        reset();
        next(result);
        x = xorig;
        y = yorig;
    }

    void getLineInformation(int &baseline, int &xheight, int &descender, int &ascender) {
        colib::bytearray a;
        getLineInformation(a);
        int n = a.dim(1);
        int pos[4];
        int blacks_max = sizeof(pos)/sizeof(*pos);
        int blacks_found = 0;
        for (int i = 0; i < n; i++) {
            if (!a.at(0,i)) {
                if (blacks_found == blacks_max)
                    throw "line info contains too many black pixels";

                pos[blacks_found++] = i;
            }
        }

        if (blacks_found < blacks_max)
            throw "line info contains too few black pixels";

        descender = pos[0];
        baseline = pos[1];
        xheight = pos[2] - pos[1];
        ascender = pos[3];
    }
};

}; // namespace

#endif // #ifndef h_grid_
