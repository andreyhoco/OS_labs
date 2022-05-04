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
