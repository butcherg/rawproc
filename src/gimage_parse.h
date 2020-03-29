#ifndef __GIMAGE_PARSE_H__
#define __GIMAGE_PARSE_H__

#include <string>
#include "gimage/gimage.h"

void paramprint(std::map<std::string,std::string> params);
bool paramexists (std::map<std::string,std::string> m, std::string k);

std::map<std::string,std::string> parse_blackwhitepoint(std::string paramstring);
std::map<std::string,std::string> parse_colorspace(std::string paramstring);
std::map<std::string,std::string> parse_crop(std::string paramstring);

#endif
