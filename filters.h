struct sobel_params {
	unsigned char** source;
	unsigned char** dest;
	int from_x;
	int from_y;
	int to_x;
	int to_y;
};

int rgb_to_grayscale(unsigned char*** source, unsigned char** container, int h, int w);

int apply_sobel_op(unsigned char** source, unsigned char** container, int from_x, int from_y, int to_x, int to_y);

void* concurency_sobel(void* params);
