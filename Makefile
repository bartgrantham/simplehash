ALL = hash.o hash_test hash_example

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
RT = -lrt
endif

all: $(ALL)

clean:
	rm -f $(ALL)

hash_test: hash.o hash_test.c
	$(CC) hash.o hash_test.c $(RT) -o $@

hash_example: hash.o hash_example.c
	$(CC) hash.o hash_example.c $(RT) -o $@

hash.o: hash.h
