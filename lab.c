#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>

#define FILENAME_LENGTH 128
#define ARCHIVE_NAME "archive.arch"
#define DIRECTORY 1
#define SIMPLE_FILE 2

struct file_header {
	char name[FILENAME_LENGTH];
	char file_type;
	int size;
};

void print_error(int errornum, char* file_name);

int copy_data(int input_descriptor, int output_decriptor, int size);

int write_in_archive(int archive_descriptor, char* directory);

int open_archive(int archive_descriptor);

char error_file[FILENAME_LENGTH];

int main() {
	// int out = open("./arch", O_WRONLY | O_CREAT, 0774);
	// if (out == -1) print_error(errno, "./arch");
	// int res = write_in_archive(out, "./papka");
	// if (res == -1) print_error(errno, error_file);

	int arch = open("./arch", O_RDONLY);
	int res = open_archive(arch);

	if (res == -1) print_error(errno, error_file);
	if (res == -2) write(2, "Unexpected file type\0", 21);

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

int write_in_archive(int archive_descriptor, char* directory) {
	DIR* dir;
	struct dirent* entry;
	struct stat file_stat;
	int size = 0;

	if ((dir = opendir(directory)) == NULL) {
		strncpy(error_file, directory, FILENAME_LENGTH);
		error_file[FILENAME_LENGTH - 1] = '\0';
		return -1;
	}
	chdir(directory);

	// Подсчет кол-ва файлов в директории
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp("..", entry->d_name) == 0 || strcmp(".", entry->d_name) == 0) continue;
		size ++;
	}
	seekdir(dir, 0);

	if (errno != 0) {
		strncpy(error_file, directory, FILENAME_LENGTH);
		error_file[FILENAME_LENGTH - 1] = '\0';
		return -1;
	}

	struct file_header dir_header;
	if (strspn(directory, "./") > 0) {
		strncpy(dir_header.name, directory + 2, FILENAME_LENGTH);
	} else {
		strncpy(dir_header.name, directory, FILENAME_LENGTH);
	}
	dir_header.file_type = DIRECTORY;
	dir_header.size = size;

	if (write(archive_descriptor, &dir_header, sizeof(struct file_header)) == -1) {
		strncpy(error_file, ARCHIVE_NAME, FILENAME_LENGTH);
		error_file[FILENAME_LENGTH - 1] = '\0';
		return -1;
	}

	// Проход по файлам директории
	while ((entry = readdir(dir)) != NULL) {
		if (strcmp("..", entry->d_name) == 0 || strcmp(".", entry->d_name) == 0) continue;

		if (stat(entry->d_name, &file_stat) == -1) {
			strncpy(error_file, entry->d_name, FILENAME_LENGTH);
			error_file[FILENAME_LENGTH - 1] = '\0';
			return -1;
		}

		// Если файл - директория, то выполнить рекурсивный вызов
		if (S_ISDIR(file_stat.st_mode)) {
			if (write_in_archive(archive_descriptor, entry->d_name) == -1) return -1;
		} else {
			int input_descriptor = open(entry->d_name, O_RDONLY);
			if (input_descriptor == -1) {
				strncpy(error_file, entry->d_name, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;
			}

			int size_in_bytes = lseek(input_descriptor, 0, SEEK_END);
			lseek(input_descriptor, 0, SEEK_SET);

			struct file_header simple_header;
			strncpy(simple_header.name, entry->d_name, FILENAME_LENGTH);
			simple_header.size = size_in_bytes;
			simple_header.file_type = SIMPLE_FILE;

			if (write(archive_descriptor, &simple_header, sizeof(struct file_header)) == -1) {
				strncpy(error_file, ARCHIVE_NAME, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;
			}

			int copy_res = copy_data(input_descriptor, archive_descriptor, size_in_bytes);
			if (copy_res == -1) {
				strncpy(error_file, ARCHIVE_NAME, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;
			} else if (copy_res == -2) {
				strncpy(error_file, entry->d_name, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;
			}

			if (close(input_descriptor) == -1) {
				strncpy(error_file, entry->d_name, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;
			}
		}
	}
	if (errno != 0) {
		strncpy(error_file, directory, FILENAME_LENGTH);
		error_file[FILENAME_LENGTH - 1] = '\0';
		return -1;
	}

	chdir("..");
	if (closedir(dir) == -1) {
		strncpy(error_file, directory, FILENAME_LENGTH);
		error_file[FILENAME_LENGTH - 1] = '\0';
		return -1;
	}

	return 0;
}

int open_archive(int archive_descriptor) {
	struct file_header header;
	if (read(archive_descriptor, &header, sizeof(header)) == -1) {
		strncpy(error_file, ARCHIVE_NAME, FILENAME_LENGTH);
		error_file[FILENAME_LENGTH - 1] = '\0';
		return -1;	
	}

	switch (header.file_type) {
		case SIMPLE_FILE: {
			int out = open(header.name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP);
			if (out == -1) {
				strncpy(error_file, header.name, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;
			}

			int copy_result = copy_data(archive_descriptor, out, header.size);

			if (copy_result == -1) {
				strncpy(error_file, ARCHIVE_NAME, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;
			} else if (copy_result == -2) {
				strncpy(error_file, header.name, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;
			}

			if (close(out) == -1) {
				strncpy(error_file, header.name, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;
			}
			break;
		}

		case DIRECTORY: {
			if (mkdir(header.name, 0774) == -1) {
				strncpy(error_file, header.name, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;	
			}

			if (chdir(header.name) == -1) {
				strncpy(error_file, header.name, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;	
			}

			for (int i = 0, open_res; i < header.size; i ++) {
				if ((open_res = open_archive(archive_descriptor)) < 0) {
					return open_res;
				}
			}

			if (chdir("..") == -1) {
				strncpy(error_file, header.name, FILENAME_LENGTH);
				error_file[FILENAME_LENGTH - 1] = '\0';
				return -1;	
			}
			break;
		}

		default: {
			return -2;		
		}
	}

	return 0;
}
