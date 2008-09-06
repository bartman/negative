
TARGET  = negative
SRCS    = neg_main.c        \
          neg_opts.c        \
          neg_file_util.c   \
          neg_rndr.c        \
	  neg_rndr_1pdf.c   \
	  neg_rndr_pdfs.c   \
	  neg_rndr_pngs.c   \
	  neg_state.c

OBJS    = ${SRCS:%.c=%.o}
DEPS    = ${SRCS:%.c=.%.c.dep}
EDEPS   = $(wildcard ${DEPS})

CCFLAGS = -I/usr/include/cairo \
	  -I/usr/include/gtk-2.0 \
	  -I/usr/include/glib-2.0 \
	  -I/usr/lib/glib-2.0/include/ \
	  -I/usr/include/librsvg-2/librsvg/
CFLAGS  = -Wall -O2 -ggdb

LIBS    = -lrsvg-2 -lcairo

.PHONY: all
all: ${TARGET}

ifneq (,${EDEPS})
include ${EDEPS}
endif

${TARGET}: ${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS}

${OBJS}: %.o: %.c .%.c.dep
	${CC} ${CCFLAGS} ${CFLAGS} -c -o $@ $<

${DEPS}: .%.c.dep: %.c
	${CC} ${CCFLAGS} ${CFLAGS} -MM $< > $@

.PHONY: clean
clean:
	rm -f *.o *~ ${TARGET} .*.c.dep
