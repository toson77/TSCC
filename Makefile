CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

tscc: $(OBJS)
		$(CC) -o tscc $(OBJS) $(LDFLAGS)

$(OBJS): tscc.h

test: tscc
		./test.sh

clean:
		rm -f tscc *.0 *~ tmp*
.PHONY: test clean
