#include <unistd.h>
#include <string.h>

#define MSG_LEN 300
#define ERROR_LEN 150

void print_type_error(char* file_name) {
	char msg[MSG_LEN];
	strncpy(msg, file_name, strlen(file_name));
	strncat(msg, ": unexpected file type\n\0", MSG_LEN - strlen(": unexpected file type\n\0") + 1);
	write(2, msg, strlen(msg) + 1);
}

void print_error(int errornum, char* file_name) {
	char *error_description = strerror(errornum);
	char error_msg[ERROR_LEN];
	strncpy(error_msg, file_name, ERROR_LEN - 2);
	strncat(error_msg, ": ", ERROR_LEN - 2 - strlen(error_msg));
	strncat(error_msg, error_description, ERROR_LEN - 2 - strlen(error_msg));
	strncat(error_msg, "\n\0", ERROR_LEN - strlen(error_msg));
	write(2, error_msg, strlen(error_msg) + 1);
}

