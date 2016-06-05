
#include "ThreadedGray.h"
#include "FreeImage16.h"
#include "util.h"


ThreadedGray::ThreadedGray(FIBITMAP *psrc, FIBITMAP *pdst, unsigned pstartrow, unsigned pincrement, double redpct, double greenpct, double bluepct)
: wxThread(wxTHREAD_JOINABLE)
{
	src = psrc;
	dst = pdst;			//calling routine needs to create this with a FreeImage_Clone(psrc)
	startrow = pstartrow;
	increment = pincrement;
	Rpct = redpct;
	Gpct = greenpct;
	Bpct = bluepct;
}

ThreadedGray::~ThreadedGray() 
{ 

}

wxThread::ExitCode ThreadedGray::Entry()
{
	unsigned x, y;
	BYTE * bsrcpix, * bdstpix;
	FIRGB16 * wsrcpix, * wdstpix;
	BYTE *bits = NULL;
	
	int bpp = FreeImage_GetBPP(src);
	int bytespp = bpp/8;

	unsigned pitch = FreeImage_GetPitch(src);
	BYTE *dibbits = (BYTE*)FreeImage_GetBits(src);

	unsigned spitch = FreeImage_GetPitch(src);
	unsigned dpitch = FreeImage_GetPitch(dst);
	void * srcbits = FreeImage_GetBits(src);
	void * dstbits = FreeImage_GetBits(dst);

	double G;
	FIRGB16 value;

	switch(bpp) {
		case 48:
			for(y = startrow; y < FreeImage_GetHeight(src); y+=increment) {
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					wdstpix = (FIRGB16 *) (dstbits + dpitch*y + 6*x);
					FIRGB16 * pixel   = (FIRGB16 *) (srcbits + spitch*y + 6*x);

					G = floor((double) pixel->red*Rpct + (double) pixel->green*Gpct + (double) pixel->blue*Bpct)+0.5;
					if (G>65535.0) G=65535.0;
					if (G<0.0) G=0.0;

					wdstpix->red = int(G);
					wdstpix->green = int(G);
					wdstpix->blue = int(G);
				}
			}
			break;	            
		case 24 :
			for(y = startrow; y < FreeImage_GetHeight(src); y+=increment) {
				for(x = 0; x < FreeImage_GetWidth(src); x++) {
					bdstpix = (BYTE *) dstbits + dpitch*y + 3*x;
					BYTE *pixel = (BYTE *) (srcbits + spitch*y + 3*x);

					G = floor((double) pixel[FI_RGBA_RED]*Rpct + (double) pixel[FI_RGBA_GREEN]*Gpct + (double) pixel[FI_RGBA_BLUE]*Bpct)+0.5;
					if (G>255.0) G=255.0;
					if (G<0.0) G=0.0;

					bdstpix[FI_RGBA_RED] = int(G);
					bdstpix[FI_RGBA_GREEN] = int(G);
					bdstpix[FI_RGBA_BLUE]= int(G);
				}
			}
			break;
	}

	return (wxThread::ExitCode)0;
}

void ThreadedGray::ApplyGray(FIBITMAP *src, FIBITMAP *dst, double redpct, double greenpct, double bluepct, int threadcount)
{
	std::vector<ThreadedGray *> t;
	int bpp = FreeImage_GetBPP(src);
	if (bpp == 24) {
		for (int i=0; i<threadcount; i++) {
			t.push_back(new ThreadedGray(src, dst, i,threadcount, redpct, greenpct, bluepct));
			t.back()->Run();
		}
		while (!t.empty()) {
			t.back()->Wait(wxTHREAD_WAIT_BLOCK);
			delete t.back();
			t.pop_back();
		}
	}
	if (bpp == 48) {
		for (int i=0; i<threadcount; i++) {
			t.push_back(new ThreadedGray(src, dst, i,threadcount, redpct, greenpct, bluepct));
			t.back()->Run();
		}
		while (!t.empty()) {
			t.back()->Wait(wxTHREAD_WAIT_BLOCK);
			delete t.back();
			t.pop_back();
		}
	}
}


