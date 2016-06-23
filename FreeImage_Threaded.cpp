#include "FreeImage.h"
#include "Curve.h"
#include "elapsedtime.h"
#include <vector>
#include <algorithm>


double ApplyCurve(FIBITMAP *src, FIBITMAP *dst, std::vector<cp> ctpts, int threadcount)
{
	mark();
	unsigned spitch = FreeImage_GetPitch(src);
	unsigned dpitch = FreeImage_GetPitch(dst);
	unsigned w = FreeImage_GetWidth(src);
	unsigned h = FreeImage_GetHeight(src);
	BYTE * srcbits = FreeImage_GetBits(src);
	BYTE * dstbits = FreeImage_GetBits(dst);

	Curve c;
	BYTE LUT8[256];
	WORD LUT16[65535];
	c.setControlPoints(ctpts);
	int bpp = FreeImage_GetBPP(src);
	if (bpp == 24) {
		c.clampto(0.0,255.0);
		for (int x=0; x<256; x++) {
			LUT8[x] = (BYTE)floor(c.getpoint(x) + 0.5); 
		}
		#pragma omp parallel for
		for(unsigned y = 0; y < h; y++) {
			for(unsigned x = 0; x < w; x++) {
				BYTE * bdstpix = (BYTE *) dstbits + dpitch*y + 3*x;
				BYTE *pixel = (BYTE *) (srcbits + spitch*y + 3*x);
				bdstpix[FI_RGBA_RED]   = LUT8[pixel[FI_RGBA_RED]];
				bdstpix[FI_RGBA_GREEN] = LUT8[pixel[FI_RGBA_GREEN]];
				bdstpix[FI_RGBA_BLUE]  = LUT8[pixel[FI_RGBA_BLUE]];

				//bdstpix[FI_RGBA_RED]   = ((BYTE *) LUT8)[pixel[FI_RGBA_RED]];
				//bdstpix[FI_RGBA_GREEN] = ((BYTE *) LUT8)[pixel[FI_RGBA_GREEN]];
				//bdstpix[FI_RGBA_BLUE]  = ((BYTE *) LUT8)[pixel[FI_RGBA_BLUE]];
			}
		}
	}
	if (bpp == 48) {
		c.scalepoints(256.0);
		c.clampto(0.0,65535.0);
		for (int x=0; x<65536; x++) {
			LUT16[x] = (WORD)floor(c.getpoint(x) + 0.5);
		}
		#pragma omp parallel for
		for(unsigned y = 0; y < h; y++) {
			for(unsigned x = 0; x < w; x++) {
				FIRGB16 * wdstpix = (FIRGB16 *) (dstbits + dpitch*y + 6*x);
				FIRGB16 * pixel   = (FIRGB16 *) (srcbits + spitch*y + 6*x);
				wdstpix->red   = LUT16[pixel->red];
				wdstpix->green = LUT16[pixel->green];
				wdstpix->blue  = LUT16[pixel->blue];

				//wdstpix->red   = ((WORD *) LUT16)[pixel->red];
				//wdstpix->green = ((WORD *) LUT16)[pixel->green];
				//wdstpix->blue  = ((WORD *) LUT16)[pixel->blue];
			}
		}
	}
	return duration();
}


