OS := $(shell uname)

obj = $(src:.c=.o)
CC = clang
CFLAGS = -Wall -I../
LDFLAGS = 

ifeq ($(OS),Linux)
	LDFLAGS += -lnsl
endif

client: simple_client partial_client
# client: simple_client mr_mit partial_client

# mr_mit: test_client.c
# 	$(CC) test_client.c -o mr_mit.out $(LDFLAGS)

simple_client: simple_client.c
	$(CC) simple_client.c -o simple_client.out $(LDFLAGS)

partial_client: partial_client.c
	$(CC) partial_client.c -o partial_client.out $(LDFLAGS)
.PHONY: clean
clean:
	rm -f $(obj) *.out
