#include <stdlib.h>
#include <string.h>

int parse(char **arguments, char *input) {
	int count = 1;
	char* token = strtok(input, " ");

	if (strchr(token, '\n') != NULL) {
		if ((arguments[0] = realloc(arguments[0], strlen(token) * sizeof(char))) == NULL) {
			return -1;
		}
		strncpy(arguments[0], token, strlen(token) - 1);
		arguments[0][strlen(token) - 1] = '\0';
	} else {
		if ((arguments[0] = realloc(arguments[0], (strlen(token) + 1) * sizeof(char))) == NULL) {
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
					return -1;
				}

				strncpy(arguments[count], token, strlen(token) - 1);
				arguments[count][strlen(token) - 1] = '\0';
			} else {
				if ((arguments[count] = realloc(arguments[count], (strlen(token) + 1) * sizeof(char))) == NULL) {
					for (int i = 0; i < count; i ++) free(arguments[i]);
					return -1;
				}

				strncpy(arguments[count], token, strlen(token));
				arguments[count][strlen(token)] = '\0';
			}
			count ++;
		}
	}

	if ((arguments[count] = realloc(arguments[count], sizeof(char))) == NULL) {
		for (int i = 0; i < count; i ++) free(arguments[i]);
		return -1;
	}
	
	arguments[count] = 0;
	return count;
}
