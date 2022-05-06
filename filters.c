#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define R_COMPRESSION 0.297
#define G_COMPRESSION 0.585
#define B_COMPRESSION 0.11

struct sobel_params {
	unsigned char** source;
	unsigned char** dest;
	int from_x;
	int from_y;
	int to_x;
	int to_y;
};

int rgb_to_grayscale(char*** source, unsigned char** container, int h, int w) {
	for (int rownum = 0; rownum < h; rownum ++) {
		for (int columnnum = 0; columnnum < w; columnnum ++) {
			int px = source[rownum][columnnum][0] * R_COMPRESSION +
			 source[rownum][columnnum][1] * G_COMPRESSION +
			  source[rownum][columnnum][2] * B_COMPRESSION;

			if (px > 255) px = 255;
			container[rownum][columnnum] = (unsigned char) px;
		}
	}
	return 0;
}

int apply_sobel_op(unsigned char** source, unsigned char** container, int from_x, int from_y, int to_x, int to_y) {
	for (int row = from_y; row <= to_y; row ++) {
		for (int column = from_x; column <= to_x; column ++) {
			int gx = (source[row + 1][column - 1] + 2 * source[row + 1][column] + source[row + 1][column + 1]) -
			 (source[row - 1][column - 1] + 2 * source[row - 1][column] + source[row - 1][column + 1]);
		 	int gy = (source[row - 1][column + 1] + 2 * source[row][column + 1] + source[row + 1][column + 1]) - 
		 	 (source[row - 1][column - 1] + 2 * source[row][column - 1] + source[row + 1][column - 1]);

		 	int m = sqrt(pow(gx, 2) + pow(gy, 2));
		 	if (m > 255) m = 255;

	 		container[row][column] = (unsigned char) m;
		}
	}
	return 0;
}

void* concurency_sobel(void* params) {
	struct sobel_params* curr_params = (struct sobel_params*) params;
	apply_sobel_op(curr_params->source, curr_params->dest, curr_params->from_x, curr_params->from_y, curr_params->to_x, curr_params->to_y);
	pthread_exit(NULL);
}
