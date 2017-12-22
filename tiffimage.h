#ifndef _tiffimage_h
#define _tiffimage_h

const char * tiffVersion();

bool _checkTIFF(const char *filename);

bool _loadTIFFInfo(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info);

char * _loadTIFF(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info, 
			std::string params="",
			char ** icc_m=NULL, 
			unsigned  *icclength=0);

bool _writeTIFF(const char *filename, 
			char *imagedata, 
			unsigned width, 
			unsigned height, 
			unsigned numcolors, 
			unsigned numbits, 
			std::map<std::string,std::string> info, 
			char *iccprofile=NULL, 
			unsigned iccprofilelength=0);

#endif

