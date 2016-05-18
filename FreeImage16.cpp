#include "FreeImage.h"
#include "FreeImage16.h"
//#include "Utilities.h"
#include "saturation.h"
#include "Curve.h"

#include <stdint.h>




bool FreeImage_AdjustCurve16(FIBITMAP *src, WORD *LUT, FREE_IMAGE_COLOR_CHANNEL channel) {
	unsigned x, y;
	//uint16_t *bits = NULL;

	//if(!FreeImage_HasPixels(src) || !LUT || (FreeImage_GetImageType(src) != FIT_BITMAP))
	//	return FALSE;

	int bpp = FreeImage_GetBPP(src);
	if(bpp != 48)
		return FALSE;

	// apply the LUT
	int bytespp = FreeImage_GetLine(src) / FreeImage_GetWidth(src);
	switch(channel) {
		case FICC_RGB :
			for(y = 0; y < FreeImage_GetHeight(src); y++) {
				//bits =  FreeImage_GetScanLine(src, y);
				FIRGB16 *bits = (FIRGB16 *)FreeImage_GetScanLine(src, y);
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					bits[x].red = LUT[bits[x].red];
					bits[x].green = LUT[bits[x].green];
					bits[x].blue = LUT[bits[x].blue];
				}
			}
			break;
/*
		case FICC_BLUE :
			for(y = 0; y < FreeImage_GetHeight(src); y++) {
				//bits =  FreeImage_GetScanLine(src, y);
				FIRGB16 *bits = (FIRGB16 *)FreeImage_GetScanLine(src, y);
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					bits[x].blue = LUT[bits[x].blue];
				}
			}
			break;

		case FICC_GREEN :
			for(y = 0; y < FreeImage_GetHeight(src); y++) {
				//bits =  FreeImage_GetScanLine(src, y);
				FIRGB16 *bits = (FIRGB16 *)FreeImage_GetScanLine(src, y);
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					bits[x].green = LUT[bits[x].green];
				}
			}
			break;

		case FICC_RED :
			for(y = 0; y < FreeImage_GetHeight(src); y++) {
				//bits =  FreeImage_GetScanLine(src, y);
				FIRGB16 *bits = (FIRGB16 *)FreeImage_GetScanLine(src, y);
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					bits[x].red = LUT[bits[x].red];
				}
			}
			break;
*/					
/*
		case FICC_ALPHA :
			if(32 == bpp) {
				for(y = 0; y < FreeImage_GetHeight(src); y++) {
					//bits =  FreeImage_GetScanLine(src, y);
					FIRGB16 *bits = (FIRGB16 *)FreeImage_GetScanLine(src, y);
					for(x = 0; x < FreeImage_GetWidth(src); x++) {
						bits[FI_RGBA_ALPHA] = LUT[ bits[FI_RGBA_ALPHA] ];	// A
								
						//bits += bytespp;
					}
				}
			}
			break;
*/
		default:
			break;
	}

	return TRUE;
}

bool FreeImage_AdjustCurveControlPoints(FIBITMAP *src, std::vector<cp> controlpoints, FREE_IMAGE_COLOR_CHANNEL channel) {
	Curve c;
	char *x, *y;
	c.setControlPoints(controlpoints);
	int bpp = FreeImage_GetBPP(src);
	if (bpp == 8 |bpp == 24 | bpp == 32) {
		BYTE LUT8[256];
		c.clampto(0.0,255.0);
		for (int x=0; x<256; x++) {
			LUT8[x] = (BYTE)floor(c.getpoint(x) + 0.5);
		}
		if (FreeImage_AdjustCurve(src, LUT8, channel)) return true;
	}
	if (bpp == 48) {
		WORD LUT16[65536];
		c.scalepoints(256.0);
		c.clampto(0.0,65535.0);
		for (int x=0; x<65536; x++) {
			LUT16[x] = (WORD)floor(c.getpoint(x) + 0.5);
		}
		if (FreeImage_AdjustCurve16(src, LUT16, channel)) return true;
	}
	return false;
}


