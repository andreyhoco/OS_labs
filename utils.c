#include <stdlib.h>

#define INT_64_LEN 20

int free_matrix(unsigned char** matrix, int len) {
	for (int row = 0; row < len; row ++) free(matrix[row]);
	free(matrix);
	return 0;
}
