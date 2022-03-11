#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

void print_error(int errornum, char* file_name) {
	char *error_description = strerror(errornum);
	char error_msg[50];
	strncpy(error_msg, file_name, 48);
	strncat(error_msg, ": ", 48 - strlen(error_msg));
	strncat(error_msg, error_description, 48 - strlen(error_msg));
	strncat(error_msg, "\n\0", 50 - strlen(error_msg));
	write(2, error_msg, strlen(error_msg) + 1);
}

int main() {
	exit(0);
}