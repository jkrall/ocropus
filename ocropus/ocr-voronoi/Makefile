#$Id: Makefile,v 1.1.1.1 1999/10/15 12:40:27 kise Exp $
CC	      = g++ -Wall
#OPT	      = -O2
#OPT	      = -pg
OPT	      = -g
LIBS	      = -lm -ltiff
DEFS	      = -DTIME 
INCLUDE_DIRS   = -I/usr/local/include
LIB_DIRS       = -L/usr/local/lib
CFLAGS	      = $(INCLUDE_DIRS) ${LIB_DIRS} ${OPT} ${DEFS}


HDRS	      = const.h \
		defs.h \
		extern.h \
		function.h \
		read_image.h

OBJS	      = img_to_site.o \
		bit_func.o \
		cline.o \
		dinfo.o \
		draw_line.o \
		label_func.o \
		edgelist.o \
		erase.o \
		geometry.o \
		hash.o \
		heap.o \
		main-be.o \
		memory.o \
		output.o \
		read_image.o \
		sites.o \
		usage.o \
		voronoi.o \
		voronoi-pageseg.o

all: be
	$(MAKE) dl --directory=drawing

be:		${OBJS}
		${CC} -o $@ ${OBJS} ${LIBS} ${CFLAGS}

${OBJS}:	${HDRS}

clean:;		@rm -f *.o core a.out be
	$(MAKE) clean --directory=drawing