double ApplyLUT(FIBITMAP *src, FIBITMAP *dst, char * LUT, int threadcount)
{
	mark();
	unsigned spitch = FreeImage_GetPitch(src);
	unsigned dpitch = FreeImage_GetPitch(dst);
	unsigned w = FreeImage_GetWidth(src);
	unsigned h = FreeImage_GetHeight(src);
	BYTE * srcbits = FreeImage_GetBits(src);
	BYTE * dstbits = FreeImage_GetBits(dst);

	BYTE LUT8[256];
	WORD LUT16[65536];
	int bpp = FreeImage_GetBPP(src);
	if (bpp == 24) {
		for (int x=0; x<256; x++) {
			LUT8[x] = ((BYTE *) LUT)[x];; 
		}
		#pragma omp parallel for
		for(unsigned y = 0; y < h; y++) {
			for(unsigned x = 0; x < w; x++) {
				BYTE * bdstpix = (BYTE *) (dstbits + dpitch*y + 3*x);
				BYTE * pixel   = (BYTE *) (srcbits + spitch*y + 3*x);
				bdstpix[FI_RGBA_RED]   = LUT8[pixel[FI_RGBA_RED]];
				bdstpix[FI_RGBA_GREEN] = LUT8[pixel[FI_RGBA_GREEN]];
				bdstpix[FI_RGBA_BLUE]  = LUT8[pixel[FI_RGBA_BLUE]];

				//bdstpix[FI_RGBA_RED]   = ((BYTE *) LUT8)[pixel[FI_RGBA_RED]];
				//bdstpix[FI_RGBA_GREEN] = ((BYTE *) LUT8)[pixel[FI_RGBA_GREEN]];
				//bdstpix[FI_RGBA_BLUE]  = ((BYTE *) LUT8)[pixel[FI_RGBA_BLUE]];


			}
		}
	}
	if (bpp == 48) {
		for (int x=0; x<65536; x++) {
			LUT16[x] = ((WORD *)LUT)[x];
		}
		#pragma omp parallel for
		for(unsigned y = 0; y < h-1; y++) {
			for(unsigned x = 0; x < w-1; x++) {
				FIRGB16 * wdstpix = (FIRGB16 *) (dstbits + dpitch*y + 6*x);
				FIRGB16 * pixel   = (FIRGB16 *) (srcbits + spitch*y + 6*x);
				wdstpix->red   = LUT16[pixel->red];
				wdstpix->green = LUT16[pixel->green];
				wdstpix->blue  = LUT16[pixel->blue];

				//wdstpix->red   = ((WORD *) LUT16)[pixel->red];
				//wdstpix->green = ((WORD *) LUT16)[pixel->green];
				//wdstpix->blue  = ((WORD *) LUT16)[pixel->blue];


			}
		}
	}
	return duration();
}

double ApplyKernel(FIBITMAP *src, FIBITMAP *dst, double kernel[3][3], int threadcount)
{
	mark();
	
	int bpp = FreeImage_GetBPP(src);

	unsigned spitch = FreeImage_GetPitch(src);
	unsigned dpitch = FreeImage_GetPitch(dst);
	BYTE * srcbits = FreeImage_GetBits(src);
	BYTE * dstbits = FreeImage_GetBits(dst);

	switch(bpp) {
		case 48:
			#pragma omp parallel for
			for(unsigned y = 1; y < FreeImage_GetHeight(src)-1; y++) {
				for(unsigned x = 1; x < FreeImage_GetWidth(src)-1; x++) {
					FIRGB16 *wdstpix = (FIRGB16 *) (dstbits + dpitch*y + 6*x);
					double R=0.0; double G=0.0; double B=0.0;
					for (unsigned kx=0; kx<3; kx++) {
						int ix=kx*3;
						for (int ky=0; ky<3; ky++) {
							int i = ix+ky;
							FIRGB16 *pixel = (FIRGB16 *) (srcbits + spitch*(y-1+ky) + 6*(x-1+kx));
							R += pixel->red   * kernel[kx][ky];
							G += pixel->green * kernel[kx][ky];
							B += pixel->blue  * kernel[kx][ky];
						}
						wdstpix->red   = std::min(std::max(int(R), 0), 65535);
						wdstpix->green = std::min(std::max(int(G), 0), 65535);
						wdstpix->blue  = std::min(std::max(int(B), 0), 65535);
					}

				}
			}
			break;	            
		case 24 :
			#pragma omp parallel for
			for(unsigned y = 1; y < FreeImage_GetHeight(src)-1; y++) {
				for(unsigned x = 1; x < FreeImage_GetWidth(src)-1; x++) {
					BYTE *bdstpix = (BYTE *) dstbits + dpitch*y + 3*x;
					double R=0.0; double G=0.0; double B=0.0;
					for (unsigned kx=0; kx<3; kx++) {
						int ix=kx*3;
						for (int ky=0; ky<3; ky++) { 
							int i = ix+ky;
							BYTE *pixel = (BYTE *) srcbits + spitch*(y-1+ky) + 3*(x-1+kx);
							R += pixel[FI_RGBA_RED]   * kernel[kx][ky];
							G += pixel[FI_RGBA_GREEN] * kernel[kx][ky];
							B += pixel[FI_RGBA_BLUE]  * kernel[kx][ky];
						}
						bdstpix[FI_RGBA_RED]   = std::min(std::max(int(R), 0), 255);
						bdstpix[FI_RGBA_GREEN] = std::min(std::max(int(G), 0), 255);
						bdstpix[FI_RGBA_BLUE]  = std::min(std::max(int(B), 0), 255);
					}
				}
			}
			break;
	}
	return duration();
}

