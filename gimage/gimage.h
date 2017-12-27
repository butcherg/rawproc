#ifndef _gimage_h
#define _gimage_h

#include <string>
#include <vector>
#include <map>
#include <lcms2.h>
#include "curve.h"
#include "half.hpp"

#define VERSION "0.2"

#ifndef PIXTYPE
#define PIXTYPE float
#endif

extern "C" {
	const char * gImageVersion();
}

using half_float::half;

struct pix {
	PIXTYPE r, g, b;
};

struct histogramdata {
	long r, g, b;
};

struct coord {
	unsigned x, y;
};

enum BPP {
	BPP_FP,
	BPP_8,
	BPP_16
};

enum GIMAGE_CHANNEL {
	CHANNEL_RGB,
	CHANNEL_RED,
	CHANNEL_GREEN,
	CHANNEL_BLUE
};

enum GIMAGE_FILETYPE {
	FILETYPE_RAW,
	FILETYPE_JPEG,
	FILETYPE_TIFF,
	FILETYPE_PNG,
	FILETYPE_UNKNOWN
};

enum RESIZE_FILTER {
	FILTER_BOX,
	FILTER_BILINEAR,
	FILTER_BSPLINE,
	FILTER_BICUBIC,
	FILTER_CATMULLROM,
	FILTER_LANCZOS3
};

enum GIMAGE_ERROR {
	GIMAGE_OK,
	GIMAGE_EXCEPTION,
	GIMAGE_UNSUPPORTED_PIXELFORMAT,
	GIMAGE_UNSUPPORTED_FILEFORMAT,
	
	GIMAGE_APPLYCOLORSPACE_BADPROFILE,
	GIMAGE_APPLYCOLORSPACE_BADINTENT,
	GIMAGE_APPLYCOLORSPACE_BADTRANSFORM,
	
	GIMAGE_ASSIGNCOLORSPACE_BADTRANSFORM
};


class gImage 
{
	public:
		gImage();
		gImage(const gImage &o);
		gImage(std::string filename);
		gImage(char *imagedata, unsigned width, unsigned height, unsigned colors, BPP bits, std::map<std::string,std::string> imageinfo, char * icc_profile=NULL, unsigned icc_profile_length=0);
		gImage(unsigned width, unsigned height,  unsigned colors, std::map<std::string,std::string> imageinfo);

		~gImage();

		//Getters
		pix getPixel(unsigned x,  unsigned y);
		std::vector<float> getPixelArray(unsigned x,  unsigned y);
		char *getImageData(BPP bits, cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		float * getImageDataFloat(bool unbounded, cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		char *getTransformedImageData(BPP bits, cmsHPROFILE profile, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		std::vector<pix>& getImageData();
		pix* getImageDataRaw();
		unsigned getWidth();
		unsigned getHeight();
		unsigned getColors();
		BPP getBits();
		std::string getBitsStr();
		std::map<std::string,std::string> getInfo();
		std::string getInfoValue(std::string name);
		char * getProfile();
		unsigned getProfileLength();
		static std::string getProfilePath();
		static void setProfilePath(std::string ppath);
		GIMAGE_ERROR getLastError();
		std::string getLastErrorMessage();
		std::string Stats();
		std::vector<long> Histogram();
		std::vector<histogramdata> Histogram(unsigned scale);
		//std::map<GIMAGE_CHANNEL,std::vector<unsigned> > Histogram(unsigned channels, unsigned scale);
		std::vector<long> Histogram(unsigned channel, unsigned &hmax);

		//Setters
		void setInfo(std::string name, std::string value);
		void setProfile(char * prof, unsigned proflength);
		void deleteProfile();

		//Static methods
		static std::string getRGBCharacteristics();
		static std::map<std::string,std::string> getInfo(const char * filename);
		static GIMAGE_FILETYPE getFileType(const char * filename);
		static GIMAGE_FILETYPE getFileNameType(const char * filename);
		static int ThreadCount();
		static std::string Version();
		static std::string LibraryVersions();

		//Image operations.  
		//threadcount=0 uses all available CPUs, n uses precisely n CPUs, and -n uses available-n CPUs
		void ApplyConvolutionKernel(double kernel[3][3], int threadcount=0);
		void ApplySharpen(int strength, int threadcount=0);
		void ApplyResize(unsigned width, unsigned height, RESIZE_FILTER filter, int threadcount=0);
		void ApplyRotate(double angle, bool crop, int threadcount=0);
		void ApplyRotate180(int threadcount=0);
		void ApplyRotate90(int threadcount=0);
		void ApplyRotate270(int threadcount=0);
		void ApplyHorizontalMirror(int threadcount=0);
		void ApplyVerticalMirror(int threadcount=0);
		void ApplyCrop(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int threadcount=0);
		void ApplySaturate(double saturate, int threadcount=0);
		void ApplyExposureCompensation(double ev, int threadcount=0);
		void ApplyTint(double red,double green,double blue, int threadcount=0);
		void ApplyGray(double redpct, double greenpct, double bluepct, int threadcount=0);
		void ApplyToneCurve(std::vector<cp> ctpts, int threadcount=0);
		void ApplyToneCurve(std::vector<cp> ctpts, GIMAGE_CHANNEL channel, int threadcount=0);
		void ApplyToneLine(double low, double high, int threadcount=0);
		void ApplyWhiteBalance(double redmult, double greenmult, double bluemult, int threadcount);
		void ApplyNLMeans(double sigma, int local, int patch, int threadcount=0);
		void ApplyRedeye(std::vector<coord> points, double threshold, unsigned limit, bool desaturate=false, double desaturatepercent=1.0, int threadcount=0);
		GIMAGE_ERROR ApplyColorspace(std::string iccfile, cmsUInt32Number intent, bool blackpointcomp=false, int threadcount=0);
		bool AssignColorspace(std::string iccfile);
		

		//Image loaders.  Return a new gImage
		static gImage loadRAW(const char * filename, std::string params);
		static gImage loadJPEG(const char * filename, std::string params);
		static gImage loadTIFF(const char * filename, std::string params);
		static gImage loadPNG(const char * filename, std::string params);
		static gImage loadImageFile(const char * filename, std::string params);
		static std::map<std::string,std::string> loadImageFileInfo(const char * filename);

		//Image savers. 
		GIMAGE_ERROR saveImageFile(const char * filename, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		GIMAGE_ERROR saveJPEG(const char * filename, BPP bits, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		GIMAGE_ERROR saveTIFF(const char * filename, BPP bits, cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		GIMAGE_ERROR savePNG(const char * filename, BPP bits, cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);

		//ICC (LittleCMS) profiles.
		static cmsHPROFILE makeLCMSProfile(const std::string name, float gamma);
		static cmsHPROFILE makeLCMSdcrawProfile(const std::string name, float gamma);
		static void makeICCProfile(cmsHPROFILE hProfile, char *& profile, cmsUInt32Number  &profilesize);

	protected:

		//void ImageBounds(unsigned *x1, unsigned *x2, unsigned *y1, unsigned *y2, bool cropbounds=false);
		//void ApplyXShear(double rangle, int threadcount);
		//void ApplyYShear(double rangle, int threadcount);
		
		int doRedRing(unsigned px, unsigned py, unsigned offset, double threshold);


	private:
		std::vector<pix> image;
		unsigned w, h, c;
		BPP b;
		std::map<std::string,std::string> imginfo;

		char *profile;
		unsigned profile_length;
		
		static std::string profilepath;
		
		GIMAGE_ERROR lasterror;
		

};



#endif
