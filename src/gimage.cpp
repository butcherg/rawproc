
#include "gimage/gimage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <exception>
#include <algorithm> 
#include <fstream>
#include <sstream>
#include <omp.h>
#include <exception>
#include "rawimage.h"
#include "jpegimage.h"
#include "tiffimage.h"
#include "pngimage.h"
#include "gimage/strutil.h"
#include "cJSON.h"
#include <rtprocess/librtprocess.h>
#include "gimage/half.hpp"
//#include <rtprocess/jaggedarray.h>  //maybe, later....

#define PI            3.14159265358979323846
#ifndef M_PI
#define M_PI PI
#endif

#if defined PIXhalf
#define PIXHALF
using half_float::half;
using namespace half_float::literal;
#define fmin half_float::fmin
#define fmax half_float::fmax
#elif defined PIXfloat
#define PIXFLOAT
//#define fmin std::fmin
//#define fmax std::fmax
#else
#define PIXDOUBLE
//#define fmin std::fmin
//#define fmax std::fmax
#endif

//Range 0.0-1.0 constants
#if defined PIXHALF
#define SCALE_16BIT 65536.0
#define SCALE_8BIT 256.0_h
#define SCALE_CURVE 256.0_h
#elif defined PIXFLOAT
#define SCALE_16BIT 65536.0f
#define SCALE_8BIT 256.0f
#define SCALE_CURVE 256.0f
#else
#define SCALE_16BIT 65536.0
#define SCALE_8BIT 256.0
#define SCALE_CURVE 256.0
#endif

inline unsigned sqr(const unsigned x) { return x*x; }

const char * gImageVersion()
{
	#ifdef VERSION
	return VERSION;
	#else 
	return "";
	#endif
}

std::string gImage::profilepath;


//Constructors/Destructor:

gImage::gImage() 
{
	w=0; 
	h=0;
	profile = NULL;
	lasterror = GIMAGE_OK;
}

gImage::gImage(const gImage &o)
{
	w = o.w;
	h = o.h;
	c = o.c;
	b = o.b;
	
	imginfo = o.imginfo;
	image = o.image;
	
	lasterror = GIMAGE_OK;

	if (o.profile && o.profile_length !=0) {
		profile = new char[o.profile_length];
		memcpy(profile, o.profile, o.profile_length);
		profile_length = o.profile_length;
	}
	else {
		profile = NULL;
		profile_length = 0;
	}
}

gImage::gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, BPP bits, std::map<std::string,std::string> imageinfo, char * icc_profile, unsigned icc_profile_length)
{
	image.resize(width*height);
	w=width;
	h=height;
	c=colors;
	b=bits;
	profile=NULL;
	profile_length=0;
	lasterror = GIMAGE_OK;

	if (bits ==BPP_16) {
		unsigned short * src = (unsigned short *) imagedata;
		if (colors == 1) {  //turn into a three-color grayscale
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (PIXTYPE) ((unsigned short) src[0]/SCALE_16BIT);
					image[pos].g = (PIXTYPE) ((unsigned short) src[0]/SCALE_16BIT);
					image[pos].b = (PIXTYPE) ((unsigned short) src[0]/SCALE_16BIT);
					src += 1;
				}
			}
			c = 3;
		}
		else if (colors == 3) {
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (PIXTYPE) ((unsigned short) src[0]/SCALE_16BIT);
					image[pos].g = (PIXTYPE) ((unsigned short) src[1]/SCALE_16BIT);
					image[pos].b = (PIXTYPE) ((unsigned short) src[2]/SCALE_16BIT);
					src += 3;
				}
			}
		}
		else if (colors == 4) {
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (PIXTYPE) ((unsigned short) src[0]/SCALE_16BIT);
					image[pos].g = (PIXTYPE) ((unsigned short) src[1]/SCALE_16BIT);
					image[pos].b = (PIXTYPE) ((unsigned short) src[2]/SCALE_16BIT);
					src += 4;
				}
			}
			c = 3;
		}
		else {
			w = 0;
			h = 0;
			return;
		}
	}					

	else if (bits == BPP_8) {
		char * src = (char *) imagedata;
		if (colors == 1) {  //turn into a three-color grayscale
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (PIXTYPE) ((unsigned char) src[0]/SCALE_8BIT);
					image[pos].g = (PIXTYPE) ((unsigned char) src[0]/SCALE_8BIT);
					image[pos].b = (PIXTYPE) ((unsigned char) src[0]/SCALE_8BIT);
					src += 1;
				}
			}
			c = 3;
		}
		else if (colors == 3) {
			for (unsigned y=0; y<height; y++) {
				for (unsigned x=0; x<width; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (PIXTYPE) ((unsigned char) src[0]/SCALE_8BIT);
					image[pos].g = (PIXTYPE) ((unsigned char) src[1]/SCALE_8BIT);
					image[pos].b = (PIXTYPE) ((unsigned char) src[2]/SCALE_8BIT);
					src += 3;
				}
			}
		}
		else if (colors == 4) {
			for (unsigned y=0; y<height; y++) {
				for (unsigned x=0; x<width; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (PIXTYPE) ((unsigned char) src[0]/SCALE_8BIT);
					image[pos].g = (PIXTYPE) ((unsigned char) src[1]/SCALE_8BIT);
					image[pos].b = (PIXTYPE) ((unsigned char) src[2]/SCALE_8BIT);
					src += 4;
				}
			}
			c = 3;
		}
		else {
			w = 0;
			h = 0;
			return;
		}
	}

	else if (bits == BPP_FP | bits == BPP_UFP) {
		float * src = (float *) imagedata;
		if (colors == 1) {  //turn into a three-color grayscale
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					image[pos].r = src[0];
					image[pos].g = src[0];
					image[pos].b = src[0];
					src += 1;
				}
			}
			c = 3;
		}
		else if (colors == 3) {
			for (unsigned y=0; y<height; y++) {
				for (unsigned x=0; x<width; x++) {
					unsigned pos = x + y*w;
					image[pos].r = src[0];
					image[pos].g = src[1];
					image[pos].b = src[2];
					src += 3;
				}
			}
		}
		else if (colors == 4) {
			for (unsigned y=0; y<height; y++) {
				for (unsigned x=0; x<width; x++) {
					unsigned pos = x + y*w;
					image[pos].r = src[0];
					image[pos].g = src[1];
					image[pos].b = src[2];
					src += 4;
				}
			}
			c = 3;
		}
		else {
			w = 0;
			h = 0;
			return;
		}

	}

	else {
		w = 0;
		h = 0;
		return;
	}

	imginfo = imageinfo;

	if (icc_profile) {
		profile = new char[icc_profile_length];
		memcpy(profile, icc_profile, icc_profile_length);
		//delete [] icc_profile;
		profile_length = icc_profile_length;
	}
	else {
		profile = NULL;
		profile_length = 0;
	}
}

gImage::gImage(unsigned width, unsigned height, unsigned colors, std::map<std::string,std::string> imageinfo)
{
	image.resize(width*height);
	w=width;
	h=height;
	c=colors;
	b=BPP_UFP;
	lasterror = GIMAGE_OK;

	for (unsigned y=0; y<height; y++) {
		for (unsigned x=0; x<width; x++) {
			unsigned pos = x + y*w;
			image[pos].r = (PIXTYPE) 0.0;
			image[pos].g = (PIXTYPE) 0.0;
			image[pos].b = (PIXTYPE) 0.0;
		}
	}

	imginfo = imageinfo;
	profile =  NULL;
	profile_length = 0;
}


gImage::~gImage()
{
	if (profile) delete [] profile;
}


//Getters:

std::string gImage::getRGBCharacteristics()
{
	std::string charac;
	if (sizeof(PIXTYPE) == 2) charac.append("half, ");
	if (sizeof(PIXTYPE) == 4) charac.append("float, ");
	if (sizeof(PIXTYPE) == 8) charac.append("double, ");
	if (SCALE_CURVE == 1.0)   charac.append("0.0 - 255.0");
	if (SCALE_CURVE == 256.0) charac.append("0.0 - 1.0");
	return charac;
}

pix nullpix = {(PIXTYPE) 0.0, (PIXTYPE) 0.0, (PIXTYPE) 0.0};

pix gImage::getPixel(unsigned x,  unsigned y)
{
	
	int i = x + y*w;
	if ((x < w) && (y < h))
		return image[i];
	else
		return nullpix;
}

std::vector<float> gImage::getPixelArray(unsigned x,  unsigned y)
{
	int i = x + y*w;
	std::vector<float> pixel;
	pixel.resize(3);
	if (!image.empty()) {
		if (i < w * h) {
			if (i < image.size()) {
				pixel[0] = (float) image[i].r;
				pixel[1] = (float) image[i].g;
				pixel[2] = (float) image[i].b;
			}
		}
	}
	else {
		pixel[0] = 0.0;
		pixel[1] = 0.0;
		pixel[2] = 0.0;
	}
	return pixel;
}


//structs for making raw images
struct cpix { unsigned char r, g, b; };
struct uspix { unsigned short r, g, b; };
struct fpix { float r, g, b; };

//Lets LittleCMS do both the profile transform and data type conversion:
char * gImage::getTransformedImageData(BPP bits, cmsHPROFILE profile, cmsUInt32Number intent)
{
	cmsHPROFILE hImgProfile;
	cmsUInt32Number informat, outformat;
	cmsHTRANSFORM hTransform;
	char * imagedata = NULL;
	pix* img = image.data();

	if (sizeof(PIXTYPE) == 2) informat = TYPE_RGB_HALF_FLT; 
	if (sizeof(PIXTYPE) == 4) informat = TYPE_RGB_FLT;
	if (sizeof(PIXTYPE) == 8) informat = TYPE_RGB_DBL;
	
	hImgProfile = cmsOpenProfileFromMem(getProfile(), getProfileLength());
	
	if (hImgProfile != NULL & profile != NULL) {
		if (bits == BPP_16) {
			imagedata = new char[w*h*c*2];
			outformat = TYPE_RGB_16;
			uspix * imgdata = (uspix *) imagedata;
			hTransform = cmsCreateTransform(hImgProfile, informat, profile, outformat, intent, 0);
			#pragma omp parallel for
			for (unsigned y=0; y<h; y++) {
				unsigned pos = y*w;
				cmsDoTransform(hTransform, &img[pos], &imgdata[pos], w);
			}
		}
		else if (bits == BPP_8) {
			imagedata = new char[w*h*c];
			outformat = TYPE_RGB_8;
			cpix * imgdata = (cpix *) imagedata;
			hTransform = cmsCreateTransform(hImgProfile, informat, profile, outformat, intent, 0);
			#pragma omp parallel for
			for (unsigned y=0; y<h; y++) {
				unsigned pos = y*w;
				cmsDoTransform(hTransform, &img[pos], &imgdata[pos], w);
			}
		}
		else if (bits == BPP_FP | bits == BPP_UFP) {
			imagedata = new char[w*h*c*sizeof(float)];
			outformat = TYPE_RGB_FLT;
			fpix * imgdata = (fpix *) imagedata;
			hTransform = cmsCreateTransform(hImgProfile, informat, profile, outformat, intent, 0);
			#pragma omp parallel for
			for (unsigned y=0; y<h; y++) {
				unsigned pos = y*w;
				cmsDoTransform(hTransform, &img[pos], &imgdata[pos], w);
			}
		}
		else
			return NULL;
	}

	return imagedata;
}

char * gImage::getTransformedImageData(BPP bits, cmsHTRANSFORM transform)
{
	cmsUInt32Number informat, outformat;
	char * imagedata = NULL;
	pix* img = image.data();

	//if (sizeof(PIXTYPE) == 2) informat = TYPE_RGB_HALF_FLT; 
	//if (sizeof(PIXTYPE) == 4) informat = TYPE_RGB_FLT;
	//if (sizeof(PIXTYPE) == 8) informat = TYPE_RGB_DBL;
	
	if (transform != NULL) {
		if (bits == BPP_16) {
			//imagedata = new char[w*h*c*2];
			imagedata = (char *) malloc(w*h*c*sizeof(unsigned short));
			//outformat = TYPE_RGB_16;
			uspix * imgdata = (uspix *) imagedata;
			#pragma omp parallel for
			for (unsigned y=0; y<h; y++) {
				unsigned pos = y*w;
				cmsDoTransform(transform, &img[pos], &imgdata[pos], w);
			}
		}
		else if (bits == BPP_8) {
			//imagedata = new char[w*h*c];
			imagedata = (char *) malloc(w*h*c);
			//outformat = TYPE_RGB_8;
			cpix * imgdata = (cpix *) imagedata;
			#pragma omp parallel for
			for (unsigned y=0; y<h; y++) {
				unsigned pos = y*w;
				cmsDoTransform(transform, &img[pos], &imgdata[pos], w);
			}
		}
		else if (bits == BPP_FP | bits == BPP_UFP) {
			//imagedata = new char[w*h*c*sizeof(float)];
			imagedata = (char *) malloc(w*h*c*sizeof(float));
			//outformat = TYPE_RGB_FLT;
			fpix * imgdata = (fpix *) imagedata;
			#pragma omp parallel for
			for (unsigned y=0; y<h; y++) {
				unsigned pos = y*w;
				cmsDoTransform(transform, &img[pos], &imgdata[pos], w);
			}
		}
		else
			return NULL;
	}

	return imagedata;
}

float * gImage::getImageDataFloat(bool unbounded, cmsHPROFILE profile, cmsUInt32Number intent)
{
	cmsHPROFILE hImgProfile;
	//cmsUInt32Number format;
	cmsUInt32Number informat;
	cmsHTRANSFORM hTransform;
	//float * imagedata;
	unsigned imagesize = w*h;
	fpix * imagedata = new fpix[imagesize];
	pix * img = image.data();

	if (unbounded)
		#pragma omp parallel for
		for (unsigned i=0; i<imagesize; i++) {
			imagedata[i].r = (PIXTYPE) img[i].r;
			imagedata[i].g = (PIXTYPE) img[i].g;
			imagedata[i].b = (PIXTYPE) img[i].b;
		}
	else
		#pragma omp parallel for
		for (unsigned i=0; i<imagesize; i++) {
			#if defined PIXHALF
			imagedata[i].r = (PIXTYPE) fmin(fmax(img[i].r,0.0_h),1.0_h); 
			imagedata[i].g = (PIXTYPE) fmin(fmax(img[i].g,0.0_h),1.0_h); 
			imagedata[i].b = (PIXTYPE) fmin(fmax(img[i].b,0.0_h),1.0_h); 
			#elif defined PIXFLOAT
			imagedata[i].r = (PIXTYPE) fmin(fmax(img[i].r,0.0f),1.0f); 
			imagedata[i].g = (PIXTYPE) fmin(fmax(img[i].g,0.0f),1.0f); 
			imagedata[i].b = (PIXTYPE) fmin(fmax(img[i].b,0.0f),1.0f); 
			#else
			imagedata[i].r = (PIXTYPE) fmin(fmax(img[i].r,0.0),1.0); 
			imagedata[i].g = (PIXTYPE) fmin(fmax(img[i].g,0.0),1.0); 
			imagedata[i].b = (PIXTYPE) fmin(fmax(img[i].b,0.0),1.0); 
			#endif
		}

	if (profile) {
		if (sizeof(PIXTYPE) == 2) informat = TYPE_RGB_HALF_FLT; 
		if (sizeof(PIXTYPE) == 4) informat = TYPE_RGB_FLT;
		if (sizeof(PIXTYPE) == 8) informat = TYPE_RGB_DBL;
		hImgProfile = cmsOpenProfileFromMem(getProfile(), getProfileLength());
		if (hImgProfile != NULL & profile != NULL) {
			hTransform = cmsCreateTransform(hImgProfile, informat, profile, TYPE_RGB_FLT, intent, 0);
			cmsCloseProfile(hImgProfile);
			cmsDoTransform(hTransform, imagedata, imagedata, imagesize);
		}
	}

	return (float *) imagedata;
}



//Converts the data to the specified BPP integer format, then performs the profile transform:
char * gImage::getImageData(BPP bits, cmsHPROFILE profile, cmsUInt32Number intent)
{
	cmsHPROFILE hImgProfile;
	cmsUInt32Number format;
	cmsHTRANSFORM hTransform;
	char * imagedata;
	if (bits == BPP_16)
		imagedata = new char[w*h*c*sizeof(unsigned short)];
	else if (bits == BPP_FP | bits == BPP_UFP)
		imagedata = new char[w*h*c*sizeof(float)];
	else if (bits == BPP_8)
		imagedata = new char[w*h*c];
	else
		return NULL;

	if (bits == BPP_16) {
		uspix * dst = (uspix *) imagedata;
		#pragma omp parallel for
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				unsigned pos = x + y*w;
				#if defined PIXHALF
				dst[pos].r = (unsigned short) lrint(fmin(fmax(image[pos].r*(half_float::half) SCALE_16BIT,0.0_h),65535.0_h)); 
				dst[pos].g = (unsigned short) lrint(fmin(fmax(image[pos].g*(half_float::half) SCALE_16BIT,0.0_h),65535.0_h));
				dst[pos].b = (unsigned short) lrint(fmin(fmax(image[pos].b*(half_float::half) SCALE_16BIT,0.0_h),65535.0_h)); 
				#elif PIXTYPE == float
				dst[pos].r = (unsigned short) lrint(fmin(fmax(image[pos].r*(float) SCALE_16BIT,0.0f),65535.0f)); 
				dst[pos].g = (unsigned short) lrint(fmin(fmax(image[pos].g*(float) SCALE_16BIT,0.0f),65535.0f));
				dst[pos].b = (unsigned short) lrint(fmin(fmax(image[pos].b*(float) SCALE_16BIT,0.0f),65535.0f)); 
				#else
				dst[pos].r = (unsigned short) lrint(fmin(fmax(image[pos].r*(double) SCALE_16BIT,0.0),65535.0)); 
				dst[pos].g = (unsigned short) lrint(fmin(fmax(image[pos].g*(double) SCALE_16BIT,0.0),65535.0));
				dst[pos].b = (unsigned short) lrint(fmin(fmax(image[pos].b*(double) SCALE_16BIT,0.0),65535.0)); 
				#endif
				}
		}
		format = TYPE_RGB_16;
	}

	if (bits == BPP_8) {
		cpix * dst = (cpix *) imagedata;
		#pragma omp parallel for
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				unsigned pos = x + y*w;
				#if defined PIXHALF
				dst[pos].r = (unsigned char) lrint(fmin(fmax(image[pos].r*(half_float::half) SCALE_8BIT,0.0_h),255.0_h)); 
				dst[pos].g = (unsigned char) lrint(fmin(fmax(image[pos].g*(half_float::half) SCALE_8BIT,0.0_h),255.0_h));
				dst[pos].b = (unsigned char) lrint(fmin(fmax(image[pos].b*(half_float::half) SCALE_8BIT,0.0_h),255.0_h)); 
				#elif defined PIXFLOAT
				dst[pos].r = (unsigned char) lrint(fmin(fmax(image[pos].r*(float) SCALE_8BIT,0.0f),255.0f)); 
				dst[pos].g = (unsigned char) lrint(fmin(fmax(image[pos].g*(float) SCALE_8BIT,0.0f),255.0f));
				dst[pos].b = (unsigned char) lrint(fmin(fmax(image[pos].b*(float) SCALE_8BIT,0.0f),255.0f)); 
				#else
				dst[pos].r = (unsigned char) lrint(fmin(fmax(image[pos].r*(double) SCALE_8BIT,0.0),255.0)); 
				dst[pos].g = (unsigned char) lrint(fmin(fmax(image[pos].g*(double) SCALE_8BIT,0.0),255.0));
				dst[pos].b = (unsigned char) lrint(fmin(fmax(image[pos].b*(double) SCALE_8BIT,0.0),255.0)); 
				#endif
			}
		}
		format = TYPE_RGB_8;
	}
	
	if (bits == BPP_FP) {
		fpix * dst = (fpix *) imagedata;
		#pragma omp parallel for
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				unsigned pos = x + y*w;
				#if defined PIXHALF
				dst[pos].r = fmin(fmax(image[pos].r,0.0_h),1.0_h); 
				dst[pos].g = fmin(fmax(image[pos].g,0.0_h),1.0_h); 
				dst[pos].b = fmin(fmax(image[pos].b,0.0_h),1.0_h); 
				#elif defined PIXFLOAT
				dst[pos].r = fmin(fmax(image[pos].r,0.0f),1.0f); 
				dst[pos].g = fmin(fmax(image[pos].g,0.0f),1.0f); 
				dst[pos].b = fmin(fmax(image[pos].b,0.0f),1.0f); 
				#else
				dst[pos].r = fmin(fmax(image[pos].r,0.0),1.0); 
				dst[pos].g = fmin(fmax(image[pos].g,0.0),1.0); 
				dst[pos].b = fmin(fmax(image[pos].b,0.0),1.0); 
				#endif
				}
		}
		format = TYPE_RGB_FLT;
	}
	
	if (bits == BPP_UFP) {
		fpix * dst = (fpix *) imagedata;
		#pragma omp parallel for
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				unsigned pos = x + y*w;
				dst[pos].r = image[pos].r; 
				dst[pos].g = image[pos].g; 
				dst[pos].b = image[pos].b; 
			}
		}
		format = TYPE_RGB_FLT;
	}

	if (profile) {
		hImgProfile = cmsOpenProfileFromMem(getProfile(), getProfileLength());
		if (hImgProfile != NULL & profile != NULL) {
			hTransform = cmsCreateTransform(hImgProfile, format, profile, format, intent, 0);
			cmsCloseProfile(hImgProfile);
			cmsDoTransform(hTransform, imagedata, imagedata, w*h);
		}
	}

	return imagedata;
}

std::vector<pix>& gImage::getImageData()
{
	return image;
}

pix* gImage::getImageDataRaw()
{
	return image.data();
}

unsigned gImage::getWidth()
{
	return w;
}

unsigned gImage::getHeight()
{
	return h;
}

unsigned gImage::getColors()
{
	return c;
}


BPP gImage::getBits()
{
	return b;
}

std::string gImage::getBitsStr()
{
	switch (b) {
		case BPP_FP: 
		case BPP_UFP:
			return "internal floating point";
		case BPP_8: return "8";
		case BPP_16: return "16";
	}
	return "";
}

GIMAGE_ERROR gImage::getLastError()
{
	return lasterror;
}

std::string gImage::getLastErrorMessage()
{
	if (lasterror == GIMAGE_OK) return "GIMAGE_OK";
	if (lasterror == GIMAGE_EXCEPTION) return "GIMAGE_EXCEPTION";
	if (lasterror == GIMAGE_UNSUPPORTED_PIXELFORMAT) return "GIMAGE_UNSUPPORTED_PIXELFORMAT";
	if (lasterror == GIMAGE_UNSUPPORTED_FILEFORMAT) return "GIMAGE_UNSUPPORTED_FILEFORMAT";
	
	if (lasterror == GIMAGE_APPLYCOLORSPACE_BADPROFILE) return "GIMAGE_APPLYCOLORSPACE_BADPROFILE";
	if (lasterror == GIMAGE_APPLYCOLORSPACE_BADINPUTPROFILE) return "GIMAGE_APPLYCOLORSPACE_BADINPUTPROFILE";
	if (lasterror == GIMAGE_APPLYCOLORSPACE_BADOUTPUTPROFILE) return "GIMAGE_APPLYCOLORSPACE_BADOUTPUTPROFILE";
	if (lasterror == GIMAGE_APPLYCOLORSPACE_BADINTENT) return "GIMAGE_APPLYCOLORSPACE_BADINTENT";
	if (lasterror == GIMAGE_APPLYCOLORSPACE_BADTRANSFORM) return "GIMAGE_APPLYCOLORSPACE_BADTRANSFORM";
	
	if (lasterror == GIMAGE_ASSIGNCOLORSPACE_BADTRANSFORM) return "GIMAGE_ASSIGNCOLORSPACE_BADTRANSFORM";
	return "(none)";
}


