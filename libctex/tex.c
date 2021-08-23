#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include "tex.h"

#ifdef DIRTY_SIZE_HACK
  #include <math.h>
#endif

static const double onethird  = 1.0d/3.0d;
static const double twothirds = 2.0d/3.0d;

/* the BC1 part of the impl */

static inline uint8_t rgb565_getr(uint16_t i) {
	return (((i>>11)&31)*2);
}

static inline uint8_t rgb565_getg(uint16_t i) {
	return ((i>>5)&63);
}

static inline uint8_t rgb565_getb(uint16_t i) {
	return ((i&31)*2);
}

static inline uint8_t bc1_getlutvalue(uint64_t i, size_t n) {
	return ((i>>(32+(n*2))) & 3);
}

static inline uint16_t bc1_getcolvalue(uint64_t i, size_t n) {
	return ((i>>(n*16))&0xFFFF);
}

typedef struct {
	uint16_t 
	  c[2];
	  
	uint32_t
	  lut[16];
} bc1_blk_t;


static inline tex_bc1_pixel_t bc1_pixel_from_rgb565(uint16_t i) {
	tex_bc1_pixel_t p;
	p.r = rgb565_getr(i);
	p.g = rgb565_getg(i);
	p.b = rgb565_getb(i);
	p.a = true;
	return p;
}

static inline bc1_blk_t bc1_blk_from_u64(uint64_t i) {
	bc1_blk_t b;
	b.c[0] = bc1_getcolvalue(i, 0);
	b.c[1] = bc1_getcolvalue(i, 1);
    for(size_t x = 0; x < 16;x++)
        b.lut[x] = bc1_getlutvalue(i, x);
	return b;
}

static inline void bc1_pixels_into_block(tex_bc1_pixel_t *p, tex_bc1_pixel_t **i, bc1_blk_t b) {
	for(size_t j = 0; j < 4; j++)
	    for(size_t k = 0; k < 4; k++) {
	    	i[j][k] = p[b.lut[j*4+k]];
	    }
}

static inline void bc1_pixel_interpolate(tex_bc1_pixel_t *i, bc1_blk_t b) {
    if(b.c[0] > b.c[1]) {
 
    	i[2].r = (onethird * i[1].r) + (twothirds * i[0].r);
    	i[2].g = (onethird * i[1].g) + (twothirds * i[0].g);
    	i[2].b = (onethird * i[1].b) + (twothirds * i[0].b);
        i[2].a = true;

    	i[3].r = (onethird * i[0].r) + (twothirds * i[1].r);
    	i[3].g = (onethird * i[0].g) + (twothirds * i[1].g);
    	i[3].b = (onethird * i[0].b) + (twothirds * i[1].b);
    	i[3].a = true;

    }
    else {    	    
    	i[2].r = (0.5 * i[0].r) + (0.5 * i[1].r);
    	i[2].g = (0.5 * i[0].g) + (0.5 * i[1].g);
    	i[2].b = (0.5 * i[0].b) + (0.5 * i[1].b);
    	i[2].a = true;
    	
    	i[3].r = 0;
    	i[3].g = 0;
    	i[3].b = 0;
    	i[3].a = false;
    }
}



extern tex_file_t tex_get_header(FILE *fp) {
    tex_file_t f;
	size_t i = 0;
	uint8_t bytes[17];

    f.fp = fp;
	f.version = 0;
	f.version2 = 0;

    

    while(i < 17) {
    	if(fread(bytes+i, 1, 1, fp) != 1) {
    		f.offset = -1;
    		return f;
    	}    	
    	switch(i) {
    		case 13:
    		f.version = bytes[13];
    		break;

    		case 9:
    		f.version2 = bytes[9];
    		break;

    		case 16:
    		f.offset = bytes[16];
    		break;

    		case 4:
    		if(memcmp(bytes, "TEX", 3) != 0) {
    			f.offset = -1;
    			return f;
    		}
    		
    	}

    	i++;
    }

    fseek(fp, f.offset, SEEK_SET);

    #ifdef DIRTY_SIZE_HACK
    {
        size_t t = ftell(fp);
        fseek(fp, 0, SEEK_END);
        f.rows = sqrt(((ftell(fp)-f.offset)/8)*16);
        f.cols = f.rows;
        fseek(fp, t, SEEK_SET);
    }
    #else
	  f.rows = 0;
	  f.cols = 0;
    #endif /* DIRTY_SIZE_HACK */
    
    return f;
}

extern int tex_check_header(tex_file_t *file) {
	return !(file == NULL || file->offset < 0 || file->rows < 4 || file->cols < 4);
}

extern int tex_check_header_version(tex_file_t *file, uint8_t flags) {
	uint8_t r = 1;
	if(flags & TVF_IS_BC1) r = r && (file->version == 0x13 && file->version2 == 0x00);
	if(flags & TVF_IS_BC7) r = r && (file->version == 0x36 && (file->version2 == 0x20 || file->version2 == 0x80));
	if(flags & TVF_IS_BROKEN) r = r && (file->version2 == 0x80);

	return r;
}

extern tex_bc1_pixel_t **tex_bc1_get_block(uint64_t block) {
    
	tex_bc1_pixel_t **o = calloc(4, sizeof(tex_bc1_pixel_t *));

    for(size_t i = 0; i < 4; i++)
        o[i] = calloc(4, sizeof(tex_bc1_pixel_t));

    bc1_blk_t b = bc1_blk_from_u64(block);
    tex_bc1_pixel_t p[4];
    
    p[0] = bc1_pixel_from_rgb565(b.c[0]);
    p[1] = bc1_pixel_from_rgb565(b.c[1]);

    bc1_pixel_interpolate(p, b);
	bc1_pixels_into_block(p, o, b);
	
	return o;
}



extern tex_bc1_pixel_t **tex_bc1_read_block(tex_file_t *file) {
    if(!(tex_check_header(file) && tex_check_header_version(file, TVF_IS_BC1))) return NULL;
    uint64_t x;
    if(fread(&x, sizeof(x), 1, file->fp) != 1) return NULL;
    return tex_bc1_get_block(x);
}
