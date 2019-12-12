#ifndef _gimage_h
#define _gimage_h

#include <string>
#include <vector>
#include <map>
#include <lcms2.h>
#include "curve.h"

#if defined PIXhalf
#include "gimage/half.hpp"
#endif

#ifndef PIXTYPE
#define PIXTYPE float
#define PIXFLOAT
#endif

extern "C" {
	const char * gImageVersion();
}

#if defined PIXhalf
using half_float::half;
#endif


struct pix {
	PIXTYPE r, g, b;
};

struct histogramdata {
	long r, g, b;
};

struct histogram {
	long rmax, gmax, bmax;
	unsigned numbins;
	std::vector<histogramdata> rhist, ghist, bhist;
};

struct coord {
	int x, y;
};

enum BPP {
	BPP_FP,
	BPP_UFP,
	BPP_8,
	BPP_16
};

enum GIMAGE_CHANNEL {
	CHANNEL_RGB,
	CHANNEL_RED,
	CHANNEL_GREEN,
	CHANNEL_BLUE,
	CHANNEL_TONE
};

enum GIMAGE_FILETYPE {
	FILETYPE_RAW,
	FILETYPE_JPEG,
	FILETYPE_TIFF,
	FILETYPE_PNG,
	FILETYPE_UNKNOWN,
	FILETYPE_BAD_FILENAME
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
	GIMAGE_APPLYCOLORSPACE_BADINPUTPROFILE,
	GIMAGE_APPLYCOLORSPACE_BADOUTPUTPROFILE,
	GIMAGE_APPLYCOLORSPACE_BADINTENT,
	GIMAGE_APPLYCOLORSPACE_BADTRANSFORM,
	
	GIMAGE_ASSIGNCOLORSPACE_BADTRANSFORM
};

enum LIBRTPROCESS_PREPOST { 
	LIBRTPROCESS_DEMOSAIC=0,
	LIBRTPROCESS_CACORRECT=2,
	LIBRTPROCESS_HLRECOVERY=4
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

		//pixel getters
		pix getPixel(unsigned x,  unsigned y);
		std::vector<float> getPixelArray(unsigned x,  unsigned y);

		//image array getters

		//Returns a pointer to a new-allocated char * array of the image, copied from the internal image and, if a profile is specified, 
		//transformed to it.  Image values are in the format specified in the bits parameter, float, unsigned short, or char, so the 
		//caller must typecast appropriately to use the data.  Caller must delete [] the array when done with it:
		char *getImageData(BPP bits, cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);

		//Returns a pointer to a new-allocated float * array of the image, copied from the internal image and, if a profile is specified, 
		//transformed to it.  Caller must delete [] the array when done with it:
		float * getImageDataFloat(bool unbounded, cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);

		//Both of these return a pointer to a char * array of the image, copied from the internal image and transformed 
		//to the specified profile.  Image values are in the format specified in the bits parameter, float, unsigned short, or char, 
		//so the caller must typecast appropriately to use the data.  Both copy and transform are done in the LittleCMS cmsTransform.  
		//The first routine new-allocates the array, the second one malloc-allocates it.  Caller must delete [] or free() the array 
		//when done with it:
		char *getTransformedImageData(BPP bits, cmsHPROFILE profile, cmsUInt32Number intent);
		char *getTransformedImageData(BPP bits, cmsHTRANSFORM transform);

		//These routines return a reference or a pointer, respectively, to the internal image array:
		std::vector<pix>& getImageData();
		pix* getImageDataRaw();


		//metadata getters
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
		std::string Stats(bool isfloat=true);
		std::map<std::string,std::string> StatsMap();

		//Setters
		void setInfo(std::string name, std::string value);
		void setProfile(char * prof, unsigned proflength);
		void deleteProfile();

		//calculators:
		std::vector<double> CalculateChannelMeans();
		std::vector<double> CalculatePatchMeans(int x, int y, float radius);
		std::vector<double> CalculateBlackWhitePoint(double blackthreshold=0.01, double whitethreshold=0.01, bool centerout=true, int whitemax=255, std::string channel="rgb");

