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

.PHONY: clean
clean:
	rm -f $(obj) server
