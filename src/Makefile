
TARGET  = negative
SRCS    = neg_main.c        \
          neg_opts.c        \
          neg_file_util.c   \
          neg_rndr.c        \
	  neg_rndr_1pdf.c   \
	  neg_rndr_pdfs.c   \
	  neg_rndr_pngs.c   \
          neg_rsvg.c        \
	  neg_state.c

OBJS    = ${SRCS:%.c=%.o}
DEPS    = ${SRCS:%.c=.%.c.dep}
EDEPS   = $(wildcard ${DEPS})

LIB_NAMES = glib-2.0 librsvg-2.0 cairo-svg libpcre libxml-2.0

MISSING = $(strip $(foreach n,${LIB_NAMES},$(if $(shell pkg-config --modversion --silence-errors ${n}),,${n})))
CCFLAGS = $(foreach n,${LIB_NAMES},$(shell pkg-config --cflags ${n}))
LIBS    = $(foreach n,${LIB_NAMES},$(shell pkg-config --libs ${n}))
CFLAGS  = -Wall -ggdb

.PHONY: all
all: lib_check ${TARGET}

ifneq (,${EDEPS})
include ${EDEPS}
endif

.PHONY: lib_check
lib_check:
	@$(if ${MISSING},$(error "FAILED DEPENDENCY CHECK! Make sure you have: ${MISSING}"))
	@echo "Dependency check successful."

${TARGET}: ${OBJS}
	${CC} -o $@ ${OBJS} ${LIBS}

${OBJS}: %.o: %.c .%.c.dep
	${CC} ${CCFLAGS} ${CFLAGS} -c -o $@ $<

${DEPS}: .%.c.dep: %.c
	${CC} ${CCFLAGS} ${CFLAGS} -MM $< > $@

.PHONY: clean
clean:
	rm -f *.o *~ ${TARGET} .*.c.dep
