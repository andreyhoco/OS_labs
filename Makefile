all: build
	
build: lab.o utils.o error_handling.o
	gcc -o main lab.o utils.o error_handling.o
	
lab.o: lab.c
	gcc -c lab.c
	
utils.o: utils.c
	gcc -c utils.c
	
error_handling.o: error_handling.c
	gcc -c error_handling.c

sanitize_build: lab.o utils.o error_handling.o
	gcc -o main lab.c -fsanitize=address -fsanitize=leak utils.o error_handling.o