#define  Pr  .299
#define  Pg  .587
#define  Pb  .114

double ApplySaturation(FIBITMAP *src, FIBITMAP *dst, double saturate, int threadcount)
{
	mark();

	int bpp = FreeImage_GetBPP(src);

	unsigned spitch = FreeImage_GetPitch(src);
	unsigned dpitch = FreeImage_GetPitch(dst);
	BYTE * srcbits = FreeImage_GetBits(src);
	BYTE * dstbits = FreeImage_GetBits(dst);


	switch(bpp) {
		case 48:
			#pragma omp parallel for
			for(unsigned y = 0; y < FreeImage_GetHeight(src); y++) {
				for(unsigned x = 0; x < FreeImage_GetWidth(src); x++) {
					FIRGB16 * wdstpix = (FIRGB16 *) (dstbits + dpitch*y + 6*x);
					FIRGB16 * pixel   = (FIRGB16 *) (srcbits + spitch*y + 6*x);

					double R = (double) pixel->red;
					double G = (double) pixel->green;
					double B = (double) pixel->blue;

					double  P=sqrt(
					R*R*Pr+
					G*G*Pg+
					B*B*Pb ) ;

					R=P+(R-P)*saturate;
					G=P+(G-P)*saturate;
					B=P+(B-P)*saturate;

					if (R>65535.0) R=65535.0;
					if (G>65535.0) G=65535.0;
					if (B>65535.0) B=65535.0;
					if (R<0.0) R=0.0;
					if (G<0.0) G=0.0;
					if (B<0.0) B=0.0;
					wdstpix->red = int(R);
					wdstpix->green = int(G);
					wdstpix->blue = int(B);
				}
			}
			break;	            
		case 24 :
			#pragma omp parallel for
			for(unsigned y = 0; y < FreeImage_GetHeight(src); y++) {
				for(unsigned x = 0; x < FreeImage_GetWidth(src); x++) {
					BYTE * bdstpix = (BYTE *) dstbits + dpitch*y + 3*x;
					BYTE *pixel = (BYTE *) (srcbits + spitch*y + 3*x);

					double R = (double) pixel[FI_RGBA_RED];
					double G = (double) pixel[FI_RGBA_GREEN];
					double B = (double) pixel[FI_RGBA_BLUE];

					double  P=sqrt(
					R*R*Pr+
					G*G*Pg+
					B*B*Pb ) ;

					R=P+(R-P)*saturate;
					G=P+(G-P)*saturate;
					B=P+(B-P)*saturate;

					if (R>255.0) R=255.0;
					if (G>255.0) G=255.0;
					if (B>255.0) B=255.0;
					if (R<0.0) R=0.0;
					if (G<0.0) G=0.0;
					if (B<0.0) B=0.0;
					bdstpix[FI_RGBA_RED] = int(R);
					bdstpix[FI_RGBA_GREEN] = int(G);
					bdstpix[FI_RGBA_BLUE]= int(B);
				}
			}
			break;
	}
	return duration();

}


