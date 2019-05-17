EXTRA_TESTS:= memgrind-dbg testabc
all: memgrind
#all: memgrind $(EXTRA_TESTS)
memgrind-dbg: memgrind.c mymalloc.c mymalloc.h
	gcc -Wall -O2 $(filter %.c,$^) -o $@
memgrind: memgrind.c mymalloc.c mymalloc.h
	gcc -Wall -O2 -DNDEBUG $(filter %.c,$^) -o $@
testabc: testabc.c mymalloc.c mymalloc.h
	gcc -Wall -DNDEBUG -O2 $(filter %.c,$^) -o $@
clean:
	rm -f x.log memgrind memgrind-dbg testabc
realclean: clean
#last line
