#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

#define BUFF_SIZE 100
#define FORMAT_LEN 2
#define FORMAT_ERR -2
#define BYTES_PER_PX 3
#define INT_64_LEN 20

struct pnm_header{
	char format[FORMAT_LEN];
	int width;
	int height;
	int color_depth;
};

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

int str_to_int(char* str, int len) {
	int number = 0;
	for (int i = 0; i < len; i ++) {
		number = number * 10 + (str[i] - '0');
	}
	return number;
}

// В случае успешного выполнения возвращает смещение относительно начала файла до растра
int load_pnm_header(int image_d, struct pnm_header* header) {
	char buff[BUFF_SIZE];
	char first_read = 1;
	int line_number = 0;
	int readed;

	while ((readed = read(image_d, buff, sizeof(buff))) > 0) {
		int handled = 0;
		char q[2] = "\n";
		char* line = strtok(buff, q);
	
		// Парсим файл на строки и читаем их пока не определим формат, размер и глубину цвета
		while (line != NULL) {
			// Первая строка - формат
			if (first_read) {
				if (strncmp(line, "P5", FORMAT_LEN) == 0) {
					header->format[0] = 'P';
					header->format[1] = '5';
					
					first_read = 0;
					handled += (FORMAT_LEN + 1);
					line = strtok(NULL, "\n");
					continue;
				} else if (strncmp(line, "P6", FORMAT_LEN) == 0) {
					header->format[0] = 'P';
					header->format[1] = '6';

					first_read = 0;
					handled += (FORMAT_LEN + 1);
					line = strtok(NULL, "\n");
					continue;
				} else {
					return FORMAT_ERR;
				}
			}

			// Пропуск комментариев
			if (line[0] == '#') {
				handled += (strlen(line) + 1);
				line = strtok(NULL, "\n");
				continue;
			}

			switch (line_number) {
				// Строка с размерами
				case 0: {
					int line_len = strlen(line);
					int space_pos = strcspn(line, " ");
					header->width = str_to_int(line, space_pos);
					header->height = str_to_int(line + space_pos + 1, line_len - 1 - space_pos);
					break;
				}
	
				// Строка с глубиной цвета
				case 1: {
					header->color_depth = str_to_int(line, strlen(line));
					break;
				}
	
				// Достигли растра, необходимо вычислить смещение до него относительно начала файла
				case 2: {
					int offset = lseek(image_d, 0, SEEK_CUR);
					if (offset == -1) return -1;

					int offset_to_rastr = offset - (readed - handled);
					if (lseek(image_d, offset_to_rastr, SEEK_SET) == -1) return -1;
					return offset_to_rastr;
				}
			}
	
			line_number ++;
			handled += (strlen(line) + 1);
			line = strtok(NULL, "\n");
		}
	}

	if (readed == -1) return -1;
	else return FORMAT_ERR;
}

int load_ppm(int image_d, unsigned char*** container, struct pnm_header* header) {
	for (int rownum = 0; rownum < header->height; rownum ++) {
		for (int columnnum = 0; columnnum < header->width; columnnum ++) {
			int readed = read(image_d, container[rownum][columnnum], BYTES_PER_PX);

			if (readed == -1) return -1;
			if (readed < BYTES_PER_PX) return FORMAT_ERR;
		}
	}

	return 0;
}

int load_pgm(int image_d, unsigned char** container, struct pnm_header* header) {
	for (int rownum = 0; rownum < header->height; rownum ++) {
		int readed = read(image_d, container[rownum], header->width);
		if (readed == -1) return -1;
		if (readed < BYTES_PER_PX) return FORMAT_ERR;
	}

	return 0;
}

int write_pgm_in_file(int file_d, unsigned char** image_matrix, struct pnm_header* header) {
	if (strncmp("P5", header->format, 2) != 0) return FORMAT_ERR;
	write(file_d, "P5\n", strlen("P5\n"));
	char w[INT_64_LEN];
	char h[INT_64_LEN];
	char d[INT_64_LEN];
	char buff[100];
	int_to_str(header->height, h);
	int_to_str(header->width, w);
	int_to_str(header->color_depth, d);

	memset(buff, '\0', sizeof(buff));
	strncat(buff, w, strlen(w));
	strncat(buff, " ", 1);
	strncat(buff, h, strlen(h));
	strncat(buff, "\n", 1);
	strncat(buff, d, strlen(d));
	strncat(buff, "\n", 1);

	if (write(file_d, buff, strlen(buff)) == -1) return -1;

	for (int i = 0; i < header->height; i ++) {
		if (write(file_d, image_matrix[i], header->width) == -1) return -1;
	}
	return 0;
}
