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

/*
	// read file 
	if (setjmp(png_jmpbuf(png_ptr)))
		return false;

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * (*height));
	for (int y=0; y<(*height); y++)
		row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

	png_read_image(png_ptr, row_pointers);
*/
	fclose(fp);


/*
	char *img, *buf;
	FILE * infile;
	uint32 w, h;
	uint16 c, b;

	TIFF* tif = TIFFOpen(filename, "r");
	if (tif) {

		size_t npixels;
		uint32* raster;

		uint32 imagelength, imagewidth;
		uint16 config, nsamples;
        
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &c);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &b);
		TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

		char *infobuf;
		if (TIFFGetField(tif, TIFFTAG_ARTIST, &infobuf)) info["Artist"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_MAKE, &infobuf))  info["Make"]=infobuf;
		if (TIFFGetField(tif, TIFFTAG_MODEL, &infobuf))  info["Model"]=infobuf;
		if (TIFFGetField(tif, TIFFTAG_SOFTWARE, &infobuf))  info["Software"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_COPYRIGHT, &infobuf))  info["Copyright"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_LENSINFO, &infobuf))  info["LensInfo"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &infobuf))  info["ImageDescription"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_DATETIME, &infobuf)) info["DateTime"]=infobuf;
		
		*width = w;
		*height = h;
		*numcolors = c;
		*numbits = b;
		TIFFClose(tif);
		return true;
	}
	else return false;
*/
	return false;
}

char * _loadPNG(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string> &info, std::string params="", char ** icc_m=NULL, unsigned  *icclength=0)
{
	char *img;
	unsigned w, h, c, b;

	png_structp png_ptr;
	png_infop info_ptr;
	png_byte color_type;
	int number_of_passes;
	png_bytep * row_pointers;

	FILE *fp = fopen(filename, "rb");

	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png) return NULL;

	png_infop pnginfo = png_create_info_struct(png);
	if(!pnginfo) return NULL;

	if(setjmp(png_jmpbuf(png))) return NULL;

	png_init_io(png, fp);

	png_read_info(png, pnginfo);








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
			return NULL;
	}

	*numbits = png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	// read file 
	if (setjmp(png_jmpbuf(png_ptr)))
		return NULL;

	row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * (*height));
	for (int y=0; y<(*height); y++)
		row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));

	png_read_image(png_ptr, row_pointers);
	fclose(fp);
	
	w = *width;
	h = *height;
	c = *numcolors;
	b = *numbits;
	img = new char[w*h*c*(b/8)];

	if (b == 8) {
		char * dst = (char *) img;
		for (unsigned y = 0; y < h; y++){
			png_byte* buf = row_pointers[y];
			char * src = (char *) buf;
			for(unsigned x=0; x < w; x++) {
				if (c == 1) {
					dst[0] = (char) src[0];
					dst+=1;
					src+=1;
				}
				else if(c == 3 ){
					dst[0] = (char) src[0];
					dst[1] = (char) src[1];
					dst[2] = (char) src[2];
					dst+=3;
					src+=3;
				}
			}
		}
	}
	else if (b == 16) {
		unsigned short * dst = (unsigned short *) img;
		for (unsigned y = 0; y < h; y++){
			png_byte* buf = row_pointers[y];
			unsigned short * src = (unsigned short *) buf;
			for(unsigned x=0; x < w; x++) {
				if (c == 1) {
					dst[0] = (unsigned short) src[0];
					dst+=1;
					src+=1;
				}
				else if(c == 3 ){
					dst[0] = (unsigned short) src[0];
					dst[1] = (unsigned short) src[1];
					dst[2] = (unsigned short) src[2];
					dst+=3;
					src+=3;
				}
			}
		}				
	}
	return img;


