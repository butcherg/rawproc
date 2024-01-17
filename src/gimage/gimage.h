#ifndef _gimage_h
#define _gimage_h

#include <string>
#include <vector>
#include <map>
#include <lcms2.h>
#include "curve.h"

#include <lensfun/lensfun.h>

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

enum GIMAGE_PLACE {
	TOP_LEFT,
	TOP_RIGHT,
	BOTTOM_LEFT,
	BOTTOM_RIGHT
};

enum GIMAGE_FILETYPE {
	FILETYPE_RAW,
	FILETYPE_JPEG,
	FILETYPE_TIFF,
	FILETYPE_PNG,
	FILETYPE_DATA,
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

enum LENS_GEOMETRY {
	GEOMETRY_RETICLINEAR,
	GEOMETRY_FISHEYE,
	GEOMETRY_PANORAMIC,
	GEOMETRY_EQUIRECTANGULAR,
	GEOMETRY_ORTHOGRAPHIC,
	GEOMETRY_STEREOGRAPHIC,
	GEOMETRY_EQUISOLID,
	GEOMETRY_THOBY
};


enum GIMAGE_ERROR {
	GIMAGE_OK,
	GIMAGE_EXCEPTION,
	GIMAGE_UNSUPPORTED_PIXELFORMAT,
	GIMAGE_UNSUPPORTED_FILEFORMAT,
	
	GIMAGE_APPLYCOLORSPACE_BADPROFILE,
	GIMAGE_APPLYCOLORSPACE_BADINPUTPROFILE,
	GIMAGE_APPLYCOLORSPACE_BADOUTPUTPROFILE,
	GIMAGE_APPLYCOLORSPACE_BADINTENT_INPUT,
	GIMAGE_APPLYCOLORSPACE_BADINTENT_OUTPUT,
	GIMAGE_APPLYCOLORSPACE_BADTRANSFORM,
	
	GIMAGE_ASSIGNCOLORSPACE_BADTRANSFORM,
	
	GIMAGE_LF_NO_DATABASE,
	GIMAGE_LF_WRONG_FORMAT,
	GIMAGE_LF_CAMERA_NOT_FOUND,
	GIMAGE_LF_LENS_NOT_FOUND,
	
	GIMAGE_EXIV2_METADATAWRITE_FAILED,
	
	GIMAGE_GMIC_ERROR
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
		std::vector<pix> getPixelPatch(unsigned x, unsigned y, unsigned radius, float multiplier, bool average=false);
		std::string getPixelString(std::vector<pix> pixvec);
		
		std::string getPixelString(unsigned x, unsigned y); //comma-separated triple
		std::string getPixelString(unsigned x, unsigned y, unsigned radius); //comma-separated triples of the area surrounding x,y, one per line
		std::string getPixelString(unsigned x,  unsigned y, unsigned radius, float multiplier);  //comma-separated triple, average of the patch
		std::vector<float> getPixelArray(unsigned x,  unsigned y);

		//image array getters

		//Returns a pointer to a new-allocated char * array of the image, copied from the internal image and, if a profile is specified, 
		//transformed to it.  Image values are in the format specified in the bits parameter, float, unsigned short, or char, so the 
		//caller must typecast appropriately to use the data.  Caller must delete [] the array when done with it:
		char *getImageData(BPP bits, cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);

		//Returns a pointer to a new-allocated float * array of the image, copied from the internal image and, if a profile is specified, 
		//transformed to it.  Caller must delete [] the array when done with it:
		float * getImageDataFloat(bool unbounded, cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);

		//Returns a pointer to a new-allocated pix * array of the image, copied from the internal image and, if a profile is specified, 
		//transformed to it.  If a softprofile is specified, the transform is a soft-proof transform.  Caller must delete [] the array 
		//when done with it:
		float * getImageDataForDisplay(bool unbounded, cmsHPROFILE displayprofile, cmsHPROFILE softprofile, cmsUInt32Number intent, cmsUInt32Number dwflags=0);

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
		bool setInfoValue(std::string name, std::string value);
		bool deleteInfoValue(std::string name);
		std::string getInfoValue(std::string name);
		char * getProfile();
		unsigned getProfileLength();
		static std::string getProfilePath();
		static void setProfilePath(std::string ppath);
		GIMAGE_ERROR getLastError();
		std::string getLastErrorMessage();
		std::string Stats(bool isfloat=true);
		std::map<std::string,float> StatsMap();

		//Setters
		void setInfo(std::string name, std::string value);
		void setProfile(char * prof, unsigned proflength);
		bool setImage(std::vector<pix> img, unsigned width, unsigned height);
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