std::map<std::string,std::string> gImage::getInfo()
{
	return imginfo;
}

std::string gImage::getInfoValue(std::string name)
{
	if (imginfo.find(name) != imginfo.end())
		return imginfo[name];
	else
		return "";
}

char * gImage::getProfile()
{
	return profile;
}

unsigned gImage::getProfileLength()
{
	return profile_length;
}

std::string gImage::getProfilePath()
{
	return profilepath;
}

std::map<std::string,std::string> gImage::getInfo(const char * filename)
{
	unsigned width, height, colors, bpp, icclength;
	char * iccprofile;
	std::map<std::string, std::string> imgdata;
	GIMAGE_FILETYPE ftype = gImage::getFileType(filename);
	
	if (ftype == FILETYPE_TIFF) _loadTIFFInfo(filename, &width, &height, &colors, &bpp, imgdata);
	if (ftype == FILETYPE_RAW) _loadRAWInfo(filename, &width, &height, &colors, &bpp, imgdata);
	if (ftype == FILETYPE_JPEG) _loadJPEGInfo(filename, &width, &height, &colors, imgdata);
	if (ftype == FILETYPE_PNG) _loadPNGInfo(filename, &width, &height, &colors, &bpp, imgdata);
	return imgdata;
}

//Check the file type of an existing image file; is it suitable for opening?
GIMAGE_FILETYPE gImage::getFileType(const char * filename)
{
	std::string fname = filename;
	std::vector<std::string> fpieces =  split(fname, ".");
	if (fpieces.size() < 1) return FILETYPE_BAD_FILENAME;

	std::string ext = fpieces.back();

	if (	(ext.compare("tif")  == 0) | 
		(ext.compare("TIF")  == 0) |
		(ext.compare("TIFF") == 0) |
		(ext.compare("tiff") == 0)) 
			if (_checkTIFF(filename)) return FILETYPE_TIFF;
	if (	(ext.compare("jpg")  == 0) | 
		(ext.compare("jpeg") == 0) | 
		(ext.compare("JPEG") == 0) |
		(ext.compare("JPG")  == 0)) 
			if (_checkJPEG(filename)) return FILETYPE_JPEG;
	if (	(ext.compare("png")  == 0) | 
		(ext.compare("PNG")  == 0)) 
			if (_checkPNG(filename)) return FILETYPE_PNG;
	if (_checkRAW(filename)) return FILETYPE_RAW;

	return FILETYPE_UNKNOWN;
}

//Check the file type of an image file name; is it suitable to use for saving?
GIMAGE_FILETYPE gImage::getFileNameType(const char * filename)
{
	std::string fname = filename;
	std::vector<std::string> fpieces =  split(fname, ".");
	std::string ext = fpieces.back();

	if (ext.compare("tif") == 0 | ext.compare("tiff") == 0) return FILETYPE_TIFF;
	if ((ext.compare("jpg") == 0) | (ext.compare("JPG") == 0)) return FILETYPE_JPEG;
	if ((ext.compare("png") == 0) | (ext.compare("PNG") == 0)) return FILETYPE_PNG;
	return FILETYPE_UNKNOWN;
}

std::string gImage::Version()
{
	#ifdef VERSION
	return VERSION;
	#else 
	return "";
	#endif
}

std::string gImage::LibraryVersions()
{
	std::string verstring;
	verstring.append("JPEG: ");
	verstring.append(jpegVersion());
	verstring.append("\nTIFF: ");
	std::string tiffver(tiffVersion());
	verstring.append(tiffver.substr(0,tiffver.find_first_of("\n")));
	verstring.append("\nPNG: ");
	std::string pngver(pngVersion());
	verstring.append(pngver);
	verstring.append("\nLibRaw: ");
	verstring.append(librawVersion());
	verstring.append("\nLittleCMS2: ");
	std::ostringstream s;
	s << (int) cmsGetEncodedCMMversion();
	verstring.append(s.str());
	//verstring.append("\n");
	return verstring;

}

//Lensfun support methods
void gImage::initInterpolation(RESIZE_FILTER interp)
{
	lensfun_interp_method = interp;  //does nothing right now, all hard-coded to nearest-neighbor
}	

PIXTYPE gImage::getR(float x, float y)
{
	unsigned xi = unsigned (x + 0.5);
	unsigned yi = unsigned (y + 0.5);
	if (xi >= w || yi >= h)
		return (PIXTYPE) 0.0;

	unsigned pos = yi * w + xi;
	return image[pos].r;
}

PIXTYPE gImage::getG(float x, float y)
{
	unsigned xi = unsigned (x + 0.5);
	unsigned yi = unsigned (y + 0.5);
	if (xi >= w || yi >= h)
		return (PIXTYPE) 0.0;

	unsigned pos = yi * w + xi;
	return image[pos].g;
}

PIXTYPE gImage::getB(float x, float y)
{
	unsigned xi = unsigned (x + 0.5);
	unsigned yi = unsigned (y + 0.5);
	if (xi >= w || yi >= h)
		return (PIXTYPE) 0.0;

	unsigned pos = yi * w + xi;
	return image[pos].b;
}

pix gImage::getRGB(float x, float y)
{
	unsigned xi = unsigned (x + 0.5);
	unsigned yi = unsigned (y + 0.5);
	if (xi >= w || yi >= h)
		return nullpix;

	unsigned pos = yi * w + xi;
	return image[pos];
}



int gImage::ThreadCount()
{
#if defined(_OPENMP)
	return omp_get_max_threads();
#else
	return 1;
#endif
}

// Setters:

void gImage::setInfo(std::string name, std::string value)
{
	imginfo[name] = value;
}

void gImage::setProfile(char * prof, unsigned proflength)
{
	if (profile) delete [] profile;
	profile = prof;
	profile_length = proflength;
}

void gImage::deleteProfile()
{
	if (profile) delete [] profile;
	profile = NULL;
	profile_length = 0;
}

void gImage::setProfilePath(std::string ppath)
{
	profilepath = ppath;
}




//Image Operations:
//
//This is a compendium of basic image operations, kept simple for 
//ease of understanding while maintaining basic image quality




//Convolution Kernels and Sharpening
//
//Credit: Various
//
//Changing a pixel depending on the values of its neighbors is probably the
//second fundamental image transformation to understand, after so-called
//curve-based lookup tables.  Particularly, there are a set of matrices
//describing weights of the center pixel and its neigbors that produce
//various effects, such as blurring and edge highlighting.  The general
//algorith traverses the image and calculates a new value for each pixel
//based on the sum of the weighted values of its neighbors.
//
//In the ConvolutionKernal algorithm presented below, the inner kx and ky 
//loops do the collection of the weighted neighbor pixel values using the 
//kernel parameter, which is hard-coded to be a 3x3 matrix of doubles.  
//Larger matrices are discussed elsewhere for more control and 'quality', 
//but the simple 3x3 matrix is quite suitable for what I consider to be the 
//most useful convolution kernel application - sharpening downsized images.
//
//The Sharpen method below defines a kernel specific to image sharpening,
//or more specifically, the enhancement of edge contrast.  The kernel is 
//described in numerous places, usually as a moderate application:
//
// 0 -1  0
//-1  5 -1
// 0 -1  0
//
//So, if you multiply the source pixel by 5, and each of the surrounding
//pixels by the corresponding values, and then sum it all up, you get a
//new value for the source pixel that will present more 'edge contrast',
//or acutance.  The method actually implements a range of sharpening by
//computing a matrix based on a linear 'strength', 0=no sharpening, 
//10=ludicrous sharpening.  The matrix values are calculated from 
//the strength number, preserving the relationship expressed in the 
//matrix described above.
//
//There are more complex algorithms available for 'sharpening', as it 
//were, but I find the simple 3x3 convolution kernel to work nicely
//for re-introducing acutance lost in down-sizing an image for web
//publication.  The application for that purpose is most valuable 
//going from 0 to 1 in strength, for ~640x480 images; 2 and above 
//start to introduce the halos and ringing that most find offensive.
//


void gImage::ApplyConvolutionKernel(double kernel[3][3], int threadcount)
{
	std::vector<pix> src = image;

	#pragma omp parallel for num_threads(threadcount)
	for(int y = 1; y < h-1; y++) {
		for(int x = 1; x < w-1; x++) {
			int pos = x + y*w;
			double R=0.0; double G=0.0; double B=0.0;
			for (int kx=0; kx<3; kx++) {
				for (int ky=0; ky<3; ky++) {
					int kpos = w*(y-1+ky) + (x-1+kx);
					R += src[kpos].r * kernel[kx][ky];
					G += src[kpos].g * kernel[kx][ky];
					B += src[kpos].b * kernel[kx][ky];
				}
			}

			image[pos].r = R;
			image[pos].g = G;
			image[pos].b = B;
		}
	} 
}

void gImage::ApplySharpen(double strength, int threadcount)
{
	double kernel[3][3] =
	{
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 0.0
	};

	//build a kernel corresponding to the specified strength
	double x = -((strength)/4.0);
	kernel[0][1] = x;
	kernel[1][0] = x;
	kernel[1][2] = x;
	kernel[2][1] = x;
	kernel[1][1] = strength+1.0;

	ApplyConvolutionKernel(kernel, threadcount);
}


//Convolution Kernels and Blurring
//
//Credit: Various
//
//It was recently brought to my attention (https://discuss.pixls.us/t/sharpening-opinions-please/9007/6)
//that for down-size resizing various noises can be mitigated by first blurring the full-sized
//image with a kernel sized based on the reduction, e.g., 3x3 kernel if downsizing 3x, as the
//interpolation to the resized pixels are going to lose that information anyway.  So, here are
//some gaussian blur methods.  There are two new Apply*ConvolutionKernel methods that both use
//std::vector<float> for the kernel, a regular 2D method and a 1D method usable with the 
//"regular" gaussian blur kernel.  A kernel compute function is specified, not part of the gImage
//class because I'm not yet ready to settle on it.


std::vector<float> gImage::Compute1DGaussianKernel(const int kernelsize, const float sigma)
{
	int ksize = kernelsize;
	if ((kernelsize % 2) == 0) ksize++;
	std::vector<float> kernel; //+1);
	float sum = 0.0f;
	float x = -(ksize/2);

	//computes the kernel using the one-dimensional function from https://en.wikipedia.org/wiki/Gaussian_blur
	float denominator = sqrt(2.0*M_PI*sigma*sigma);
	for (int i=0; i<ksize; i++) {
		float e_val = exp(-(x*x)/(2.0*sigma*sigma));
		float k = (1.0/denominator)*e_val; //per the Wikipedia function
		kernel.push_back(k);
		sum += k;
		x++;  //distance in terms of whole pixels, so center-to-center
	}

	//normalize kernel:
	for (int i=0; i<ksize; i++)
		kernel[i] /= sum;

	return kernel;
}



//arbitrarily-dimensioned 1D kernels, applied in two passes:
void gImage::Apply1DConvolutionKernel(std::vector<float> kernel, int threadcount)
{
	std::vector<pix> src = image;
	int kerneldimension = kernel.size();
	unsigned offset = floor(kerneldimension/2.0);


	#pragma omp parallel for num_threads(threadcount)
	for(int y = offset; y < h-offset; y++) {
		for(int x = offset; x < w-offset; x++) {
			int pos = x + y*w;
			double R=0.0; double G=0.0; double B=0.0;
			for (int k=0; k<kerneldimension; k++) {
				int kpos = x-offset+k + y*w;  //left-to-right, from offset
				R += src[kpos].r * kernel[k];
				G += src[kpos].g * kernel[k];
				B += src[kpos].b * kernel[k];
			}
			image[pos].r = R;
			image[pos].g = G;
			image[pos].b = B;
		}
	}

	src = image;

	#pragma omp parallel for num_threads(threadcount)
	for(int y = offset; y < h-offset; y++) {
		for(int x = offset; x < w-offset; x++) {
			int pos = x + y*w;
			double R=0.0; double G=0.0; double B=0.0;
			for (int k=0; k<kerneldimension; k++) {
				int kpos = x + (y-offset+k)*w;  //top-to-bottom, from offset
				R += src[kpos].r * kernel[k];
				G += src[kpos].g * kernel[k];
				B += src[kpos].b * kernel[k];

			}
			image[pos].r = R;
			image[pos].g = G;
			image[pos].b = B;
		}
	}
}

//arbitrarily-dimensioned 2D kernels, access with p = x + y*kerneldimension :
void gImage::Apply2DConvolutionKernel(std::vector<float> kernel, int kerneldimension, int threadcount)
{
	std::vector<pix> src = image;
	unsigned offset = floor(kerneldimension/2.0);

	#pragma omp parallel for num_threads(threadcount)
	for(int y = offset; y < h-offset; y++) {
		for(int x = offset; x < w-offset; x++) {
			int pos = x + y*w;
			double R=0.0; double G=0.0; double B=0.0;
			for (int kx=0; kx<kerneldimension; kx++) {
				for (int ky=0; ky<kerneldimension; ky++) {
					int k = kx + ky * kerneldimension;
					int kpos = w*(y-offset+ky) + (x-offset+kx);
					R += src[kpos].r * kernel[k];
					G += src[kpos].g * kernel[k];
					B += src[kpos].b * kernel[k];
				}
			}

			image[pos].r = R;
			image[pos].g = G;
			image[pos].b = B;
		}
	} 
}

void gImage::ApplyGaussianBlur(double sigma, unsigned kernelsize, int threadcount)
{
	std::vector<float> kernel;
	kernel =  Compute1DGaussianKernel(kernelsize, sigma);
	Apply1DConvolutionKernel(kernel, threadcount);
}





//Rotate
//
//Credit: Alan Paeth, "A Fast Algorithm for General Raster Rotation", Graphics Interface '86
//	  http://supercomputingblog.com/graphics/coding-bilinear-interpolation/
//
//Image rotation is a deceivingly complex operation.  One would think it to be sufficient to 
//apply the sine and cosine of the rotation angle to each pixel coordinate, but you also have to
//manage the new image boundary, and the image can develop blocky aliasing in the edges.
//I started my exploration of image rotation with such an approach, and I've left it in 
//commented-out.
//
//These days, a lot of image rotation is done using Paeth's approach, which is the 
//application of three shears, one in the X direction, one in the Y direction, and a final
//one in the X direction to produce the rotation.  One wonders about the thinking required 
//to determine such transformations would produce the desired result.  
//
//Three methods are presented to perform Paeth roatation: XShear and YShear, and a Rotate
//method to use them in the proscribed order.  Not implemented at this writing is an 
//intelligent adjustment of the rotated pixel to reflect its changed relationship with 
//adjunct pixels.  This does not seem to be an egregious issue with large out-of-camera
//images at small angles of rotation.  I've replaced my shear implementation of ApplyRotate,
//but I've left the code in, commented-out, for the time being (1/29/2017)
//
//After messing around with interpolation in shears, I decided to switch back to a sin-cos
//rotation approach.  The key consideration was straightforward application of bilinear
//interpolation, which requires sampling the pixels comprising the "destination" pixel from
//the reverse rotation.  This hurt my head for a bit, but all came clear when I built the 
//loop around the new (resized) destination image.  My bilinear interpolation is a copy-paste
//from the supercomputingblog.com code, so here's the compliance comment for its use:

/*

This code is brought to you by the supercomputingblog.com
You may use this code for any purpose. However,
you may redistribute this code only if this comment
remains intact.

*/

//There's no name I can find on the blog site to which to attribute the source; thanks, whomever
//you are...



#define ROTATE_BILINEAR_INTERPOLATION

// sin-cos rotate, with bilnear interpolation adapted from
// http://supercomputingblog.com/graphics/coding-bilinear-interpolation/
// Comment out the #define of ROTATE_BILINEAR_INTERPOLATION, recompile, and see the difference...

void gImage::ApplyRotate(double angle, bool crop, int threadcount)
{
	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;

	double rangle = -(-angle * PI / 180.0); // negate for destination interpolation
        double cosine = cos(rangle);
        double sine = sin(rangle);

	int width = w;
	int height = h;
	double cX = (double)width/2.0f;
	double cY = (double)height/2.0f;

	double brangle = (-angle * PI / 180.0);
	double bcosine = cos(brangle);
	double bsine = sin(brangle);

	//compute bounding box:
	int x = 0;
	int y = 0;
	int x1 = cX+(x-cX)*bcosine+(y-cY)*bsine;
	int y1 = cY-(x-cX)*bsine+(y-cY)*bcosine;

	x = w;
	y = 0;
	int x2 = cX+(x-cX)*bcosine+(y-cY)*bsine;
	int y2 = cY-(x-cX)*bsine+(y-cY)*bcosine;

	x = 0;
	y = h;
	int x3 = cX+(x-cX)*bcosine+(y-cY)*bsine;
	int y3 = cY-(x-cX)*bsine+(y-cY)*bcosine;

	x = w;
	y = h;
	int x4 = cX+(x-cX)*bcosine+(y-cY)*bsine;
	int y4 = cY-(x-cX)*bsine+(y-cY)*bcosine;


	int minx = std::min(x1, std::min(x2, std::min(x3,x4)));
	int maxx = std::max(x1, std::max(x2, std::max(x3,x4)));

	int miny = std::min(y1, std::min(y2, std::min(y3,y4)));
	int maxy = std::max(y1, std::max(y2, std::max(y3,y4)));

	int nw = maxx-minx;
	int nh = maxy-miny;

	// variables for coordinate translation:
	int dw = nw - w;
	int dh = nh - h ;

	int tx = nw/2;
	int ty = nh/2;

	//prep image for rotated result:
	image.resize(nw*nh);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<nh; y++) {
		for (unsigned x=0; x<nw; x++) {

			int ux = x - tx;
			int uy = y - ty;

			double du = ( cosine * ux + sine * uy);
			double dv = (-sine * ux + cosine * uy);

			int u = du + tx - dw/2;
			int v = dv + ty - dh/2;


			if (u < 0 | u >= w | v < 0 | v >= h) {
				image[x + y*nw].r = 0.0;
				image[x + y*nw].g = 0.0;
				image[x + y*nw].b = 0.0;
			}
	
		else {

#ifdef ROTATE_BILINEAR_INTERPOLATION 
			//The following makes more sense when studied with the graphic on page 1
			//of the supercomputingblog.com post

			double xPrime = du + double(tx) - double(dw)/2.0;
			double yPrime = dv + double(ty) - double(dh)/2.0;

			int q12x = (int)floor(xPrime);
			int q12y = (int)floor(yPrime);
			q12x = std::max(0, q12x);
			q12y = std::max(0, q12y);
			q12x = std::min(width-1, q12x);
			q12y = std::min(height-1, q12y);
			int q22x = (int)ceil(xPrime);
			int q22y = q12y;
			q22x = std::min(width-1, q22x);
			q22x = std::max(0, q22x);
			int q11x = q12x;
			int q11y = (int)ceil(yPrime);
			q11y = std::min(height-1, q11y);
			q11y = std::max(0, q11y);
			int q21x = q22x;
			int q21y = q11y;
			pix q11 = src[q11y*w + q11x];
			pix q12 = src[q12y*w + q12x];
			pix q21 = src[q21y*w + q21x];
			pix q22 = src[q22y*w + q22x];

			double factor1;
			double factor2;
			
			if ( q21x == q11x ) // special case to avoid divide by zero
			{
				factor1 = 1; // They're at the same X coordinate, so just force the calculatione to one point
				factor2 = 0;
			}
			else
			{
				factor1 = (((double)q21x - (double)xPrime)/((double)q21x - (double)q11x));
				factor2 = (((double)xPrime - (double)q11x)/((double)q21x - (double)q11x));
			}

			double R1r = factor1 * (double)q11.r + factor2*(double)q21.r;
			double R1g = factor1 * (double)q11.g + factor2*(double)q21.g;
			double R1b = factor1 * (double)q11.b + factor2*(double)q21.b;
			double R2r = factor1 * (double)q12.r + factor2*(double)q22.r;
			double R2g = factor1 * (double)q12.g + factor2*(double)q22.g;
			double R2b = factor1 * (double)q12.b + factor2*(double)q22.b;
			double factor3;
			double factor4;
			if (q12y == q11y) // special case to avoid divide by zero
			{
				factor3 = 1;
				factor4 = 0;
			}
			else
			{
				factor3 = ((double) q12y - yPrime)/((double)q12y - (double)q11y);
				factor4 = (yPrime - (double)q11y)/((double)q12y - (double)q11y);
			}

			//remainder modified to use the gImage pix struct:
			pix finalpix;
	
			finalpix.r = ((factor3 * R1r) + (factor4*R2r));
			finalpix.g = ((factor3 * R1g) + (factor4*R2g));
			finalpix.b = ((factor3 * R1b) + (factor4*R2b));

			image[x + y*nw] = finalpix;

#else
			//Plain old nearest-source to destination copy operation
			image[x + y*nw] = src[u + v*w];

#endif //ROTATE_BILINEAR_INTERPOLATION 

			}

		}
	}

	w = nw;
	h = nh;
	delete s;
	if (crop) {
		if (rangle < 0)
			ApplyCrop(x3+dw/2, y1+dh/2, x2+dw/2,  y4+dh/2, threadcount);
		else
			ApplyCrop(x1+dw/2, y2+dh/2, x4+dw/2,  y3+dh/2, threadcount);
	}
}


//Trivial Rotate, 90, 180, and 270 degrees
//
//I was only going to implement the interpolation-based +/-45 degree rotate,
//but it turns out the 'cardinal' rotations are needed to accommodate the silliness
//of the EXIF Orientation tag.  When you turn your camera off-axis to shoot portrait
//or somesuch, the camera probably doesn't rotate the image.  Instead, it sets the Orientation
//tag to a corresponding code, thinking that all downstream software will use it to 
//properly orient your image for display.  Well, no, not all do that, and further, 
//some programs that actually modify your image with the appropriate rotation neglect
//to set the Orientation tag accordingly, which further confuses software that wants
//to use it.  Geesh.
//
//So, here are the needed transforms for 'really intelligent' software to modify-rotate
//the image to the intended orientation, and pray that the Orientation flag will be 
//set to 0 to make the world right...
//


void gImage::ApplyRotate180(int threadcount)
{
	std::vector<pix> src = image;
	
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned spos = x + y*w;
			unsigned dpos = (w-x-1) + (h-y-1)*w;
			image[dpos] = src[spos];
		}
	}
}

void gImage::ApplyRotate90(int threadcount)
{
	std::vector<pix> src = image;
	
	unsigned dw = h;
	unsigned dh = w;
	
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned spos = x + y*w;
			//unsigned dpos = (dw-y-1) + (dh-x-1)*dw;
			unsigned dpos = (dw-y-1) + x * dw;
			image[dpos] = src[spos];
		}
	}
	w = dw;
	h = dh;
}

void gImage::ApplyRotate270(int threadcount)
{
	std::vector<pix> src = image;
	
	unsigned dw = h;
	unsigned dh = w;
	
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned spos = x + y*w;
			unsigned dpos = y + (dh-x-1)*dw;
			image[dpos] = src[spos];
		}
	}
	w = dw;
	h = dh;
}

void gImage::ApplyHorizontalMirror(int threadcount)
{
	std::vector<pix> src = image;
	
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned spos = x + y*w;
			unsigned dpos = (w-x-1) + y*w;
			image[dpos] = src[spos];
		}
	}
}

