all: build

OUT = main

SOURCES = lab.c error_handling.c archive.c

OBJ1 = main.o

SANITIZE_FALGS = -fsanitize=address -fsanitize=leak

build: main.o error_handling.o archive.o
	gcc -o $(OUT) main.o error_handling.o archive.o
	
main.o: lab.c error_handling.h archive.h
	gcc -c -o main.o lab.c
	
error_handling.o:
	gcc -c error_handling.c
	
archive.o:
	gcc -c archive.c
	
clean:
	-rm *.o $(OUT)
	
clean_build: clean build

sanitize_build: main.o error_handling.o archive.o
	gcc -o $(OUT) $(SANITIZE_FLAGS) main.o error_handling.o archive.o
	
check:
	cppcheck --enable=all $(SOURCES)

safe_build: check sanitize_build
