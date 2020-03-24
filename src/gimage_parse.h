#ifndef __GIMAGE_PARSE_H__
#define __GIMAGE_PARSE_H__

#include <string>
#include "gimage/gimage.h"

void paramprint(std::map<std::string,std::string> params);
bool paramexists (std::map<std::string,std::string> m, std::string k);
int getThreadCount(int threadcount);

std::map<std::string,std::string> parse_blackwhitepoint(std::string params);
std::map<std::string,std::string> process_blackwhitepoint(gImage &dib, std::map<std::string,std::string> params);

#endif
