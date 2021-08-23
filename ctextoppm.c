#include <ppm.h>
#include <pbm.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <libctex/tex.h>

int main(int argc, char **argv) {
    if(argc < 3) {
        printf("%s [input].tex [color output].ppm [alpha output].pbm\n", argv[0]);
        return 1;
    }
    
    fopen(argv[1], "rb");
    FILE *output_color = fopen(argv[2], "wb");
    FILE *output_alpha = fopen(argv[3], "wb");

    tex_file_t infile = tex_get_header(fopen(argv[1], "rb"));
    if(!tex_check_header(&infile)) {
    	printf("Invalid input.\n");
    	return 1;
    }

    if(!tex_check_header_version(&infile, TVF_IS_BC1)) {
    	printf("Input is not BC1.");
    	return 1;
    }

    pixel **o_col = ppm_allocarray(infile.rows, infile.cols);
    bit   **o_alp = pbm_allocarray(infile.rows, infile.cols);

    tex_bc1_pixel_t **tmp;

    for(size_t j = 0; j < infile.rows; j+=4)
        for(size_t k = 0; k < infile.cols; k+=4) {
        	tmp = tex_bc1_read_block(&infile);
        	if(tmp == NULL) break;
            for(size_t l = 0; l < 4; l++)
                for(size_t m = 0; m < 4; m++) {
                	o_col[j+l][k+m].r = tmp[l][m].r;
                	o_col[j+l][k+m].g = tmp[l][m].g;
                	o_col[j+l][k+m].b = tmp[l][m].b;
                	
                	o_alp[j+l][k+m]   = !tmp[l][m].a;
                }
        	for(size_t x; x < 4; x++) free(tmp[x]);
        	free(tmp);
        }
	fclose(infile.fp);
	ppm_writeppm(output_color, o_col, infile.cols, infile.rows, 64, 0);
	pbm_writepbm(output_alpha, o_alp, infile.cols, infile.rows, 0);
	ppm_freearray(o_col, infile.rows);
	pbm_freearray(o_alp, infile.rows);
}
