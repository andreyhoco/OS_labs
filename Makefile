all: build

SANITIZE_FALGS = -fsanitize=address -fsanitize=leak

SOURCES = lab.c filters.c pnm.c error_handling.c utils.c

build: main.o filters.o pnm.o error_handling.o utils.o
	gcc -D_REENTRANT -o main main.o filters.o pnm.o error_handling.o utils.o -lm -lpthread
	
main.o: lab.c filters.h pnm.h error_handling.h utils.h
	gcc -c -o main.o lab.c
	
error_handling.o: error_handling.c
	gcc -c error_handling.c
	
filters.o: filters.c
	gcc -c filters.c
	
pnm.o: pnm.c
	gcc -c pnm.c

utils.o: utils.c
	gcc -c utils.c

sanitize_build: main.o filters.o pnm.o error_handling.o utils.o
	gcc -o main $(SANITIZE_FLAGS) main.o filters.o pnm.o error_handling.o utils.o -lm
	
check:
	cppcheck --enable=all $(SOURCES)

safe_build: check sanitize_build