		//histograms:
		std::vector<long> Histogram();
		std::vector<long> RedHistogram();
		std::vector<long> GreenHistogram();
		std::vector<long> BlueHistogram();
		std::vector<histogramdata> Histogram(unsigned scale);
		std::vector<histogramdata> Histogram(unsigned scale, int &zerobucket, int &onebucket, float &dmin, float &dmax);
		//std::map<GIMAGE_CHANNEL,std::vector<unsigned> > Histogram(unsigned channels, unsigned scale);
		std::vector<long> Histogram(unsigned channel, unsigned &hmax);

		
		//Lensfun support methods
		void initInterpolation(RESIZE_FILTER method);  //only accepts FILTER_BILINEAR and FILTER_LANCZOS3
		PIXTYPE getR(float x, float y);
		PIXTYPE getG(float x, float y);
		PIXTYPE getB(float x, float y);
		pix getRGB(float x, float y);

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
		//image operations with a std::string params parameter parse a string to apply the operation

		//kernel-based algorithms:
		void ApplyConvolutionKernel(double kernel[3][3], int threadcount=0);
		void Apply1DConvolutionKernel(std::vector<float> kernel, int threadcount=0);
		void Apply2DConvolutionKernel(std::vector<float> kernel, int kerneldimension, int threadcount=0);
		void ApplyGaussianBlur(double sigma, unsigned kernelsize, int threadcount=0);
		void ApplySharpen(double strength, int threadcount=0);


		//basic color/tone operations:
		void ApplySaturate(double saturate, int threadcount=0);
		void ApplyExposureCompensation(double ev, int threadcount=0);
		float ApplyExposureCompensation(int x, int y, float radius, float destinationev, int threadcount);
		void ApplyToneCurve(std::vector<cp> ctpts, int threadcount=0);
		void ApplyToneCurve(std::vector<cp> ctpts, GIMAGE_CHANNEL channel, int threadcount=0);
		void ApplyToneLine(double low, double high, GIMAGE_CHANNEL channel, int threadcount=0);
		void ApplySubtract(double subtract, bool clampblack=true, int threadcount=0);
		bool ApplySubtract(std::string filename, bool clampblack=true, int threadcount=0);


		//image geometry algorithms:
		void ApplyResize(unsigned width, unsigned height, RESIZE_FILTER filter, int threadcount=0);
		void ApplyResize(std::string params, int threadcount=0);
		void ApplyRatioCrop(float x1, float y1, float x2, float y2, int threadcount);
		void ApplyCrop(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int threadcount=0);

		//rotation algorithms:
		void ApplyRotate(double angle, bool crop, int threadcount=0);
		void ApplyRotate180(int threadcount=0);
		void ApplyRotate90(int threadcount=0);
		void ApplyRotate270(int threadcount=0);
		void ApplyHorizontalMirror(int threadcount=0);
		void ApplyVerticalMirror(int threadcount=0);
		void NormalizeRotation(int threadcount=0);

		//white balance algorithms:
		std::vector<double> ApplyCameraWhiteBalance(double redmult, double greenmult, double bluemult, int threadcount=0);
		std::vector<double> ApplyPatchWhiteBalance(float patchx, float patchy, double patchradius, int threadcount=0);
		std::vector<double> ApplyWhiteBalance(double redmult, double greenmult, double bluemult, int threadcount=0);
		std::vector<double> ApplyWhiteBalance(int threadcount=0);

		//demosaic algorithms:
		bool ApplyDemosaicHalf(bool resize=false, int threadcount=0);
		bool ApplyMosaicColor(int threadcount=0);
#ifdef USE_LIBRTPROCESS
		bool ApplyDemosaicVNG(LIBRTPROCESS_PREPOST prepost=LIBRTPROCESS_DEMOSAIC, int threadcount=0);
		bool ApplyDemosaicRCD(LIBRTPROCESS_PREPOST prepost=LIBRTPROCESS_DEMOSAIC, int threadcount=0);
		bool ApplyDemosaicDCB(LIBRTPROCESS_PREPOST prepost=LIBRTPROCESS_DEMOSAIC, int iterations=1, bool dcb_enhance=false, int threadcount=0);
		bool ApplyDemosaicAMAZE(LIBRTPROCESS_PREPOST prepost=LIBRTPROCESS_DEMOSAIC, double initGain=1.0, int border=0, float inputScale=1.0, float outputScale=1.0, int threadcount=0);
		bool ApplyDemosaicIGV(LIBRTPROCESS_PREPOST prepost=LIBRTPROCESS_DEMOSAIC, int threadcount=0);
		bool ApplyDemosaicAHD(LIBRTPROCESS_PREPOST prepost=LIBRTPROCESS_DEMOSAIC, int threadcount=0);
		bool ApplyDemosaicLMMSE(LIBRTPROCESS_PREPOST prepost=LIBRTPROCESS_DEMOSAIC, int iterations=1, int threadcount=0);
		bool ApplyDemosaicXTRANSFAST(LIBRTPROCESS_PREPOST prepost=LIBRTPROCESS_DEMOSAIC, int threadcount=0);
		bool ApplyDemosaicXTRANSMARKESTEIJN(LIBRTPROCESS_PREPOST prepost=LIBRTPROCESS_DEMOSAIC, int passes=1, bool useCieLab=false, int threadcount=0);
		bool ApplyCACorrect( int threadcount);
		bool ApplyHLRecover(int threadcount);

#endif

