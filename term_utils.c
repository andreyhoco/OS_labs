#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int come_dir(char* path) {
	if (strncmp(path, "~", strlen(path)) == 0) {
		char* home = getenv("HOME");
		if (home == NULL) return -1;
		if (chdir(home) == -1) return -1;
	} else if (chdir(path) == -1) return -1;
	return 0;
}
