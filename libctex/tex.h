#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#define DIRTY_SIZE_HACK

typedef struct {
	uint8_t 
	  r, g, b; /* The values were originally stored as RGB565. 
	            * Here, the red and blue components are multiplied
	            * by 2 to ease further processing. 
	            * Therefore, all values range from 0-63. */
	bool
	  a;       /* Textures have 1-bit alpha. */
} tex_bc1_pixel_t;

typedef struct {
	uint16_t 
	  r, g, b, a; /* These values are stored as 16 bit unsigned integers.
	                      * This pixel format is an intermediate
	                      * used for implementation agnostic to the underlying
	                      * format. */
} tex_universal_pixel_t;

typedef struct {
	int16_t 
	  offset;  /* Where the image data begins. */

	uint8_t  
	  version,  /* The version. For BC1 images, 0x13. For BC7 images, 0x36. For Switch images, 0x02.*/ 
	  version2; /* A byte in the header. Purpose unknown, however, 
	             * BC1 = 0x00, BC7 = 0x20, Broken BC7 = 0x80. */
	uint32_t
	  rows,    /* The amount of rows, or height. As of now, unknown, 
	            * but can be computed if square, which is usually the case.
	            * If DIRTY_SIZE_HACK is defined, this will be done.
	            * The same goes for the below. */
	  cols;    /* The amount of columns, or width. */

    FILE
      *fp;
} tex_file_t;

/* gets the known header fields from the FILE pointer fp. */
extern tex_file_t  tex_get_header(FILE *fp);

/* checks the header for being ok */
extern int tex_check_header(tex_file_t *file);

/* checks the version */
#define TVF_IS_BC1     1
#define TVF_IS_BC7     2
#define TVF_IS_BROKEN  4
#define TVF_IS_SWITCH  8
#define TVF_IS_UNKNOWN 16
extern int tex_check_header_version(tex_file_t *file, uint8_t flags);

/* gets a BC1-compressed block and turns it into a pixel array. */
extern tex_bc1_pixel_t **tex_bc1_get_block(uint64_t block);
extern tex_bc1_pixel_t **tex_bc1_read_block(tex_file_t *file);
/*
extern tex_universal_pixel_t tex_universal_get_block(uint8_t flags, uint64_t block);
extern tex_universal_pixel_t tex_universal_read_block(tex_file_t *file);
*/
