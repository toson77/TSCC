CFLAGS=-std=c11 -g -static
tscc: tscc.c
test: tscc
		./test.sh
clean:
		rm -f tscc *.0 *~ tmp*
.PHONY: test clean