void gImage::ApplyVerticalMirror(int threadcount)
{
	std::vector<pix> src = image;
	
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned spos = x + y*w;
			unsigned dpos = x + (h-y-1)*w;
			image[dpos] = src[spos];
		}
	}
}

void gImage::NormalizeRotation(int threadcount)
{
	int rotation = atoi(getInfoValue("Orientation").c_str());
	if (rotation != 1) {
		if (rotation == 2) ApplyHorizontalMirror(threadcount);
		if (rotation == 3) ApplyRotate180(threadcount);
		if (rotation == 4) ApplyVerticalMirror(threadcount);
		if (rotation == 5) {ApplyRotate90(threadcount); ApplyHorizontalMirror(threadcount);}
		if (rotation == 6) ApplyRotate90(threadcount);
		if (rotation == 7) {ApplyRotate270(threadcount); ApplyHorizontalMirror(threadcount); }
		if (rotation == 8) ApplyRotate270(threadcount);
		setInfo("Orientation","1");
	}
}



/* Rotate by three Shears:
void gImage::ApplyRotate(double angle, int threadcount)
{
	//gImage I, J, K, L;
	unsigned x1, x2, y1, y2;
	unsigned cx1, cx2, cy1, cy2;
	double rangle = angle * PI / 180.0;

	ApplyXShear(rangle,threadcount);
	ApplyYShear(rangle,threadcount);
	ApplyXShear(rangle,threadcount);
	ImageBounds(&x1, &x2, &y1, &y2);
	//ImageBounds(&cx1, &cx2, &cy1, &cy2, true);
	ApplyCrop(x1, y1, x2, y2,threadcount);
	//ApplyCrop(cx1, cy1, cx2, cy2,threadcount);
}
*/

/*  original shears, use these if you just want to try the method without slinging code:
void gImage::ApplyXShear(double rangle, int threadcount)
{
	//double sine = sin(rangle);
	double tangent = tan(rangle/2.0);
	int dw = tangent * (double) h;
	unsigned nw = w+abs(dw);

	//nearest neighbor stuff:
	double ddw = tangent * (double) h;
	double lw = ddw - (double) dw;
	double rw = 1.0 - lw;

	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	image.resize(nw*h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i = 0; i<image.size(); i++) {
		image[i].r = 0.0;
		image[i].g = 0.0;
		image[i].b = 0.0;
	}

	if (dw < 0) dw = 0;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned u = (x - tangent * y) + dw;
			unsigned v = y;
			if (u >= nw) continue;
			if (v >= h) continue;
			unsigned dpos =u + v*nw;
			unsigned spos = x + y*w;
			image[dpos].r = src[spos].r;
			image[dpos].g = src[spos].g;
			image[dpos].b = src[spos].b;
		}
	}

	w = nw;
	delete s;
}

void gImage::ApplyYShear(double rangle, int threadcount)
{
	double sine = sin(rangle);
	int dh = sine * (double) w;
	unsigned nh = h+abs(dh);
	unsigned dw = w;

	//nearest neighbor stuff:
	double ddh = sine * (double) (w);
	double lw = ddh - (double) dh;
	double rw = 1.0 - lw;

	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	std::vector<pix>& dst = getImageData();
	dst.resize(w*nh);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i = 0; i<image.size(); i++) {
		image[i].r = 0.0;
		image[i].g = 0.0;
		image[i].b = 0.0;
	}

	if (dh < 0) dh = -dh; else dh = 0;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned u = x;
			unsigned v = (y + sine * x) + dh;
			if (u >= w) continue;
			if (v >= nh) continue;
			unsigned dpos = u + v*dw;
			unsigned spos = x + y*w;
			dst[dpos].r = src[spos].r;
			dst[dpos].g = src[spos].g;
			dst[dpos].b = src[spos].b;
		}
	}

	h = nh;
	delete s;
}
*/  //end original shears


//second generation shears, not working...
/*
void gImage::ApplyXShear(double rangle, int threadcount)
{
	double tangent = tan(rangle/2.0);
	int dw = tangent * (double) h;
	unsigned nw = w+abs(dw);

	//nearest neighbor stuff:
	double ddw = tangent * (double) h;
	double lw = ddw - (double) dw;
	double rw = 1.0 - lw;

	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	image.resize(nw*h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i = 0; i<image.size(); i++) {
		image[i].r = 0.0;
		image[i].g = 0.0;
		image[i].b = 0.0;
	}

	if (dw < 0) dw = 0;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned u = (x - tangent * y) + dw;
			unsigned v = y;
			if (u >= nw) continue;
			if (v >= h) continue;
			unsigned dpos =u + v*nw;
			unsigned spos = x + y*w;
			unsigned spos1 = spos+1;
			if (spos1 >=w) spos1--;
			image[dpos].r = ((src[spos].r*lw)+(src[spos1].r*rw));
			image[dpos].g = ((src[spos].g*lw)+(src[spos1].g*rw));
			image[dpos].b = ((src[spos].b*lw)+(src[spos1].b*rw));
		}
	}


	w = nw;
	delete s;
}

void gImage::ApplyYShear(double rangle, int threadcount)
{
	double sine = sin(rangle);
	int dh = sine * (double) w;
	unsigned nh = h+abs(dh);
	unsigned dw = w;

	//nearest neighbor stuff:
	double ddh = sine * (double) (w);
	double lw = ddh - (double) dh;
	double rw = 1.0 - lw;

	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	std::vector<pix>& dst = getImageData();
	dst.resize(w*nh);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i = 0; i<image.size(); i++) {
		image[i].r = 0.0;
		image[i].g = 0.0;
		image[i].b = 0.0;
	}

	if (dh < 0) dh = -dh; else dh = 0;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned u = x;
			unsigned v = (y + sine * x) + dh;
			if (u >= w) continue;
			if (v >= nh) continue;
			unsigned dpos = u + v*dw;
			unsigned spos = x + y*w;
			unsigned spos1 = spos;
			if (y < h-1) unsigned spos1 = x + (y+1)*h;
			image[dpos].r = ((src[spos].r*lw)+(src[spos1].r*rw));
			image[dpos].g = ((src[spos].g*lw)+(src[spos1].g*rw));
			image[dpos].b = ((src[spos].b*lw)+(src[spos1].b*rw));
		}
	}

	h = nh;
	delete s;
}
*/  //end second generation shears

/* Paeth's shears, kinda...
void gImage::ApplyXShear(double rangle, int threadcount)
{
	double tangent = tan(rangle/2.0);
	int dw = tangent * (double) h;
	unsigned nw = w+abs(dw);

	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	image.resize(nw*h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i = 0; i<image.size(); i++) {
		image[i].r = 0.0;
		image[i].g = 0.0;
		image[i].b = 0.0;
	}

	if (dw < 0) dw = 0;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		//double skew = tangent * (y + 0.5);
		double skew = (tangent >= 0)? tangent * (y + 0.5) : tangent * (double(y) - h + 0.5);
		double skewi = floor(skew);
		double skewf = skew - double(skewi);

		pix left, oleft;
		oleft.r = 0.0; oleft.g = 0.0; oleft.b = 0.0;
		for (unsigned x=0; x<w; x++) {
			unsigned u = (x - tangent * y) + dw;
			unsigned v = y;
			if (u >= nw) continue;
			if (v >= h) continue;
			unsigned dpos =u + v*nw;
			unsigned spos = x + y*w;
						
			left.r = src[spos].r * skewf;
			left.g = src[spos].g * skewf;
			left.b = src[spos].b * skewf;

			image[dpos].r = src[spos].r - left.r + oleft.r;
			image[dpos].g = src[spos].g - left.g + oleft.g;
			image[dpos].b = src[spos].b - left.b + oleft.b;
			oleft = left;
		}
	}


	w = nw;
	delete s;
}

void gImage::ApplyYShear(double rangle, int threadcount)
{
	double sine = sin(rangle);
	int dh = sine * (double) w;
	unsigned nh = h+abs(dh);
	unsigned dw = w;

	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	std::vector<pix>& dst = getImageData();
	dst.resize(w*nh);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i = 0; i<image.size(); i++) {
		image[i].r = 0.0;
		image[i].g = 0.0;
		image[i].b = 0.0;
	}

	if (dh < 0) dh = -dh; else dh = 0;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		//double skew = sine * (x + 0.5);
		double skew = (sine >= 0)? sine * (x + 0.5) : sine * (double(x) - w + 0.5);
		double skewi = floor(skew);
		double skewf = skew - double(skewi);
		pix left, oleft;
		oleft.r = 0.0; oleft.g = 0.0; oleft.b = 0.0;
		for (unsigned y=0; y<h; y++) {
			unsigned u = x;
			unsigned v = (y + sine * x) + dh;
			if (u >= w) continue;
			if (v >= nh) continue;
			unsigned dpos = u + v*dw;
			unsigned spos = x + y*w;

			left.r = src[spos].r * skewf;
			left.g = src[spos].g * skewf;
			left.b = src[spos].b * skewf;

			image[dpos].r = src[spos].r - left.r + oleft.r;
			image[dpos].g = src[spos].g - left.g + oleft.g;
			image[dpos].b = src[spos].b - left.b + oleft.b;
			oleft = left;
		}
	}

	h = nh;
	delete s;
}
*/  //end Paeth's shears

/* I used ImageBounds to shrink the resulting image of three shears to its minimum size.  Not much use beyond that...
void gImage::ImageBounds(unsigned *x1, unsigned *x2, unsigned *y1, unsigned *y2, bool cropbounds)
{
	*x1 = 0; *x2 = w; *y1 = 0; *y2 = h;
	std::vector<pix>& src = getImageData();
	for (int x=0; x<w; x++) {
		for (int y=0; y<h; y++) {
			unsigned pos = x + y*w;
			if (src[pos].r + src[pos].g + src[pos].b > 0.0) {
				if (cropbounds) *y1 = y; else *x1 = x;
				goto endx1;
			}
		}
	}
	endx1:
	for (int x=w-1; x>0; x--) {
		for (int y=0; y<h; y++) {
			unsigned pos = x + y*w;
			if (src[pos].r + src[pos].g + src[pos].b > 0.0) {
				if (cropbounds) *y2 = y; else *x2 = x;
				goto endx2;
			}
		}
	}
	endx2:
	for (int y=0; y<h; y++) {
		for (int x=0; x<w; x++) {
			unsigned pos = x + y*w;
			if (src[pos].r + src[pos].g + src[pos].b > 0.0) {
				if (cropbounds) *x1 = x; else *y1 = y;
				goto endy1;
			}
		}
	}
	endy1:
	for (int y=h-1; y>0; y--) {
		for (int x=0; x<w; x++) {
			unsigned pos = x + y*w;
			if (src[pos].r + src[pos].g + src[pos].b > 0.0) {
				if (cropbounds) *x2 = x; else *y2 = y;
				goto endy2;
			}
		}
	}
	endy2:
	return;
}
*/


//Crop
//
//Credit: me
//
//Cropping an image is trivial.  This method uses a left-top and right-bottom bound to 
//extricate the portion of the image of interest.
//

//This version takes crop dimensions as fractions of width and height:
void gImage::ApplyRatioCrop(float x1, float y1, float x2, float y2, int threadcount)
{
	unsigned ux1, uy1, ux2, uy2;
	if (x1 < 1.0)  ux1 = (unsigned) (x1 * w); else ux1 = (unsigned) x1;
	if (y1 < 1.0)  uy1 = (unsigned) (y1 * h); else uy1 = (unsigned) y1;
	if (x2 < 1.0)  ux2 = (unsigned) (x2 * w); else ux2 = (unsigned) x2;
	if (y2 < 1.0)  uy2 = (unsigned) (y2 * h); else uy2 = (unsigned) y2;
	ApplyCrop(ux1, uy1, ux2, uy2, threadcount);
}

void gImage::ApplyCrop(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int threadcount)
{
	if (x1>w | y1>h | x2>w | y2>h) return;
	if (x1>=x2 | y1>=y2) return;
	
	unsigned dw = x2-x1;
	unsigned dh = y2-y1;

	std::vector<pix> src = image;
	image.resize(dw * dh);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i = 0; i<image.size(); i++) {
		image[i].r = 0.0;
		image[i].g = 0.0;
		image[i].b = 0.0;
	}

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<dw; x++) {
		for (unsigned y=0; y<dh; y++) {
			unsigned dpos = x + y*dw;
			unsigned spos = x1+x + ((y+y1) * w);
			image[dpos].r = src[spos].r;
			image[dpos].g = src[spos].g;
			image[dpos].b = src[spos].b;
		}
	}
	w=dw;
	h=dh;
}



//Curve
//
//Credit: Tino Kluge, http://kluge.in-chemnitz.de/opensource/spline/, GPLV2
//
//The so-called curve is an extremely useful construct for manipulating image attributes.
//Really, the concept is that of a lookup-table, where a tone or color is looked up to 
//determine a new value, based on some previously computed translation to produce the 
//table.  The curve notion provides an intuitive (well, to some) paradigm for building
//these lookup tables. When  you want to boost the tone of shadows, for instance, you 
//want to manage the transition from that transform into the highlights, or the image
//will show the manipulation quite obviously.  A spline-based graph used to  produce
//the lookup table helps to manage the 'budget' of tones when you go to make a transform.
//
//The algorithm implemented below uses a C++ spline library published by Tino Kluge. Its
//input is a list of 'control points', through which a smooth curve is posited, and its
//output is a Y value for a specified X coordinate at any point on the curve.  A
//challenge is presented in gimage by using floating point RGB values, in that a 
//lookup table for distinct values is impractical.  Accordingly, the actual transformation 
//is not a table lookup but instead the actual spline coordinate computation.  This makes
//the curve application computationally-intensive, and correspondingly slower than its
//lookup equivalent.
//
//I added the selection of an individual channel to facilitate playing with such things 
//as manual white balance correction and grayscale tinting.
//
//Also presented with ApplyCurve is an ApplyLine method, presented in the notion that
//a number of useful 'curves' are really just straight lines, such as those used for
//contrast, brightness, and black-white point adjustment, and should require just a simple
//slope calculation rather than the spline interpolation.  At this writing, the benefit
//is not clear.
//

void gImage::ApplyToneCurve(std::vector<cp> ctpts, int threadcount)
{
	Curve c;
	c.setControlPoints(ctpts);
	c.scalepoints(1.0/SCALE_CURVE);

	#pragma omp parallel for num_threads(threadcount)
	for (int x=0; x<w; x++) {
		for (int y=0; y<h; y++) {
			int pos = x + y*w;;
			image[pos].r = c.getpoint(image[pos].r);
			image[pos].g = c.getpoint(image[pos].g);
			image[pos].b = c.getpoint(image[pos].b);
		}
	}
}

#define  Pr  .299
#define  Pg  .587
#define  Pb  .114

void gImage::ApplyToneCurve(std::vector<cp> ctpts, GIMAGE_CHANNEL channel, int threadcount)
{
	Curve c;
	c.setControlPoints(ctpts);
	c.scalepoints(1.0/SCALE_CURVE);

	if (channel == CHANNEL_RGB) {
		#pragma omp parallel for num_threads(threadcount)
		for (int x=0; x<w; x++) {
			for (int y=0; y<h; y++) {
				int pos = x + y*w;;
				image[pos].r = c.getpoint(image[pos].r);
				image[pos].g = c.getpoint(image[pos].g);
				image[pos].b = c.getpoint(image[pos].b);
			}
		}
	}
	else if (channel == CHANNEL_RED) {
		#pragma omp parallel for num_threads(threadcount)
		for (int x=0; x<w; x++) {
			for (int y=0; y<h; y++) {
				int pos = x + y*w;;
				image[pos].r = c.getpoint(image[pos].r);
			}
		}
	}
	else if (channel == CHANNEL_GREEN) {
		#pragma omp parallel for num_threads(threadcount)
		for (int x=0; x<w; x++) {
			for (int y=0; y<h; y++) {
				int pos = x + y*w;;
				image[pos].g = c.getpoint(image[pos].g);
			}
		}
	}
	else if (channel == CHANNEL_BLUE) {
		#pragma omp parallel for num_threads(threadcount)
		for (int x=0; x<w; x++) {
			for (int y=0; y<h; y++) {
				int pos = x + y*w;;
				image[pos].b = c.getpoint(image[pos].b);
			}
		}
	}

//finding the delta tone, adjusting each channel by it instead of three separate channel lookups on the curve:
	else if (channel == CHANNEL_TONE) {
// 10/16/2018 - we'll try deltatone for the time being...
		#pragma omp parallel for num_threads(threadcount)
		for (int x=0; x<w; x++) {
			for (int y=0; y<h; y++) {
				int pos = x + y*w;

				double T = (image[pos].r*0.21) + (image[pos].g*0.72) + (image[pos].b*0.07);
				double dT = c.getpoint(T)/T;
				image[pos].r *= dT;
				image[pos].g *= dT;
				image[pos].b *= dT;
/* //no appreciable difference with average...
				double T = (image[pos].r + image[pos].g + image[pos].b) / 3.0;
				double dT = c.getpoint(T)/T;
				image[pos].r *= dT;
				image[pos].g *= dT;
				image[pos].b *= dT;
*/
			}
		}

	}
}


//takes either low/high in the range 0.0-255.0, or 0.0-1.0 and scales accordingly:
void gImage::ApplyToneLine(double low, double high, GIMAGE_CHANNEL channel, int threadcount)
{
	if (low < 0.0) low = 0.0;
	if (high <= 1.0) {
		low *= 256.0;
		high *= 256.0;
	}

	double slope = 255.0 / (high-low);
	low = low / 255.0;

	if (channel == CHANNEL_RGB) {
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned x=0; x<w; x++) {
			for (unsigned y=0; y<h; y++) {
				unsigned pos = x + y*w;
				image[pos].r >= 0.0 ? image[pos].r = (image[pos].r - low) * slope : image[pos].r = 0.0;
				image[pos].g >= 0.0 ? image[pos].g = (image[pos].g - low) * slope : image[pos].g = 0.0;
				image[pos].b >= 0.0 ? image[pos].b = (image[pos].b - low) * slope : image[pos].b = 0.0;
			}
		}
	}
	else if (channel == CHANNEL_RED) {
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned x=0; x<w; x++) {
			for (unsigned y=0; y<h; y++) {
				unsigned pos = x + y*w;
				image[pos].r >= 0.0 ? image[pos].r = (image[pos].r - low) * slope : image[pos].r = 0.0;
			}
		}
	}
	else if (channel == CHANNEL_GREEN) {
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned x=0; x<w; x++) {
			for (unsigned y=0; y<h; y++) {
				unsigned pos = x + y*w;
				image[pos].g >= 0.0 ? image[pos].g = (image[pos].g - low) * slope : image[pos].g = 0.0;
			}
		}
	}
	else if (channel == CHANNEL_BLUE) {
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned x=0; x<w; x++) {
			for (unsigned y=0; y<h; y++) {
				unsigned pos = x + y*w;
				image[pos].b >= 0.0 ? image[pos].b = (image[pos].b - low) * slope : image[pos].b = 0.0;
			}
		}
	}
	else if (channel == CHANNEL_TONE) {
		//ToDo: need to figure out ApplyToneLine with a luminance...

	}
}



// Subtract and Add
//
// Credit: Kindergarten, if I recall correctly...
//
// A lot of cameras require subtraction of a particular value to establish
// black.  Also, subtracting a dark frame is useful in low-light applications
// such as astrophotography.  So, ApplySubtract, in two forms...
// 
void gImage::ApplySubtract(double subtract, bool clampblack, int threadcount)
{
	//if (subtract == 0.0) return;  //why bother... 12/2019: bother if clampblack...
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r -= subtract; if (clampblack & image[pos].r < 0.0) image[pos].r = 0.0;
			image[pos].g -= subtract; if (clampblack & image[pos].g < 0.0) image[pos].g = 0.0;
			image[pos].b -= subtract; if (clampblack & image[pos].b < 0.0) image[pos].b = 0.0;
		}
	}
}

bool gImage::ApplySubtract(std::string filename, bool clampblack, int threadcount)
{
	gImage darkfile = gImage::loadImageFile(filename.c_str(), "");
	if (darkfile.getWidth() == w & darkfile.getHeight() == h) { 
		std::vector<pix> &dark = darkfile.getImageData();
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned x=0; x<w; x++) {
			for (unsigned y=0; y<h; y++) {
				unsigned pos = x + y*w;
				image[pos].r -= dark[pos].r; if (clampblack & image[pos].r < 0.0) image[pos].r = 0.0;
				image[pos].g -= dark[pos].g; if (clampblack & image[pos].g < 0.0) image[pos].g = 0.0;
				image[pos].b -= dark[pos].b; if (clampblack & image[pos].b < 0.0) image[pos].b = 0.0;
			}
		}
		return true;
	}
	else {
		return false;
	}
}

bool gImage::ApplySubtract(gImage& subtractimage, bool clampblack, int threadcount)
{
	if (subtractimage.getWidth() == w & subtractimage.getHeight() == h) { 
		std::vector<pix> &subtract = subtractimage.getImageData();
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned x=0; x<w; x++) {
			for (unsigned y=0; y<h; y++) {
				unsigned pos = x + y*w;
				image[pos].r -= subtract[pos].r; if (clampblack & image[pos].r < 0.0) image[pos].r = 0.0;
				image[pos].g -= subtract[pos].g; if (clampblack & image[pos].g < 0.0) image[pos].g = 0.0;
				image[pos].b -= subtract[pos].b; if (clampblack & image[pos].b < 0.0) image[pos].b = 0.0;
			}
		}
		return true;
	}
	else {
		return false;
	}
}


void gImage::ApplyAdd(double add, bool clampblack, int threadcount)
{
	//if (add == 0.0) return;  //why bother... 12/2019: bother if clampblack...
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r += add; if (clampblack & image[pos].r < 0.0) image[pos].r = 0.0;
			image[pos].g += add; if (clampblack & image[pos].g < 0.0) image[pos].g = 0.0;
			image[pos].b += add; if (clampblack & image[pos].b < 0.0) image[pos].b = 0.0;
		}
	}
}

bool gImage::ApplyAdd(std::string filename, bool clampblack, int threadcount)
{
	gImage darkfile = gImage::loadImageFile(filename.c_str(), "");
	if (darkfile.getWidth() == w & darkfile.getHeight() == h) { 
		std::vector<pix> &dark = darkfile.getImageData();
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned x=0; x<w; x++) {
			for (unsigned y=0; y<h; y++) {
				unsigned pos = x + y*w;
				image[pos].r += dark[pos].r; if (clampblack & image[pos].r < 0.0) image[pos].r = 0.0;
				image[pos].g += dark[pos].g; if (clampblack & image[pos].g < 0.0) image[pos].g = 0.0;
				image[pos].b += dark[pos].b; if (clampblack & image[pos].b < 0.0) image[pos].b = 0.0;
			}
		}
		return true;
	}
	else {
		return false;
	}
}

bool gImage::ApplyAdd(gImage& addimage, bool clampblack, int threadcount)
{
	if (addimage.getWidth() == w & addimage.getHeight() == h) { 
		std::vector<pix> &add = addimage.getImageData();
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned x=0; x<w; x++) {
			for (unsigned y=0; y<h; y++) {
				unsigned pos = x + y*w;
				image[pos].r += add[pos].r; if (clampblack & image[pos].r < 0.0) image[pos].r = 0.0;
				image[pos].g += add[pos].g; if (clampblack & image[pos].g < 0.0) image[pos].g = 0.0;
				image[pos].b += add[pos].b; if (clampblack & image[pos].b < 0.0) image[pos].b = 0.0;
			}
		}
		return true;
	}
	else {
		return false;
	}
}