int FreeImage_GetAdjustColorsLookupTable16(WORD *LUT, double brightness, double contrast, double gamma, BOOL invert) {
	double dblLUT[65535];
	double value;
	int result = 0;

	if ((brightness == 0.0) && (contrast == 0.0) && (gamma == 1.0) && (!invert)) {
		// nothing to do, if all arguments have their default values
		// return a blind LUT
		for (int i = 0; i < 65535; i++) {
			LUT[i] = i;
		}
		return 0;
	}

	// first, create a blind LUT, which does nothing to the image
	for (int i = 0; i < 65536; i++) {
		dblLUT[i] = i;
	}

	if (contrast != 0.0) {
		// modify lookup table with contrast adjustment data
		const double v = (100.0 + contrast) / 100.0;
		for (int i = 0; i < 65536; i++) {
			value = 32767 + (dblLUT[i] - 32768) * v;
			dblLUT[i] = MAX(0.0, MIN(value, 65535.0));
		}
		result++;
	}

	if (brightness != 0.0) {
		// modify lookup table with brightness adjustment data
		const double v = (100.0 + brightness) / 100.0;
		for (int i = 0; i < 65536; i++) {
			value = dblLUT[i] * v;
			dblLUT[i] = MAX(0.0, MIN(value, 65535.0));
		}
		result++;
	}

	if ((gamma > 0) && (gamma != 1.0)) {
		// modify lookup table with gamma adjustment data
		double exponent = 1 / gamma;
		const double v = 65535.0 * (double)pow((double)65535, -exponent);
		for (int i = 0; i < 65536; i++) {
			value = pow(dblLUT[i], exponent) * v;
			dblLUT[i] = MAX(0.0, MIN(value, 65535.0));
		}
		result++;
	}

	if (!invert) {
		for (int i = 0; i < 65536; i++) {
    		LUT[i] = (WORD)floor(dblLUT[i] + 0.5);
		}
	} else {
		for (int i = 0; i < 65536; i++) {
			 LUT[i] = 65535 - (WORD)floor(dblLUT[i] + 0.5);
		}
		result++;
	}
	// return the number of adjustments made
	return result;
}

void FreeImage_PrintImageType(FREE_IMAGE_TYPE t)
{
    switch (t) {
        case FIT_UNKNOWN:
            printf("FIT_UNKNOWN: Unknown format (returned value only, never use it as input value)\n");
            break;
        case FIT_BITMAP:
            printf("FIT_BITMAP: Standard image: 1-, 4-, 8-, 16-, 24-, 32-bit\n");
            break;
        case FIT_UINT16:
            printf("FIT_UINT16: Array of unsigned short: unsigned 16-bit\n");
            break;
        case FIT_INT16:
            printf("FIT_INT16: Array of short: signed 16-bit\n");
            break;
        case FIT_UINT32:
            printf("FIT_UINT32: Array of unsigned long: unsigned 32-bit\n");
            break;
        case FIT_INT32:
            printf("FIT_INT32: Array of long: signed 32-bit\n");
            break;
        case FIT_FLOAT:
            printf("FIT_FLOAT: Array of float: 32-bit IEEE floating point\n");
            break;
        case FIT_DOUBLE:
            printf("FIT_DOUBLE Array of double: 64-bit IEEE floating point\n");
            break;
        case FIT_COMPLEX:
            printf("FIT_COMPLEX Array of FICOMPLEX: 2 x 64-bit IEEE floating point\n");
            break;
        case FIT_RGB16:
            printf("FIT_RGB16 48-bit RGB image: 3 x unsigned 16-bit\n");
            break;
        case FIT_RGBA16:
            printf("FIT_RGBA16 64-bit RGBA image: 4 x unsigned 16-bit\n");
            break;
        case FIT_RGBF:
            printf("FIT_RGBF 96-bit RGB float image: 3 x 32-bit IEEE floating point\n");
            break;
        case FIT_RGBAF:
            printf("FIT_RGBAF 128-bit RGBA float image: 4 x 32-bit IEEE floating point\n");
            break;
    }
}

