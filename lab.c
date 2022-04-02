#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_INPUT 2048

int main() {
	pid_t child_pid;
	char input[MAX_INPUT];
	char** arguments = calloc(MAX_INPUT + 1, sizeof(char*));
	int prev_count = 0;

	if (arguments == NULL) {
		perror(NULL);
		exit(-1);
	}

	while (1) {
		if ((fgets(input, MAX_INPUT, stdin) == NULL) && (ferror(stdin) != 0)) {
			perror(NULL);
			continue;
		}

		if (strcmp(input, "\n\0") == 0) {
			continue;
		}

		int count = 1;
		char* token = strtok(input, " ");
		
		if (strchr(token, '\n') != NULL) {
			if ((arguments[0] = realloc(arguments[0], strlen(token) * sizeof(char))) == NULL) {
				perror(NULL);
				continue;
			}
			strncpy(arguments[0], token, strlen(token) - 1);
			arguments[0][strlen(token) - 1] = '\0';
		} else {
			if ((arguments[0] = realloc(arguments[0], (strlen(token) + 1) * sizeof(char))) == NULL) {
				perror(NULL);
				continue;
			}

			strncpy(arguments[0], token, strlen(token));
			arguments[0][strlen(token)] = '\0';
		}

		while ((token = strtok(NULL, " ")) != NULL) {
			if ((strcmp(token, "") != 0) && (strcmp(token, "\n\0") != 0)) {
				if (strchr(token, '\n') != NULL) {
					if ((arguments[count] = realloc(arguments[count], strlen(token) * sizeof(char))) == NULL) {
						perror(NULL);
						continue;
					}

					strncpy(arguments[count], token, strlen(token) - 1);
					arguments[count][strlen(token) - 1] = '\0';
				} else {
					if ((arguments[count] = realloc(arguments[count], (strlen(token) + 1) * sizeof(char))) == NULL) {
						perror(NULL);
						continue;
					}

					strncpy(arguments[count], token, strlen(token));
					arguments[count][strlen(token)] = '\0';
				}
				count ++;
			}
		}

		arguments[count] = realloc(arguments[count], sizeof(char));
		arguments[count] = 0;

		int delta = prev_count - count;
		if (delta > 0) {
			for (int i = 1; i < delta + 1; i ++) {
				free(arguments[count + delta]);
			}
		}
		prev_count = count;

		for (int i = 0; i < count; i ++) {
			printf("\n%d: %s|", i, arguments[i]);
		}
	}

	exit(0);
}


