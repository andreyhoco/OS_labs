#include <stdlib.h>

#define INT_64_LEN 20

int int_to_str(int number, char* str_num) {
	int num = number;
	char buff[INT_64_LEN + 1];
	int i = 0;

	while (num > 0) {
		buff[i] = num % 10 + '0';
		num = num / 10;
		i ++;
	}
	str_num[i] = '\0';
	i --;
	for (int j = 0; i >= 0; i --, j ++) str_num[j] = buff[i];

	return 0;
}

int free_matrix(unsigned char** matrix, int len) {
	for (int row = 0; row < len; row ++) free(matrix[row]);
	free(matrix);
	return 0;
}
