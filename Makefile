ALL = hash.o hash_test hash_example

all: $(ALL)

clean:
	rm -f $(ALL)

hash_test: hash.o hash_test.c
	$(CC) $^ -o $@

hash_example: hash.o hash_example.c
	$(CC) $^ -o $@

hash.o: hash.h