pix correct_pixel(pix input, gImage& clutimage, unsigned int level)
{
	pix output;
	std::vector<pix>& clut = clutimage.getImageData();
	float color, red, green, blue;
	int i, j;
	float tmp[6], r, g, b;
	level *= level;

	red = input.r * (float)(level - 1);
	if(red > level - 2)
		red = (float)level - 2;
	if(red < 0)
		red = 0;

	green = input.g * (float)(level - 1);
	if(green > level - 2)
		green = (float)level - 2;
	if(green < 0)
		green = 0;

	blue = input.b * (float)(level - 1);
	if(blue > level - 2)
		blue = (float)level - 2;
	if(blue < 0)
		blue = 0;

	r = input.r * (float)(level - 1) - red;
	g = input.g * (float)(level - 1) - green;
	b = input.b * (float)(level - 1) - blue;

	color = red + green * level + blue * level * level;

	i = color; // * 3;
	j = (color + 1); // * 3;

	tmp[0] = clut[i].r * (1 - r) + clut[j].r * r;
	tmp[1] = clut[i].g * (1 - r) + clut[j].g * r;
	tmp[2] = clut[i].b * (1 - r) + clut[j].b * r;

	i = (color + level); // * 3;
	j = (color + level + 1); // * 3;

	tmp[3] = clut[i].r * (1 - r) + clut[j].r * r;
	tmp[4] = clut[i].g * (1 - r) + clut[j].g * r;
	tmp[5] = clut[i].b * (1 - r) + clut[j].b * r;

	output.r = tmp[0] * (1 - g) + tmp[3] * g;
	output.g = tmp[1] * (1 - g) + tmp[4] * g;
	output.b = tmp[2] * (1 - g) + tmp[5] * g;

	i = (color + level * level); // * 3;
	j = (color + level * level + 1); // * 3;

	tmp[0] = clut[i].r * (1 - r) + clut[j].r * r;
	tmp[1] = clut[i].g * (1 - r) + clut[j].g * r;
	tmp[2] = clut[i].b * (1 - r) + clut[j].b * r;

	i = (color + level + level * level); // * 3;
	j = (color + level + level * level + 1); // * 3;

	tmp[3] = clut[i].r * (1 - r) + clut[j].r * r;
	tmp[4] = clut[i].g * (1 - r) + clut[j].g * r;
	tmp[5] = clut[i].b * (1 - r) + clut[j].b * r;

	tmp[0] = tmp[0] * (1 - g) + tmp[3] * g;
	tmp[1] = tmp[1] * (1 - g) + tmp[4] * g;
	tmp[2] = tmp[2] * (1 - g) + tmp[5] * g;

	output.r = output.r * (1 - b) + tmp[0] * b;
	output.g = output.g * (1 - b) + tmp[1] * b;
	output.b = output.b * (1 - b) + tmp[2] * b;

	return output;
}


// HaldCLUT
//
// Concept: http://www.quelsolaar.com/technology/clut.html
// Application: http://rawpedia.rawtherapee.com/Film_Simulation
// Math: http://im.snibgo.com/edithald.htm
//
// Work In Progress
// 
bool gImage::ApplyHaldCLUT(std::string filename, int threadcount)
{
printf("loading %s... ",filename.c_str());
	gImage hclut = gImage::loadImageFile(filename.c_str(), "");
printf("done.\n");
	if (hclut.getWidth() != hclut.getHeight()) return false;

	int x = hclut.getWidth();
	unsigned int level;
	for(level = 1; level * level * level < x; level++);
	if (level * level * level > x) return false;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos] = correct_pixel(image[pos], hclut, level);
		}
	}

	return true;
}


// ToneMapping
//
// Credit: 
//	http://filmicworlds.com/blog/filmic-tonemapping-operators/
//
//	Erik Reinhard, Mike Stark, Peter Shirley and Jim Ferwerda, 'Photographic Tone 
//	Reproduction for Digital Images', ACM Transactions on Graphics, 21(3), 
//	pp 267--276, July 2002 (Proceedings of SIGGRAPH 2002).
//
// Sometimes, the histogram is just too big.  The reference image I use in writing rawproc
// is underexposed, with the exception of a locomotive headlight.  No amount of curving was 
// bringing the headlight values down toward the rest of the data. The tonemapping operator
// implemented, Reinhart, kinda splits the difference, bringing that blip at the upper end
// of the histogram more toward the upper end of the main hump of data.  Now, the overall
// image can be properly displayed and the headlight can retain some detail.
//

//


static inline float Log2( float x)
{
  return logf(x) / logf(2);
}

void gImage::ApplyToneMapGamma(float gamma, int threadcount)
{
	double exponent = 1.0 / gamma;
	//double v = 255.0 * (double)pow((double)255, -exponent);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i=0; i< image.size(); i++) {
		image[i].r = pow(std::max((float) image[i].r,0.0f),exponent);
		image[i].g = pow(std::max((float) image[i].g,0.0f),exponent);
		image[i].b = pow(std::max((float) image[i].b,0.0f),exponent);				
	}
}

void gImage::ApplyToneMapLog2(int threadcount)
{
	float N = 8.0;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned i=0; i< image.size(); i++) {
		image[i].r <= 0.0 ? image[i].r = 0.0 : image[i].r = (log2(image[i].r)+N)/N;
		image[i].g <= 0.0 ? image[i].g = 0.0 : image[i].g = (log2(image[i].g)+N)/N;
		image[i].b <= 0.0 ? image[i].b = 0.0 : image[i].b = (log2(image[i].b)+N)/N;
	}
}

void gImage::ApplyToneMapReinhard(bool channel, bool normalize, int threadcount)
{
	// The Reinhard algorithm implemented is the basic algorithm:
	// R(x) = x/(x+1)
	// Default applies it to each channel, normalized to 0.0-1.0 ( 1/1+1 )
	// alternate computes the delta tone as a multiplier, which is then applied to the channels 

	float norm = 1.0;
	if (normalize) norm = 2.0;

	if (channel) {
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned pos=0; pos<image.size(); pos++) {	
			image[pos].r = (image[pos].r/(1.0+image[pos].r))*norm;
			image[pos].g = (image[pos].g/(1.0+image[pos].g))*norm;
			image[pos].b = (image[pos].b/(1.0+image[pos].b))*norm;
		}
	}
	else {
		#pragma omp parallel for num_threads(threadcount)
		for (unsigned pos=0; pos<image.size(); pos++) {
			double L = (image[pos].r*0.21) + (image[pos].g*0.72) + (image[pos].b*0.07);
			double Ld = (L/(1+L));  //*norm;  //not the correct place, wip...
			//double Ld = L*(1+(L/pow(1,2))) / (1+L);  //chroma-preserving, wip...
			double dT = Ld/L;
			image[pos].r *= dT;
			image[pos].g *= dT;
			image[pos].b *= dT;
		}
	}
}

void gImage::ApplyToneMapFilmic(float A, float B, float C, float D, float power, bool normalize, int threadcount)
{
	// The filmic algorithm is the original one, attributed to HP Duiker, copied from John Hable's blog:
	// R(x) = pow((x(6.2x+.5))/(x(6.2x+1.7)+0.06),2.2), where A=6.2, B=0.05, C=1.7, and D=0.06.
	// normalize=true stretches curve values to cover 0.0 - 1.0

	float norm;
	if (normalize)
		norm = (A+B) / (A+C+D);
	else
		norm = 1;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned pos=0; pos<image.size(); pos++) {
		image[pos].r > 0.0 ? image[pos].r = pow(((image[pos].r*(A*image[pos].r+B)) / (image[pos].r*(A*image[pos].r+C) + D)),power) / norm : image[pos].r = 0.0;
		image[pos].g > 0.0 ? image[pos].g = pow(((image[pos].g*(A*image[pos].g+B)) / (image[pos].g*(A*image[pos].g+C) + D)),power) / norm : image[pos].g = 0.0;
		image[pos].b > 0.0 ? image[pos].b = pow(((image[pos].b*(A*image[pos].b+B)) / (image[pos].b*(A*image[pos].b+C) + D)),power) / norm : image[pos].b = 0.0;
	}
}


void gImage::ApplyToneMapLogGamma(int threadcount)
{
	//The HEVC version of the ARIB STD-B67 algorithm, as defined in:
	//https://en.wikipedia.org/wiki/Hybrid_Log-Gamma

	double a = 0.17883277, b = 0.28466892, c = 0.55991073;
	double rubicon = 1.0/12.0;
	double sqrt3 = sqrt(3);
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned pos=0; pos<image.size(); pos++) {
		if (image[pos].r > 0.0) {
			if (image[pos].r > rubicon) 
				image[pos].r = a * log(12*image[pos].r - b) + c; 
			else 
				image[pos].r = sqrt3 * pow(image[pos].r, 0.5); 
		}
		if (image[pos].g > 0.0) {
			if (image[pos].g > rubicon) 
				image[pos].g = a * log(12*image[pos].g - b) + c; 
			else 
				image[pos].g = sqrt3 * pow(image[pos].g, 0.5); 
		}
		if (image[pos].b > 0.0) {
			if (image[pos].b > rubicon) 
				image[pos].b = a * log(12*image[pos].b - b) + c; 
			else 
				image[pos].b = sqrt3 * pow(image[pos].b, 0.5); 
		}
	}
}



//White Balance
//
//Credit: Guillermo Luijk, dcraw Tutorial,
//http://www.guillermoluijk.com/tutorial/dcraw/index_en.htm
//
//Guillermo demonstrates using single channel curves to manually apply white balance 
//multipliers, and this is automatically implemented below using a tone line
//akin to ApplyToneLine() above.  The multipliers should be percentages of the 
//largest channel value of a selected neutral pixel, e,g, for a RGB triplet of 
//165,172,204, the RGB multipliers should be 0.8088,0.8431,1.00
//

std::vector<double>  gImage::ApplyWhiteBalance(double redmult, double greenmult, double bluemult, int threadcount)
{
	std::vector<double> a;
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r *= redmult; 
			image[pos].g *= greenmult; 
			image[pos].b *= bluemult; 
		}
	}
	a.push_back(redmult);
	a.push_back(greenmult);
	a.push_back(bluemult);	
	return a;
}

/*
std::vector<double>  gImage::ApplyCameraWhiteBalance(double redmult, double greenmult, double bluemult, int threadcount)
{
	std::vector<double> a = {0.0, 0.0, 0.0};
	if (imginfo.find("LibrawCFAPattern") == imginfo.end()) return a;
	
	std::vector<unsigned> q = {0, 1, 1, 2};  //default pattern is RGGB, where R=0, G=1, B=2
	if (imginfo["LibrawCFAPattern"] == "GRBG") q = {1, 0, 2, 1};
	if (imginfo["LibrawCFAPattern"] == "GBRG") q = {1, 2, 0, 1};
	if (imginfo["LibrawCFAPattern"] == "BGGR") q = {2, 1, 1, 0};

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h-1; y+=2) {
		for (unsigned x=0; x<w-1; x+=2) {
			unsigned pos[4];
			pos[0] = x + y*w;  //upper left
			pos[1] = (x+1) + y*w; //upper right
			pos[2] = x + (y+1)*w; //lower leftfs
			pos[3] = (x+1) + (y+1)*w;  //lower right
			for (unsigned i=0; i<q.size(); i++) {
				if (q[i] == 0) image[pos[i]].r *= redmult;  //use r, in grayscale, they're all the same...
				if (q[i] == 1) image[pos[i]].r *= greenmult;  //use r, in grayscale, they're all the same...
				if (q[i] == 2) image[pos[i]].r *= bluemult;  //use r, in grayscale, they're all the same...
				image[pos[i]].g = image[pos[i]].b = image[pos[i]].r;
			}
		}
	}
	
	a[0] = redmult;
	a[1] = greenmult;
	a[2] = bluemult;	
	return a;
}
*/

std::vector<double>  gImage::ApplyCameraWhiteBalance(double redmult, double greenmult, double bluemult, int threadcount)
{
	std::vector<double> a = {0.0, 0.0, 0.0};
	if (imginfo.find("LibrawCFAArray") == imginfo.end()) return a;

	std::string cfa = imginfo["LibrawCFAArray"];
	std::vector<unsigned> q;
	for (unsigned i = 0; i < cfa.size(); i ++) q.push_back(cfa[i] - '0');
	
	unsigned cfadim = 2;
	if (q.size() == 36) cfadim = 6;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h-1; y+=2) {
		for (unsigned x=0; x<w-1; x+=2) {
			for (unsigned j = 0; j < cfadim; j++) {
				for (unsigned k = 0; k < cfadim; k++) {
					unsigned pos = (x + j) + (y + k) * w;
					if (q[j+k*cfadim] == 0) image[pos].r *= redmult;  //use r, in grayscale, they're all the same...
					if (q[j+k*cfadim] == 1) image[pos].r *= greenmult;  //use r, in grayscale, they're all the same...
					if (q[j+k*cfadim] == 2) image[pos].r *= bluemult;  //use r, in grayscale, they're all the same...
					if (q[j+k*cfadim] == 3) image[pos].r *= greenmult;  //use r, in grayscale, they're all the same...
					image[pos].g = image[pos].b = image[pos].r;
				}
			}
		}
	}
	
	a[0] = redmult;
	a[1] = greenmult;
	a[2] = bluemult;

	return a;
}


//uses a patch from the image presumed to represent neutral or white:
std::vector<double>  gImage::ApplyPatchWhiteBalance(float patchx, float patchy, double patchradius, int threadcount)
{
	//handle percentage coordinates:
	if (patchx <= 1.0 & patchy <= 1.0) {
		patchx = (float) w * patchx;
		patchy = (float) h * patchy;
	}

	double redmult = 1.0, greenmult = 1.0, bluemult = 1.0;
	std::vector<double> a = CalculatePatchMeans( (int) patchx, (int) patchy, patchradius);
	redmult = a[1] / a[0]; // gm/rm
	bluemult = a[1] / a[2]; // gm/bm

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r *= redmult; 
			image[pos].g *= greenmult; 
			image[pos].b *= bluemult; 
		}
	}
	a[0] = redmult;
	a[1] = greenmult;
	a[2] = bluemult;	
	return a;
}

//automatically adjusts WB based on 'gray world' image average:
std::vector<double> gImage::ApplyWhiteBalance(int threadcount)
{	
	double redmult = 1.0, greenmult = 1.0, bluemult = 1.0;

	std::vector<double> a = CalculateChannelMeans();
	redmult = a[1] / a[0]; // gm/rm
	bluemult = a[1] / a[2]; // gm/bm

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r *= redmult; 
			image[pos].g *= greenmult; 
			image[pos].b *= bluemult; 
		}
	}
	
	a[0] = redmult;
	a[1] = greenmult;
	a[2] = bluemult;
	return a;
}

bool f(double d)
{
	return true;
}

float ** RT_malloc(unsigned w, unsigned h)
{
	float **rawdata = (float **)malloc(h * sizeof(float *));
	rawdata[0] = (float *)malloc(w*h * sizeof(float));
	for (unsigned i=1; i<h; i++) 
		rawdata[i] = rawdata[i - 1] + w; 
	return rawdata;
}

void RT_free(float ** rawdata)
{
	free (rawdata[0]);
	free( rawdata );	
}

//Demosaic
//
//Turns an out-of-camera image mosaic into a the RGB image array we've come to know and love.  The 
//main algorithms used come courtesy of RawTherapee, and the librtprocess library.
//
//half is a "toy" demosaic, for instructional purposes. "HALF" simply takes the RGGB (or other pattern)
//quad and turns it into a single RGB pixel. Since each four-pixel quad is turned into a single pixel, 
//the results of this algorithm are an image that is half the size of the original, hence the catchy
//name.  half_resize will produce the half image and then re-scale it to the original dimensions.
//
//"color" doesn't demosaic, it zeros out the other 
//channels in the quad pixels so the unmosaiced image can be regarded as what was recorded
//through the color filter array, in color.  Still, it's hard to see even at 200%
//

// Demosaic helpers:



bool gImage::xtranArray(unsigned (&xtarray)[6][6])
{
	if (imginfo.find("LibrawCFAArray") != imginfo.end()) {
		std::string cfstr = imginfo["LibrawCFAArray"];
		if (cfstr.size() == 36) { //x-trans 6x6 array
			for (int r=0; r<6; r++) {
				for (int c=0; c<6; c++) {
					int pos = c + r*6;
					xtarray[r][c] = (unsigned) (cfstr[pos] - '0');
				}
			}
			return true;
		}
		else return false;
	} 
	else return false;
}

bool gImage::cfArray(unsigned (&cfarray)[2][2])
{
	if (imginfo.find("LibrawCFAArray") != imginfo.end()) {
		std::string cfstr = imginfo["LibrawCFAArray"];
		if (cfstr.size() == 4) { //bayer 2x2 array
			for (int r=0; r<2; r++) {
				for (int c=0; c<2; c++) {
					int pos = c + r*2;
					cfarray[r][c] = (unsigned) (cfstr[pos] - '0');
				}
			}
			return true;
		}
		else return false;
	}
	else return false;
}

bool gImage::rgbCam(float (&rgb_cam)[3][4])
{
	if (imginfo.find("LibrawRGBCam") != imginfo.end()) {
		std::string rgb_cam_str = imginfo["LibrawRGBCam"];
		std::vector<std::string> rgb_cam_list  = split(rgb_cam_str, ",");
		rgb_cam[0][0] = atof(rgb_cam_list[0].c_str());
		rgb_cam[0][1] = atof(rgb_cam_list[1].c_str());
		rgb_cam[0][2] = atof(rgb_cam_list[2].c_str());
		rgb_cam[1][0] = atof(rgb_cam_list[3].c_str());
		rgb_cam[1][1] = atof(rgb_cam_list[4].c_str());
		rgb_cam[1][2] = atof(rgb_cam_list[5].c_str());
		rgb_cam[2][0] = atof(rgb_cam_list[6].c_str());
		rgb_cam[2][1] = atof(rgb_cam_list[7].c_str());
		rgb_cam[2][2] = atof(rgb_cam_list[8].c_str());
		return true;
	}
	else return false;
}

bool gImage::ApplyDemosaicHalf(bool resize, int threadcount)
{
	unsigned cfarray[2][2];
	if (!cfArray(cfarray)) return false;  //only set up for Bayer raws
	int arraydim = 2;

	std::vector<pix> halfimage;
	halfimage.resize((h/2)*(w/2));

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h-(arraydim-1); y+=arraydim) {
		for (unsigned x=0; x<w-(arraydim-1); x+=arraydim) {
			unsigned Hpos = (x/2) + (y/2)*(w/2);
			float pix[4] = {0.0, 0.0, 0.0, 0.0};
			for (unsigned i=0; i<arraydim; i++) {  //walk the 2x2 image subset, collect the channel values 
				for (unsigned j=0; j<arraydim; j++) {
					int pos = (x+i) + (y+j) * w;
					pix[cfarray[i][j]] += image[pos].r;
				}
			}
			pix[1] = (pix[1] + pix[3]) / 2.0; //make a single green of G1 and G2

			//put the result in the appropriate place in the halfsize image:
			halfimage[Hpos].r = pix[0];
			halfimage[Hpos].g = pix[1];
			halfimage[Hpos].b = pix[2];
		}
	}

	image = halfimage;
	w /=2;
	h /=2;
	c = 3;
	if (resize) 
		ApplyResize(w*2, h*2, FILTER_LANCZOS3, threadcount);
	return true;
}

bool gImage::ApplyMosaicColor(int threadcount)
{
	unsigned cfarray[2][2];
	if (!cfArray(cfarray)) return false;
	int arraydim = 2;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h-(arraydim-1); y+=arraydim) {
		for (unsigned x=0; x<w-(arraydim-1); x+=arraydim) {
			unsigned Hpos = (x/2) + (y/2)*(w/2);
			float pix[4] = {0.0, 0.0, 0.0, 0.0};
			for (unsigned i=0; i<arraydim; i++) {
				for (unsigned j=0; j<arraydim; j++) {
					int pos = (x+i) + (y+j) * w;
					switch (cfarray[i][j]) {
						case 0:  //red
							image[pos].g = 0.0;
							image[pos].b = 0.0;
							break;
						case 1:  //green
						case 3:
							image[pos].r = 0.0;
							image[pos].b = 0.0;
							break;
						case 2:  //blue
							image[pos].r = 0.0;
							image[pos].g = 0.0;
							break;
					}
				}
			}
		}
	}


	c = 3;
	return true;
}

#ifdef USE_LIBRTPROCESS


bool gImage::ApplyDemosaicVNG(LIBRTPROCESS_PREPOST prepost, int threadcount)
{
	unsigned cfarray[2][2];
	if (!cfArray(cfarray)) return false;

	//allocation "by-hand":
	float **rawdata = (float **)malloc(h * sizeof(float *));
	rawdata[0] = (float *)malloc(w*h * sizeof(float));
	for (unsigned i=1; i<h; i++) 
		rawdata[i] = rawdata[i - 1] + w; 

	float **red     = (float **)malloc(h * sizeof(float *)); 
	red[0] = (float *)malloc(w*h * sizeof(float));
	for (unsigned i=1; i<h; i++) 
		red[i]     = red[i - 1] + w;

	float **green     = (float **)malloc(h * sizeof(float *)); 
	green[0] = (float *)malloc(w*h * sizeof(float));
	for (unsigned i=1; i<h; i++) 
		green[i]     = green[i - 1] + w;

	float **blue     = (float **)malloc(h * sizeof(float *)); 
	blue[0] = (float *)malloc(w*h * sizeof(float));
	for (unsigned i=1; i<h; i++) 
		blue[i]     = blue[i - 1] + w;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			rawdata[y][x] = image[pos].r * 65535.f;
		}
	}
	
	vng4_demosaic (w, h, rawdata, red, green, blue, cfarray, f);

	#pragma omp parallel for num_threads(threadcount)	
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = red[y][x] /65535.f;
			image[pos].g = green[y][x] /65535.f;
			image[pos].b = blue[y][x] /65535.f;
		}
	}
		
	free (blue[0]);
	free( blue );
	free (green[0]);
	free( green );
	free (red[0]);
	free( red );
	free (rawdata[0]);
	free( rawdata );
	return true;
}

bool gImage::ApplyDemosaicRCD(LIBRTPROCESS_PREPOST prepost, int threadcount)
{
	unsigned cfarray[2][2];
	if (!cfArray(cfarray)) return false;

	//convert to 3-color:
	for (int y=0; y<2; y++) 
		for (int x=0; x<2; x++) 
			if (cfarray[y][x] == 3) cfarray[y][x] = 1;

	//allocation using JaggedArray, doesn't require free:
	//librtprocess::JaggedArray<float> rawdata(w, h);
	//librtprocess::JaggedArray<float> red(w, h);
	//librtprocess::JaggedArray<float> green(w, h);
	//librtprocess::JaggedArray<float> blue(w, h);

	float ** rawdata = RT_malloc(w, h);
	float ** red = RT_malloc(w, h);
	float ** green = RT_malloc(w, h);
	float ** blue = RT_malloc(w, h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			rawdata[y][x] = image[pos].r * 65535.f;
		}
	}
	
	rcd_demosaic (w, h, rawdata, red, green, blue, cfarray, f);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = red[y][x]   / 65535.f;
			image[pos].g = green[y][x] / 65535.f;
			image[pos].b = blue[y][x]  / 65535.f;
		}
	}

	RT_free(blue);
	RT_free(green);
	RT_free(red);
	RT_free(rawdata);
	return true;
}

