.PHONY: all clean
all clean:
	${MAKE} -C src $@

.PHONY: lcl-clean
clean: lcl-clean
lcl-clean:
	rm -f *~
