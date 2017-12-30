#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>
#include <zlib.h>

#include <string>
#include <map>

#include "gimage/strutil.h"


const char * pngVersion()
{
	return PNG_LIBPNG_VER_STRING;
}

#define PNG_BYTES_TO_CHECK 8
bool _checkPNG(const char *filename)
{
	char buf[PNG_BYTES_TO_CHECK];
	FILE *fp;

	/* Open the prospective PNG file. */
	if ((fp = fopen(filename, "rb")) == NULL)
		return 0;

	/* Read in some of the signature bytes */
	if (fread(buf, 1, PNG_BYTES_TO_CHECK, fp) != PNG_BYTES_TO_CHECK)
		return 0;

	/* Compare the first PNG_BYTES_TO_CHECK bytes of the signature.
	Return nonzero (true) if they match */

	return(!png_sig_cmp((png_bytep)buf, 0, PNG_BYTES_TO_CHECK));
}

bool _loadPNGInfo(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string> &info)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_byte color_type;
	int number_of_passes;
	png_bytep * row_pointers;

	FILE *fp;

	if ((fp = fopen(filename, "rb")) == NULL)
		return false;


	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (!png_ptr)
		return false;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		return false;

	if (setjmp(png_jmpbuf(png_ptr)))
		return false;

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	*width = png_get_image_width(png_ptr, info_ptr);
	*height = png_get_image_height(png_ptr, info_ptr);

	color_type = png_get_color_type(png_ptr, info_ptr);
	switch (color_type) {
		case PNG_COLOR_TYPE_GRAY:
			*numcolors = 1;
			break;
		case PNG_COLOR_TYPE_RGB:
			*numcolors = 3;
			break;
		default:
			return false;
	}

	*numbits = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	fclose(fp);
	return true;

}

/*
char * _loadPNGSimple(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string> &info, std::string params="", char ** icc_m=NULL, unsigned  *icclength=0)
{
	char *img;
	unsigned w, h, c, b;

	png_image pimage;
	memset(&pimage, 0, (sizeof pimage));
	pimage.version = PNG_IMAGE_VERSION;
	
	png_image_begin_read_from_file(&pimage, filename);
	pimage.format = PNG_FORMAT_RGB;
	
	img = (char *) malloc(PNG_IMAGE_SIZE(pimage));
	w = pimage.width;
	h = pimage.height;
	c = PNG_IMAGE_PIXEL_CHANNELS(pimage.format);
	b = PNG_IMAGE_PIXEL_COMPONENT_SIZE(pimage.format);
	
	int stride = PNG_IMAGE_ROW_STRIDE(pimage);
	png_image_finish_read(&pimage, NULL, img, 0, NULL);
	png_image_free(&pimage);
	*width = w;
	*height = h;
	*numcolors = c;
	*numbits = b;
	
	std::map<std::string,std::string> inf;
	info = inf;

	*icc_m = NULL;
	*icclength = 0;

	return img;

}
*/

char * _loadPNG(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string> &info, std::string params="", char ** icc_m=NULL, unsigned  *icclength=0)
{
	char *img;
	unsigned w, h, c, b;

	png_byte color_type;
	png_byte bit_depth;
	png_bytep *row_pointers;

//printf("_loadPNG: open file...\n");
	FILE *fp = fopen(filename, "rb");

//printf("_loadPNG: create png read struct...\n");
	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) return NULL;

//printf("_loadPNG: create png info struct...\n");
	png_infop pinfo = png_create_info_struct(png);
	if(!pinfo) return NULL;

//printf("_loadPNG: setjmp...\n");
	if(setjmp(png_jmpbuf(png))) return NULL;

//printf("_loadPNG: png_init_io...\n");
	png_init_io(png, fp);

//printf("_loadPNG: png_read_info...\n");
	png_read_info(png, pinfo);

	*width      = png_get_image_width(png, pinfo);
  	*height     = png_get_image_height(png, pinfo);
	*numcolors  = png_get_channels(png, pinfo);
	*numbits    = png_get_bit_depth(png, pinfo);
	
	img = new char[(*width)*(*height)*(*numcolors)*((*numbits)/8)];

//printf("_loadPNG: png_read_update_info...\n");
	png_read_update_info(png, pinfo);
	
	unsigned stride = png_get_rowbytes(png,pinfo);

//printf("_loadPNG: allocate rows...\n");
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * (*height));
	for(int y = 0; y < (*height); y++) {
		row_pointers[y] = (png_byte*)malloc(stride);
	}

//printf("_loadPNG: png_read_image...\n");
	png_read_image(png, row_pointers);

//printf("_loadPNG: copy to img...\n");
	char * dst = img;
	for(int y = 0; y < (*height); y++) {
		memcpy(dst, row_pointers[y], stride);
		dst += stride;
	}

//printf("_loadPNG: free rows...\n");
	for(int y = 0; y < (*height); y++) {
		free(row_pointers[y]);
	}
	free(row_pointers);

	fclose(fp);
	
	std::map<std::string,std::string> inf;
	info = inf;

	*icc_m = NULL;
	*icclength = 0;
	
	png_destroy_read_struct(&png, &pinfo, NULL);
	png=NULL;
	pinfo=NULL;
	
	if (png && pinfo)
	png_destroy_write_struct(&png, &pinfo);

	return img;

}

/*
bool _writePNG(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors, unsigned numbits, std::map<std::string,std::string> info, std::string params, char *iccprofile, unsigned iccprofilelength)
{
	png_image pimage;
	memset(&pimage, 0, (sizeof pimage));
	pimage.version = PNG_IMAGE_VERSION;
	pimage.width = width;
	pimage.height = height;
	pimage.format = PNG_FORMAT_RGB;

	png_image_write_to_file (&pimage, filename, 0, imagedata, 0, NULL);
	png_image_free(&pimage);

	return true;
}
*/

bool _writePNG(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors, unsigned numbits, std::map<std::string,std::string> info, std::string params, char *iccprofile, unsigned iccprofilelength)
{
	std::map<std::string,std::string> p = parseparams(params);

	png_bytep *row_pointers;
	
	FILE *fp = fopen(filename, "wb");
	if(!fp) return NULL;

	png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png) return NULL;

	png_infop pinfo = png_create_info_struct(png);
	if (!pinfo) return NULL;

	if (setjmp(png_jmpbuf(png))) return NULL;

	png_init_io(png, fp);
	
	int compression = 3;
	if (p.find("compression") != p.end()) 
		compression = atoi(p["compression"].c_str());

	png_set_compression_level(png, compression);
	png_set_compression_strategy(png, 2);
	png_set_filter(png,0,PNG_FILTER_SUB);
	
	png_set_IHDR(
		png,
		pinfo,
		width, height,
		numbits,
		PNG_COLOR_TYPE_RGB,
		PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT,
		PNG_FILTER_TYPE_DEFAULT
	);

//printf("_writePNG: writing info...\n");
	png_write_info(png, pinfo);
	
	png_bytep img = (png_bytep) imagedata;
	unsigned stride = png_get_rowbytes(png,pinfo);

//printf("_writePNG: row pointers...\n");
	row_pointers = (png_bytep*)malloc(sizeof(png_bytep) * height);
	for(int y = 0; y < height; y++) {
		row_pointers[y] = img;
		img += stride;
	}

//printf("_writePNG: writing image...\n");
	png_write_image(png, row_pointers);
//printf("_writePNG: writing end of file...\n");
	png_write_end(png, NULL);
//printf("_writePNG: free pointers...\n");
	free(row_pointers);

	fclose(fp);
	
	return true;
	
}