/*
	char *img, *buf;
	FILE * infile;
	uint32 w, h;
	uint16 c, b;


	TIFF* tif = TIFFOpen(filename, "r");
	if (tif) {

		size_t npixels;
		uint32* raster;

		uint32 imagelength, imagewidth;
		uint16 config, nsamples, sampleformat;

		unsigned len;
		char * buffer;
        
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &c);
		TIFFGetField(tif, TIFFTAG_BITSPERSAMPLE, &b);
		TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

		char *infobuf;
		if (TIFFGetField(tif, TIFFTAG_ARTIST, &infobuf)) info["Artist"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_MAKE, &infobuf))  info["Make"]=infobuf;
		if (TIFFGetField(tif, TIFFTAG_MODEL, &infobuf))  info["Model"]=infobuf;
		if (TIFFGetField(tif, TIFFTAG_SOFTWARE, &infobuf))  info["Software"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_COPYRIGHT, &infobuf))  info["Copyright"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_LENSINFO, &infobuf))  info["LensInfo"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_IMAGEDESCRIPTION, &infobuf))  info["ImageDescription"]=infobuf; 
		if (TIFFGetField(tif, TIFFTAG_DATETIME, &infobuf)) info["DateTime"]=infobuf;
		if (TIFFGetField(tif, TIFFTAG_SAMPLEFORMAT, &sampleformat)) {
			if (sampleformat == SAMPLEFORMAT_UINT) info["SampleFormat"]="uint";
			if (sampleformat == SAMPLEFORMAT_INT) info["SampleFormat"]="int";
			if (sampleformat == SAMPLEFORMAT_IEEEFP) info["SampleFormat"]="float";
			if (sampleformat == SAMPLEFORMAT_VOID) info["SampleFormat"]="void";
		}

		if (TIFFGetField(tif, TIFFTAG_ICCPROFILE, &len, &buffer)) {
			*icc_m = new char[len];
			memcpy(*icc_m, buffer, len);
			*icclength = len;
		}
		else {
			*icc_m = NULL;
			*icclength = 0;
		}

		img = new char[w*h*c*(b/8)];
		buf = (char *) _TIFFmalloc(TIFFScanlineSize(tif));
		
		if (config != PLANARCONFIG_CONTIG) return NULL;
		
		if (b == 8) {
			char * dst = (char *) img;
			for (unsigned y = 0; y < h; y++){
				TIFFReadScanline(tif, buf, y, 0);
				char * src = (char *) buf;
				for(unsigned x=0; x < w; x++) {
					if (c == 1) {
						dst[0] = (char) src[0];
						dst+=1;
						src+=1;
					}
					else if(c == 3 ){
						dst[0] = (char) src[0];
						dst[1] = (char) src[1];
						dst[2] = (char) src[2];
						dst+=3;
						src+=3;
					}
				}
			}
		}
		else if (b == 16) {
			unsigned short * dst = (unsigned short *) img;
			for (unsigned y = 0; y < h; y++){
				TIFFReadScanline(tif, buf, y, 0);
				unsigned short * src = (unsigned short *) buf;
				for(unsigned x=0; x < w; x++) {
					if (c == 1) {
						dst[0] = (unsigned short) src[0];
						dst+=1;
						src+=1;
					}
					else if(c == 3 ){
						dst[0] = (unsigned short) src[0];
						dst[1] = (unsigned short) src[1];
						dst[2] = (unsigned short) src[2];
						dst+=3;
						src+=3;
					}
				}
			}			
			
		}
		else if (b == 32) {
			float * dst = (float *) img;
			for (unsigned y = 0; y < h; y++){
				TIFFReadScanline(tif, buf, y, 0);
				float * src = (float *) buf;
				for(unsigned x=0; x < w; x++) {
					if (c == 1) {
						dst[0] = (float) src[0];
						dst+=1;
						src+=1;
					}
					else if(c == 3 ){
						dst[0] = (float) src[0];
						dst[1] = (float) src[1];
						dst[2] = (float) src[2];
						dst+=3;
						src+=3;
					}
				}
			}			
		}
		else return NULL;
		
		*width = w;
		*height = h;
		*numcolors = c;
		*numbits = b;
		if (buf) _TIFFfree(buf);
		TIFFClose(tif);
		return img;
	}
	else return NULL;
*/
	return NULL;
}

bool _writePNG(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors, unsigned numbits, std::map<std::string,std::string> info, char *iccprofile, unsigned iccprofilelength)
{
/*
	char *img;
	unsigned char *buf;
	uint32 w, h;
	uint16 c, b;

	TIFF* tif = TIFFOpen(filename, "w");
	if (tif) {

		TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, width);  
		TIFFSetField(tif, TIFFTAG_IMAGELENGTH, height);    
		TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, numcolors);   
		TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, numbits);
		if (numbits == 8 | numbits == 16)
			TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_UINT);
		else if (numbits == 32)
			TIFFSetField(tif, TIFFTAG_SAMPLEFORMAT, SAMPLEFORMAT_IEEEFP);
		else
			return false;
		TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);   

		TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
		TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
		// We set the strip size of the file to be size of one row of pixels
		TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, TIFFDefaultStripSize(tif, width*numcolors));

		if (info.find("Artist") != info.end())  TIFFSetField(tif, TIFFTAG_ARTIST, info["Artist"].c_str());
		if (info.find("Make") != info.end())  TIFFSetField(tif, TIFFTAG_MAKE, info["Make"].c_str());
		if (info.find("Model") != info.end())  TIFFSetField(tif, TIFFTAG_MODEL, info["Model"].c_str());
		if (info.find("Software") != info.end())  TIFFSetField(tif, TIFFTAG_SOFTWARE, info["Software"].c_str());
		if (info.find("Copyright") != info.end())  TIFFSetField(tif, TIFFTAG_COPYRIGHT, info["Copyright"].c_str());
		if (info.find("LensInfo") != info.end())  TIFFSetField(tif, TIFFTAG_LENSINFO, info["LensInfo"].c_str());
		if (info.find("DateTime") != info.end()) TIFFSetField(tif, TIFFTAG_DATETIME, info["DateTime"].c_str());
		if (info.find("ImageDescription") != info.end())  TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, info["ImageDescription"].c_str());

		if (iccprofile) TIFFSetField(tif, TIFFTAG_ICCPROFILE, iccprofilelength, iccprofile);


		unsigned scanlinesize = TIFFScanlineSize(tif);
		buf = (unsigned char *) _TIFFmalloc(scanlinesize);
		img = imagedata;

		for (unsigned row = 0; row < height; row++)
		{
			memcpy(buf, img, scanlinesize);
			if (TIFFWriteScanline(tif, buf, row, 0) < 0) {
				printf("TIFFWriteScanline got an error...\n");
				TIFFError(NULL,NULL);
				break;
			}
			img+=scanlinesize;
		}
	}

	(void) TIFFClose(tif);
	if (buf) _TIFFfree(buf);
	return true;
*/
	return true;
}

