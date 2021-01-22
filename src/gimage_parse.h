#ifndef __GIMAGE_PARSE_H__
#define __GIMAGE_PARSE_H__

#include <string>
#include "gimage/gimage.h"

void paramprint(std::map<std::string,std::string> params);
bool paramexists (std::map<std::string,std::string> m, std::string k);

std::map<std::string,std::string> parse_add(std::string paramstring);
std::map<std::string,std::string> parse_blackwhitepoint(std::string paramstring);
std::map<std::string,std::string> parse_colorspace(std::string paramstring);
std::map<std::string,std::string> parse_crop(std::string paramstring);
std::map<std::string,std::string> parse_cropspectrum(std::string paramstring);
std::map<std::string,std::string> parse_curve(std::string paramstring);
std::map<std::string,std::string> parse_demosaic(std::string paramstring);
std::map<std::string,std::string> parse_denoise(std::string paramstring);
std::map<std::string,std::string> parse_exposure(std::string paramstring);
std::map<std::string,std::string> parse_gray(std::string paramstring);
std::map<std::string,std::string> parse_hlrecover(std::string paramstring);
#ifdef USE_LENSFUN
std::map<std::string,std::string> parse_lenscorrection(std::string paramstring);
#endif
std::map<std::string,std::string> parse_redeye(std::string paramstring);
std::map<std::string,std::string> parse_resize(std::string paramstring);
std::map<std::string,std::string> parse_rotate(std::string paramstring);
std::map<std::string,std::string> parse_saturation(std::string paramstring);
std::map<std::string,std::string> parse_sharpen(std::string paramstring);
std::map<std::string,std::string> parse_subtract(std::string paramstring);
std::map<std::string,std::string> parse_tone(std::string paramstring);
std::map<std::string,std::string> parse_whitebalance(std::string paramstring);

#endif
