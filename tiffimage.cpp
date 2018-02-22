#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tiffio.h"
#include "tif_dir.h"

#include <string>
#include <map>

#include "gimage/strutil.h"


const char * tiffVersion()
{
	return TIFFGetVersion();
}

bool _checkTIFF(const char *filename)
{
	if (TIFFOpen(filename, "r") != NULL) return true;
	return false;
}

bool _loadTIFFInfo(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string> &info)
{
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
		
		uint32 read_dir_offset; uint32 count;
		float fval;
		uint16 * sval;
		TIFFGetField(tif, TIFFTAG_EXIFIFD, &read_dir_offset );
		TIFFReadEXIFDirectory(tif, read_dir_offset);
		if (TIFFGetField( tif, EXIFTAG_FNUMBER, &fval)) info["FNumber"] = tostr(fval);
		if (TIFFGetField( tif, EXIFTAG_EXPOSURETIME, &fval)) info["ExposureTime"] = tostr(fval);
		if (TIFFGetField( tif, EXIFTAG_FOCALLENGTH, &fval)) info["FocalLength"] = tostr(fval);
		if (TIFFGetField( tif, EXIFTAG_ISOSPEEDRATINGS, &count, &sval)) info["ISOSpeedRatings"] = tostr(*sval);
		
		*width = w;
		*height = h;
		*numcolors = c;
		*numbits = b;
		TIFFClose(tif);
		return true;
	}
	else return false;
}

char * _loadTIFF(const char *filename, unsigned *width, unsigned *height, unsigned *numcolors, unsigned *numbits, std::map<std::string,std::string> &info, std::string params="", char ** icc_m=NULL, unsigned  *icclength=0)
{
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
		
		uint32 read_dir_offset; uint32 count;
		float fval;
		uint16 * sval;
		TIFFGetField(tif, TIFFTAG_EXIFIFD, &read_dir_offset );
		TIFFReadEXIFDirectory(tif, read_dir_offset);
		if (TIFFGetField( tif, EXIFTAG_FNUMBER, &fval)) info["FNumber"] = tostr(fval);
		if (TIFFGetField( tif, EXIFTAG_EXPOSURETIME, &fval)) info["ExposureTime"] = tostr(fval);
		if (TIFFGetField( tif, EXIFTAG_FOCALLENGTH, &fval)) info["FocalLength"] = tostr(fval);
		if (TIFFGetField( tif, EXIFTAG_ISOSPEEDRATINGS, &count, &sval)) info["ISOSpeedRatings"] = tostr(*sval);
		
		*width = w;
		*height = h;
		*numcolors = c;
		*numbits = b;
		if (buf) _TIFFfree(buf);
		TIFFClose(tif);
		return img;
	}
	else return NULL;
}

bool _writeTIFF(const char *filename, char *imagedata, unsigned width, unsigned height, unsigned numcolors, unsigned numbits, std::map<std::string,std::string> info, char *iccprofile, unsigned iccprofilelength)
{
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

	TIFFWriteDirectory( tif );
	
	uint64 dir_offset = 0;
	TIFFCreateEXIFDirectory(tif);

	if (info.find("ISOSpeedRatings") != info.end())  {
		uint16 iso = (uint16) atoi(info["ISOSpeedRatings"].c_str());
		TIFFSetField(tif, EXIFTAG_ISOSPEEDRATINGS, 1, &iso);
	}

	if (info.find("ExposureTime") != info.end())  {
		double extime = atof(info["ExposureTime"].c_str());
		TIFFSetField(tif, EXIFTAG_EXPOSURETIME, extime);
	}

	if (info.find("FNumber") != info.end())  {
		double fnbr = atof(info["FNumber"].c_str());
		TIFFSetField(tif, EXIFTAG_FNUMBER, fnbr);
	}

	if (info.find("FocalLength") != info.end())  {
		double flen = atof(info["FocalLength"].c_str());
		TIFFSetField(tif, EXIFTAG_FOCALLENGTH, flen);
	}

	TIFFWriteCustomDirectory( tif, &dir_offset);
	TIFFSetDirectory(tif, 0);
	TIFFSetField(tif, TIFFTAG_EXIFIFD, dir_offset );

	(void) TIFFClose(tif);
	if (buf) _TIFFfree(buf);
	return true;
}