		//matrix operations:
		void ApplyMatrixMultiply(double matrix[3][3], int threadcount);

		//kernel-based algorithms:
		void ApplyConvolutionKernel(double kernel[3][3], int threadcount=0);
		void Apply1DConvolutionKernel(std::vector<float> kernel, int threadcount=0);
		void Apply2DConvolutionKernel(std::vector<float> kernel, int kerneldimension, int threadcount=0);
		void ApplyGaussianBlur(double sigma, unsigned kernelsize, int threadcount=0);
		void ApplySharpen(double strength, int threadcount=0);

		//math operations:
		void ApplySubtract(double subtractr, double subtractg1, double subtractg2, double subtractb, bool clampblack=false, int threadcount=0);
		void ApplyCFASubtract(float blackarray[6][6], bool clampblack, int threadcount);
		void ApplySubtract(double subtract, GIMAGE_CHANNEL channel=CHANNEL_RGB, bool clampblack=false, int threadcount=0);
		bool ApplySubtract(std::string filename, bool clampblack=false, GIMAGE_PLACE place=TOP_LEFT, int threadcount=0);
		bool ApplySubtract(gImage& subtractimage, bool clampblack=false, int threadcount=0);
		void ApplyAdd(double add, GIMAGE_CHANNEL channel=CHANNEL_RGB, bool clampwhite=false, int threadcount=0);
		bool ApplyAdd(std::string filename, bool clampwhite=false, GIMAGE_PLACE place=TOP_LEFT, int threadcount=0);
		bool ApplyAdd(gImage& addimage, bool clampwhite=false, int threadcount=0);


		//basic color/tone operations:
		void ApplySaturate(double saturate, int threadcount=0);
		void ApplySaturate(double saturate, GIMAGE_CHANNEL channel=CHANNEL_RGB, float threshold=0.0, int threadcount=0);
		void ApplyExposureCompensation(double ev, int threadcount=0);
		float ApplyExposureCompensation(int x, int y, float radius, float destinationev, int threadcount);
		void ApplyToneCurve(std::vector<cp> ctpts, int threadcount=0);
		void ApplyToneCurve(std::vector<cp> ctpts, GIMAGE_CHANNEL channel, int threadcount=0);
		void ApplyToneLine(double low, double high, GIMAGE_CHANNEL channel, int threadcount=0);
		bool ApplyHaldCLUT(std::string filename, int threadcount=0);
		bool Apply1DLUT(std::vector<pix> lut, int threadcount=0);


		//image geometry algorithms:
		void ApplyResize(unsigned width, unsigned height, RESIZE_FILTER filter, int threadcount=0);
		void ApplyResize(std::string params, int threadcount=0);
		void ApplyRatioCrop(float x1, float y1, float x2, float y2, int threadcount);
		void ApplyCrop(unsigned x1, unsigned y1, unsigned x2, unsigned y2, int threadcount=0);
		std::vector<unsigned> ApplySpectralCrop(unsigned band, float greenthreshold, int threadcount= 0); //specifically used to pull spectra for SSF construction

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
		std::vector<double> ApplyWhiteBalance(double redmult, double greenmult, double bluemult,  float bluethreshold, int threadcount=0);

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
		bool ApplyCACorrect(const bool autoCA=true, size_t autoIterations=1, const double cared=0.0, const double cablue=0.0, bool avoidColourshift=true, int threadcount=0);
		bool ApplyHLRecover(int threadcount=0);

#endif

		//tonemap algorithms:
		//void ApplyToneMap(GIMAGE_TONEMAP algorithm=TONE_REINHARD_CHANNEL, int threadcount=0);
		void ApplyToneMapGamma(float gamma, int threadcount=0);
		void ApplyToneMapLog2(int threadcount=0);
		void ApplyToneMapReinhard(bool channel=true, bool normalize=false, int threadcount=0);
		void ApplyToneMapFilmic(float A=6.2f, float B=0.5f, float C=1.7f, float D=0.06f, float power=2.2f, bool normalize=false, int threadcount=0);
		void ApplyToneMapLogGamma(int threadcount=0);
		void ApplyNormalization(float newmin, float newmax, int threadcount=0);
		void ApplyToneMapDualLogistic(std::map<std::string, std::string> parameters, int threadcount=0);

		//blur/noise algorithms:
		void ApplyNLMeans(double sigma, int local, int patch, float threshold=-1.0, int threadcount=0); //-1 bypasses threshold, 0.0 and up bypasses any pixel >= threshold
		void ApplyWaveletDenoise(double strength, int threadcount=0);