//FIBITMAP * FreeImage_Saturate16(FIBITMAP *src, double percentage) {
bool FreeImage_Saturate16(FIBITMAP *src, double percentage) {
	double value;
	unsigned x, y;
	BYTE *bits = NULL;
    
	if(!FreeImage_HasPixels(src))
		return false;
	
	int bpp = FreeImage_GetBPP(src);
	int bytespp = FreeImage_GetLine(src) / FreeImage_GetWidth(src);

	//const double scale = (100 + percentage) / 100;
	
	double R,G,B;
	
	switch(bpp) {
		case 48:
			for(y = 0; y < FreeImage_GetHeight(src); y++) {
				//bits =  FreeImage_GetScanLine(src, y);
				FIRGB16 *bits = (FIRGB16 *)FreeImage_GetScanLine(src, y);
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					R = (double) bits[x].red;
					G = (double) bits[x].green;
					B = (double) bits[x].blue;
					changeSaturation(&R,&G,&B, percentage);
					if (R>65535.0) R=65535.0;
					if (G>65535.0) G=65535.0;
					if (B>65535.0) B=65535.0;
					if (R<0.0) R=0.0;
					if (G<0.0) G=0.0;
					if (B<0.0) B=0.0;
					bits[x].red = int(R);
					bits[x].green = int(G);
					bits[x].blue = int(B);
				}
			}
			break;
            
		case 24 :
			for(y = 0; y < FreeImage_GetHeight(src); y++) {
				bits =  FreeImage_GetScanLine(src, y);
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					R = (double) bits[FI_RGBA_RED];	// B
					G = (double) bits[FI_RGBA_GREEN];	// G
					B = (double) bits[FI_RGBA_BLUE];		// R
					changeSaturation(&R,&G,&B, percentage);
					if (R>255.0) R=255.0;
					if (G>255.0) G=255.0;
					if (B>255.0) B=255.0;
					if (R<0.0) R=0.0;
					if (G<0.0) G=0.0;
					if (B<0.0) B=0.0;
					bits[FI_RGBA_RED] = int(R);	// B
					bits[FI_RGBA_GREEN] = int(G);	// G
					bits[FI_RGBA_BLUE] =  int(B);
					
					bits += bytespp;
				}
			}
			break;
		default:
			return false;
			break;
	}
	return true;
}

bool FreeImage_Gray16(FIBITMAP *src, double Rpct, double Gpct, double Bpct) {
	double value;
	unsigned x, y;
	BYTE *bits = NULL;
    
	if(!FreeImage_HasPixels(src))
		return false;
	
	int bpp = FreeImage_GetBPP(src);
	int bytespp = FreeImage_GetLine(src) / FreeImage_GetWidth(src);

	double G;

	switch(bpp) {
		case 48:
			for(y = 0; y < FreeImage_GetHeight(src); y++) {
				//bits =  FreeImage_GetScanLine(src, y);
				FIRGB16 *bits = (FIRGB16 *)FreeImage_GetScanLine(src, y);
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					G = floor((double) bits[x].red*Rpct + (double) bits[x].green*Gpct + (double) bits[x].blue*Bpct)+0.5;
					if (G>65535.0) G=65535.0;
					if (G<0.0) G=0.0;
					bits[x].red = int(G);
					bits[x].green = int(G);
					bits[x].blue = int(G);
				}
			}
			break;
            
		case 24 :
			for(y = 0; y < FreeImage_GetHeight(src); y++) {
				bits =  FreeImage_GetScanLine(src, y);
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					G = floor((double) bits[FI_RGBA_RED]*Rpct + (double) bits[FI_RGBA_GREEN]*Gpct + (double) bits[FI_RGBA_BLUE]*Bpct)+0.5;
					if (G>255.0) G=255.0;
					if (G<0.0) G=0.0;
					bits[FI_RGBA_BLUE] = int(G);
					bits[FI_RGBA_GREEN] = int(G);
					bits[FI_RGBA_RED] =  int(G);
					
					bits += bytespp;
				}
			}
			break;
		default:
			return false;
			break;
	}
	return true;
}


bool FreeImage_GetPixelColor16(FIBITMAP *dib, unsigned x, unsigned y, FIRGB16 *value)
{

	int bpp = FreeImage_GetBPP(dib);
	//int bytespp = FreeImage_GetLine(dib) / FreeImage_GetWidth(dib);
	int bytespp = bpp/8;

	//unsigned width = FreeImage_GetWidth(dib);
	//unsigned height = FreeImage_GetHeight(dib);
	unsigned pitch = FreeImage_GetPitch(dib);

	BYTE *bits = (BYTE*)FreeImage_GetBits(dib);
	bits += (pitch*(y))+(x*(bytespp));

	//BYTE *bits = FreeImage_GetScanLine(dib, y);
	//bits += x*bytespp;

	FIRGB16 *pixel;
	switch(bpp) {
		case 48:
			pixel = (FIRGB16 *)bits;
			value->blue          = pixel->blue;   // B
			value->green         = pixel->green;  // G
			value->red           = pixel->red;    // R

			return true;
			break;
		case 24:
			value->blue          = bits[FI_RGBA_BLUE];   // B
			value->green         = bits[FI_RGBA_GREEN];  // G
			value->red           = bits[FI_RGBA_RED];    // R

			return true;
			break;
	}

	return false;
}

