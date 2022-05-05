#include <unistd.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "filters.h"
#include "pnm.h"
#include "error_handling.h"
#include "utils.h"

int main() {
	char image_name[PATH_MAX];
	char out_name[PATH_MAX];
	int image_d;

	memset(image_name, '\0', PATH_MAX);
	memset(out_name, '\0', PATH_MAX);
	strncpy(image_name, "mauer.ppm", strlen("mauer.ppm"));
	strncpy(out_name, "sobel.pbm", strlen("sobel.pbm"));

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

	apply_sopel_op(grayscale_matrix, sobel_matrix, 1, 1, header.width - 2, header.height - 2);

	struct pnm_header sobel_header;
	strncpy(sobel_header.format, "P5", FORMAT_LEN);
	sobel_header.width = header.width;
	sobel_header.height = header.height;
	sobel_header.color_depth = header.color_depth;

	free_matrix(grayscale_matrix, header.height);

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

	exit(0);
}