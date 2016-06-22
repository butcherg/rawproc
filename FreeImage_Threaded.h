#include "FreeImage.h"
#include "Curve.h"
#include <vector>


double ApplyCurve(FIBITMAP *src, FIBITMAP *dst, std::vector<cp> ctpts, int threadcount);
double ApplyLUT(FIBITMAP *src, FIBITMAP *dst, char * LUT, int threadcount);
double ApplyKernel(FIBITMAP *src, FIBITMAP *dst, double kernel[3][3], int threadcount);
double ApplySaturation(FIBITMAP *src, FIBITMAP *dst, double saturate, int threadcount);
double ApplyNLMeans(FIBITMAP *src, FIBITMAP *dst, double strength, int threadcount);

