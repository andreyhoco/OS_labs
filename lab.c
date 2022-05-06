#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include "filters.h"
#include "pnm.h"
#include "error_handling.h"
#include "utils.h"

int main(int argc, char* argv[]) {
	char image_name[PATH_MAX];
	char time_name[PATH_MAX];
	char out_name[PATH_MAX];
	int threads_num = 4;
	int image_d;
	int opt;
	int is_tracking = 0;

	memset(image_name, '\0', PATH_MAX);
	memset(time_name, '\0', PATH_MAX);
	memset(out_name, '\0', PATH_MAX);
	strncpy(image_name, "mauer.ppm", strlen("mauer.ppm"));
	strncpy(time_name, "time", strlen("time"));
	strncpy(out_name, "sobel.pbm", strlen("sobel.pbm"));

	while((opt = getopt(argc, argv, "i:o:j:t")) != -1) {
		switch(opt) {
			case 'i': {
				if (strcmp(optarg, "-o") == 0 || strcmp(optarg, "-j") == 0 || strcmp(optarg, "-t") == 0) {
					write(2, "-i: arg expected\n\0", strlen("-i: arg expected\n\0") + 1);
					exit(-1);
				}
				
				strncpy(image_name, optarg, PATH_MAX);
				break;
			}

			case 'o': {
				if (strcmp(optarg, "-i") == 0 || strcmp(optarg, "-j") == 0 || strcmp(optarg, "-t") == 0) {
					write(2, "-o: arg expected\n\0", strlen("-o: arg expected\n\0") + 1);
					exit(-1);
				}
				
				strncpy(out_name, optarg, PATH_MAX);
				break;
			}

			case 'j': {
				if (strcmp(optarg, "-i") == 0 || strcmp(optarg, "-o") == 0 || strcmp(optarg, "-t") == 0) {
					write(2, "-j: arg expected\n\0", strlen("-j: arg expected\n\0") + 1);
					exit(-1);
				}
				
				int number = 0;
				for (int i = 0; i < strlen(optarg); i ++) {
					number = number * 10 + (optarg[i] - '0');
				}
				
				threads_num = number;
				break;
			}

			case 't': {
				is_tracking = 1;
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

	image_d = open(image_name, O_RDONLY);
	if (image_d == -1) {
		print_error(errno, image_name);
		exit(-1);
	}

	struct pnm_header header;
	int offset = load_pnm_header(image_d, &header);

	switch (offset) {
		case -1: {
			close(image_d);
			print_error(errno, image_name);
			exit(-1);
		}

		case FORMAT_ERR: {
			close(image_d);
			print_format_error(image_name);
			exit(-1);
		}
	}

	// Выделяем память для изображения в серых тонах
	unsigned char** grayscale_matrix = malloc(sizeof(unsigned char*) * header.height);

	if (grayscale_matrix == NULL) {
		print_error(errno, "grayscale_matrix");
		close(image_d);
		exit(-1);
	}
	for (int i = 0; i < header.height; i ++) {
		grayscale_matrix[i] = malloc(sizeof(unsigned char) * header.width);

		if (grayscale_matrix[i] == NULL) {
			free_matrix(grayscale_matrix, i);
			print_error(errno, "grayscale_matrix");
			close(image_d);
			exit(-1);
		}
	}

	if (strncmp(header.format, "P6", FORMAT_LEN) == 0) {
		// Выделяем память для цветного изображения
		unsigned char*** image_matrix = malloc(sizeof(unsigned char**) * header.height);

		if (image_matrix == NULL) {
			free_matrix(grayscale_matrix, header.height);

			print_error(errno, "rgb matrix");
			close(image_d);
			exit(-1);
		}
		for (int i = 0; i < header.height; i ++) {
			image_matrix[i] = malloc(sizeof(char*) * header.width);

			if (image_matrix[i] == NULL) {
				free_matrix(grayscale_matrix, header.height);

				for (int row = 0; row < i; row ++) {
					for (int column = 0; column < header.width; column ++) free(image_matrix[row][column]);
					free(image_matrix[row]);
				}
				free(image_matrix);
				print_error(errno, "rgb matrix");
				close(image_d);
				exit(-1);
			}

			for (int j = 0; j < header.width; j ++) {
				image_matrix[i][j] = malloc(sizeof(unsigned char) * BYTES_PER_PX);

				if (image_matrix[i][j] == NULL) {
					free_matrix(grayscale_matrix, header.height);

					for (int row = 0; row <= i; row ++) {
						if (row == i) {
							for (int column = 0; column < j; column ++) free(image_matrix[row][column]);
						} else {
							for (int column = 0; column < header.width; column ++) free(image_matrix[row][column]);
						}
						free(image_matrix[row]);
					}
					free(image_matrix);
					print_error(errno, "rgb matrix");
					close(image_d);
					exit(-1);
				}
			}
		}

		int result;
		if ((result = load_ppm(image_d, image_matrix, &header)) != 0) {
			if (result == FORMAT_ERR) print_format_error(image_name);
			else print_error(errno, image_name);
			
			free_matrix(grayscale_matrix, header.height);

			for (int row = 0; row < header.height; row ++) {
				for (int column = 0; column < header.width; column ++) free(image_matrix[row][column]);
				free(image_matrix[row]);
			}
			free(image_matrix);
			print_error(errno, image_name);
			close(image_d);
			exit(-1);
		}

		rgb_to_grayscale(image_matrix, grayscale_matrix, header.height, header.width);

		for (int row = 0; row < header.height; row ++) {
			for (int column = 0; column < header.width; column ++) free(image_matrix[row][column]);
			free(image_matrix[row]);
		}
		free(image_matrix);
	} else {
		int result;
		if ((result = load_pgm(image_d, grayscale_matrix, &header)) != 0) {
			if (result == FORMAT_ERR) print_format_error(image_name);
			else print_error(errno, image_name);

			free_matrix(grayscale_matrix, header.height);
			close(image_d);
			exit(-1);
		}
	}

	if (close(image_d) == -1) print_error(errno, image_name);

	unsigned char** sobel_matrix = malloc(sizeof(unsigned char*) * header.height);

	if (sobel_matrix == NULL) {
		free_matrix(grayscale_matrix, header.height);

		print_error(errno, "sobel matrix");
		close(image_d);
		exit(-1);
	}

	for (int i = 0; i < header.height; i ++) {
		sobel_matrix[i] = malloc(header.width * sizeof(unsigned char));

		if (sobel_matrix[i] == NULL) {
			free_matrix(sobel_matrix, i);
			free_matrix(grayscale_matrix, header.height);
			
			print_error(errno, "sobel matrix");
			exit(-1);
		}
	}

	if (threads_num > (header.height - 2)) threads_num = header.height - 2;

	pthread_t** threads = malloc(sizeof(pthread_t*) * threads_num);
	if (threads == NULL) {
		free_matrix(sobel_matrix, header.height);
		free_matrix(grayscale_matrix, header.height);
		print_error(errno, "memory for threads");

		exit(-1);
	}

	struct sobel_params** parameters = malloc(sizeof(struct sobel_params*) * threads_num);
	if (threads == NULL) {
		free(threads);
		free_matrix(sobel_matrix, header.height);
		free_matrix(grayscale_matrix, header.height);
		print_error(errno, "memory for threads params");

		exit(-1);
	}

	for (int i = 0; i < threads_num; i ++) {
		threads[i] = malloc(sizeof(pthread_t));
		parameters[i] = malloc(sizeof(struct sobel_params));

		if (threads[i] == NULL || parameters[i] == NULL) {
			free_matrix(sobel_matrix, header.height);
			free_matrix(grayscale_matrix, header.height);

			for (int k = 0; k < i; k ++) {
				free(threads[k]);
				free(parameters[k]);
			}

			if (threads[i] == NULL) {
				free(parameters[i]);
				print_error(errno, "memory for threads");
			}
			if (parameters[i] == NULL) {
				free(threads[i]);
				print_error(errno, "memory for threads params");
			}
			free(threads);
			free(parameters);

			exit(-1);
		}
	}

	for (int i = 0; i < threads_num; i ++) {
		parameters[i]->source = grayscale_matrix;
		parameters[i]->dest = sobel_matrix;
		parameters[i]->from_x = 1;
		parameters[i]->from_y = header.height / threads_num * i + 1;
		parameters[i]->to_x = header.width - 2;
		parameters[i]->to_y = i != (threads_num - 1) ? header.height / threads_num * (i + 1) : header.height - 2;
	}

	int start = clock();

	for (int i = 0; i < threads_num; i ++) {
		if (pthread_create(threads[i], NULL, concurency_sobel, parameters[i]) != 0) {
			free_matrix(sobel_matrix, header.height);
			free_matrix(grayscale_matrix, header.height);

			for (int k = 0; k < threads_num; k ++) {
				free(threads[k]);
				free(parameters[k]);
			}
			free(threads);
			free(parameters);

			print_error(errno, "create thread");
			exit(-1);
		}
	}

	void* res = NULL;
	for (int i = 0; i < threads_num; i ++) {
		if (pthread_join(*(threads[i]), res) != 0) {
			free_matrix(sobel_matrix, header.height);
			free_matrix(grayscale_matrix, header.height);

			for (int k = 0; k < threads_num; k ++) {
				free(threads[k]);
				free(parameters[k]);
			}
			free(threads);
			free(parameters);

			print_error(errno, "join thread");
			exit(-1);
		}
	}

	int end = clock();
	free_matrix(grayscale_matrix, header.height);
	for (int k = 0; k < threads_num; k ++) {
		free(threads[k]);
		free(parameters[k]);
	}
	free(threads);
	free(parameters);

	double time = ((double)(end - start) / CLOCKS_PER_SEC);
	char str_time[INT_64_LEN + 2];
	double_to_str(str_time, time, 5);

	struct pnm_header sobel_header;
	strncpy(sobel_header.format, "P5", FORMAT_LEN);
	sobel_header.width = header.width;
	sobel_header.height = header.height;
	sobel_header.color_depth = header.color_depth;


	int out = open(out_name, O_WRONLY | O_CREAT, 0664);
	if (out == -1) {
		free_matrix(sobel_matrix, header.height);

		print_error(errno, out_name);
		close(image_d);
		exit(-1);
	}

	int result = write_pgm_in_file(out, sobel_matrix, &sobel_header);

	free_matrix(sobel_matrix, header.height);
	close(out);
	close(image_d);

	switch (result) {
		case -1: {
			print_error(errno, out_name);
			exit(-1);
		}

		case FORMAT_ERR: {
			print_format_error("image format");
			exit(-1);
		}
	}

	if (is_tracking) {
		int time_d = open(time_name, O_WRONLY | O_CREAT | O_APPEND, 0664);
		if (time_d == -1) {
			print_error(errno, time_name);
			write(1, "processing time: ", strlen("processing time: "));
			write(1, str_time, strlen(str_time));
		} else {
			if (write(time_d, str_time, strlen(str_time)) == -1) {
				print_error(errno, time_name);
				write(1, "processing time: ", strlen("processing time: "));
				write(1, str_time, strlen(str_time));
			}
			write(time_d, "\n", strlen("\n"));
			close(time_d);
		}
	}

	exit(0);
}
