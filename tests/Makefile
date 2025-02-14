OS := $(shell uname)

obj = $(src:.c=.o)
CC = clang
CFLAGS = -Wall -I../
LDFLAGS = 

ifeq ($(OS),Linux)
	LDFLAGS += -lnsl
endif

client: simple_client partial_client

simple_client: simple_client.c
	$(CC) simple_client.c -o simple_client.out $(LDFLAGS)

partial_client: partial_client.c
	$(CC) partial_client.c -o partial_client.out $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) *.out
