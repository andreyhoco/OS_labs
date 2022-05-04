#define FORMAT_LEN 2
#define FORMAT_ERR -2

struct pnm_header {
	char format[FORMAT_LEN];
	int width;
	int height;
	int color_depth;
};

int load_pnm_header(int image_d, struct pnm_header* header);