FIBITMAP * FreeImage_3x3Convolve16(FIBITMAP *src, double kernel[3][3], FIBITMAP *mask, int threshold)
{
	unsigned x, y;
	BYTE *bits = NULL;

	if(!FreeImage_HasPixels(src))
		return false;
	
	int bpp = FreeImage_GetBPP(src);
	//int bytespp = FreeImage_GetLine(src) / FreeImage_GetWidth(src);
	int bytespp = bpp/8;

	unsigned pitch = FreeImage_GetPitch(src);
	BYTE *dibbits = (BYTE*)FreeImage_GetBits(src);
	//bits = dibbits +(pitch*(y))+(x*(bytespp));

	double R, G, B;
	FIRGB16 value;
	//RGBQUAD value;
	FIBITMAP *dst = FreeImage_Clone(src);

	switch(bpp) {
		case 48:
			for(y = 1; y < FreeImage_GetHeight(src)-1; y++) {
				//bits =  FreeImage_GetScanLine(src, y);
				FIRGB16 *srcbits = (FIRGB16 *)FreeImage_GetScanLine(src, y);
				FIRGB16 *dstbits = (FIRGB16 *)FreeImage_GetScanLine(dst, y);
				BYTE * maskbits = (BYTE *)FreeImage_GetScanLine(mask,y);
				for(x = 1; x < FreeImage_GetWidth(src)-1; x++) {
					if (mask && (maskbits[0] < threshold)) {
						dstbits[x].red   = srcbits[x].red;
						dstbits[x].green = srcbits[x].green;
						dstbits[x].blue  = srcbits[x].blue;

					}
					else {
						R=0.0; G=0.0; B=0.0;
						for (int kx=0; kx<3; kx++) {
							for (int ky=0; ky<3; ky++) {
								int ix = x-1+kx;
								int iy = y-1+ky;

								FreeImage_GetPixelColor16(src, ix, iy, &value);
								//FIRGB16 *pixel = (FIRGB16 *) dibbits +(pitch*(iy))+(ix*(bytespp));

								R += value.red   * kernel[kx][ky];
								G += value.green * kernel[kx][ky];
								B += value.blue  * kernel[kx][ky];

							}
						}
						dstbits[x].red   = MIN(MAX(int(R), 0), 65535);
						dstbits[x].green = MIN(MAX(int(G), 0), 65535);
						dstbits[x].blue  = MIN(MAX(int(B), 0), 65535);
					}

				}
			}
			break;
            
		case 24 :

			for(y = 1; y < FreeImage_GetHeight(src)-1; y++) {
				//bits =  FreeImage_GetScanLine(src, y);
				BYTE *srcbits = FreeImage_GetScanLine(src, y);
				BYTE *dstbits = FreeImage_GetScanLine(dst, y);
				BYTE * maskbits = (BYTE *)FreeImage_GetScanLine(mask,y);
				for(x = 1; x < FreeImage_GetWidth(src)-1; x++) {
					if (mask && (maskbits[0] < threshold)) {
						dstbits[FI_RGBA_RED]   = srcbits[FI_RGBA_RED];
						dstbits[FI_RGBA_GREEN] = srcbits[FI_RGBA_GREEN];
						dstbits[FI_RGBA_BLUE]  = srcbits[FI_RGBA_BLUE];
					}
					else {
						R=0.0; G=0.0; B=0.0;
						for (int kx=0; kx<3; kx++) {
							for (int ky=0; ky<3; ky++) {
								int ix = x-1+kx;
								int iy = y-1+ky;
	
								FreeImage_GetPixelColor16(src, ix, iy, &value);
								//BYTE *pixel =  dibbits +(pitch*(iy))+(ix*(bytespp));
	
								R += value.red   * kernel[kx][ky];
								G += value.green * kernel[kx][ky];
								B += value.blue  * kernel[kx][ky];

							}
						}
						dstbits[FI_RGBA_RED]   = MIN(MAX(int(R), 0), 255);
						dstbits[FI_RGBA_GREEN] = MIN(MAX(int(G), 0), 255);
						dstbits[FI_RGBA_BLUE]  = MIN(MAX(int(B), 0), 255);
					}
					srcbits += 3;
					dstbits += 3;
					maskbits++;

				}
			}
			break;
		default:
			return NULL;
			break;
	}

	return dst;
}

