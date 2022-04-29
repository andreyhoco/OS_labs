#include <unistd.h>
#include <sys/types.h>
#include <linux/limits.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include "term_utils.h"
#include "error_handling.h"

void kill_child(int sig);

pid_t child_pid = -2;
/*
* После вызова kill переменную errno может перезаписать другой вызов,
* поэтому ошибки вызова kill выделена отдельная переменная
*/
int kill_err = 0;

int main() {
	char input[INPUT_MAX + 1];
	char dir[PATH_MAX];
	char path[PATH_MAX];
	char inv[PATH_MAX + 2];
	char** arguments = calloc(INPUT_MAX / 2 + 2, sizeof(char*));
	int prev_count = 0;

	if (arguments == NULL) {
		print_error(errno, "input buffer");
		free(arguments);
		exit(-1);
	}

	struct sigaction action;
	action.sa_handler = kill_child;
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;

	if (sigaction(SIGINT, &action, 0) == -1) {
		print_error(errno, "signal handler");
		free(arguments);
		exit(-1);
	}

	getcwd(dir, PATH_MAX);
	write(1, "Welcome to custom terminal\n\0", strlen("Welcome to custom terminal\n\0") + 1);

	while (1) {
		int num_of_readed;

		if (kill_err != 0) {
			if (kill_err != ESRCH) print_error(kill_err, "kill child process");
			kill_err = 0;
			continue;
		}

		strncpy(inv, dir, strlen(dir));
		inv[strlen(dir)] = '\0';
		strncat(inv, ">\0", strlen(dir) + 2);
		write(1, inv, strlen(inv) + 1);

		if ((num_of_readed = read(STDIN_FILENO, input, INPUT_MAX)) == -1) {
			if (errno == EINTR) write(1, "\n\0", strlen("\n\0") + 1);
			else print_error(errno, "stdin");

			errno = 0;
			continue;
		}

		input[num_of_readed] = '\0';

		if (strcmp(input, "\n\0") == 0) {
			continue;
		}

		int index = 1;
		char* token = strtok(input, " ");

		// Handle cd command
		if ((strcmp(token, "cd") == 0) || (strcmp(token, "cd\n") == 0)) {
			if (strlen(token) == 2) {
				token = strtok(NULL, " ");
				strncpy(path, token , strlen(token));
				if (strchr(token, '\n') != NULL) path[strlen(token) - 1] = '\0';
				else path[strlen(token)] = '\0';
			} else {
				strncpy(path, "~", strlen("~"));
				path[strlen("~")] = '\0';
			}
			if (come_dir(path) == -1) print_error(errno, path);
			else {
				getcwd(dir, PATH_MAX);
			}

			for (int i = 0; i < prev_count; i ++) {
				free(arguments[i]);
				arguments[i] = NULL;
			}
			prev_count = 0;
			continue;
		}

		if (strchr(token, '\n') != NULL) {
			if ((arguments[0] = realloc(arguments[0], strlen(token) * sizeof(char))) == NULL) {
				print_error(errno, "arguments");
				continue;
			}
			strncpy(arguments[0], token, strlen(token) - 1);
			arguments[0][strlen(token) - 1] = '\0';
		} else {
			if ((arguments[0] = realloc(arguments[0], (strlen(token) + 1) * sizeof(char))) == NULL) {
				print_error(errno, "arguments");
				continue;
			}

			strncpy(arguments[0], token, strlen(token));
			arguments[0][strlen(token)] = '\0';
		}

		while ((token = strtok(NULL, " ")) != NULL) {
			if ((strcmp(token, "") != 0) && (strcmp(token, "\n\0") != 0)) {
				if (strchr(token, '\n') != NULL) {
					if ((arguments[index] = realloc(arguments[index], strlen(token) * sizeof(char))) == NULL) {
						print_error(errno, "arguments");
						continue;
					}

					strncpy(arguments[index], token, strlen(token) - 1);
					arguments[index][strlen(token) - 1] = '\0';
				} else {
					if ((arguments[index] = realloc(arguments[index], (strlen(token) + 1) * sizeof(char))) == NULL) {
						print_error(errno, "arguments");
						continue;
					}

					strncpy(arguments[index], token, strlen(token));
					arguments[index][strlen(token)] = '\0';
				}
				index ++;
			}
		}

		index --;

		int is_background = 0;
		if ((index > 1) && (strcmp(arguments[index], "&") == 0)) {
			free(arguments[index]);
			arguments[index] = NULL;
			is_background = 1;
			index --;
		}

		int delta = prev_count - index;
		if (delta > 0) {
			for (int i = 1; i <= delta; i ++) {
				free(arguments[index + i]);
				arguments[index + i] = NULL;
			}
		}
		prev_count = index;

		child_pid = fork();
		switch (child_pid) {
			case -1: {
				print_error(errno, "child process");
				break;
			}
			case 0: {
				// Чилды должны игнорировать SIGINT
				action.sa_handler = SIG_IGN;
				sigemptyset(&action.sa_mask);
				action.sa_flags = 0;
				if (sigaction(SIGINT, &action, 0) == -1) {
					print_error(errno, arguments[0]);
				}

				if (execvp(arguments[0], arguments) == -1) {
					print_error(errno, arguments[0]);

					for (int i = 0; i <= index; i ++) {
						free(arguments[i]);
						arguments[i] = NULL;
					}
					free(arguments);

					exit(-1);
				}
				break;
			}
			default: {
				if (is_background) {
					child_pid = -2;
					break;
				}
				waitpid(child_pid, NULL, 0);
				break;
			}
		}
	}

	exit(0);
}

void kill_child(int sig) {
	if (child_pid != -2) {
		if (kill(child_pid, SIGTERM) == -1) kill_err = errno;
	}
}
