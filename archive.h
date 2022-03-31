#define FILENAME_LENGTH 128
#define ARCHIVE_NAME "./archive.arch"
#define DIRECTORY 1
#define SIMPLE_FILE 2
#define MODE_ARCH 1
#define MODE_UNARCH 2
#define SKIP_ARG 1

int copy_data(int input_descriptor, int output_decriptor, int size);

int write_in_archive(int archive_descriptor, char* directory);

int open_archive(int archive_descriptor);

char error_file[FILENAME_LENGTH];