bool gImage::ApplyDemosaicDCB(LIBRTPROCESS_PREPOST prepost, int iterations, bool dcb_enhance, int threadcount)
{
	unsigned cfarray[2][2];
	if (!cfArray(cfarray)) return false;

	//convert to 3-color:
	for (int y=0; y<2; y++)
		for (int x=0; x<2; x++) 
			if (cfarray[y][x] == 3) cfarray[y][x] = 1;

	float ** rawdata = RT_malloc(w, h);
	float ** red = RT_malloc(w, h);
	float ** green = RT_malloc(w, h);
	float ** blue = RT_malloc(w, h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			rawdata[y][x] = image[pos].r * 65535.f;
		}
	}
	
	dcb_demosaic (w, h, rawdata, red, green, blue, cfarray, f, iterations, dcb_enhance);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = red[y][x]   / 65535.f;
			image[pos].g = green[y][x] / 65535.f;
			image[pos].b = blue[y][x]  / 65535.f;
		}
	}

	RT_free(blue);
	RT_free(green);
	RT_free(red);
	RT_free(rawdata);
	return true;
}


bool gImage::ApplyDemosaicAMAZE(LIBRTPROCESS_PREPOST prepost, double initGain, int border, float inputScale, float outputScale, int threadcount)
{
	unsigned cfarray[2][2];
	if (!cfArray(cfarray)) return false;

	//convert to 3-color:
	for (int y=0; y<2; y++) 
		for (int x=0; x<2; x++) 
			if (cfarray[y][x] == 3) cfarray[y][x] = 1;

	float ** rawdata = RT_malloc(w, h);
	float ** red = RT_malloc(w, h);
	float ** green = RT_malloc(w, h);
	float ** blue = RT_malloc(w, h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			rawdata[y][x] = image[pos].r * 65535.f;
		}
	}

	//this short-circuits the initGain parameter...
	std::vector<std::string> camwb = split(getInfoValue("LibrawWhiteBalance"), ",");
	double camwbR = atof(camwb[0].c_str());
	double camwbG = atof(camwb[1].c_str());
	double camwbB = atof(camwb[2].c_str());
	double initialGain = fmax(camwbR,fmax(camwbG,camwbB)) / fmin(camwbR,fmin(camwbG,camwbB));
	
	amaze_demosaic (w, h,0,0,w,h, rawdata, red, green, blue, cfarray, f, initialGain, border, inputScale, outputScale);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = red[y][x]   / 65535.f;
			image[pos].g = green[y][x] / 65535.f;
			image[pos].b = blue[y][x]  / 65535.f;
		}
	}

	RT_free(blue);
	RT_free(green);
	RT_free(red);
	RT_free(rawdata);
	return true;
}

bool gImage::ApplyDemosaicIGV(LIBRTPROCESS_PREPOST prepost, int threadcount)
{
	unsigned cfarray[2][2];
	if (!cfArray(cfarray)) return false;

	//convert to 3-color:
	for (int y=0; y<2; y++) 
		for (int x=0; x<2; x++) 
			if (cfarray[y][x] == 3) cfarray[y][x] = 1;

	float ** rawdata = RT_malloc(w, h);
	float ** red = RT_malloc(w, h);
	float ** green = RT_malloc(w, h);
	float ** blue = RT_malloc(w, h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			rawdata[y][x] = image[pos].r * 65535.f;
		}
	}
	
	igv_demosaic (w, h, rawdata, red, green, blue, cfarray, f);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = red[y][x]   / 65535.f;
			image[pos].g = green[y][x] / 65535.f;
			image[pos].b = blue[y][x]  / 65535.f;
		}
	}

	RT_free(blue);
	RT_free(green);
	RT_free(red);
	RT_free(rawdata);
	return true;
}

bool gImage::ApplyDemosaicAHD(LIBRTPROCESS_PREPOST prepost, int threadcount)
{
	double fitParams[2][2][16];

	unsigned cfarray[2][2];
	if (!cfArray(cfarray)) return false;

	float rgb_cam[3][4];
	if (!rgbCam(rgb_cam)) return false;

	//convert to 3-color:
	for (int y=0; y<2; y++) 
		for (int x=0; x<2; x++) 
			if (cfarray[y][x] == 3) cfarray[y][x] = 1;

	float ** rawdata = RT_malloc(w, h);
	float ** red = RT_malloc(w, h);
	float ** green = RT_malloc(w, h);
	float ** blue = RT_malloc(w, h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			rawdata[y][x] = image[pos].r;
		}
	}

	//prepost = LIBRTPROCESS_PREPOST(prepost | LIBRTPROCESS_CACORRECT);
	//prepost = LIBRTPROCESS_PREPOST(prepost | LIBRTPROCESS_HLRECOVERY);

	if ((prepost & LIBRTPROCESS_CACORRECT) == LIBRTPROCESS_CACORRECT) 
		CA_correct(0,0,w,h, true, 1, 0.0, 0.0, true, rawdata, rawdata, cfarray, f, fitParams, false, 1.0, 1.0);

	ahd_demosaic (w, h, rawdata, red, green, blue, cfarray, rgb_cam, f);

	if ((prepost & LIBRTPROCESS_HLRECOVERY) == LIBRTPROCESS_HLRECOVERY) {

		float chmax[3] = {0.0, 0.0, 0.0};

		#pragma omp parallel num_threads(threadcount)
		{
			float ra = 0; float ga = 0; float ba = 0;
	
			#pragma omp for 
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					red[y][x]   = image[pos].r;
					green[y][x] = image[pos].g;
					blue[y][x]  = image[pos].b;

					if (ra < image[pos].r) ra = image[pos].r;
					if (ga < image[pos].g) ga = image[pos].g;
					if (ba < image[pos].b) ba = image[pos].b;
				}
			}

			#pragma omp critical
			{
				if (ra > chmax[0]) chmax[0] = ra;
				if (ga > chmax[1]) chmax[1] = ga;
				if (ba > chmax[2]) chmax[2] = ba;
			}
		}	

		float clmax[3];
		std::vector<std::string> camwb = split(getInfoValue("LibrawWhiteBalance"), ",");
		float clippoint = 1.0;
		clmax[0] = clippoint * atof(camwb[0].c_str());
		clmax[1] = clippoint * atof(camwb[1].c_str());
		clmax[2] = clippoint * atof(camwb[2].c_str());
		//printf("chmax[0]:%f chmax[1]:%f chmax[2]:%f\nclmax[0]:%f clmax[1]:%f clmax[2]:%f\n",chmax[0],chmax[1],chmax[2],clmax[0],clmax[1],clmax[2]); fflush(stdout);
		HLRecovery_inpaint(w,h, red, green, blue, chmax, clmax, f);
	}

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = red[y][x]; 
			image[pos].g = green[y][x]; 
			image[pos].b = blue[y][x]; 
		}
	}

	RT_free(blue);
	RT_free(green);
	RT_free(red);
	RT_free(rawdata);
	return true;
}


bool gImage::ApplyDemosaicLMMSE(LIBRTPROCESS_PREPOST prepost, int iterations, int threadcount)
{
	unsigned cfarray[2][2];
	if (!cfArray(cfarray)) return false;

	//convert to 3-color:
	for (int y=0; y<2; y++) 
		for (int x=0; x<2; x++) 
			if (cfarray[y][x] == 3) cfarray[y][x] = 1;

	float ** rawdata = RT_malloc(w, h);
	float ** red = RT_malloc(w, h);
	float ** green = RT_malloc(w, h);
	float ** blue = RT_malloc(w, h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			rawdata[y][x] = image[pos].r * 65535.f;
		}
	}

	lmmse_demosaic(w, h, rawdata, red, green, blue, cfarray, f, iterations);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = red[y][x]   / 65535.f;
			image[pos].g = green[y][x] / 65535.f;
			image[pos].b = blue[y][x]  / 65535.f;
		}
	}

	RT_free(blue);
	RT_free(green);
	RT_free(red);
	RT_free(rawdata);
	return true;
}

bool gImage::ApplyDemosaicXTRANSFAST(LIBRTPROCESS_PREPOST prepost, int threadcount)
{
	unsigned xtarray[6][6];
	if (!xtranArray(xtarray)) return false;

	float ** rawdata = RT_malloc(w, h);
	float ** red = RT_malloc(w, h);
	float ** green = RT_malloc(w, h);
	float ** blue = RT_malloc(w, h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			rawdata[y][x] = image[pos].r * 65535.f;
		}
	}

	xtransfast_demosaic(w, h, rawdata, red, green, blue, xtarray, f);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = red[y][x]   / 65535.f;
			image[pos].g = green[y][x] / 65535.f;
			image[pos].b = blue[y][x]  / 65535.f;
		}
	}

	RT_free(blue);
	RT_free(green);
	RT_free(red);
	RT_free(rawdata);
	return true;
}

bool gImage::ApplyDemosaicXTRANSMARKESTEIJN(LIBRTPROCESS_PREPOST prepost, int passes, bool useCieLab, int threadcount)
{
	unsigned xtarray[6][6];
	if (!xtranArray(xtarray)) return false;

	float rgb_cam[3][4];
	if (!rgbCam(rgb_cam)) return false;

	float ** rawdata = RT_malloc(w, h);
	float ** red = RT_malloc(w, h);
	float ** green = RT_malloc(w, h);
	float ** blue = RT_malloc(w, h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			rawdata[y][x] = image[pos].r * 65535.f;
		}
	}

	markesteijn_demosaic(w, h, rawdata, red, green, blue, xtarray, rgb_cam, f, passes, useCieLab);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = red[y][x]   / 65535.f;
			image[pos].g = green[y][x] / 65535.f;
			image[pos].b = blue[y][x]  / 65535.f;
		}
	}

	RT_free(blue);
	RT_free(green);
	RT_free(red);
	RT_free(rawdata);
	return true;
}

bool gImage::ApplyCACorrect( int threadcount)
{
	if (imginfo["Libraw::Mosaiced"] == "0") return false;  //Mosaic data only

	unsigned cfarray[2][2];
	if (!cfArray(cfarray)) return false;   //Bayer data only... ?
	for (int y=0; y<2; y++) //convert to 3-color
		for (int x=0; x<2; x++) 
			if (cfarray[y][x] == 3) cfarray[y][x] = 1;

	double fitParams[2][2][16];
	float ** rawdata = RT_malloc(w, h);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			rawdata[y][x] = image[pos].r;
		}
	}

	CA_correct(0,0,w,h, true, 1, 0.0, 0.0, true, rawdata, rawdata, cfarray, f, fitParams, false, 1.0, 1.0);
	
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = rawdata[y][x];
		}
	}

	RT_free(rawdata);
	return true;
}

bool gImage::ApplyHLRecover(int threadcount)
{
	if (imginfo["Libraw::Mosaiced"] == "1") return false;  //RGB data only

	float ** red = RT_malloc(w, h);
	float ** green = RT_malloc(w, h);
	float ** blue = RT_malloc(w, h);

	float chmax[3] = {0.0, 0.0, 0.0};

	#pragma omp parallel num_threads(threadcount)
	{
		float ra = 0; float ga = 0; float ba = 0;
	
		#pragma omp for 
		for (unsigned y=0; y<h; y++) {
			for (unsigned x=0; x<w; x++) {
				unsigned pos = x + y*w;
				red[y][x]   = image[pos].r;
				green[y][x] = image[pos].g;
				blue[y][x]  = image[pos].b;

				if (ra < image[pos].r) ra = image[pos].r;
				if (ga < image[pos].g) ga = image[pos].g;
				if (ba < image[pos].b) ba = image[pos].b;
			}
		}

		#pragma omp critical
		{
			if (ra > chmax[0]) chmax[0] = ra;
			if (ga > chmax[1]) chmax[1] = ga;
			if (ba > chmax[2]) chmax[2] = ba;
		}
	}

	float clmax[3];
	std::vector<std::string> camwb = split(getInfoValue("LibrawWhiteBalance"), ",");
	float clippoint = 1.0;
	clmax[0] = clippoint * atof(camwb[0].c_str());
	clmax[1] = clippoint * atof(camwb[1].c_str());
	clmax[2] = clippoint * atof(camwb[2].c_str());

	printf("chmax[0]:%f chmax[1]:%f chmax[2]:%f\nclmax[0]:%f clmax[1]:%f clmax[2]:%f\n",chmax[0],chmax[1],chmax[2],clmax[0],clmax[1],clmax[2]); fflush(stdout);

	HLRecovery_inpaint(w,h, red, green, blue, chmax, clmax, f);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned y=0; y<h; y++) {
		for (unsigned x=0; x<w; x++) {
			unsigned pos = x + y*w;
			image[pos].r = red[y][x];
			image[pos].g = green[y][x];
			image[pos].b = blue[y][x];
		}
	}

	RT_free(blue);
	RT_free(green);
	RT_free(red);
	return true;
}

#endif //USE_LIBRTPROCESS




//Saturation
//
//Credit: Darel Rex Finley, http://alienryderflex.com/saturation.html, public domain
//
//This is a simple HSL saturation routine.  There are better ways to 
//manipulate color, but this one is self-contained to a RGB color space.
//Maybe later...
//

//commented out here, moved to above the ApplyCurve methods to accommodate a 'bright' luminance channel curve:
//#define  Pr  .299
//#define  Pg  .587
//#define  Pb  .114

void gImage::ApplySaturate(double saturate, int threadcount)
{
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			double R = image[pos].r;
			double G = image[pos].g;
			double B = image[pos].b;

			double  P=sqrt(
			R*R*Pr+
			G*G*Pg+
			B*B*Pb ) ;

			image[pos].r=P+(R-P)*saturate;
			image[pos].g=P+(G-P)*saturate;
			image[pos].b=P+(B-P)*saturate;

		}
	}
}


//Exposure Compensation
//
//Multiplies each R, G, and B value of each pixel by 2**ev
//

void gImage::ApplyExposureCompensation(double ev, int threadcount)
{
	double mult = pow(2.0,ev);
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r *= mult;
			image[pos].g *= mult;
			image[pos].b *= mult;
		}
	}
}

//Multiplies each R, G, and B value of each pixel by a multiplier that will shift the patch to the destination ev
//

float gImage::ApplyExposureCompensation(int x, int y, float radius, float destinationev, int threadcount)
{
	std::vector<double> patchrgb = CalculatePatchMeans(x, y, radius);
	float tone = (patchrgb[0] + patchrgb[1] + patchrgb[2]) / 3.0;
	double mult = destinationev/tone;
	
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r *= mult;
			image[pos].g *= mult;
			image[pos].b *= mult;
		}
	}
	return sqrt(mult);
}



//Tint
//
//Credit:
//
//Tint allows the addition or subtraction of a single value from a single
//channel.  My use case is to introduce tint to grayscale images.
//A better approach would be to modify ApplyCurve to do individual channels.  A
//subject of a subsequent version...
//

void gImage::ApplyTint(double red,double green,double blue, int threadcount)
{
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r += red;
			image[pos].g += green;
			image[pos].b += blue;

		}
	}
}


//Grayscaling
//
//Credit: Various
//
//Grayscaling an image is essentially computing a single tone value from the red, green, and blue
//components of each pixel.  This is typically done by giving each component a percentage weight
//in a sum; the typical percentages used are red=0.21, green=0.72, and blue=0.07; see 
//https://en.wikipedia.org/wiki/Grayscale for a discussion.
//
//The algorithm implemented allows for individual specification of the percentages.  It does not
//produce a single-valued image matrix; it loads the same gray tone in all three channels.  My
//thought would be to make an option to convert to, say, an 8-bit grayscale image when saving.
//Retaining the three-channel image allows subsequent manipulation of the channels to introduce
//tint.

void gImage::ApplyGray(double redpct, double greenpct, double bluepct, int threadcount)
{
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			//double G = floor(image[pos].r*redpct + image[pos].g*greenpct + image[pos].b*bluepct)+0.5; 
			double G = image[pos].r*redpct + image[pos].g*greenpct + image[pos].b*bluepct;
			image[pos].r=G;
			image[pos].g=G;
			image[pos].b=G;

		}
	}
}



//NLMeans Denoise
//
//Credit: Antoni Buades, et.al., and David Tschumperlé, principal programmer of GMIC, non-commercial use
//
//Denoising an image removes the "speckle" produced usually by low exposure values, where
//the light signal is not too far from the noise signal inherent in the sensor.  The
//essential operation is to 'average' surrounding pixels as the result value for the 
//source pixel.  In that regard, a 'blur' operation reduces noise, but at the expense of
//image detail because the average doesn't recognize edges.
//
//The Non-Local Means algorithm, first described by Antoni Buades, Bartomeu Coll and 
//Jean-Michel Morel in the IPOL Journal attempts to address this expense by recognizing similar
//toned pixels in the group, and giving them more weight in the average.  The implementation
//below is a simplistic but straightforward interpretation of the algorithm presented as a 
//GMIC script by David Tschumperlé in his blog at http://gmic.eu
//

void gImage::ApplyNLMeans(double sigma, int local, int patch, int threadcount)
{
	std::vector<pix> *s = new std::vector<pix>(image);
	std::vector<pix> &src = *s;
	std::vector<pix>& dst = getImageData();

	unsigned spitch = w;
	unsigned dpitch = w;

	unsigned iw = w;
	unsigned ih = h;
	double sigma2 = pow(2*(sigma/SCALE_CURVE),2);  //UI sigmas are relevant to 0-255

	//y|x upper|lower bound computations, used to offset patch to avoid out-of-image references 
	unsigned yplb = patch+local+1;
	unsigned ypub = ih-yplb;
	unsigned xplb = yplb;
	unsigned xpub = iw-xplb;

	#pragma omp parallel for num_threads(threadcount)
	for(unsigned y = local; y < ih-local; y++) {
		unsigned py = y;
		if (py<yplb) py = yplb;
		if (py>ypub) py = ypub;
		for(unsigned x = local; x < iw-local; x++) {
			unsigned px = x;
			if (px<xplb) px = xplb;
			if (px>xpub) px = xpub;
			unsigned wdstpix = dpitch*y + x;
			double valueR = 0.0;
			double valueG = 0.0;
			double valueB = 0.0;
			double sum_weightsR = 0.0;
			double sum_weightsG = 0.0;
			double sum_weightsB = 0.0;
			for (int q = -local; q<=local; ++q) {
				for (int p = -local; p<=local; ++p) {
					double diffR = 0.0;
					double diffG = 0.0;
					double diffB = 0.0;
					for (int s = -patch; s<=patch; ++s) {
						for (int r = -patch; r<=patch; ++r) {
							unsigned ppix = spitch*(py+q+s) + (px+p+r);
							unsigned lpix = spitch*(py+s) + (px+r);
							diffR += pow((src[ppix].r - src[lpix].r),2);
							diffG += pow((src[ppix].g - src[lpix].g),2);
							diffB += pow((src[ppix].b - src[lpix].b),2);
							//gmic: diff += pow(i[x+p+r,y+q+s] - i[x+r,y+s],2);
						}
					}
					double weightR = exp(-diffR/sigma2);
					double weightG = exp(-diffG/sigma2);
					double weightB = exp(-diffB/sigma2);
					unsigned localpix = spitch*(y+q) + (x+p);
					valueR += weightR*src[localpix].r;
					valueG += weightG*src[localpix].g;
					valueB += weightB*src[localpix].b;
					//gmic: value += weight*i(x+p,y+q);
					sum_weightsR += weightR;
					sum_weightsG += weightG;
					sum_weightsB += weightB;
				}
			}
			dst[wdstpix].r   = (valueR/(1e-5 + sum_weightsR));
			dst[wdstpix].g = (valueG/(1e-5 + sum_weightsG));
			dst[wdstpix].b  = (valueB/(1e-5 + sum_weightsB));
		}
	}
	delete s;
}


//Wavelet Denoise
//
//Credit: dcraw wavelet_denoise(): https://www.cybercom.net/~dcoffin/dcraw/dcraw.c
//
//This is a straight translation of the dcraw routine from the unsigned short mosaic
//to the rawproc PIXTYPE floating point RGB.

void gImage::ApplyWaveletDenoise(double threshold, int threadcount)
{
	unsigned width = w;
	unsigned height = h;
	unsigned colors = c;
	PIXTYPE (*img)[3];
	float thold;
	int size; 
	static const float noise[] = { 0.8002,0.2735,0.1202,0.0585,0.0291,0.0152,0.0080,0.0044 };

	img = (PIXTYPE (*)[3]) image.data();
	size = height*width;;

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned c=0; c < colors; c++) {			// denoise R,G,B individually
		int lev, hpass, lpass;
		//float *fimg = (float *) malloc ((size*3 + height + width) * sizeof *fimg);
		float *fimg = 0;
		fimg = new float[(size*3 + height + width) ];
		float *temp = fimg + size*3;

		for (unsigned i=0; i < size; i++)
			//fimg[i] = sqrt(img[i][c]);
			fimg[i] = img[i][c];

		for (hpass=lev=0; lev < 5; lev++) {
			lpass = size*((lev & 1)+1);
			for (unsigned row=0; row < height; row++) {
				//hat_transform (temp, fimg+hpass+row*width, 1, width, 1 << lev);
				
				//inline:
				{
					float *base = fimg+hpass+row*width; 
					int st=1;
					int size=width; 
					int sc=1 << lev;
					int i;
					for (i=0; i < sc; i++)
						temp[i] = 2*base[st*i] + base[st*(sc-i)] + base[st*(i+sc)];
					for (; i+sc < size; i++)
						temp[i] = 2*base[st*i] + base[st*(i-sc)] + base[st*(i+sc)];
					for (; i < size; i++)
						temp[i] = 2*base[st*i] + base[st*(i-sc)] + base[st*(2*size-2-(i+sc))];
				}
				
				for (unsigned col=0; col < width; col++)
					fimg[lpass + row*width + col] = temp[col] * 0.25;
			}
			for (unsigned col=0; col < width; col++) {
				//hat_transform (temp, fimg+lpass+col, width, height, 1 << lev);
				
				//inline:
				{
					float *base = fimg+lpass+col; 
					int st=width;
					int size=height; 
					int sc=1 << lev;
					int i;
					for (i=0; i < sc; i++)
						temp[i] = 2*base[st*i] + base[st*(sc-i)] + base[st*(i+sc)];
					for (; i+sc < size; i++)
						temp[i] = 2*base[st*i] + base[st*(i-sc)] + base[st*(i+sc)];
					for (; i < size; i++)
						temp[i] = 2*base[st*i] + base[st*(i-sc)] + base[st*(2*size-2-(i+sc))];
				}
				
				for (unsigned row=0; row < height; row++)
					fimg[lpass + row*width + col] = temp[row] * 0.25;
			}
			
			thold = threshold * noise[lev];
			for (unsigned i=0; i < size; i++) {
				fimg[hpass+i] -= fimg[lpass+i];
				if	(fimg[hpass+i] < -thold) fimg[hpass+i] += thold;
				else if (fimg[hpass+i] >  thold) fimg[hpass+i] -= thold;
				else	 fimg[hpass+i] = 0;
				if (hpass) fimg[i] += fimg[hpass+i];
			}
			hpass = lpass;
		}

		for (unsigned i=0; i < size; i++)
			//img[i][c] = sqr(fimg[i]+fimg[lpass+i]);
			img[i][c] = fimg[i]+fimg[lpass+i];
		
		//free (fimg);
		delete [] fimg;
	}
	
}



