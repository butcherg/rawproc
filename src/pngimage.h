#ifndef _pngimage_h
#define _pngimage_h

const char * pngVersion();

bool _checkPNG(const char *filename);

bool _loadPNGInfo(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info);

char * _loadPNG(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info, 
			std::string params="",
			char ** icc_m=NULL, 
			unsigned  *icclength=0);

bool _writePNG(const char *filename, 
			char *imagedata, 
			unsigned width, 
			unsigned height, 
			unsigned numcolors, 
			unsigned numbits, 
			std::map<std::string,std::string> info, 
			std::string params="",
			char *iccprofile=NULL, 
			unsigned iccprofilelength=0);

#endif

