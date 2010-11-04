ALL = hash.o hash_test hash_example

all: $(ALL)

clean:
	rm -f $(ALL)

hash_test: hash.o hash_test.c
	$(CC) hash.o hash_test.c -o $@

hash_example: hash.o hash_example.c
	$(CC) hash.o hash_example.c -o $@

hash.o: hash.h
