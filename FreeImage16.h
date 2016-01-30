#ifndef __FREEIMAGE16_H__
#define __FREEIMAGE16_H__

#include "FreeImage.h"
#include <vector>
#include "Curve.h"

//#include <stdint.h>

bool FreeImage_AdjustCurveControlPoints(FIBITMAP *src, std::vector<cp> controlpoints, FREE_IMAGE_COLOR_CHANNEL channel);

bool FreeImage_AdjustCurve16(FIBITMAP *src, WORD *LUT, FREE_IMAGE_COLOR_CHANNEL channel);

int FreeImage_GetAdjustColorsLookupTable16(WORD *LUT, double brightness, double contrast, double gamma, BOOL invert);

void FreeImage_PrintImageType(FREE_IMAGE_TYPE t);

bool FreeImage_Saturate16(FIBITMAP *src, double percentage);

bool FreeImage_Gray16(FIBITMAP *src, double Rpct, double Gpct, double Bpct);

#endif


