all: build
	
build: lab.o term_utils.o
	gcc -o main lab.o term_utils.o
	
lab.o: lab.c
	gcc -c lab.c
	
term_utils.o: term_utils.c
	gcc -c term_utils.c
	
sanitize_build: lab.o term_utils.o
	gcc -o main lab.c -fsanitize=address -fsanitize=leak term_utils.o
