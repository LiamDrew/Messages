OS := $(shell uname)

src = $(wildcard *.c)
obj = $(src:.c=.o)
CC = clang
LDFLAGS =

ifeq ($(OS),Linux)
	LDFLAGS += -lnsl
endif

CFLAGS = -Wall

server: $(obj)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean test testone
clean:
	rm -f $(obj) server

testone:
	make -C tests > /dev/null
	tests/one.sh

test:
	make -C tests > /dev/null
	tests/test.sh
