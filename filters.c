#include <stdlib.h>

#define R_COMPRESSION 0.297
#define G_COMPRESSION 0.585
#define B_COMPRESSION 0.11

int rgb_to_grayscale(char*** source, unsigned char** container, int h, int w) {
	for (int rownum = 0; rownum < h; rownum ++) {
		for (int columnnum = 0; columnnum < w; columnnum ++) {
			container[rownum][columnnum] = source[rownum][columnnum][0] * R_COMPRESSION +
			 source[rownum][columnnum][1] * G_COMPRESSION +
			  source[rownum][columnnum][2] * B_COMPRESSION;
		}
	}
	return 0;
}