obj = $(src:.c=.o)

CC = gcc
CFLAGS = -Wall -I../
LDFLAGS = -lnsl

client: simple_client mr_mit partial_client

# header_client:
# 	$(CC) header_client.c -o header_client.out $(LDFLAGS)

# message_client: message_client.o dispatch.o
# 	$(CC) message_client.o dispatch.o -o message_client.out $(LDFLAGS)

# message_client.o: message_client.c ../dispatch.h
# 	$(CC) $(CFLAGS) -c message_client.c -o message_client.o

simple_client: simple_client.c
	$(CC) simple_client.c -o simple_client.out $(LDFLAGS)

mr_mit: test_client.c
	$(CC) test_client.c -o mr_mit.out $(LDFLAGS)

partial_client: partial_client.c
	$(CC) partial_client.c -o partial_client.out $(LDFLAGS)



.PHONY: clean
clean:
	rm -f $(obj) *.out