		//tonemap algorithms:
		//void ApplyToneMap(GIMAGE_TONEMAP algorithm=TONE_REINHARD_CHANNEL, int threadcount=0);
		void ApplyToneMapGamma(float gamma, int threadcount=0);
		void ApplyToneMapLog2(int threadcount=0);
		void ApplyToneMapReinhard(bool channel=true, bool normalize=false, int threadcount=0);
		void ApplyToneMapFilmic(float A=6.2f, float B=0.5f, float C=1.7f, float D=0.06f, float power=2.2f, bool normalize=false, int threadcount=0);
		void ApplyToneMapLogGamma(int threadcount=0);

		//blur/noise algorithms:
		void ApplyNLMeans(double sigma, int local, int patch, int threadcount=0);
		void ApplyWaveletDenoise(double strength, int threadcount);

		//specialty algorithms:
		void ApplyRedeye(std::vector<coord> points, double threshold, unsigned limit, bool desaturate=false, double desaturatepercent=1.0, int threadcount=0);
		void ApplyTint(double red,double green,double blue, int threadcount=0);
		void ApplyGray(double redpct, double greenpct, double bluepct, int threadcount=0);

		GIMAGE_ERROR ApplyColorspace(std::string iccfile, cmsUInt32Number intent, bool blackpointcomp=false, int threadcount=0);
		GIMAGE_ERROR AssignColorspace(std::string iccfile);
		

		//Image loaders.  Return a new gImage
		static gImage loadRAW(const char * filename, std::string params);
		static gImage loadJPEG(const char * filename, std::string params);
		static gImage loadTIFF(const char * filename, std::string params);
		static gImage loadPNG(const char * filename, std::string params);
		static gImage loadImageFile(const char * filename, std::string params);
		static std::map<std::string,std::string> loadImageFileInfo(const char * filename);

#ifdef USE_DCRAW
		static void setdcrawPath(std::string path);
#endif

		//Image savers. 
		GIMAGE_ERROR saveImageFile(const char * filename, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		GIMAGE_ERROR saveImageFileNoProfile(const char * filename, std::string params);
		GIMAGE_ERROR saveJPEG(const char * filename, BPP bits, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		GIMAGE_ERROR saveTIFF(const char * filename, BPP bits, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		GIMAGE_ERROR savePNG(const char * filename, BPP bits, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);

		//ICC (LittleCMS) profiles.
		static cmsHPROFILE myCmsOpenProfileFromFile(const std::string filename);
		static cmsHPROFILE makeLCMSProfile(const std::string json);
		static cmsHPROFILE makeLCMSProfile(const std::string name, float gamma);
		static cmsHPROFILE makeLCMSCamConstProfile(std::string camconstfile, std::string camera);
		static cmsHPROFILE makeLCMSAdobeCoeffProfile(std::string adobecoeff);
		static cmsHPROFILE makeLCMSdcrawProfile(const std::string name, float gamma);
		static void makeICCProfile(cmsHPROFILE hProfile, char *& profile, cmsUInt32Number  &profilesize);

		//demosaic support routines:
		bool xtranArray(unsigned (&xtarray)[6][6]);
		bool cfArray(unsigned (&cfarray)[2][2]);
		bool rgbCam(float (&rgb_cam)[3][4]);

	protected:

		std::vector<float> Compute1DGaussianKernel(const int kernelsize, const float sigma);
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

		RESIZE_FILTER lensfun_interp_method;

};



#endif
