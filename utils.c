#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_MAX 2048

int parse(char **arguments, char *input_command) {
	int input_len = strchr(input_command, '\0') == NULL ? strlen(input_command) : strlen(input_command) + 1;
	char *input = malloc(input_len * sizeof(char));
	strncpy(input, input_command, input_len);

	int count = 1;
	char* token = strtok(input, " ");

	if (strchr(token, '\n') != NULL) {
		if ((arguments[0] = realloc(arguments[0], strlen(token) * sizeof(char))) == NULL) {
			free(input);
			return -1;
		}
		strncpy(arguments[0], token, strlen(token) - 1);
		arguments[0][strlen(token) - 1] = '\0';
	} else {
		if ((arguments[0] = realloc(arguments[0], (strlen(token) + 1) * sizeof(char))) == NULL) {
			free(input);
			return -1;
		}

		strncpy(arguments[0], token, strlen(token));
		arguments[0][strlen(token)] = '\0';
	}

	while ((token = strtok(NULL, " ")) != NULL) {
		if ((strcmp(token, "") != 0) && (strcmp(token, "\n\0") != 0)) {
			if (strchr(token, '\n') != NULL) {
				if ((arguments[count] = realloc(arguments[count], strlen(token) * sizeof(char))) == NULL) {
					for (int i = 0; i < count; i ++) free(arguments[i]);
					free(input);
					return -1;
				}

				strncpy(arguments[count], token, strlen(token) - 1);
				arguments[count][strlen(token) - 1] = '\0';
			} else {
				if ((arguments[count] = realloc(arguments[count], (strlen(token) + 1) * sizeof(char))) == NULL) {
					for (int i = 0; i < count; i ++) free(arguments[i]);
					free(input);
					return -1;
				}

				strncpy(arguments[count], token, strlen(token));
				arguments[count][strlen(token)] = '\0';
			}
			count ++;
		}
	}

	free(input);

	if ((arguments[count] = malloc(sizeof(char))) == NULL) {
		for (int i = 0; i < count; i ++) free(arguments[i]);
		return -1;
	}

	arguments[count] = 0;
	return count;
}

void free_args(char **args, int args_count) {
	for (int i = 0; i <= args_count; i ++) free(args[i]);
	free(args);
}

int encrypt_data(int data_d, int key_d, int out_d) {
	char data_buff[1024];
	char key_buff[1024];
	char out_buff[1024];
	int data_readed;

	while ((data_readed = read(data_d, data_buff, sizeof(data_buff))) > 0) {
		int key_readed;
		int delta;

		key_readed = read(key_d, key_buff, sizeof(key_buff));
		if (key_readed == -1) return -2;

		if ((delta = data_readed - key_readed) > 0) {
			char tmp_buff[1024];
			if (lseek(key_d, 0, SEEK_SET) == -1) return -2;

			key_readed = read(key_d, tmp_buff, delta);
			if (key_readed == -1) return -2;

			for (int i = key_readed, j = 0; i < (key_readed + delta); i ++, j ++) key_buff[i] = tmp_buff[j];
		}

		for (int i = 0; i < data_readed; i ++) out_buff[i] = data_buff[i] ^ key_buff[i];
		if (write(out_d, out_buff, data_readed) == -1) return -3;
	}

	if (data_readed == -1) return -1;
	return 0;
}
