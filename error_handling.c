#include <unistd.h>
#include <string.h>

void print_type_error(char* file_name) {
	char msg[150];
	strncpy(msg, file_name, strlen(file_name));
	strncat(msg, ": unexpected file type\n\0", 150 - strlen(": unexpected file type\n\0") + 1);
	write(2, msg, strlen(msg) + 1);
}

void print_error(int errornum, char* file_name) {
	char *error_description = strerror(errornum);
	char error_msg[50];
	strncpy(error_msg, file_name, 48);
	strncat(error_msg, ": ", 48 - strlen(error_msg));
	strncat(error_msg, error_description, 48 - strlen(error_msg));
	strncat(error_msg, "\n\0", 50 - strlen(error_msg));
	write(2, error_msg, strlen(error_msg) + 1);
}

