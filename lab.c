#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_MAX 2048

int main(int argc, char* argv[]) {
	int opt;
	char input_data[INPUT_MAX];
	char key_data[INPUT_MAX];

	memset(input_data, '\0', INPUT_MAX);
	memset(key_data, '\0', INPUT_MAX);

	while((opt = getopt(argc, argv, "d:k:")) != -1) {
		switch(opt) {
			case 'd': {
				if (strcmp(optarg, "-k") == 0) {
					write(2, "-d: arg expected\n\0", strlen("-d: arg expected\n\0") + 1);
					exit(-1);
				}
				
				strncpy(input_data, optarg, INPUT_MAX);
				input_data[strlen(optarg)] = '\0';
				break;
			}

			case 'k': {
				if (strcmp(optarg, "-d") == 0) {
					write(2, "-k: arg expected\n\0", strlen("-k: arg expected\n\0") + 1);
					exit(-1);
				}

				strncpy(key_data, optarg, INPUT_MAX);
				key_data[strlen(optarg)] = '\0';
				break;
			}
		}
	}

	if (argc == 1) {
		write(2, "Options expected\n\0", strlen("Options expected\n\0") + 1);
		exit(-1);
	}

	if (input_data[0] == '\0' || key_data[0] == '\0') {
		write(2, "Bad options\n\0", strlen("Bad options\n\0") + 1);
		exit(-1);
	}

	write(1, input_data, strlen(input_data) + 1);
	write(1, key_data, strlen(key_data) + 1);

	exit(0);
}