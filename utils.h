#define INPUT_MAX 2048
#define MIN_INT(a, b) (((a) < (b)) ? (a) : (b))

int parse(char **arguments, char *input) ;

void free_args(char **args, int args_count);

int encrypt_data(int data_d, int key_d, int out_d);

