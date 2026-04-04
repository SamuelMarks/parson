CC = gcc
CFLAGS = -O0 -g -Wall -Wextra -std=c89 -pedantic-errors -DTESTS_MAIN

CPPC = g++
CPPFLAGS = -O0 -g -Wall -Wextra -DTESTS_MAIN 

all: test testcpp test_hash_collisions

.PHONY: test testcpp test_hash_collisions
test: tests/tests.c parson.c
	$(CC) $(CFLAGS) -I. -o $@ tests/tests.c parson.c
	./$@

testcpp: tests/tests.c parson.c
	$(CPPC) $(CPPFLAGS) -I. -o $@ tests/tests.c parson.c
	./$@

test_hash_collisions: tests/tests.c parson.c
	$(CC) $(CFLAGS) -DPARSON_FORCE_HASH_COLLISIONS -I. -o $@ tests/tests.c parson.c
	./$@
clean:
	rm -f test *.o