//Resizing
//
//Credit: Graphics Gems III: Schumacher, Dale A., General Filtered Image Rescaling, p. 8-16
//https://github.com/erich666/GraphicsGems/blob/master/gemsiii/filter.c, public domain
//
//Resizing an image is either an interpolation (reduction) or extrapolation (expansion) of an image's 
//dimensions.  In the former case, it is the summarization of a group of pixels into one; in the 
//latter, it is essentially the creation of information not captured in the original image. This
//endeavor has to be well-formed in order to render a pleasing result.
//
//The essential operation of resize has two parts: 1) reduction/expansion in one dimension, then
//2) reduction/expansion of the intermediate image in the other dimension.  So, the inter/extrapolation
//of a pixel is peformed with its row or column neighbors.  The amalgamation of neighbor pixels is
//performed with a filter algorithm, essentially a lookup of an equation based on distance from the
//source pixel.  Numerous filters have been presented in the literature; a representative sample is 
//included in this library.  
//
//The resize algorithm presented by Schumacher in Graphics Gems optimizes the application of a filter in 
//four steps: 1) for the destination image size in one dimension, calculate the filter contributions
//for each pixel in one row/colum of that dimension; 2) apply those contributions to the source image to 
//produce an intermediate image changed in size for that dimension; 3) calculate the filter contributions
//for each pixel in one row/colum of the other dimension; 4) apply those contributions to the
//intermediate image to produce the destination image.
//
//The relevant code below are the functions for each of the filters, followed by the Resize method 
//programmed in the pattern describe above.  You'll recognize a lot of Schumacher's code; the
//contribution collection loops and data structures are pasted verbatim, as well as the filter
//functions.
//


double sinc(double x)
{
	x *= PI;
	if(x != 0) return(sin(x) / x);
	return(1.0);
}

#define Lanczos3_support (3.0)

double Lanczos3_filter(double t)
{
	if(t < 0) t = -t;
	if(t < 3.0) return(sinc(t) * sinc(t/3.0));
	return(0.0);
}


#define CatmullRom_support (2.0)

double CatmullRom_filter(double t) { 
	if(t < -2) return 0;
	if(t < -1) return (0.5*(4 + t*(8 + t*(5 + t))));
	if(t < 0)  return (0.5*(2 + t*t*(-5 - 3*t)));
	if(t < 1)  return (0.5*(2 + t*t*(-5 + 3*t)));
	if(t < 2)  return (0.5*(4 + t*(-8 + t*(5 - t))));
	return 0;
}


#define	Bspline_support	(2.0)

double Bspline_filter(double t)
{
	double tt;

	if(t < 0) t = -t;
	if(t < 1) {
		tt = t * t;
		return((.5 * tt * t) - tt + (2.0 / 3.0));
	} else if(t < 2) {
		t = 2 - t;
		return((1.0 / 6.0) * (t * t * t));
	}
	return(0.0);
}


#define	Bicubic_support	(2.0)

#define	B	(1.0 / 3.0)
#define	C	(1.0 / 3.0)

double Bicubic_filter(double t)
{
	double tt;

	tt = t * t;
	if(t < 0) t = -t;
	if(t < 1.0) {
		t = (((12.0 - 9.0 * B - 6.0 * C) * (t * tt))
		   + ((-18.0 + 12.0 * B + 6.0 * C) * tt)
		   + (6.0 - 2 * B));
		return(t / 6.0);
	} else if(t < 2.0) {
		t = (((-1.0 * B - 6.0 * C) * (t * tt))
		   + ((6.0 * B + 30.0 * C) * tt)
		   + ((-12.0 * B - 48.0 * C) * t)
		   + (8.0 * B + 24 * C));
		return(t / 6.0);
	}
	return(0.0);
}

#define	bilinear_support	(1.0)

double bilinear_filter(double t)
{
	if(t < 0.0) t = -t;
	if(t < 1.0) return(1.0 - t);
	return(0.0);
}


#define	box_support		(0.5)

double box_filter(double t)
{
	if((t > -0.5) & (t <= 0.5)) return(1.0);
	return(0.0);
}



