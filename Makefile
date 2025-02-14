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

.PHONY: clean test testall
clean:
	rm -f $(obj) server

test:
	make -C tests > /dev/null
	tests/one.sh
# change this to test.sh eventually

testall:
	make -C tests > /dev/null
	tests/test.sh
