all: build
	
build: lab.o term_utils.o error_handling.o
	gcc -o main lab.o term_utils.o error_handling.o
	
lab.o: lab.c
	gcc -c lab.c
	
term_utils.o: term_utils.c
	gcc -c term_utils.c
	
error_handling.o: error_handling.c
	gcc -c error_handling.c	

sanitize_build: lab.o term_utils.o error_handling.o
	gcc -o main lab.c -fsanitize=address -fsanitize=leak term_utils.o error_handling.o
