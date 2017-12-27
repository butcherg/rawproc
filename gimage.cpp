
#include "gimage/gimage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <exception>
#include <algorithm> 
#include <sstream>
#include <omp.h>
#include <exception>

#include "rawimage.h"
#include "jpegimage.h"
#include "tiffimage.h"
#include "pngimage.h"
#include "gimage/strutil.h"

#define PI            3.14159265358979323846

//Range 0.0-255.0 constants
//#define SCALE_16BIT 256.0
//#define SCALE_8BIT 1.0
//#define SCALE_CURVE 1.0

//Range 0.0-1.0 constants
#define SCALE_16BIT 65536.0
#define SCALE_8BIT 256.0
#define SCALE_CURVE 256.0


const char * gImageVersion()
{
	return VERSION;
}

std::string gImage::profilepath = "";


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

	if (o.profile) {
		profile = new char[o.profile_length];
		memcpy(profile, o.profile, o.profile_length);
		profile_length = o.profile_length;
	}
	else profile = NULL;
}

gImage::gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, BPP bits, std::map<std::string,std::string> imageinfo, char * icc_profile, unsigned icc_profile_length)
{
	image.resize(width*height);
	w=width;
	h=height;
	c=colors;
	b=bits;
	lasterror = GIMAGE_OK;

	if (bits ==BPP_16) {
		unsigned short * src = (unsigned short *) imagedata;
		if (colors == 1) {  //turn into a three-color grayscale
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (unsigned short) src[0]/SCALE_16BIT;
					image[pos].g = (unsigned short) src[0]/SCALE_16BIT;
					image[pos].b = (unsigned short) src[0]/SCALE_16BIT;
					src += 1;
				}
			}
			c = 3;
		}
		else if (colors == 3) {
			for (unsigned y=0; y<h; y++) {
				for (unsigned x=0; x<w; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (unsigned short) src[0]/SCALE_16BIT;
					image[pos].g = (unsigned short) src[1]/SCALE_16BIT;
					image[pos].b = (unsigned short) src[2]/SCALE_16BIT;
					src += 3;
				}
			}
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
					image[pos].r = (unsigned char) src[0]/SCALE_8BIT;
					image[pos].g = (unsigned char) src[0]/SCALE_8BIT;
					image[pos].b = (unsigned char) src[0]/SCALE_8BIT;
					src += 1;
				}
			}
			c = 3;
		}
		else if (colors == 3) {
			for (unsigned y=0; y<height; y++) {
				for (unsigned x=0; x<width; x++) {
					unsigned pos = x + y*w;
					image[pos].r = (unsigned char) src[0]/SCALE_8BIT;
					image[pos].g = (unsigned char) src[1]/SCALE_8BIT;
					image[pos].b = (unsigned char) src[2]/SCALE_8BIT;
					src += 3;
				}
			}
		}
		else {
			w = 0;
			h = 0;
			return;
		}
	}

	else if (bits == BPP_FP) {
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
	b=BPP_FP;
	lasterror = GIMAGE_OK;

	for (unsigned y=0; y<height; y++) {
		for (unsigned x=0; x<width; x++) {
			unsigned pos = x + y*w;
			image[pos].r = 0.0;
			image[pos].g = 0.0;
			image[pos].b = 0.0;
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


pix gImage::getPixel(unsigned x,  unsigned y)
{
	pix nullpix = {(PIXTYPE) 0.0, (PIXTYPE) 0.0, (PIXTYPE) 0.0};
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
struct cpix { char r, g, b; };
struct uspix { unsigned short r, g, b; };
struct fpix { float r, g, b; };

//Lets LittleCMS do both the profile transform and data type conversion:
char * gImage::getTransformedImageData(BPP bits, cmsHPROFILE profile, cmsUInt32Number intent)
{
	cmsHPROFILE hImgProfile;
	cmsUInt32Number informat, outformat;
	cmsHTRANSFORM hTransform;
	char * imagedata;
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
		else if (bits == BPP_FP) {
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

float * gImage::getImageDataFloat(bool unbounded, cmsHPROFILE profile, cmsUInt32Number intent)
{
	cmsHPROFILE hImgProfile;
	cmsUInt32Number format;
	cmsHTRANSFORM hTransform;
	//float * imagedata;
	unsigned imagesize = w*h;
	fpix * imagedata = new fpix[imagesize];
	pix * img = image.data();

	if (unbounded)
		#pragma omp parallel for
		for (unsigned i=0; i<imagesize; i++) {
			imagedata[i].r = (float) img[i].r;
			imagedata[i].g = (float) img[i].g;
			imagedata[i].b = (float) img[i].b;
		}
	else
		#pragma omp parallel for
		for (unsigned i=0; i<imagesize; i++) {
			imagedata[i].r = fmin(fmax(img[i].r,0.0),1.0); 
			imagedata[i].g = fmin(fmax(img[i].g,0.0),1.0); 
			imagedata[i].b = fmin(fmax(img[i].b,0.0),1.0); 
		}

	if (profile) {
		hImgProfile = cmsOpenProfileFromMem(getProfile(), getProfileLength());
		if (hImgProfile != NULL & profile != NULL) {
			hTransform = cmsCreateTransform(hImgProfile, TYPE_RGB_FLT, profile, TYPE_RGB_FLT, intent, 0);
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
	else if (bits == BPP_FP)
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
				dst[pos].r = (unsigned short) lrint(fmin(fmax(image[pos].r*SCALE_16BIT,0.0),65535.0)); 
				dst[pos].g = (unsigned short) lrint(fmin(fmax(image[pos].g*SCALE_16BIT,0.0),65535.0));
				dst[pos].b = (unsigned short) lrint(fmin(fmax(image[pos].b*SCALE_16BIT,0.0),65535.0)); 
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
				dst[pos].r = (unsigned char) lrint(fmin(fmax(image[pos].r*SCALE_8BIT,0.0),255.0)); 
				dst[pos].g = (unsigned char) lrint(fmin(fmax(image[pos].g*SCALE_8BIT,0.0),255.0));
				dst[pos].b = (unsigned char) lrint(fmin(fmax(image[pos].b*SCALE_8BIT,0.0),255.0)); 
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
				dst[pos].r = fmin(fmax(image[pos].r,0.0),1.0); 
				dst[pos].g = fmin(fmax(image[pos].g,0.0),1.0); 
				dst[pos].b = fmin(fmax(image[pos].b,0.0),1.0); 
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
		case BPP_FP: return "internal floating point";
		case BPP_8: return "8";
		case BPP_16: return "16";
	}
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
	return imgdata;
}

//Check the file type of an existing image file; is it suitable for opening?
GIMAGE_FILETYPE gImage::getFileType(const char * filename)
{
	std::string fname = filename;
	std::vector<std::string> fpieces =  split(fname, ".");
	std::string ext = fpieces.back();

	if (ext.compare("tif") == 0 | ext.compare("tiff") == 0) if (_checkTIFF(filename)) return FILETYPE_TIFF;
	if ((ext.compare("jpg") == 0) | (ext.compare("JPG") == 0)) if (_checkJPEG(filename)) return FILETYPE_JPEG;
	if ((ext.compare("png") == 0) | (ext.compare("PNG") == 0)) if (_checkPNG(filename)) return FILETYPE_PNG;
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
	return FILETYPE_UNKNOWN;
}

std::string gImage::Version()
{
	return VERSION;
}

std::string gImage::LibraryVersions()
{
	std::string verstring;
	verstring.append("JPEG: ");
	verstring.append(jpegVersion());
	verstring.append("\nTIFF: ");
	std::string tiffver(tiffVersion());
	verstring.append(tiffver.substr(0,tiffver.find_first_of("\n")));
	verstring.append("\nLibRaw: ");
	verstring.append(librawVersion());
	verstring.append("\nLittleCMS2: ");
	std::ostringstream s;
	s << (int) cmsGetEncodedCMMversion();
	verstring.append(s.str());
	//verstring.append("\n");
	return verstring;

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

void gImage::ApplySharpen(int strength, int threadcount)
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
	kernel[1][1] = strength+1;

	ApplyConvolutionKernel(kernel, threadcount);
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
			unsigned dpos = (dw-y-1) + (dh-x-1)*dw;
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
}

void gImage::ApplyToneLine(double low, double high, int threadcount)
{
	double slope = 255.0 / (high-low);

	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r = image[pos].r * slope;
			image[pos].g = image[pos].g * slope;
			image[pos].b = image[pos].b * slope;
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

void gImage::ApplyWhiteBalance(double redmult, double greenmult, double bluemult, int threadcount)
{
	double rslope = 255.0 / (255.0*redmult);
	double gslope = 255.0 / (255.0*greenmult);
	double bslope = 255.0 / (255.0*bluemult);
	
	
	#pragma omp parallel for num_threads(threadcount)
	for (unsigned x=0; x<w; x++) {
		for (unsigned y=0; y<h; y++) {
			unsigned pos = x + y*w;
			image[pos].r = image[pos].r * rslope;
			image[pos].g = image[pos].g * gslope;
			image[pos].b = image[pos].b * bslope;
		}
	}
}



//Saturation
//
//Credit: Darel Rex Finley, http://alienryderflex.com/saturation.html, public domain
//
//This is a simple HSL saturation routine.  There are better ways to 
//manipulate color, but this one is self-contained to a RGB color space.
//Maybe later...
//

#define  Pr  .299
#define  Pg  .587
#define  Pb  .114

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
		delete contrib[i].p;
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
		delete contrib[i].p;
	}
	delete[] contrib;
	delete t;
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

inline unsigned sqr(const unsigned x) { return x*x; }

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
}

GIMAGE_ERROR gImage::ApplyColorspace(std::string iccfile, cmsUInt32Number intent, bool blackpointcomp, int threadcount)
{
	cmsUInt32Number format;
	cmsHTRANSFORM hTransform;
	cmsUInt32Number dwFlags = 0;

	if (profile == NULL) {lasterror = GIMAGE_APPLYCOLORSPACE_BADPROFILE; return lasterror;}
	
	if (blackpointcomp) dwFlags = cmsFLAGS_BLACKPOINTCOMPENSATION;

	if (sizeof(PIXTYPE) == 2) format = TYPE_RGB_HALF_FLT; 
	if (sizeof(PIXTYPE) == 4) format = TYPE_RGB_FLT;
	if (sizeof(PIXTYPE) == 8) format = TYPE_RGB_DBL;

	cmsHPROFILE gImgProf = cmsOpenProfileFromMem(profile, profile_length);
	cmsHPROFILE hImgProf = cmsOpenProfileFromFile(iccfile.c_str(), "r");

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

bool gImage::AssignColorspace(std::string iccfile)
{
	cmsHPROFILE hImgProf = cmsOpenProfileFromFile(iccfile.c_str(), "r");
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

std::string gImage::Stats()
{
	double rmin, rmax, gmin, gmax, bmin, bmax, tmin, tmax;
	rmin = image[0].r; rmax = image[0].r; gmin=image[0].g; gmax = image[0].g; bmin = image[0].b; bmax = image[0].b;
	tmin = SCALE_CURVE; tmax = 0.0;
	int iter = 0;

	for(unsigned y = 1; y < h; y++) {
		for(unsigned x = 1; x < w; x++) {
			unsigned pos = x + y*w;
			if (image[pos].r > rmax) rmax = image[pos].r;
			if (image[pos].g > gmax) gmax = image[pos].g;
			if (image[pos].b > bmax) bmax = image[pos].b;
			if (image[pos].r < rmin) rmin = image[pos].r;
			if (image[pos].g < gmin) gmin = image[pos].g;
			if (image[pos].b < bmin) bmin = image[pos].b;
			double tone = (image[pos].r + image[pos].g + image[pos].b) / 3.0; 
			if (tone > tmax) tmax = tone; if (tone < tmin) tmin = tone;
			iter++;
		}
	}
	return string_format("rmin: %f\trmax: %f\ngmin: %f\tgmax: %f\nbmin: %f\tbmax: %f\n\ntonemin: %f\ttonemax: %f\n", rmin, rmax, gmin, gmax, bmin, bmax, tmin, tmax);
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

//rgb histogram, scale=number of buckets, 256 or 65536...
std::vector<histogramdata> gImage::Histogram(unsigned scale)
{
	histogramdata zerodat = {0,0,0};
	std::vector<histogramdata> histogram(scale, zerodat);
	
	#pragma omp parallel
	{
		std::vector<unsigned> pr(scale,0);
		std::vector<unsigned> pg(scale,0);
		std::vector<unsigned> pb(scale,0);
		
		#pragma omp for
		for(unsigned y = 0; y < h; y++) {
			for(unsigned x = 0; x < w; x++) {
				unsigned pos = x + y*w;
				pr[(unsigned) lrint(fmin(fmax(image[pos].r*scale,0.0),scale-1))]++;
				pg[(unsigned) lrint(fmin(fmax(image[pos].g*scale,0.0),scale-1))]++;
				pb[(unsigned) lrint(fmin(fmax(image[pos].b*scale,0.0),scale-1))]++;
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
	return histogram;
}


//single-channel histogram, OBE...
std::vector<long> gImage::Histogram(unsigned channel, unsigned &hmax)
{
	unsigned scale;
	if (b == BPP_16) scale = 65536;
	else scale = 256;
	hmax = 0;
	
	std::vector<long> hdata(scale,0);

	//#pragma omp parallel for
	for(unsigned y = 0; y < h; y++) {
		for(unsigned x = 0; x < w; x++) {
			unsigned pos = x + y*w;
			unsigned d; 
			if (channel == CHANNEL_RED)   d = (unsigned) fmin(fmax(image[pos].r*scale,0.0),scale-1);
			if (channel == CHANNEL_GREEN) d = (unsigned) fmin(fmax(image[pos].g*scale,0.0),scale-1);
			if (channel == CHANNEL_BLUE)  d = (unsigned) fmin(fmax(image[pos].b*scale,0.0),scale-1);
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
	std::string params = "";
	GIMAGE_FILETYPE ext = gImage::getFileType(filename);

	if (ext == FILETYPE_TIFF) {
		_loadTIFFInfo(filename, &width, &height, &colors, &bpp, imgdata); 
	}
	else if (ext == FILETYPE_JPEG) {
		_loadJPEGInfo(filename, &width, &height, &colors, imgdata); 
	}
	else {
		_loadRAWInfo(filename, &width, &height, &colors, &bpp, imgdata); 
	}
	return imgdata;
}

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
	gImage I(image, width, height, colors, BPP_8, imgdata, iccprofile, icclength);
	delete [] image;
	if (icclength && iccprofile != NULL) delete [] iccprofile;
	return I;
}


gImage gImage::loadTIFF(const char * filename, std::string params)
{
	unsigned width, height, colors, bpp, icclength;
	BPP bits;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	char * image = _loadTIFF(filename, &width, &height, &colors, &bpp, imgdata, params, &iccprofile, &icclength);
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

gImage gImage::loadPNG(const char * filename, std::string params)
{
	unsigned width, height, colors, bpp, icclength;
	BPP bits;
	char * iccprofile;
	std::map<std::string,std::string> imgdata;
	char * image = _loadPNG(filename, &width, &height, &colors, &bpp, imgdata, params, &iccprofile, &icclength);
	switch (bpp) {
		case 8: 
			bits = BPP_8;
			break;
		case 16:
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


//Savers:


GIMAGE_ERROR gImage::saveImageFile(const char * filename, std::string params, cmsHPROFILE profile, cmsUInt32Number intent)
{
	BPP bitfmt = BPP_8;
	std::map<std::string, std::string> p = parseparams(params);
	if (p.find("channelformat") != p.end()) {
		if (p["channelformat"] == "8bit")  bitfmt = BPP_8;
		if (p["channelformat"] == "16bit") bitfmt = BPP_16;
		if (p["channelformat"] == "float") bitfmt = BPP_FP;
	}

	GIMAGE_FILETYPE ftype = gImage::getFileNameType(filename);

	if (ftype == FILETYPE_TIFF) {
		if (profile)
			return saveTIFF(filename, bitfmt, profile, intent);
		else
			return saveTIFF(filename, bitfmt);
	}
	if (ftype == FILETYPE_JPEG) {
		if (profile)
			return saveJPEG(filename, bitfmt, params, profile, intent);
		else
			return saveJPEG(filename, bitfmt, params);
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
			delete iccprofile;
			return lasterror;
		}

		delete iccprofile;
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

GIMAGE_ERROR gImage::saveTIFF(const char * filename, BPP bits, cmsHPROFILE profile, cmsUInt32Number intent)
{
	unsigned b = 0;
	if (bits == BPP_16) b = 16;
	else if (bits == BPP_8)  b = 8;
	else if (bits == BPP_FP) b = 32;
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
			delete iccprofile;
			return lasterror;
		}

		delete iccprofile;
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

GIMAGE_ERROR gImage::savePNG(const char * filename, BPP bits, cmsHPROFILE profile, cmsUInt32Number intent)
{
/*
	unsigned b = 0;
	if (bits == BPP_16) b = 16;
	else if (bits == BPP_8)  b = 8;
	else if (bits == BPP_FP) b = 32;
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
			delete iccprofile;
			return lasterror;
		}

		delete iccprofile;
	}
	else {
		if (this->profile)
			_writeTIFF(filename, getImageData(bits),  w, h, c, b, imginfo, this->profile, profile_length);	
		else
			_writeTIFF(filename, getImageData(bits),  w, h, c, b, imginfo);	
	}
	lasterror = GIMAGE_OK; 
	return lasterror;
*/
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




