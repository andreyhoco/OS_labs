#include <unistd.h>
#include <stdlib.h>

int main() {
	char* msg = "Hello world!\n\0";
	write(1, msg, 14);
	exit(0);
}