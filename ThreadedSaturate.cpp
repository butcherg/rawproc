
#include "ThreadedSaturate.h"
#include "FreeImage16.h"
#include "util.h"


ThreadedSaturate::ThreadedSaturate(FIBITMAP *psrc, FIBITMAP *pdst, unsigned pstartrow, unsigned pincrement, double psaturate)
: wxThread(wxTHREAD_JOINABLE)
{
	src = psrc;
	dst = pdst;			//calling routine needs to create this with a FreeImage_Clone(psrc)
	startrow = pstartrow;
	increment = pincrement;
	percent = psaturate;
}

ThreadedSaturate::~ThreadedSaturate() 
{ 

}

wxThread::ExitCode ThreadedSaturate::Entry()
{
	unsigned x, y;
	BYTE * bsrcpix, * bdstpix;
	FIRGB16 * wsrcpix, * wdstpix;
	BYTE *bits = NULL;
	
	int bpp = FreeImage_GetBPP(src);
	int bytespp = bpp/8;

	unsigned pitch = FreeImage_GetPitch(src);
	BYTE *dibbits = (BYTE*)FreeImage_GetBits(src);
	//bits = dibbits +(pitch*(y))+(x*(bytespp));

	unsigned spitch = FreeImage_GetPitch(src);
	unsigned dpitch = FreeImage_GetPitch(dst);
	void * srcbits = FreeImage_GetBits(src);
	void * dstbits = FreeImage_GetBits(dst);

	double R, G, B;
	FIRGB16 value;

	switch(bpp) {
		case 48:
			for(y = startrow; y < FreeImage_GetHeight(src); y+=increment) {
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					wdstpix = (FIRGB16 *) (dstbits + dpitch*y + 6*x);
					FIRGB16 * pixel   = (FIRGB16 *) (srcbits + spitch*y + 6*x);

					R = (double) pixel->red;
					G = (double) pixel->green;
					B = (double) pixel->blue;

					double  P=sqrt(
					R*R*Pr+
					G*G*Pg+
					B*B*Pb ) ;

					R=P+(R-P)*percent;
					G=P+(G-P)*percent;
					B=P+(B-P)*percent;

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
			for(y = startrow; y < FreeImage_GetHeight(src); y+=increment) {
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					bdstpix = (BYTE *) dstbits + dpitch*y + 3*x;
					BYTE *pixel = (BYTE *) (srcbits + spitch*y + 3*x);

					R = (double) pixel[FI_RGBA_RED];
					G = (double) pixel[FI_RGBA_GREEN];
					B = (double) pixel[FI_RGBA_BLUE];

					double  P=sqrt(
					R*R*Pr+
					G*G*Pg+
					B*B*Pb ) ;

					R=P+(R-P)*percent;
					G=P+(G-P)*percent;
					B=P+(B-P)*percent;

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

	return (wxThread::ExitCode)0;
}
