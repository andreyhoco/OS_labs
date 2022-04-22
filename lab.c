#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "error_handling.h"
#include "utils.h"

int main(int argc, char* argv[]) {
	int opt;
	char input_data[INPUT_MAX + 1];
	char key_data[INPUT_MAX + 1];
	char output_file[INPUT_MAX];
	char gamma_file[INPUT_MAX];

	strncpy(output_file, "encrypted_data", sizeof("encrypted_data"));
	strncpy(gamma_file, "gamma", sizeof("gamma"));
	memset(input_data, '\0', INPUT_MAX);
	memset(key_data, '\0', INPUT_MAX);

	while((opt = getopt(argc, argv, "d:k:o:g:")) != -1) {
		switch(opt) {
			case 'd': {
				if ((strcmp(optarg, "-k") == 0) || (strcmp(optarg, "-o") == 0)) {
					write(2, "-d: arg expected\n\0", strlen("-d: arg expected\n\0") + 1);
					exit(-1);
				}
				
				strncpy(input_data, optarg, INPUT_MAX);
				int null_pos = MIN_INT(strlen(optarg), INPUT_MAX);
				input_data[null_pos] = '\0';
				break;
			}

			case 'k': {
				if ((strcmp(optarg, "-d") == 0) || (strcmp(optarg, "-o") == 0) || (strcmp(optarg, "-g") == 0)) {
					write(2, "-k: arg expected\n\0", strlen("-k: arg expected\n\0") + 1);
					exit(-1);
				}

				strncpy(key_data, optarg, INPUT_MAX);
				int null_pos = MIN_INT(strlen(optarg), INPUT_MAX);
				key_data[null_pos] = '\0';
				break;
			}

			case 'o': {
				if ((strcmp(optarg, "-d") == 0) || (strcmp(optarg, "-k") == 0) || (strcmp(optarg, "-g") == 0) || (strcmp(optarg, "") == 0)){
					write(2, "-o: arg expected\n\0", strlen("-o: arg expected\n\0") + 1);
					exit(-1);
				}

				strncpy(output_file, optarg, INPUT_MAX);
				break;
			}

			case 'g': {
				if ((strcmp(optarg, "-d") == 0) || (strcmp(optarg, "-k") == 0) || (strcmp(optarg, "-o") == 0) || (strcmp(optarg, "") == 0)){
					write(2, "-g: arg expected\n\0", strlen("-g: arg expected\n\0") + 1);
					exit(-1);
				}

				strncpy(gamma_file, optarg, INPUT_MAX);
				break;
			}

			case '?': {
				write(2, "Bad options\n\0", strlen("Bad options\n\0") + 1);
				exit(-1);
			}
		}
	}

	if (argc == 1) {
		write(2, "Options expected\n\0", strlen("Options expected\n\0") + 1);
		exit(-1);
	}

	if (input_data[0] == '\0' || key_data[0] == '\0') {
		write(2, "Bad options\n\0", strlen("Bad options\n\0") + 1);
		exit(-1);
	}

	char **input_args = calloc(INPUT_MAX / 2 + 1, sizeof(char*));
	char **key_args = calloc(INPUT_MAX / 2 + 1, sizeof(char*));

	int input_args_count;
	int key_args_count;

	if ((input_args_count = parse(input_args, input_data)) == -1) {
		print_error(errno, input_data);
		exit(-1);
	}
	if ((key_args_count = parse(key_args, key_data)) == -1) {
		print_error(errno, key_data);
		exit(-1);	
	}

	int input_pipes[2];
	if (pipe(input_pipes) == -1) {
		print_error(errno, "input pipes");
		free_args(input_args, input_args_count);
		free_args(key_args, key_args_count);
		exit(-1);
	}

	pid_t input_fork = fork();
	switch(input_fork) {
		case -1: {
			free_args(input_args, input_args_count);
			free_args(key_args, key_args_count);
			print_error(errno, "input fork");
			exit(-1);
		}

		case 0: {
			dup2(input_pipes[1], 1);
			close(input_pipes[0]);
			close(input_pipes[1]);

			if (errno != 0) {
				print_error(errno, input_data);
				free_args(input_args, input_args_count);
				free_args(key_args, key_args_count);
				exit(-1);
			}

			if (execvp(input_args[0], input_args) == -1) {
				print_error(errno, input_data);
				free_args(input_args, input_args_count);
				free_args(key_args, key_args_count);
				exit(-1);
			}
			break;
		}
	}

	free_args(input_args, input_args_count);
	if (close(input_pipes[1]) == -1) {
		print_error(errno, input_data);
		free_args(key_args, key_args_count);
		exit(-1);
	}

	int key_pipes[2];
	if (pipe(key_pipes) == -1) {
		print_error(errno, "key pipes");
		free_args(key_args, key_args_count);
		exit(-1);
	}

	pid_t key_fork = fork();
	switch(key_fork) {
		case -1: {
			free_args(key_args, key_args_count);
			print_error(errno, "key fork");
			exit(-1);
		}

		case 0: {
			dup2(key_pipes[1], 1);
			close(key_pipes[0]);
			close(key_pipes[1]);

			if (errno != 0) {
				print_error(errno, key_data);
				free_args(key_args, key_args_count);
				exit(-1);
			}

			if (execvp(key_args[0], key_args) == -1) {
				print_error(errno, key_data);
				free_args(key_args, key_args_count);
				exit(-1);
			}
			break;
		}
	}
	free_args(key_args, key_args_count);
	if (close(key_pipes[1]) == -1) {
		print_error(errno, key_data);
	}

	int gamma;
	if ((gamma = open(gamma_file, O_RDWR | O_CREAT, 0664)) == -1) {
		print_error(errno, gamma_file);
		exit(-1);
	}

	char buff[1024];
	int num_of_readed;
	while ((num_of_readed = read(key_pipes[0], buff, sizeof(buff))) > 0) {
		if (write(gamma, buff, num_of_readed) == -1) {
			print_error(errno, gamma_file);
			close(gamma);
			close(input_pipes[0]);
			close(key_pipes[0]);
			exit(-1);
		}
	}
	if (num_of_readed == -1) {
		print_error(errno, "key pipe");
		close(gamma);
		close(input_pipes[0]);
		close(key_pipes[0]);
		exit(-1);
	}
	close(key_pipes[0]);

	if (lseek(gamma, 0, SEEK_SET) == -1) {
		print_error(errno, gamma_file);
		close(gamma);
		close(input_pipes[0]);
		exit(-1);
	}

	int output;
	if ((output = open(output_file, O_WRONLY | O_CREAT, 0664)) == -1) {
		print_error(errno, output_file);
		exit(-1);
	}

	int res = encrypt_data(input_pipes[0], gamma, output);
	switch (res) {
		case -1: {
			print_error(errno, "input pipe");
			close(output);
			close(input_pipes[0]);
			close(gamma);
			exit(-1);
		}

		case -2: {
			print_error(errno, "key pipe");
			close(output);
			close(input_pipes[0]);
			close(gamma);
			exit(-1);
		}

		case -3: {
			print_error(errno, output_file);
			close(output);
			close(input_pipes[0]);
			close(gamma);
			exit(-1);
		}
	}

	close(output);
	close(gamma);
	close(input_pipes[0]);

	exit(0);
}