double ApplyGray(FIBITMAP *src, FIBITMAP *dst, double redpct, double greenpct, double bluepct, int threadcount)
{
	mark();

	int bpp = FreeImage_GetBPP(src);

	unsigned spitch = FreeImage_GetPitch(src);
	unsigned dpitch = FreeImage_GetPitch(dst);
	BYTE * srcbits = FreeImage_GetBits(src);
	BYTE * dstbits = FreeImage_GetBits(dst);


	switch(bpp) {
		case 48:
			#pragma omp parallel for
			for(unsigned y = 0; y < FreeImage_GetHeight(src); y++) {
				for(unsigned x = 0; x < FreeImage_GetWidth(src); x++) {
					FIRGB16 * wdstpix = (FIRGB16 *) (dstbits + dpitch*y + 6*x);
					FIRGB16 * pixel   = (FIRGB16 *) (srcbits + spitch*y + 6*x);

					double G = floor((double) pixel->red*redpct + (double) pixel->green*greenpct + (double) pixel->blue*bluepct)+0.5;
					if (G>65535.0) G=65535.0;
					if (G<0.0) G=0.0;

					wdstpix->red = int(G);
					wdstpix->green = int(G);
					wdstpix->blue = int(G);
				}
			}
			break;	            
		case 24 :
			#pragma omp parallel for
			for(unsigned y = 0; y < FreeImage_GetHeight(src); y++) {
				for(unsigned x = 0; x < FreeImage_GetWidth(src); x++) {
					BYTE * bdstpix = (BYTE *) dstbits + dpitch*y + 3*x;
					BYTE *pixel = (BYTE *) (srcbits + spitch*y + 3*x);

					double G = floor((double) pixel[FI_RGBA_RED]*redpct + (double) pixel[FI_RGBA_GREEN]*greenpct + (double) pixel[FI_RGBA_BLUE]*bluepct)+0.5;
					if (G>255.0) G=255.0;
					if (G<0.0) G=0.0;

					bdstpix[FI_RGBA_RED] = int(G);
					bdstpix[FI_RGBA_GREEN] = int(G);
					bdstpix[FI_RGBA_BLUE]= int(G);

				}
			}
			break;
	}
	return duration();

}

double ApplyNLMeans(FIBITMAP *src, FIBITMAP *dst, double strength, int threadcount)
{
	mark();

	int bpp = FreeImage_GetBPP(src);

	unsigned spitch = FreeImage_GetPitch(src);
	unsigned dpitch = FreeImage_GetPitch(dst);
	BYTE * srcbits = FreeImage_GetBits(src);
	BYTE * dstbits = FreeImage_GetBits(dst);

	//unsigned hl=


	switch(bpp) {
		case 48:
			#pragma omp parallel for
			for(unsigned y = 0; y < FreeImage_GetHeight(src); y++) {
				for(unsigned x = 0; x < FreeImage_GetWidth(src); x++) {
					FIRGB16 * wdstpix = (FIRGB16 *) (dstbits + dpitch*y + 6*x);
					FIRGB16 * pixel   = (FIRGB16 *) (srcbits + spitch*y + 6*x);

					double R = (double) pixel->red;
					double G = (double) pixel->green;
					double B = (double) pixel->blue;


					if (R>65535.0) R=65535.0;
					if (G>65535.0) G=65535.0;
					if (B>65535.0) B=65535.0;
					if (R<0.0) R=0.0;
					if (G<0.0) G=0.0;
					if (B<0.0) B=0.0;
					wdstpix->red = int(R);
					wdstpix->green = int(G);
					wdstpix->blue = int(B);
				}
			}
			break;	            
		case 24 :
			#pragma omp parallel for
			for(unsigned y = 0; y < FreeImage_GetHeight(src); y++) {
				for(unsigned x = 0; x < FreeImage_GetWidth(src); x++) {
					BYTE * bdstpix = (BYTE *) dstbits + dpitch*y + 3*x;
					BYTE *pixel = (BYTE *) (srcbits + spitch*y + 3*x);

					double R = (double) pixel[FI_RGBA_RED];
					double G = (double) pixel[FI_RGBA_GREEN];
					double B = (double) pixel[FI_RGBA_BLUE];



					if (R>255.0) R=255.0;
					if (G>255.0) G=255.0;
					if (B>255.0) B=255.0;
					if (R<0.0) R=0.0;
					if (G<0.0) G=0.0;
					if (B<0.0) B=0.0;
					bdstpix[FI_RGBA_RED] = int(R);
					bdstpix[FI_RGBA_GREEN] = int(G);
					bdstpix[FI_RGBA_BLUE]= int(B);
				}
			}
			break;
	}
	return duration();

}

