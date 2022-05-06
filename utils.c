#include <stdlib.h>

#define INT_64_LEN 20

int free_matrix(unsigned char** matrix, int len) {
	for (int row = 0; row < len; row ++) free(matrix[row]);
	free(matrix);
	return 0;
}

int double_to_str(char* dest, double number, int accuracy) {
	double num = number;
	for (int i = 0; i < accuracy; i ++) num *= 10;
	int int_num = (int) num;
	char buff[INT_64_LEN + 2];
	int i = 0;

	while (int_num > 0) {
		if (i == accuracy) {
			buff[i] = '.';
		} else {
			buff[i] = int_num % 10 + '0';
			int_num = int_num / 10;
		}
		i ++;
	}
	if ((number < 1) && (number > -1)) {
		buff[i] = '.';
		buff[i + 1] = '0';
		i += 2;
	}
	dest[i] = '\0';
	i --;
	for (int j = 0; i >= 0; i --, j ++) dest[j] = buff[i];

	return 0;
}
