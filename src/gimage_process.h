#ifndef __GIMAGE_PROCESS_H__
#define __GIMAGE_PROCESS_H__

#include <string>
#include "gimage/gimage.h"

std::map<std::string,std::string> process_add(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_blackwhitepoint(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_colorspace(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_crop(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_cropspectrum(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_curve(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_demosaic(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_denoise(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_exposure(gImage &dib, std::map<std::string,std::string> params);
#ifdef USE_GMIC
std::map<std::string,std::string> process_gmic(gImage &dib, std::map<std::string,std::string> params);
#endif
std::map<std::string,std::string> process_gray(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_hlrecover(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_lenscorrection(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_lensdistortion(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_lensvignetting(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_redeye(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_resize(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_rotate(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_saturation(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_sharpen(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_spot(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_subtract(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_tone(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_whitebalance(gImage &dib, std::map<std::string,std::string> params);
std::map<std::string,std::string> process_group(gImage &dib, std::map<std::string,std::string> params, bool verbose);


#endif