		//specialty algorithms:
		void ApplyRedeye(std::vector<coord> points, double threshold, unsigned limit, bool desaturate=false, double desaturatepercent=1.0, int threadcount=0);
		void ApplyTint(double red,double green,double blue, int threadcount=0);
		void ApplyGray(double redpct, double greenpct, double bluepct, int threadcount=0);
		void ApplySpotRemovalRadial(unsigned spotx, unsigned spoty, float spotradius, int threadcount=0);
		void ApplySpotRemovalClone(unsigned spotx, unsigned spoty, unsigned patchx, unsigned patchy, unsigned patchsize, int threadcount=0);

		//colorspace operators:
		GIMAGE_ERROR ApplyColorspace(std::string iccfile, cmsUInt32Number intent, bool blackpointcomp=false, int threadcount=0);
		GIMAGE_ERROR AssignColorspace(std::string iccfile);
		
		//Lensfun support methods
		bool initInterpolation(RESIZE_FILTER method);  //only accepts FILTER_BILINEAR and FILTER_LANCZOS3
		PIXTYPE getR(float x, float y);
		PIXTYPE getG(float x, float y);
		PIXTYPE getB(float x, float y);
		pix getRGB(float x, float y);

		//Lensfun database and correction methods. 
		//From the lensfun library, gImage users need to know the lfDatabase class and the LF_MODIFY_XXXX flags
		GIMAGE_ERROR lensfunLoadLensDatabase(std::string lensfundatadir, lfDatabase **ldb);
		GIMAGE_ERROR lensfunFindCameraLens(lfDatabase * ldb, std::string camera, std::string lens);
		int lensfunAvailableModifications(lfDatabase * ldb, std::string camera, std::string lens); //returns OR-ed set of LF_MODIFY_XXXX flags
		GIMAGE_ERROR ApplyLensCorrection(lfDatabase * ldb, int modops, LENS_GEOMETRY geometry, RESIZE_FILTER algo, int threadcount=0, std::string camera=std::string(), std::string lens=std::string());
		//if camera or lens is empty, the method attempts to use the imginfo["Model"] and imginfo["Lens"]
		
		//Lens correction algorithms
		GIMAGE_ERROR ApplyDistortionCorrectionPTLens(float a, float b, float c, float d, int threadcount);
		GIMAGE_ERROR ApplyDistortionCorrectionAdobe(float k0, float k1, float k2, float k3, int threadcount);

#ifdef USE_GMIC
		//uses libgmic to apply a G'MIC script to the gImage:
		GIMAGE_ERROR ApplyGMICScript(std::string script);
#endif

		//Image loaders.  Return a new gImage
		static gImage loadRAW(const char * filename, std::string params);
		static gImage loadJPEG(const char * filename, std::string params);
		static gImage loadTIFF(const char * filename, std::string params);
		static gImage loadPNG(const char * filename, std::string params);
		static gImage loadImageFile(const char * filename, std::string params);
		static std::map<std::string,std::string> loadMetadata(const char * filename);
		
#ifdef USE_DCRAW
		static void setdcrawPath(std::string path);
#endif

		//Image savers. 
		GIMAGE_ERROR saveImageFile(const char * filename, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		GIMAGE_ERROR saveImageFileNoProfile(const char * filename, std::string params);
		GIMAGE_ERROR saveJPEG(const char * filename, BPP bits, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		GIMAGE_ERROR saveTIFF(const char * filename, BPP bits, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		GIMAGE_ERROR savePNG(const char * filename, BPP bits, std::string params="", cmsHPROFILE profile=NULL, cmsUInt32Number intent=INTENT_PERCEPTUAL);
		GIMAGE_ERROR saveData(const char * filename, BPP bits, std::string params);
		
		//ICC (LittleCMS) profiles.
		static cmsHPROFILE myCmsOpenProfileFromFile(const std::string filename);
		static cmsHPROFILE makeLCMSStoredProfile(const std::string profilename);
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

		GIMAGE_ERROR insertMetadata(std::string filename, cmsHPROFILE profile=NULL, bool excludeexif=false);
		GIMAGE_ERROR getMetadata(std::string filename);



	private:
		std::vector<pix> image;
		unsigned w, h, c;
		BPP b;
		std::map<std::string,std::string> imginfo;

		char *profile;
		unsigned profile_length;
		
		static std::string profilepath;
		
		GIMAGE_ERROR lasterror;
		std::string lasterrormsg;

		RESIZE_FILTER lensfun_interp_method;


};



#endif
