DEST=
PREFIX=/usr

-include config.mk

.PHONY: all clean
all clean:
	${MAKE} -C src $@

.PHONY: lcl-clean
clean: lcl-clean
lcl-clean:
	rm -f *~

.PHONY: install
install: all
	install -D --mode=0755 src/negative ${DEST}${PREFIX}/bin/negative
