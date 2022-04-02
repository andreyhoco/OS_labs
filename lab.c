#include <unistd.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <sys/wait.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define INPUT_MAX 2048

int is_stream_empty(FILE* stream);

int main() {
	pid_t child_pid;
	char input[INPUT_MAX];
	char dir[PATH_MAX];
	char** arguments = calloc(INPUT_MAX + 1, sizeof(char*));
	int prev_count = 0;

	if (arguments == NULL) {
		perror(NULL);
		exit(-1);
	}
	getcwd(dir, PATH_MAX);

	printf("Welcome to custom terminal\n");
	printf("Current dir: %s\n", dir);

	while (1) {
		if ((fgets(input, INPUT_MAX, stdin) == NULL) && (ferror(stdin) != 0)) {
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

		child_pid = fork();
		switch (child_pid) {
			case -1: {
				perror(NULL);
				break;
			}
			case 0: {
				execvp(arguments[0], arguments);
				break;
			}
			default: {
				if ((count > 1) && (strcmp(arguments[count - 1], "&") == 0)) break;
				waitpid(child_pid, NULL, 0);
				break;
			}
		}
	}

	exit(0);
}

int is_stream_empty(FILE* stream) {
	printf("Check stream");
	char test;
	if ((test = getchar()) == EOF) {
		printf("Empty stream|");
		printf("Test = %c|", test);	
		return 1;
	}
	else ungetc(test, stream);
	printf("Test = %c|", test);
	return 0;
}
