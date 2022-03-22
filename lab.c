#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

struct file_header {
	char name[128];
	char file_type;
	int size;
};

void print_error(int errornum, char* file_name);

int copy_data(int input_descriptor, int output_decriptor, int size);


int main() {

	exit(0);
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

int copy_data(int input_descriptor, int output_decriptor, int size) {
	char buffer[1024];
	int num_of_readed;
	int kbytes_num = size / 1024;
	char bytes_buff[size % 1024];

	// Сначала пытаемся копировать по кб
	for (int i = 0; i < kbytes_num; i ++) {
		num_of_readed = read(input_descriptor, buffer, sizeof(buffer));
		if (write(output_decriptor, buffer, num_of_readed) == -1) return -1;
	}

	// Оставшуюся часть файла копируем в байтах
	num_of_readed = read(input_descriptor, bytes_buff, sizeof(bytes_buff));
	if (write(output_decriptor, bytes_buff, num_of_readed) == -1) return -2;

	if (num_of_readed == -1) return -2;
	return 0;
}
