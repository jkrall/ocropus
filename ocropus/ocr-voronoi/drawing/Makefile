# $Id: Makefile,v 1.1.1.1 1999/10/15 12:26:21 kise Exp $
CC	      = gcc -Wall

LIBS	      = -lm -ltiff
OPT	      = -O3
#OPT	      = -g
DEFS	      =
INCLUDE_DIRS   = -I/usr/local/include
LIB_DIRS       = -L/usr/local/lib
CFLAGS	      = $(INCLUDE_DIRS) ${LIB_DIRS} ${OPT} ${DEFS}



HDRS	      = color.h read_image.h

OBJS	      = util.o \
		draw_line.o \
		draw_point.o \
		draw_rect.o \
		make_map.o \
		main.o \
		read_image.o \
		write_image.o
dl:		${OBJS}
		${CC} ${CFLAGS} ${OBJS} ${LIBS} -o $@

${OBJS}:	${HDRS}

clean:;		@rm -f ${OBJS} a.out dl core
