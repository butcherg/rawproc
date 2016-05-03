
#include "ThreadedCurve.h"
//#include "util.h"


ThreadedCurve::ThreadedCurve(FIBITMAP *psrc, FIBITMAP *pdst, unsigned pstartrow, unsigned pincrement, void * pLUT)
: wxThread(wxTHREAD_JOINABLE)
{
	src = psrc;
	dst = pdst;			//calling routine needs to create this with a FreeImage_Clone(psrc)
	startrow = pstartrow;
	increment = pincrement;
	LUT = pLUT;
}

ThreadedCurve::~ThreadedCurve() 
{ 

}

wxThread::ExitCode ThreadedCurve::Entry()
{
	unsigned x, y;
	BYTE * bsrcpix, * bdstpix;
	FIRGB16 * wsrcpix, * wdstpix;
	
	int bpp = FreeImage_GetBPP(src);

	unsigned spitch = FreeImage_GetPitch(src);
	unsigned dpitch = FreeImage_GetPitch(dst);
	void * srcbits = FreeImage_GetBits(src);
	void * dstbits = FreeImage_GetBits(dst);
	unsigned w = FreeImage_GetWidth(src);
	unsigned h = FreeImage_GetHeight(src);

	switch(bpp) {
		case 48:
			for(y = startrow; y < h; y+=increment) {
				for(x = 0; x < w; x++) {
					wdstpix = (FIRGB16 *) (dstbits + dpitch*y + 6*x);
					FIRGB16 * pixel   = (FIRGB16 *) (srcbits + spitch*y + 6*x);
					wdstpix->red   = ((WORD *) LUT)[pixel->red];
					wdstpix->green = ((WORD *) LUT)[pixel->green];
					wdstpix->blue  = ((WORD *) LUT)[pixel->blue];
				}
			}
			break;	            
		case 24 :
			for(y = startrow; y < h; y+=increment) {
				for(x = 0; x < w; x++) {
					bdstpix = (BYTE *) dstbits + dpitch*y + 3*x;
					BYTE *pixel = (BYTE *) (srcbits + spitch*y + 3*x);
					bdstpix[FI_RGBA_RED]   = ((BYTE *) LUT)[pixel[FI_RGBA_RED]];
					bdstpix[FI_RGBA_GREEN] = ((BYTE *) LUT)[pixel[FI_RGBA_GREEN]];
					bdstpix[FI_RGBA_BLUE]  = ((BYTE *) LUT)[pixel[FI_RGBA_BLUE]];
				}
			}
			break;
	}

	return (wxThread::ExitCode)0;
}

void ThreadedCurve::ApplyCurve(FIBITMAP *src, FIBITMAP *dst, std::vector<cp> ctpts, int threadcount)
{
	//int threadcount;
	std::vector<ThreadedCurve *> t;
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
		for (int i=0; i<threadcount; i++) {
			t.push_back(new ThreadedCurve(src, dst, i,threadcount, LUT8));
			t.back()->Run();
		}
		while (!t.empty()) {
			t.back()->Wait(wxTHREAD_WAIT_BLOCK);
			t.pop_back();
		}
	}
	if (bpp == 48) {
		c.scalepoints(256.0);
		c.clampto(0.0,65535.0);
		for (int x=0; x<65536; x++) {
			LUT16[x] = (WORD)floor(c.getpoint(x) + 0.5);
		}
		for (int i=0; i<threadcount; i++) {
			t.push_back(new ThreadedCurve(src, dst, i,threadcount, LUT16));
			t.back()->Run();
		}
		while (!t.empty()) {
			t.back()->Wait(wxTHREAD_WAIT_BLOCK);
			t.pop_back();
		}
	}
}
