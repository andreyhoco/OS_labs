#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "error_handling.h"
#include "archive.h"

#define ARCHIVE_NAME "./archive.arch"
#define MODE_ARCH 1
#define MODE_UNARCH 2
#define SKIP_ARG 1

int main(int argc, char* argv[]) {
	char mode = 0;
	int res;
	int opt;

	char archive_path[FILENAME_LENGTH];
	char directory_path[FILENAME_LENGTH];
	char input_dir[FILENAME_LENGTH];

	strncpy(archive_path, ARCHIVE_NAME, strlen(ARCHIVE_NAME));
	strncpy(directory_path, ".", strlen("."));

	while((opt = getopt(argc, argv, "d:a:o:")) != -1) {
		switch(opt) {
			case 'd': {
				if (mode != 0) {
					write(2, "Illegal opt -d\n\0", strlen("Illegal opt -a\n\0") + 1);
					exit(-1);
				}
				mode = MODE_ARCH;

				if (strcmp(optarg, "-a") == 0 || strcmp(optarg, "-o") == 0) {
					write(2, "-d: arg expected\n\0", strlen("-d: arg expected\n\0") + 1);
					exit(-1);
				}
				
				strncpy(input_dir, optarg, FILENAME_LENGTH);
				break;
			}

			case 'a': {
				if (mode != 0) {
					write(2, "Illegal opt -a\n\0", strlen("Illegal opt -a\n\0") + 1);
					exit(-1);
				} 

				mode = MODE_UNARCH;
				if (strcmp(optarg, "-d") == 0 || strcmp(optarg, "-o") == 0) {
					write(2, "-a: arg expected\n\0", strlen("-a: arg expected\n\0") + 1);
					exit(-1);
				}

				strncpy(archive_path, optarg, FILENAME_LENGTH);
				break;
			}

			case 'o': {
				if (strcmp(optarg, "-d") == 0 || strcmp(optarg, "-a") == 0) {
					write(2, "-o: arg expected\n\0", strlen("-o: arg expected\n\0") + 1);
					exit(-1);
				}

				if (mode == MODE_ARCH) {
					strncpy(archive_path, optarg, FILENAME_LENGTH);
				} else if (mode == MODE_UNARCH) {
					strncpy(directory_path, optarg, FILENAME_LENGTH);
				}
				break;
			}
		}
	}

	if (argc == 1) {
		write(2, "Options expected\n\0", strlen("Options expected\n\0") + 1);
		exit(-1);
	}

	switch (mode) {
		case MODE_ARCH: {
			int output = open(archive_path, O_WRONLY | O_CREAT, 0664);
			if (output == -1) {
				print_error(errno, archive_path);
				exit(-1);
			}

			res = write_in_archive(output, input_dir);
			if (res == -1) {
				print_error(errno, error_file);

				if (close(output) == -1) print_error(errno, archive_path);
				if (unlink(archive_path) == -1) print_error(errno, archive_path);
				exit(-1);
			}

			if (close(output)) {
				print_error(errno, archive_path);
				exit(-1);
			}
			break;
		}

		case MODE_UNARCH: {
			int archive_descriptor = open(archive_path, O_RDONLY);
			if (archive_descriptor == -1) {
				print_error(errno, archive_path);
				exit(-1);
			}

			if (chdir(directory_path) == -1) {
				if (errno == 2) {
					if (mkdir(directory_path, 0775) == -1) {
						print_error(errno, directory_path);
						exit(-1);
					}

					if (chdir(directory_path) == -1) {
						print_error(errno, directory_path);
						exit(-1);
					}
				} else {
					print_error(errno, directory_path);
				exit(-1);
				}
			}

			res = open_archive(archive_descriptor);

			if (res == -1) {
				print_error(errno, error_file);
				exit(-1);
			} else if (res == -2) {
				print_type_error(error_file);
				exit(-1);
			}

			if (close(archive_descriptor)) {
				print_error(errno, archive_path);
				exit(-1);
			}
			break;
		}

		default: {
			write(2, "Bad options\n\0", strlen("Bad options\n\0") + 1);
			exit(-1);
		}
	}

	exit(0);
}