void gImage::ApplyResize(unsigned width, unsigned height, RESIZE_FILTER filter, int threadcount)
{
	typedef struct {
		int	pixel;
		double	weight;
	} CONTRIB;

	typedef struct {
		int	n;		// number of contributors 
		CONTRIB	*p;		// pointer to list of contributions 
	} CLIST;


	CLIST	*contrib;		// array of contribution lists 

	double xscale, yscale;		// zoom scale factors 
	int i, j, k;			// loop variables
	int n;				// pixel number 
	double center, left, right;	// filter calculation variables 
	double wi, fscale, weight;	// filter calculation variables 

	double fwidth;
	double (*filterf)(double);

	switch (filter) {
		case FILTER_BOX:
			fwidth = box_support;
			filterf = box_filter;
			break;
		case FILTER_BILINEAR:
			fwidth = bilinear_support;
			filterf = bilinear_filter;
			break;
		case FILTER_BSPLINE:
			fwidth = Bspline_support;
			filterf = Bspline_filter;
			break;
		case FILTER_BICUBIC:
			fwidth = Bicubic_support;
			filterf = Bicubic_filter;
			break;
		case FILTER_CATMULLROM:
			fwidth = CatmullRom_support;
			filterf = CatmullRom_filter;
			break;
		case FILTER_LANCZOS3:
			fwidth = Lanczos3_support;
			filterf = Lanczos3_filter;
			break;
	}


	xscale = (double) width / (double) w;
	yscale = (double) height / (double) h;

	std::vector<pix>& src = getImageData();
	std::vector<pix> *t = new std::vector<pix>(image);
	std::vector<pix> &tmp = *t;

	tmp.resize(width*h);
	std::vector<pix>&dst = getImageData();


	// Compute row contributions:
	contrib = new CLIST[width];
	if(xscale < 1.0) {
		wi = fwidth / xscale;
		fscale = 1.0 / xscale;
		for(i = 0; i < width; ++i) {
			contrib[i].n = 0;
			contrib[i].p = new CONTRIB[(int) (wi * 2 + 1)];
			center = (double) i / xscale;
			left = ceil(center - wi);
			right = floor(center + wi);
			for(j = (int)left; j <= (int)right; ++j) {
				weight = center - (double) j;
				weight = (*filterf)(weight / fscale) / fscale;
				if(j < 0) {
					n = -j;
				} else if(j >= w) {
					n = (w - j) + w - 1;
				} else {
					n = j;
				}
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	} else {
		for(i = 0; i < width; ++i) {
			contrib[i].n = 0;
			contrib[i].p = new CONTRIB[(int) (fwidth * 2 + 1)];
			center = (double) i / xscale;
			left = ceil(center - fwidth);
			right = floor(center + fwidth);
			for(j = (int)left; j <= (int)right; ++j) {
				weight = center - (double) j;
				weight = (*filterf)(weight);
				if(j < 0) {
					n = -j;
				} else if(j >= w) {
					n = (w - j) + w - 1;
				} else {
					n = j;
				}
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	}
	//dumpContrib(contrib,width);

	// Apply row contributions:
	#pragma omp parallel for num_threads(threadcount)
	for(unsigned y = 0; y < h; ++y) {
		unsigned raster = w*y;
		for(unsigned x = 0; x < width; ++x) {
			unsigned pos = x + width*y;
			double weightr = 0.0;
			double weightg = 0.0;
			double weightb = 0.0;
			for(unsigned j = 0; j < contrib[x].n; ++j) {
				unsigned rpos = raster+contrib[x].p[j].pixel;
				weightr += image[rpos].r * contrib[x].p[j].weight;
				weightg += image[rpos].g * contrib[x].p[j].weight;
				weightb += image[rpos].b * contrib[x].p[j].weight;
			}
			tmp[pos].r = weightr;
			tmp[pos].g = weightg;
			tmp[pos].b = weightb;
		}
	}


	//delete the memory allocated for horizontal filter weights:
	for(i = 0; i < width; ++i) {
		delete [] contrib[i].p;
	}
	delete[] contrib;


	// Compute column contributions:
	contrib = new CLIST[height];
	if(yscale < 1.0) {
		wi = fwidth / yscale;
		fscale = 1.0 / yscale;
		for(i = 0; i < height; ++i) {
			contrib[i].n = 0;
			contrib[i].p = new CONTRIB[(int) (wi * 2 + 1)];
			center = (double) i / yscale;
			left = ceil(center - wi);
			right = floor(center + wi);
			for(j = (int)left; j <= (int)right; ++j) {
				weight = center - (double) j;
				weight = (*filterf)(weight / fscale) / fscale;
				if(j < 0) {
					n = -j;
				} else if(j >= h) {
					n = (h - j) + h - 1;
				} else {
					n = j;
				}
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	} else {
		for(i = 0; i < height; ++i) {
			contrib[i].n = 0;
			contrib[i].p = new CONTRIB[(int) (fwidth * 2 + 1)];
			center = (double) i / yscale;
			left = ceil(center - fwidth);
			right = floor(center + fwidth);
			for(j = (int)left; j <= (int)right; ++j) {
				weight = center - (double) j;
				weight = (*filterf)(weight);
				if(j < 0) {
					n = -j;
				} else if(j >= h) {
					n = (h - j) + h - 1;
				} else {
					n = j;
				}
				k = contrib[i].n++;
				contrib[i].p[k].pixel = n;
				contrib[i].p[k].weight = weight;
			}
		}
	}

	dst.resize(width*height);
	w = width;
	h = height;

	// Apply column contributions:
	#pragma omp parallel for num_threads(threadcount)
	for(unsigned x = 0; x < width; ++x) {
		for(unsigned y = 0; y < height; ++y) {
			unsigned pos = x + y*width;
			double weightr = 0.0;
			double weightg = 0.0;
			double weightb = 0.0;
			for(unsigned j = 0; j < contrib[y].n; ++j) {
				unsigned cpos = x+(width*contrib[y].p[j].pixel);
				weightr += tmp[cpos].r * contrib[y].p[j].weight;
				weightg += tmp[cpos].g * contrib[y].p[j].weight;
				weightb += tmp[cpos].b * contrib[y].p[j].weight;
			}
			dst[pos].r = weightr;
			dst[pos].g = weightg;
			dst[pos].b = weightb;

		}
	}

	//delete the memory allocated for vertical filter weights:
	for(i = 0; i < height; ++i) {
		delete [] contrib[i].p;
	}
	delete[] contrib;
	delete t;
}

void gImage::ApplyResize(std::string params, int threadcount)
{
	char cmd[256];
	strncpy(cmd, params.c_str(), 255);
	unsigned w, h;  //don't read defaults from properties
	std::string algo = "catmullrom";
	char *wstr = strtok(cmd,", ");
	char *hstr = strtok(NULL,", ");
	char *astr = strtok(NULL," ");
	if (wstr == NULL) {
		printf("Error: resize needs at least one parameter.\n");
	}
	else {
		if (wstr) w = atoi(wstr);
		if (hstr) h = atoi(hstr);
		if (astr) algo = std::string(astr);
		unsigned dw = getWidth();
		unsigned dh = getHeight();

		//if only one number is provided, put it in the largest dimension, set the other to 0
		if (hstr == NULL) {
			if (dh > dw) {
				h = w;
				w = 0;
			}
			else {
				h =0;
			}
		}

		if (h ==  0) h = dh * ((float)w/(float)dw);
		if (w == 0)  w = dw * ((float)h/(float)dh); 

		RESIZE_FILTER filter = FILTER_CATMULLROM; 
		if (algo == "box") filter = FILTER_BOX;
		if (algo == "bilinear") filter = FILTER_BILINEAR;
		if (algo == "bspline") filter = FILTER_BSPLINE;
		if (algo == "bicubic") filter = FILTER_BICUBIC;
		if (algo == "catmullrom") filter = FILTER_CATMULLROM;
		if (algo == "lanczos3") filter = FILTER_LANCZOS3;

		ApplyResize(w,h, filter, threadcount);
	}
}




//Redeye
//
//I started with an algorithm that walked concentric rings out from the specified center 
//point of the red eye, but the rectangular corners were evident in a pixel-peep.  My second
//try, below is a simple walk of the rectangular patch, doing a distance check of each
//pixel to the center and moving on if it was farther than the limit.  I put the omp
//pragma on the outside loop; each eye gets its own thread. The whole thing isn't very onerous,
//but my max test has been four eyes (two people...)

//The essential algorithm is to calculate the red intensity proportionate to the averaged
//green and blue intensities; if greater than the threshold, the red value is replaced 
//with the average of the green and blue.  This means that things like light reflections
//(white highlights) and other "non-red" pixels are not touched, leaving a well-formed eye.
//If the center point is centered well and the limit is just larger than the red spot, it 
//can be hard to tell the pixels were touched.
//
//I also added a desaturate option, converts each pixel to it's grayscale equivalent.  This
//seemed necessary to handle non-symmetric color spaces like ProPhoto, in which the original 
//treatment rendered a green cast.
//


void gImage::ApplyRedeye(std::vector<coord> points, double threshold, unsigned limit, bool desaturate, double desaturatepercent, int threadcount)
{
	#pragma omp parallel for num_threads(threadcount)
	for (int i=0; i< points.size(); i++) {
		if (points[i].x > w) continue;
		if (points[i].y > h) continue;
		unsigned cx = points[i].x - limit;
		unsigned cy = points[i].y - limit;
		for (unsigned y=cy; y<cy+limit*2; y++) {
			for (unsigned x=cx; x<cx+limit*2; x++) {
				unsigned pos = x + y*w;
				if (pos > w*h) continue;
				unsigned d = sqrt(sqr(x - points[i].x) + sqr(y - points[i].y));
				if (d > limit) continue;
				double ri = image[pos].r / ((image[pos].g + image[pos].b) /2.0);
				if (ri > threshold) {
					image[pos].r = (image[pos].g + image[pos].b) / 2.0;

				}
				if (desaturate) {
					double d = ((image[pos].r +image[pos].g +image[pos].b)/3) * desaturatepercent;
					image[pos].r = d;
					image[pos].g = d;
					image[pos].b = d;
				}
			}
		}
	}
}

GIMAGE_ERROR gImage::ApplyColorspace(std::string iccfile, cmsUInt32Number intent, bool blackpointcomp, int threadcount)
{
	cmsUInt32Number format;
	cmsHTRANSFORM hTransform;
	cmsUInt32Number dwFlags = 0;

	if (profile == NULL) {lasterror = GIMAGE_APPLYCOLORSPACE_BADINPUTPROFILE; return lasterror;}
	
	if (blackpointcomp) dwFlags = cmsFLAGS_BLACKPOINTCOMPENSATION;

	if (sizeof(PIXTYPE) == 2) format = TYPE_RGB_HALF_FLT; 
	if (sizeof(PIXTYPE) == 4) format = TYPE_RGB_FLT;
	if (sizeof(PIXTYPE) == 8) format = TYPE_RGB_DBL;

	cmsHPROFILE gImgProf = cmsOpenProfileFromMem(profile, profile_length);
	cmsHPROFILE hImgProf;
	if (std::count(iccfile.begin(), iccfile.end(), ',') == 8)
		hImgProf = makeLCMSAdobeCoeffProfile(iccfile);
	else
		hImgProf = myCmsOpenProfileFromFile(iccfile);
	
	if (!gImgProf) {lasterror = GIMAGE_APPLYCOLORSPACE_BADINPUTPROFILE; return lasterror;}
	if (!hImgProf) {lasterror = GIMAGE_APPLYCOLORSPACE_BADOUTPUTPROFILE; return lasterror;}

	if (!cmsIsIntentSupported(gImgProf, intent, LCMS_USED_AS_INPUT))  {lasterror = GIMAGE_APPLYCOLORSPACE_BADINTENT; return lasterror;}
	if (!cmsIsIntentSupported(hImgProf, intent, LCMS_USED_AS_OUTPUT)) {lasterror = GIMAGE_APPLYCOLORSPACE_BADINTENT; return lasterror;}

	if (gImgProf) {
		if (hImgProf) {
			hTransform = cmsCreateTransform(gImgProf, format, hImgProf, format, intent, dwFlags);
			if (hTransform == NULL) {lasterror = GIMAGE_APPLYCOLORSPACE_BADTRANSFORM; return lasterror;}
			
			pix* img = image.data();
			#pragma omp parallel for num_threads(threadcount)
			for (unsigned y=0; y<h; y++) {
				unsigned pos = y*w;
				cmsDoTransform(hTransform, &img[pos], &img[pos], w);

			}
			
			char * prof; cmsUInt32Number proflen;	
			gImage::makeICCProfile(hImgProf, prof, proflen);
			setProfile(prof, proflen);
		}
	}
	{lasterror = GIMAGE_OK; return lasterror;}
}


GIMAGE_ERROR gImage::AssignColorspace(std::string iccfile)
{
	cmsHPROFILE hImgProf;
	if (std::count(iccfile.begin(), iccfile.end(), ',') == 8)
		hImgProf = makeLCMSAdobeCoeffProfile(iccfile);
	else
		hImgProf = myCmsOpenProfileFromFile(iccfile);
	if (hImgProf) {
		char * prof; cmsUInt32Number proflen;	
		gImage::makeICCProfile(hImgProf, prof, proflen);
		setProfile(prof, proflen);
		lasterror = GIMAGE_OK; 
		return lasterror;
	}
	else {
		lasterror = GIMAGE_ASSIGNCOLORSPACE_BADTRANSFORM; 
		return lasterror;
	}
}



// End of Image Manipulations



//Image Information:

std::string gImage::Stats(bool isfloat)
{
	std::string stats_string;
	std::map<std::string,float> statmap = StatsMap();

	stats_string = "channels:\n";
	if (statmap.find("rmin") != statmap.end()) {
		stats_string += string_format("rmin:  %11.6f",statmap["rmin"]);
		if (statmap.find("rmax") != statmap.end()) stats_string +=  string_format("\trmax:  %11.6f",statmap["rmax"]);
		stats_string += "\n";
	}

	if (statmap.find("g1min") != statmap.end()) {
		stats_string += string_format("g1min: % 11.6f",statmap["g1min"]);
		if (statmap.find("g1max") != statmap.end()) stats_string += string_format("\tg1max: %11.6f",statmap["g1max"]);
		stats_string += "\n";
		if (statmap.find("g2min") != statmap.end())  {
			stats_string += string_format("g2min: % 11.6f",statmap["g2min"]);
			if (statmap.find("g2max") != statmap.end()) stats_string += string_format("\tg2max: % 11.6f",statmap["g2max"]);
			stats_string += "\n";
		}
	}
	else if (statmap.find("gmin") != statmap.end()) {
		stats_string += string_format("gmin:  % 11.6f",statmap["gmin"]);
		if (statmap.find("gmax") != statmap.end()) stats_string += string_format("\tgmax:  % 11.6f",statmap["gmax"]);
		stats_string += "\n";
	}

	if (statmap.find("bmin") != statmap.end()) {
		stats_string += string_format("bmin:  % 11.6f",statmap["bmin"]);
		if (statmap.find("bmax") != statmap.end()) stats_string += string_format("\tbmax:  % 11.6f",statmap["bmax"]);
		stats_string += "\n";
	}

	if (statmap.find("g1mean") != statmap.end()) {
		stats_string += "\nchannel means:\n";
		if (statmap.find("rmean") != statmap.end())  stats_string += string_format("rmean:  % 11.6f\n",statmap["rmean"]);
		if (statmap.find("g1mean") != statmap.end()) stats_string += string_format("g1mean: % 11.6f\n",statmap["g1mean"]);
		if (statmap.find("g2mean") != statmap.end()) stats_string += string_format("g2mean: % 11.6f\n",statmap["g2mean"]);
		if (statmap.find("bmean") != statmap.end())  stats_string += string_format("bmean:  % 11.6f\n",statmap["bmean"]);
				
	}
	else if (statmap.find("gmean") != statmap.end()) {
		stats_string += "\nchannel means:\n";
		if (statmap.find("rmean") != statmap.end()) stats_string +=  string_format("rmean:  % 11.6f\n",statmap["rmean"]);
		if (statmap.find("gmean") != statmap.end()) stats_string +=  string_format("gmean:  % 11.6f\n",statmap["gmean"]);
		if (statmap.find("bmean") != statmap.end()) stats_string +=  string_format("bmean:  % 11.6f\n",statmap["bmean"]);
	}
	
	if (statmap.find("tmin") != statmap.end()) {
		stats_string += "\ntone:\n";
		stats_string += string_format("tmin:  % 11.6f",statmap["tmin"]);
		if (statmap.find("tmax") != statmap.end()) stats_string += string_format("\ttmax:  % 11.6f",statmap["tmax"]);
		stats_string += "\n";
	}
	
	return stats_string;
}

std::map<std::string,float> gImage::StatsMap()
{
	double rmin, rmax, gmin, gmax, bmin, bmax, g1min, g1max, g2min, g2max;
	rmin=rmax=image[0].r; gmin=gmax=g1min=g1max=g2min=g2max=image[0].g; bmin=bmax=image[0].b;
	double tmin=1.0, tmax=0.0, rmean, gmean, bmean; 
	double rsum=0.0, g1sum=0.0, g2sum=0.0, gsum=0.0, bsum=0.0; 
	long rcount=0, g1count=0, g2count=0, gcount=0, bcount=0;
	
	pix maxpix, minpix;
	long pcount = 0;
	tmin = SCALE_CURVE; tmax = 0.0;
	int iter = 0;

	std::map<std::string, float> stats_map;
	double toneupperthreshold = 1.0; 
	double tonelowerthreshold = 0.0;

	if (imginfo.find("LibrawMosaiced") != imginfo.end() && imginfo["LibrawMosaiced"] == "1")  //R,G,B has to be collected on the mosaic pattern
	{


		unsigned cfarray[2][2];
		unsigned xtarray[6][6];
		int arraydim = 0;

		if (cfArray(cfarray)) {
			arraydim = 2;
			for (unsigned i=0; i<arraydim; i++)
				for (unsigned j=0; j<arraydim; j++)
					xtarray[i][j] = cfarray[i][j];
		}
		else if (xtranArray(xtarray)) {
			arraydim = 6; 
		}
		else return stats_map;

		#pragma omp parallel
		{
			double prmin, prmax, pg1min, pg1max, pg2min, pg2max, pbmin, pbmax, prsum, pg1sum, pg2sum, pbsum;
			prmin=prmax=image[0].r;
			pg1min=pg1max=image[0].g;
			pg2min=pg2max=image[0].g;
			pbmin=pbmax=image[0].b;
			prsum=pg1sum=pg2sum=pbsum=0.0;

			long prcount=0, pg1count=0, pg2count=0, pbcount=0;

			#pragma omp parallel for 
			for (unsigned y=0; y<h-(arraydim-1); y+=arraydim) {
				for (unsigned x=0; x<w-(arraydim-1); x+=arraydim) {
					unsigned Hpos = (x/2) + (y/2)*(w/2);
					float pix[4] = {0.0, 0.0, 0.0, 0.0};
					for (unsigned i=0; i<arraydim; i++) {  //walk the CFA image subset, collect the channel values 
						for (unsigned j=0; j<arraydim; j++) {
							int pos = (x+i) + (y+j) * w;
							switch (xtarray[i][j]) {
								case 0:
									if (image[pos].r > prmax) prmax = image[pos].r; 
									if (image[pos].r < prmin) prmin = image[pos].r;
									prsum += image[pos].r;
									prcount++;
									break;
								case 1:
									if (image[pos].r > pg1max) pg1max = image[pos].r;
									if (image[pos].r < pg1min) pg1min = image[pos].r;
									pg1sum += image[pos].r;
									pg1count++;
									break;
								case 2:
									if (image[pos].r > pbmax) pbmax = image[pos].r;
									if (image[pos].r < pbmin) pbmin = image[pos].r;
									pg2sum += image[pos].r;
									pg2count++;
									break;
								case 3:
									if (image[pos].r > pg2max) pg2max = image[pos].r;
									if (image[pos].r < pg2min) pg2min = image[pos].r;
									pbsum += image[pos].r;
									pbcount++;
									break;
							}
						}
					}
				}
			}

			#pragma omp critical
			{
				if (prmax > rmax) rmax = prmax; if (prmin < rmin) rmin = prmin;
				if (pg1max > g1max) g1max = pg1max; if (pg1min < g1min) g1min = pg1min;
				if (pg2max > g2max) g2max = pg2max; if (pg2min < g2min) g2min = pg2min;
				if (pbmax > bmax) bmax = pbmax; if (pbmin < bmin) bmin = pbmin;
				rsum += prsum; g1sum += pg1sum; g2sum += pg2sum; bsum += pbsum; 
				rcount += prcount; g1count += pg1count; g2count += pg2count; bcount += pbcount;
			}
		}

		//channel mins/maxs:
		stats_map["rmin"] = rmin; stats_map["rmax"] = rmax;
		stats_map["g1min"] = g1min; stats_map["g1max"] = g1max;
		stats_map["g2min"] = g2min; stats_map["g2max"] = g2max;
		stats_map["gmin"] = std::min(g1min, g2min); stats_map["gmax"] = std::max(g1max, g2max);
		stats_map["bmin"] = bmin; stats_map["bmax"] = bmax;
		stats_map["rmean"] = rsum / (double) rcount;
		stats_map["g1mean"] = g1sum / (double) g1count;
		stats_map["g2mean"] = g2sum / (double) g2count;
		stats_map["bmean"] = bsum / (double) bcount;

	}
	else {

		#pragma omp parallel
		{
			double prmin, prmax, pgmin, pgmax, pbmin, pbmax, ptmin, ptmax;
			double prsum=0.0, pgsum=0.0, pbsum=0.0;
			prmin=prmax=image[0].r;
			pgmin=pgmax=image[0].g;
			pbmin=pbmax=image[0].b;
			ptmin=1.0; ptmax=0.0;
			long ppcount = 0;

			#pragma omp for 
			for(unsigned y = 1; y < h; y++) {
				for(unsigned x = 1; x < w; x++) {
					unsigned pos = x + y*w;
					prsum += image[pos].r; pgsum += image[pos].g; pbsum += image[pos].b;
					ppcount++;
					if (image[pos].r > prmax) prmax = image[pos].r;
					if (image[pos].g > pgmax) pgmax = image[pos].g;
					if (image[pos].b > pbmax) pbmax = image[pos].b;
					if (image[pos].r < prmin) prmin = image[pos].r;
					if (image[pos].g < pgmin) pgmin = image[pos].g;
					if (image[pos].b < pbmin) pbmin = image[pos].b;
					double tone = (image[pos].r + image[pos].g + image[pos].b) / 3.0; 
					//if (tone > ptmax & tone < toneupperthreshold) {ptmax = tone; maxpix = image[pos];}
					//if (tone < ptmin & tone > tonelowerthreshold) {ptmin = tone; minpix = image[pos];}
					if (tone > ptmax) {ptmax = tone; maxpix = image[pos];}
					if (tone < ptmin) {ptmin = tone; minpix = image[pos];}
					iter++;
				}
			}

			#pragma omp critical
			{
				if (prmax > rmax) rmax = prmax; if (prmin < rmin) rmin = prmin;
				if (pgmax > gmax) gmax = pgmax; if (pgmin < gmin) gmin = pgmin;
				if (pbmax > bmax) bmax = pbmax; if (pbmin < bmin) bmin = pbmin;
				if (ptmax > tmax) tmax = ptmax; if (ptmin < tmin) tmin = ptmin;
				rsum += prsum; gsum += pgsum; bsum += pbsum; pcount += ppcount;
			}
		}

		//channel mins/maxs:
		stats_map["rmin"] = rmin; stats_map["rmax"] = rmax;
		stats_map["gmin"] = gmin; stats_map["gmax"] = gmax;
		stats_map["bmin"] = bmin; stats_map["bmax"] = bmax;

		//tone min/max:
		stats_map["tmin"] = tmin; stats_map["tmax"] = tmax;

		//channel means:
		stats_map["rmean"] = rsum/(double)pcount;
		stats_map["gmean"] = gsum/(double)pcount;
		stats_map["bmean"] = bsum/(double)pcount;
	}

	return stats_map;
}

//calculate averages of red, green, and blue channels:
std::vector<double> gImage::CalculateChannelMeans()
{
	std::vector<double> rgbmeans;
	double rsum=0.0, gsum=0.0, bsum=0.0;
	long pcount = 0;

	#pragma omp parallel for 
	for(unsigned y = 1; y < h; y++) {
		for(unsigned x = 1; x < w; x++) {
			unsigned pos = x + y*w;
			rsum += image[pos].r; 
			gsum += image[pos].g; 
			bsum += image[pos].b;
			pcount++;
		}
	}

	rgbmeans.push_back(rsum / (double) pcount);
	rgbmeans.push_back(gsum / (double) pcount);
	rgbmeans.push_back(bsum / (double) pcount);

	return rgbmeans;
}

std::vector<double> gImage::CalculatePatchMeans(int x, int y, float radius)
{
	std::vector<double> means;
	double rsum=0.0, gsum=0.0, bsum=0.0;
	int count = 0;
	for (int i=x-radius; i<x+radius; i++) {
		for (int j=y-radius; j<y+radius; j++) {
			pix p = getRGB(i,j);
			rsum += p.r;
			gsum += p.g;
			bsum += p.b;
			count++;
		}
	}
	means.push_back(rsum/(double)count);
	means.push_back(gsum/(double)count);
	means.push_back(bsum/(double)count);
	return means;
}

//calculate a normalized black and white point, expressed in the 0-255 range:
std::vector<double> gImage::CalculateBlackWhitePoint(double blackthreshold, double whitethreshold, bool centerout, int whitemax, std::string channel)
{
	std::vector<double> bwpoints;
	std::vector<long> hdata;
	if (channel == "red")   hdata = RedHistogram();
	else if (channel == "green") hdata = GreenHistogram();
	else if (channel == "blue")  hdata = BlueHistogram();
	else if (channel == "min") {
		std::map<std::string,float> stats = StatsMap();
		float rmax = stats["rmax"];
		float gmax = stats["gmax"];
		float bmax = stats["bmax"];
		if ((rmax < gmax) & (rmax < bmax)) hdata = RedHistogram();
		else if ((gmax < rmax) & (gmax < bmax)) hdata = GreenHistogram();
		else hdata = BlueHistogram();
	}
	else hdata = Histogram();
	long hmax=0;
	int maxpos;
	long htotal = 0;
	int i;
				
	for (i=0; i<whitemax; i++) {  //240 avoids calculating the max on clipping
		htotal += hdata[i];
		if (hmax < hdata[i]) {
			maxpos = i;
			hmax = hdata[i];
		}
	}

	if (centerout) {
		//find black threshold:
		long hblack = 0;
		for (i=maxpos; i>0; i--) 
			if ((double) hdata[i] / (double) hmax < blackthreshold) break;
		bwpoints.push_back((double) i);

		//find white threshold:
		long hwhite = 0;
		for (i=maxpos; i<255; i++) 
			if ((double) hdata[i] / (double) hmax < whitethreshold) break;
		bwpoints.push_back((double) i);
	}
	return bwpoints;
}

//simple grayscale histogram:
std::vector<long> gImage::Histogram()
{
	std::vector<long> histogram(256, 0);

	for(unsigned y = 0; y < h; y++) {
		for(unsigned x = 0; x < w; x++) {
			unsigned pos = x + y*w;
			double t = ((image[pos].r + image[pos].g + image[pos].b) / 3.0) * SCALE_CURVE;
			if (t < 0.0) t = 0.0;
			if (t > 255.0) t = 255.0;
			histogram[floor(t+0.5)]++;
		}
	}
	return histogram;
}

//simple red histogram:
std::vector<long> gImage::RedHistogram()
{
	std::vector<long> histogram(256, 0);

	for(unsigned y = 0; y < h; y++) {
		for(unsigned x = 0; x < w; x++) {
			unsigned pos = x + y*w;
			double t = image[pos].r * SCALE_CURVE;
			if (t < 0.0) t = 0.0;
			if (t > 255.0) t = 255.0;
			histogram[floor(t+0.5)]++;
		}
	}
	return histogram;
}

//simple green histogram:
std::vector<long> gImage::GreenHistogram()
{
	std::vector<long> histogram(256, 0);

	for(unsigned y = 0; y < h; y++) {
		for(unsigned x = 0; x < w; x++) {
			unsigned pos = x + y*w;
			double t = image[pos].g * SCALE_CURVE;
			if (t < 0.0) t = 0.0;
			if (t > 255.0) t = 255.0;
			histogram[floor(t+0.5)]++;
		}
	}
	return histogram;
}

//simple blue histogram:
std::vector<long> gImage::BlueHistogram()
{
	std::vector<long> histogram(256, 0);

	for(unsigned y = 0; y < h; y++) {
		for(unsigned x = 0; x < w; x++) {
			unsigned pos = x + y*w;
			double t = image[pos].b * SCALE_CURVE;
			if (t < 0.0) t = 0.0;
			if (t > 255.0) t = 255.0;
			histogram[floor(t+0.5)]++;
		}
	}
	return histogram;
}


std::vector<histogramdata> gImage::Histogram(unsigned scale, int &zerobucket, int &onebucket, float &dmin, float &dmax)
{
	std::map<std::string,float> stats = StatsMap();
	#if defined PIXHALF
	dmin = fmin((half_float::half) stats["bmin"], fmin((half_float::half) stats["rmin"],(half_float::half) stats["gmin"]));
	dmax = fmax((half_float::half) stats["bmax"], fmax((half_float::half) stats["rmax"],(half_float::half) stats["gmax"]));
	#else
	dmin = fmin(stats["bmin"], fmin(stats["rmin"],stats["gmin"]));
	dmax = fmax(stats["bmax"], fmax(stats["rmax"],stats["gmax"]));
	#endif

	float inc = (dmax - dmin) / (float) scale;
	zerobucket = abs(dmin) /inc;
	onebucket = zerobucket + (1.0 / inc);

	histogramdata zerodat = {0,0,0};
	std::vector<histogramdata> histogram(scale, zerodat);


	if (imginfo.find("LibrawMosaiced") != imginfo.end() && imginfo["LibrawMosaiced"] == "1") { //R,G,B has to be collected on the mosaic pattern

		unsigned cfarray[2][2];
		unsigned xtarray[6][6];
		int arraydim = 0;

		if (cfArray(cfarray)) {
			arraydim = 2;
			for (unsigned i=0; i<arraydim; i++)
				for (unsigned j=0; j<arraydim; j++)
					xtarray[i][j] = cfarray[i][j];
		}
		else if (xtranArray(xtarray)) {
			arraydim = 6; 
		}
		else return histogram;

		#pragma omp parallel
		{
			std::vector<unsigned> pr(scale,0);
			std::vector<unsigned> pg(scale,0);
			std::vector<unsigned> pb(scale,0);

			std::vector<unsigned> pg1(scale,0);
			std::vector<unsigned> pg2(scale,0);

			#pragma omp parallel for 
			for (unsigned y=0; y<h-(arraydim-1); y+=arraydim) {
				for (unsigned x=0; x<w-(arraydim-1); x+=arraydim) {
					unsigned Hpos = (x/2) + (y/2)*(w/2);
					float pix[4] = {0.0, 0.0, 0.0, 0.0};
					for (unsigned i=0; i<arraydim; i++) {  //walk the CFA image subset, collect the channel values 
						for (unsigned j=0; j<arraydim; j++) {
							int pos = (x+i) + (y+j) * w;
							unsigned bin = std::min((unsigned) ((image[pos].r-dmin)/inc),scale-1);
							switch (xtarray[i][j]) {
								case 0:
									pr[bin]++;
									break;
								case 1:
									pg1[bin]++;
									break;
								case 2:
									pb[bin]++;
									break;
								case 3:
									pg2[bin]++;
									break;
							}
						}
					}
				}
			}

			#pragma omp critical
			{
				for (unsigned i=0; i<scale; i++) {
					histogram[i].r += pr[i];
					histogram[i].g += (pg1[i]+pg2[i])/2.0;
					histogram[i].b += pb[i];
				}
			}
		}

	}
	else { //R,G,B has to be collected on the RGB pattern

		#pragma omp parallel
		{
			std::vector<unsigned> pr(scale,0);
			std::vector<unsigned> pg(scale,0);
			std::vector<unsigned> pb(scale,0);
		
			#pragma omp for
			for(unsigned y = 0; y < h; y++) {
				for(unsigned x = 0; x < w; x++) {
					unsigned pos = x + y*w;
					pr[std::min((unsigned) ((image[pos].r-dmin)/inc),scale-1)]++;
					pg[std::min((unsigned) ((image[pos].g-dmin)/inc),scale-1)]++;
					pb[std::min((unsigned) ((image[pos].b-dmin)/inc),scale-1)]++;
				}
			}
		
			#pragma omp critical 
			{
				for (unsigned i=0; i<scale; i++) {
					histogram[i].r += pr[i];
					histogram[i].g += pg[i];
					histogram[i].b += pb[i];
				}
			}
		}
	
	}
	
	return histogram;
}


//rgb histogram, scale=number of buckets...
std::vector<histogramdata> gImage::Histogram(unsigned scale)
{
	#if defined PIXHALF
	half s = (half_float::half) scale;
	#elif defined PIXFLOAT
	float s = (float) scale;
	#else
	double s = (double) scale;
	#endif
	
	histogramdata zerodat = {0,0,0};
	std::vector<histogramdata> histogram(scale, zerodat);

	if (imginfo.find("LibrawMosaiced") != imginfo.end() && imginfo["LibrawMosaiced"] == "1") { //R,G,B has to be collected on the mosaic pattern

		std::vector<unsigned> q = {0, 1, 1, 2};  //default pattern is RGGB, where R=0, G=1, B=2
		if (imginfo["LibrawCFAPattern"] == "GRBG") q = {1, 0, 2, 1};
		if (imginfo["LibrawCFAPattern"] == "GBRG") q = {1, 2, 0, 1};
		if (imginfo["LibrawCFAPattern"] == "BGGR") q = {2, 1, 1, 0};

		#pragma omp parallel
		{
			std::vector<unsigned> pr(scale,0);
			std::vector<unsigned> pg(scale,0);
			std::vector<unsigned> pb(scale,0);


			#pragma omp for
			for (unsigned y=0; y<h-1; y+=2) {
				for (unsigned x=0; x<w-1; x+=2) {
					unsigned pos[4];
					pos[0] = x + y*w;  //upper left
					pos[1] = (x+1) + y*w; //upper right
					pos[2] = x + (y+1)*w; //lower leftfs
					pos[3] = (x+1) + (y+1)*w;  //lower right
					for (unsigned i=0; i<q.size(); i++) {
						#if defined PIXHALF
						if (q[i] == 0) pr[(unsigned) lrint(fmin(fmax(image[pos[i]].r*s,0.0_h),s-1.0_h))]++;
						if (q[i] == 1) pg[(unsigned) lrint(fmin(fmax(image[pos[i]].r*s,0.0_h),s-1.0_h))]++;
						if (q[i] == 2) pb[(unsigned) lrint(fmin(fmax(image[pos[i]].r*s,0.0_h),s-1.0_h))]++;
						#elif defined PIXFLOAT
						if (q[i] == 0) pr[(unsigned) lrint(fmin(fmax(image[pos[i]].r*s,0.0f),s-1.0f))]++;
						if (q[i] == 1) pg[(unsigned) lrint(fmin(fmax(image[pos[i]].r*s,0.0f),s-1.0f))]++;
						if (q[i] == 2) pb[(unsigned) lrint(fmin(fmax(image[pos[i]].r*s,0.0f),s-1.0f))]++;
						#else
						if (q[i] == 0) pr[(unsigned) lrint(fmin(fmax(image[pos[i]].r*s,0.0),s-1.0))]++;
						if (q[i] == 1) pg[(unsigned) lrint(fmin(fmax(image[pos[i]].r*s,0.0),s-1.0))]++;
						if (q[i] == 2) pb[(unsigned) lrint(fmin(fmax(image[pos[i]].r*s,0.0),s-1.0))]++;
						#endif
					}
				}
			}

			#pragma omp critical 
			{
				for (unsigned i=0; i<scale; i++) {
					histogram[i].r += pr[i];
					histogram[i].g += pg[i];
					histogram[i].b += pb[i];
				}
			}
		}
	}
	else { //regular RGB image

		#pragma omp parallel
		{
			std::vector<unsigned> pr(scale,0);
			std::vector<unsigned> pg(scale,0);
			std::vector<unsigned> pb(scale,0);
		
			#pragma omp for
			for(unsigned y = 0; y < h; y++) {
				for(unsigned x = 0; x < w; x++) {
					unsigned pos = x + y*w;
					#if defined PIXHALF
					pr[(unsigned) lrint(fmin(fmax(image[pos].r*s,0.0_h),s-1.0_h))]++;
					pg[(unsigned) lrint(fmin(fmax(image[pos].g*s,0.0_h),s-1.0_h))]++;
					pb[(unsigned) lrint(fmin(fmax(image[pos].b*s,0.0_h),s-1.0_h))]++;
					#elif defined PIXFLOAT
					pr[(unsigned) lrint(fmin(fmax(image[pos].r*s,0.0f),s-1.0f))]++;
					pg[(unsigned) lrint(fmin(fmax(image[pos].g*s,0.0f),s-1.0f))]++;
					pb[(unsigned) lrint(fmin(fmax(image[pos].b*s,0.0f),s-1.0f))]++;
					#else
					pr[(unsigned) lrint(fmin(fmax(image[pos].r*s,0.0),s-1.0))]++;
					pg[(unsigned) lrint(fmin(fmax(image[pos].g*s,0.0),s-1.0))]++;
					pb[(unsigned) lrint(fmin(fmax(image[pos].b*s,0.0),s-1.0))]++;
					#endif
				}
			}
		
			#pragma omp critical 
			{
				for (unsigned i=0; i<scale; i++) {
					histogram[i].r += pr[i];
					histogram[i].g += pg[i];
					histogram[i].b += pb[i];
				}
			}
		}

	}

	return histogram;
}


//single-channel histogram, OBE...
std::vector<long> gImage::Histogram(unsigned channel, unsigned &hmax)
{
	#if defined PIXHALF
	half s;
	if (b == BPP_16) s = 65536.0_h;
	else s = 256.0_h;
	#elif defined PIXFLOAT
	float s;
	if (b == BPP_16) s = 65536.0f;
	else s = 256.0f;
	#else
	double s;
	if (b == BPP_16) s = 65536.0;
	else s = 256.0;
	#endif
	
	hmax = 0;
	
	std::vector<long> hdata(s,0);

	//#pragma omp parallel for
	for(unsigned y = 0; y < h; y++) {
		for(unsigned x = 0; x < w; x++) {
			unsigned pos = x + y*w;
			unsigned d; 
			#if defined PIXHALF
			if (channel == CHANNEL_RED)   d = (unsigned) fmin(fmax(image[pos].r*s,0.0_h),s-1.0_h);
			if (channel == CHANNEL_GREEN) d = (unsigned) fmin(fmax(image[pos].g*s,0.0_h),s-1.0_h);
			if (channel == CHANNEL_BLUE)  d = (unsigned) fmin(fmax(image[pos].b*s,0.0_h),s-1.0_h);
			#elif defined PIXFLOAT
			if (channel == CHANNEL_RED)   d = (unsigned) fmin(fmax(image[pos].r*s,0.0f),s-1.0f);
			if (channel == CHANNEL_GREEN) d = (unsigned) fmin(fmax(image[pos].g*s,0.0f),s-1.0f);
			if (channel == CHANNEL_BLUE)  d = (unsigned) fmin(fmax(image[pos].b*s,0.0f),s-1.0f);
			#else
			if (channel == CHANNEL_RED)   d = (unsigned) fmin(fmax(image[pos].r*s,0.0),s-1.0);
			if (channel == CHANNEL_GREEN) d = (unsigned) fmin(fmax(image[pos].g*s,0.0),s-1.0);
			if (channel == CHANNEL_BLUE)  d = (unsigned) fmin(fmax(image[pos].b*s,0.0),s-1.0);
			#endif
			
			hdata[d]++;
			if (hmax < hdata[d]) hmax = hdata[d];
		}
	}
	
	return hdata;
}


//Loaders:

gImage gImage::loadImageFile(const char * filename, std::string params)
{
	GIMAGE_FILETYPE ext = gImage::getFileType(filename);

	if (ext == FILETYPE_TIFF) return gImage::loadTIFF(filename, params);
	else if (ext == FILETYPE_JPEG) return gImage::loadJPEG(filename, params);
	else if (ext == FILETYPE_PNG) return gImage::loadPNG(filename, params);
	else return gImage::loadRAW(filename, params);
}

std::map<std::string,std::string> gImage::loadImageFileInfo(const char * filename)
{
	unsigned width, height, bpp, colors, icclength;
	BPP bits;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	std::string params;
	GIMAGE_FILETYPE ext = gImage::getFileType(filename);

	if (ext == FILETYPE_TIFF) {
		_loadTIFFInfo(filename, &width, &height, &colors, &bpp, imgdata); 
	}
	else if (ext == FILETYPE_JPEG) {
		_loadJPEGInfo(filename, &width, &height, &colors, imgdata); 
	}
	else if (ext == FILETYPE_PNG) {
		_loadPNGInfo(filename, &width, &height, &colors, &bpp, imgdata); 
	}
	else {
		_loadRAWInfo(filename, &width, &height, &colors, &bpp, imgdata); 
	}
	return imgdata;
}


#ifdef USE_DCRAW
void gImage::setdcrawPath(std::string path)
{
	setdcrawpath(path);
}
#endif



gImage gImage::loadRAW(const char * filename, std::string params)
{
	unsigned width, height, bpp, colors, icclength;
	BPP bits;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	char * image = _loadRAW(filename, &width, &height, &colors, &bpp, imgdata, params, &iccprofile, &icclength);
	if (image == NULL) return gImage();
	switch (bpp) {
		case 8: 
			bits = BPP_8;
			break;
		case 16:
			bits = BPP_16;
			break;
	}
	gImage I(image, width, height, colors, bits, imgdata, iccprofile, icclength);
	delete [] image;
	if (icclength && iccprofile) delete [] iccprofile;
	return I;

}

gImage gImage::loadJPEG(const char * filename, std::string params)
{
	unsigned width, height, colors, bpp, icclength;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	char * image = _loadJPEG(filename, &width, &height, &colors, imgdata, "", &iccprofile, &icclength);
	if (image) {
		gImage I(image, width, height, colors, BPP_8, imgdata, iccprofile, icclength);
		delete [] image;
		if (icclength && iccprofile != NULL) delete [] iccprofile;
		return I;
	}
	else return gImage();
}


gImage gImage::loadTIFF(const char * filename, std::string params)
{
	unsigned width, height, colors, bpp, icclength;
	BPP bits;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	char * image = _loadTIFF(filename, &width, &height, &colors, &bpp, imgdata, params, &iccprofile, &icclength);
	if (image) {
		switch (bpp) {
			case 8: 
				bits = BPP_8;
				break;
			case 16:
				bits = BPP_16;
				break;
			case 32:
				bits = BPP_FP;
				break;
			default: 
				return gImage();
		}
		gImage I(image, width, height, colors, bits, imgdata, iccprofile, icclength);
		delete [] image;
		if (icclength && iccprofile != NULL) delete [] iccprofile;
		return I;
	}
	else return gImage();
}

gImage gImage::loadPNG(const char * filename, std::string params)
{
	unsigned width, height, colors, bpp, icclength;
	BPP bits;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	char * image = _loadPNG(filename, &width, &height, &colors, &bpp, imgdata, params, &iccprofile, &icclength);
	if (image) {
		switch (bpp) {
			case 8: //1 for simplified
				bits = BPP_8;
				break;
			case 16: //2 for simplified
				bits = BPP_16;
				break;
			default: 
				return gImage();
		}
		gImage I(image, width, height, colors, bits, imgdata, iccprofile, icclength);
		delete [] image;
		if (icclength && iccprofile != NULL) delete [] iccprofile;
		return I;
	}
	else return gImage();
}


//Savers:


GIMAGE_ERROR gImage::saveImageFile(const char * filename, std::string params, cmsHPROFILE profile, cmsUInt32Number intent)
{
	BPP bitfmt = BPP_8;
	//$ <li><b>channelformat</b>=8bit|16bit|float|unboundedfloat: Applies to PNG (8bit, 16bit) and TIFF (8bit, 16bit, float).  Specifies the output numeric format.  For float TIFFs, the data is saved 'unbounded', that is, not clipped to 0.0-1.0. </li>
	std::map<std::string, std::string> p = parseparams(params);
	if (p.find("channelformat") != p.end()) {
		if (p["channelformat"] == "8bit")  bitfmt = BPP_8;
		if (p["channelformat"] == "16bit") bitfmt = BPP_16;
		if (p["channelformat"] == "float") bitfmt = BPP_FP;
		if (p["channelformat"] == "unboundedfloat") bitfmt = BPP_UFP;
	}

	GIMAGE_FILETYPE ftype = gImage::getFileNameType(filename);

	if (ftype == FILETYPE_TIFF) {
		if (profile)
			return saveTIFF(filename, bitfmt, params, profile, intent);
		else
			return saveTIFF(filename, bitfmt, params);
	}
	if (ftype == FILETYPE_JPEG) {
		if (profile)
			return saveJPEG(filename, bitfmt, params, profile, intent);
		else
			return saveJPEG(filename, bitfmt, params);
	}
	if (ftype == FILETYPE_PNG) {
		if (profile)
			return savePNG(filename, bitfmt, params, profile, intent);
		else
			return savePNG(filename, bitfmt, params);
	}
	lasterror = GIMAGE_UNSUPPORTED_FILEFORMAT; 
	return lasterror;
}

GIMAGE_ERROR gImage::saveImageFileNoProfile(const char * filename, std::string params)
{
	BPP bitfmt = BPP_8;
	//$ <li><b>channelformat</b>=8bit|16bit|float|unboundedfloat: Applies to PNG (8bit, 16bit) and TIFF (8bit, 16bit, float).  Specifies the output numeric format.  For float TIFFs, the data is saved 'unbounded', that is, not clipped to 0.0-1.0. </li>
	std::map<std::string, std::string> p = parseparams(params);
	if (p.find("channelformat") != p.end()) {
		if (p["channelformat"] == "8bit")  bitfmt = BPP_8;
		if (p["channelformat"] == "16bit") bitfmt = BPP_16;
		if (p["channelformat"] == "float") bitfmt = BPP_FP;
		if (p["channelformat"] == "unboundedfloat") bitfmt = BPP_UFP;
	}

	GIMAGE_FILETYPE ftype = gImage::getFileNameType(filename);

	if (ftype == FILETYPE_TIFF) {
		return saveTIFF(filename, bitfmt, params);
	}
	if (ftype == FILETYPE_JPEG) {
		return saveJPEG(filename, bitfmt, params);
	}
	if (ftype == FILETYPE_PNG) {
		return savePNG(filename, bitfmt, params);
	}
	lasterror = GIMAGE_UNSUPPORTED_FILEFORMAT; 
	return lasterror;
}



GIMAGE_ERROR gImage::saveJPEG(const char * filename, BPP bits, std::string params, cmsHPROFILE profile, cmsUInt32Number intent)
{
	unsigned b = 8;
	if (bits == BPP_8)  b = 8;
	else {lasterror = GIMAGE_UNSUPPORTED_PIXELFORMAT; return lasterror;}
	
	if (profile) {
		char * iccprofile;
		cmsUInt32Number iccprofilesize;
		makeICCProfile(profile, iccprofile, iccprofilesize);

		try {
			//Pick one, getTransformedImageData() seems to produce less noise, but is slower:
			_writeJPEG(filename, getTransformedImageData(BPP_8, profile, intent),  w, h, c, b, imginfo, params, iccprofile, iccprofilesize); 
			//_writeJPEG(filename, getImageData(BPP_8, profile),  w, h, c, imginfo, params); 
		}
		catch (std::exception &e) {
			lasterror = GIMAGE_EXCEPTION;
			delete [] iccprofile;
			return lasterror;
		}

		delete [] iccprofile;
	}
	else {
		if (this->profile)
			_writeJPEG(filename, getImageData(BPP_8),  w, h, c, b, imginfo, params, this->profile, profile_length);
		else
			_writeJPEG(filename, getImageData(BPP_8),  w, h, c, b, imginfo, params);
	}
	lasterror = GIMAGE_OK; 
	return lasterror;
}

GIMAGE_ERROR gImage::saveTIFF(const char * filename, BPP bits, std::string params, cmsHPROFILE profile, cmsUInt32Number intent)
{
	unsigned b = 0;
	if (bits == BPP_16) b = 16;
	else if (bits == BPP_8)  b = 8;
	else if (bits == BPP_FP | bits == BPP_UFP) b = 32;
	else {lasterror = GIMAGE_UNSUPPORTED_PIXELFORMAT; return lasterror;}

	if (profile) {
		char * iccprofile;
		cmsUInt32Number iccprofilesize;
		makeICCProfile(profile, iccprofile, iccprofilesize);

		try {
			//Pick one, getTransformedImageData() seems to produce less noise, but is slower:
			_writeTIFF(filename, getTransformedImageData(bits, profile, intent),  w, h, c, b, imginfo, iccprofile, iccprofilesize);
			//_writeTIFF(filename, getImageData(bits, profile),  w, h, c, b, imginfo, iccprofile, iccprofilesize);
		}
		catch (std::exception &e) {
			lasterror = GIMAGE_EXCEPTION;
			delete [] iccprofile;
			return lasterror;
		}

		delete [] iccprofile;
	}
	else {
		if (this->profile)
			_writeTIFF(filename, getImageData(bits),  w, h, c, b, imginfo, this->profile, profile_length);	
		else
			_writeTIFF(filename, getImageData(bits),  w, h, c, b, imginfo);	
	}
	lasterror = GIMAGE_OK; 
	return lasterror;
}

GIMAGE_ERROR gImage::savePNG(const char * filename, BPP bits, std::string params, cmsHPROFILE profile, cmsUInt32Number intent)
{

	unsigned b = 0;
	if (bits == BPP_16) b = 16;
	else if (bits == BPP_8)  b = 8;
	else {lasterror = GIMAGE_UNSUPPORTED_PIXELFORMAT; return lasterror;}

	if (profile) {
		char * iccprofile;
		cmsUInt32Number iccprofilesize;
		makeICCProfile(profile, iccprofile, iccprofilesize);

		try {
			//Pick one, getTransformedImageData() seems to produce less noise, but is slower:
			_writePNG(filename, getTransformedImageData(bits, profile, intent),  w, h, c, b, imginfo, params, iccprofile, iccprofilesize);
			//_writeTIFF(filename, getImageData(bits, profile),  w, h, c, b, imginfo, iccprofile, iccprofilesize);
		}
		catch (std::exception &e) {
			lasterror = GIMAGE_EXCEPTION;
			delete [] iccprofile;
			return lasterror;
		}

		delete [] iccprofile;
	}
	else {
		if (this->profile)
			_writePNG(filename, getImageData(bits),  w, h, c, b, imginfo, params, this->profile, profile_length);	
		else
			_writePNG(filename, getImageData(bits),  w, h, c, b, imginfo, params);	
	}
	lasterror = GIMAGE_OK; 
	return lasterror;
}


//ICC Profiles:
//
//Primaries and black/white points from Elle Stone's make-elles-profiles.c, 
//https://github.com/ellelstone/elles_icc_profiles, GPL V2
//
cmsCIExyY d50_romm_spec= {0.3457, 0.3585, 1.0};
cmsCIEXYZ d50_romm_spec_media_whitepoint = {0.964295676, 1.0, 0.825104603};
cmsCIExyY d65_srgb_adobe_specs = {0.3127, 0.3290, 1.0};
cmsCIEXYZ d65_media_whitepoint = {0.95045471, 1.0, 1.08905029};

cmsCIExyYTRIPLE aces_primaries_prequantized = 
{
{0.734704192222, 0.265298276252,  1.0},
{-0.000004945077, 0.999992850272,  1.0},
{0.000099889199, -0.077007518685,  1.0}
};

cmsCIExyYTRIPLE romm_primaries = {
{0.7347, 0.2653, 1.0},
{0.1596, 0.8404, 1.0},
{0.0366, 0.0001, 1.0}
};

cmsCIExyYTRIPLE widegamut_pascale_primaries = {
{0.7347, 0.2653, 1.0},
{0.1152, 0.8264, 1.0},
{0.1566, 0.0177, 1.0}
};

cmsCIExyYTRIPLE adobe_primaries_prequantized = {
{0.639996511, 0.329996864, 1.0},
{0.210005295, 0.710004866, 1.0},
{0.149997606, 0.060003644, 1.0}
};

cmsCIExyYTRIPLE srgb_primaries_pre_quantized = {
{0.639998686, 0.330010138, 1.0},
{0.300003784, 0.600003357, 1.0},
{0.150002046, 0.059997204, 1.0}
};

cmsCIExyYTRIPLE identity_primaries = {
{1.0, 0.0, 1.0},
{0.0, 1.0, 1.0},
{0.0, 0.0, 1.0}
};


const cmsCIExyY cmsCIEXYZ2cmsCIExyY(cmsCIEXYZ in)
{
	cmsCIExyY out;
	double s = in.X+in.Y+in.Z;
	out.x = in.X/s;
	out.y = in.Y/s;
	out.Y = in.Y;
	return out;
}

//use in place of cmsOpenProfileFromFile() to include .json files:
cmsHPROFILE gImage::myCmsOpenProfileFromFile(const std::string filename)
{
	if (!file_exists(filename)) return NULL;

	size_t pos = filename.find_last_of(".");
	if (pos != std::string::npos) {
		if (filename.substr(pos+1) == "json") {
			std::ifstream j(filename.c_str(), std::ifstream::in);
			std::stringstream json;
			json << j.rdbuf();
			return makeLCMSProfile(json.str());
		}			
		else {
			return cmsOpenProfileFromFile(filename.c_str(), "r");
		}
	}
	else return NULL;
}


//psuedoinverse, from dcraw.c:
void pseudoinverse (double (*in)[3], double (*out)[3], int size)
{
 double work[3][6], num;
  int i, j, k;

  for (i=0; i < 3; i++) {
    for (j=0; j < 6; j++)
      work[i][j] = j == i+3;
    for (j=0; j < 3; j++)
      for (k=0; k < size; k++)
        work[i][j] += in[k][i] * in[k][j];
  }
  for (i=0; i < 3; i++) {
    num = work[i][i];
    for (j=0; j < 6; j++)
      work[i][j] /= num;
    for (k=0; k < 3; k++) {
      if (k==i) continue;
      num = work[k][i];
      for (j=0; j < 6; j++)
        work[k][j] -= work[i][j] * num;
    }
  }
  for (i=0; i < size; i++)
    for (j=0; j < 3; j++)
      for (out[i][j]=k=0; k < 3; k++)
        out[i][j] += work[j][k+3] * in[i][k];
}

//make a profile from a camconst.json entry
cmsHPROFILE gImage::makeLCMSCamConstProfile(std::string camconstfile, std::string camera)
{
        FILE *f = NULL;
        long len = 0;
        char *data = NULL;
	int result;

        /* open in read binary mode */
        f = fopen(camconstfile.c_str(),"rb");
        /* get the length */
        fseek(f, 0, SEEK_END);
        len = ftell(f);
        fseek(f, 0, SEEK_SET);

        data = (char*)malloc(len + 1);

        result = fread(data, 1, len, f);
        data[len] = '\0';
        fclose(f);

	cJSON *prof = cJSON_Parse(data);
	if (prof == NULL) return NULL;

	//todo:  look up coeff based on camera profile
	//call makeLCMSAdobeCoeffProfile(coeff)
	return NULL;
}

void printrgb(double (*p)[3])
{
	int i, j;
	for (i=0; i<3; i++) {
		for (j=0; j<3; j++) {
			printf("%f ",p[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

// make a linear D65 profile from a dcraw adobe_coeff entry, 
// e.g., D7000: 8198,-2239,-724,-4871,12389,2798,-1043,2050,7181
// also takes a json array, e.g., [ 8198, -2239, -724, -4871, 12389, 2798, -1043, 2050, 7181 ]
cmsHPROFILE gImage::makeLCMSAdobeCoeffProfile(std::string adobecoeff)
{
	double in_XYZ[3][3], inverse[3][3], out_XYZ[3][3];

	//if json, remove the extraneous characters;
	adobecoeff.erase(std::remove(adobecoeff.begin(), adobecoeff.end(), ' '), adobecoeff.end());
	adobecoeff.erase(std::remove(adobecoeff.begin(), adobecoeff.end(), '['), adobecoeff.end());
	adobecoeff.erase(std::remove(adobecoeff.begin(), adobecoeff.end(), ']'), adobecoeff.end());
	
	std::vector<std::string> mat = split(adobecoeff, ",");
	for (unsigned i=0; i<3; i++) {
		for (unsigned j=0; j<3; j++) {
			unsigned pos = i*3+j;
			if (pos < mat.size())
				in_XYZ[i][j] = atof(mat[pos].c_str())/10000.0;
			else
				in_XYZ[i][j] = 0.0;
		}
	}

	pseudoinverse(in_XYZ, inverse, 3);
	//because pseudoinverse delivers it rotated:
	for (unsigned i=0; i<3; i++)
		for (unsigned j=0; j<3; j++)
			out_XYZ[i][j] = inverse[j][i];

	
	//printrgb(in_XYZ);
	//printf("\n");
	//printrgb(out_XYZ);
	
	cmsHPROFILE profile;
	cmsCIExyYTRIPLE c;
	cmsCIExyY cw;
	cmsCIEXYZ p, w;
	cmsToneCurve *curve[3], *tonecurve;

	cw = cmsCIEXYZ2cmsCIExyY(d65_media_whitepoint);

	p.X = out_XYZ[0][0]; p.Y = out_XYZ[1][0]; p.Z = out_XYZ[2][0]; 
	c.Red = cmsCIEXYZ2cmsCIExyY(p);
	p.X = out_XYZ[0][1]; p.Y = out_XYZ[1][1]; p.Z = out_XYZ[2][1]; 
	c.Green = cmsCIEXYZ2cmsCIExyY(p);
	p.X = out_XYZ[0][2]; p.Y = out_XYZ[1][2]; p.Z = out_XYZ[2][2]; 
	c.Blue = cmsCIEXYZ2cmsCIExyY(p);

	tonecurve = cmsBuildGamma (NULL, 1.0);  //hardcoded linear, for now...
	curve[0] = curve[1] = curve[2] = tonecurve;

	profile = cmsCreateRGBProfile (&cw, &c, curve);
	
	if (profile) {
		std::string descr = "adobe_coeff linear profile";
		cmsMLU *description;
		description = cmsMLUalloc(NULL, 1);
		cmsMLUsetASCII(description, "en", "US", descr.c_str());
		cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
		cmsMLUfree(description);
	}

	return profile;
}


//make a profile for a dcamprof json string:
cmsHPROFILE gImage::makeLCMSProfile(const std::string json)
{
	cmsHPROFILE profile;
	cmsCIExyYTRIPLE c;
	cmsCIExyY cw;
	cmsCIEXYZ p, w;
	cmsToneCurve *curve[3], *tonecurve;

	cJSON *pentry;
	cJSON *prof = cJSON_Parse(json.c_str());
	if (prof == NULL) return NULL;

	pentry = NULL;
	pentry = cJSON_GetObjectItemCaseSensitive(prof, "Whitepoint");
	if (pentry) {
		w.X = cJSON_GetArrayItem(pentry, 0)->valuedouble;
		w.Y = cJSON_GetArrayItem(pentry, 1)->valuedouble;
		w.Z = cJSON_GetArrayItem(pentry, 2)->valuedouble;
	}
	else return NULL;

	cw = cmsCIEXYZ2cmsCIExyY(w);

	pentry = NULL;
	pentry = cJSON_GetObjectItemCaseSensitive(prof, "ForwardMatrix");
	if (pentry) {
		cJSON *X = cJSON_GetArrayItem(pentry, 0); 
		cJSON *Y = cJSON_GetArrayItem(pentry, 1); 
		cJSON *Z = cJSON_GetArrayItem(pentry, 2);
		p.X = cJSON_GetArrayItem(X, 0)->valuedouble;
		p.Y = cJSON_GetArrayItem(Y, 0)->valuedouble;
		p.Z = cJSON_GetArrayItem(Z, 0)->valuedouble;
		c.Red = cmsCIEXYZ2cmsCIExyY(p);

		p.X = cJSON_GetArrayItem(X, 1)->valuedouble;
		p.Y = cJSON_GetArrayItem(Y, 1)->valuedouble;
		p.Z = cJSON_GetArrayItem(Z, 1)->valuedouble;
		c.Green = cmsCIEXYZ2cmsCIExyY(p);
		 
		p.X = cJSON_GetArrayItem(X, 2)->valuedouble;
		p.Y = cJSON_GetArrayItem(Y, 2)->valuedouble;
		p.Z = cJSON_GetArrayItem(Z, 2)->valuedouble;
		c.Blue = cmsCIEXYZ2cmsCIExyY(p);
	}
	else return NULL;

	pentry = NULL;
	pentry = cJSON_GetObjectItemCaseSensitive(prof, "RedTRC");
	if (pentry) 
		curve[0] = cmsBuildGamma (NULL, pentry->valuedouble);
	else return NULL;

	pentry = NULL;
	pentry = cJSON_GetObjectItemCaseSensitive(prof, "GreenTRC");
	if (pentry) 
		curve[1] = cmsBuildGamma (NULL, pentry->valuedouble);
	else return NULL;

	pentry = NULL;
	pentry = cJSON_GetObjectItemCaseSensitive(prof, "BlueTRC");
	if (pentry) 
		curve[2] = cmsBuildGamma (NULL, pentry->valuedouble);
	else return NULL;

	profile = cmsCreateRGBProfile (&cw, &c, curve);

	pentry = NULL;
	std::string descr;
	pentry = cJSON_GetObjectItemCaseSensitive(prof, "Description");
	if (pentry) 
		if (cJSON_IsString(pentry))
			descr = std::string(pentry->valuestring);
		else
			descr = "unreadable profile description";
	else 
		descr = "no profile description";
	cmsMLU *description;
	description = cmsMLUalloc(NULL, 1);
	cmsMLUsetASCII(description, "en", "US", descr.c_str());
	cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
	cmsMLUfree(description);
	
	return profile;
}


//make a profile for one of the dcraw colorspace names:
cmsHPROFILE gImage::makeLCMSProfile(const std::string name, float gamma)
{
	cmsHPROFILE profile;
	cmsCIExyYTRIPLE c;

	if (name == "srgb") c =  srgb_primaries_pre_quantized;
	else if (name == "wide") c =  widegamut_pascale_primaries;
	else if (name == "adobe") c =  adobe_primaries_prequantized;
	else if (name == "prophoto") c =  romm_primaries;
	else if (name == "identity") c =  identity_primaries;
	else return NULL;

	cmsToneCurve *curve[3], *tonecurve;
	tonecurve = cmsBuildGamma (NULL, gamma);
	curve[0] = curve[1] = curve[2] = tonecurve;

	profile =  cmsCreateRGBProfile ( &d65_srgb_adobe_specs, &c, curve);

	std::string descr = "rawproc D65-"+name+", Primaries and black/white points from Elle Stone's make-elles-profiles.c, https://github.com/ellelstone/elles_icc_profiles";
	cmsMLU *description;
	description = cmsMLUalloc(NULL, 1);
	cmsMLUsetASCII(description, "en", "US", descr.c_str());
	cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
	cmsMLUfree(description);

	std::string copyrt = "GPL V2";
	cmsMLU *copyright;
	copyright = cmsMLUalloc(NULL, 1);
	cmsMLUsetASCII(copyright, "en", "US", copyrt.c_str());
	cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
	cmsMLUfree(copyright);

	return profile;
	
}

struct icc_entry {
	cmsCIExyY white;
	cmsCIExyY black;
	cmsCIExyYTRIPLE primaries;
};



icc_entry dcraw_icc_table[5] = {
	//adobe:*/
	{
		{0.950455,1.000000,1.089050},
		{0.000000,0.000000,0.000000},
		{
			{0.609787,0.311142,0.019485},
			{0.205246,0.625656,0.060898},
			{0.149200,0.063217,0.744675}
		}
	},

	//prophoto:
	{
		{0.950455,1.000000,1.089050},
		{0.000000,0.000000,0.000000},
		{
			{0.797745,0.288071,0.000000},
			{0.135178,0.711868,0.000031},
			{0.031311,0.000061,0.825027}
		}
	},

	//srgb:
	{
		{0.950455,1.000000,1.089050},
		{0.000000,0.000000,0.000000},
		{
			{0.436081,0.222504,0.013931},
			{0.385086,0.716888,0.097092},
			{0.143051,0.060608,0.714020}
		}
	},

	//wide:
	{
		{0.950455,1.000000,1.089050},
		{0.000000,0.000000,0.000000},
		{
			{0.716156,0.258209,0.000000},
			{0.100906,0.724930,0.051788},
			{0.147156,0.016861,0.773254}
		}
	},

	//xyz:
	{
		{0.950455,1.000000,1.089050},
		{0.000000,0.000000,0.000000},
		{
			{1.047836,0.029556,-0.009216},
			{0.022903,0.990479,0.015045},
			{-0.050125,-0.017044,0.752029}
		}
	}

};

cmsHPROFILE gImage::makeLCMSdcrawProfile(const std::string name, float gamma)
{
	cmsHPROFILE profile;
	cmsCIExyYTRIPLE c;

	int profileid;

	if (name == "adobe")         profileid = 0;
	else if (name == "prophoto") profileid = 1;
	else if (name == "srgb")     profileid = 2;
	else if (name == "wide")     profileid = 3;
	else if (name == "xyz")      profileid = 4;
	else return NULL;

	c = dcraw_icc_table[profileid].primaries;

	cmsToneCurve *curve[3], *tonecurve;
	tonecurve = cmsBuildGamma (NULL, gamma);
	curve[0] = curve[1] = curve[2] = tonecurve;

	profile =  cmsCreateRGBProfile ( &d65_srgb_adobe_specs, &c, curve);

	std::string descr = "dcraw D65-"+name+", gamma "+tostr((double) gamma)+", extracted for use in rawproc";
	cmsMLU *description;
	description = cmsMLUalloc(NULL, 1);
	cmsMLUsetASCII(description, "en", "US", descr.c_str());
	cmsWriteTag(profile, cmsSigProfileDescriptionTag, description);
	cmsMLUfree(description);

	std::string copyrt = "GPL V2";
	cmsMLU *copyright;
	copyright = cmsMLUalloc(NULL, 1);
	cmsMLUsetASCII(copyright, "en", "US", copyrt.c_str());
	cmsWriteTag(profile, cmsSigCopyrightTag, copyright);
	cmsMLUfree(copyright);

	return profile;
}



void gImage::makeICCProfile(cmsHPROFILE hProfile, char *& profile, cmsUInt32Number  &profilesize)
{
	cmsSaveProfileToMem(hProfile, NULL, &profilesize);
	profile = new char[profilesize];
	cmsSaveProfileToMem(hProfile, profile, &profilesize);
}




