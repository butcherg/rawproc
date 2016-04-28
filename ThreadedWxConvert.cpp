
#include <wx/fileconf.h>
#include "ThreadedWxConvert.h"
#include "util.h"


ThreadedWxConvert::ThreadedWxConvert(FIBITMAP *psrc, unsigned char *pdst, unsigned pstartrow, unsigned pincrement)
: wxThread(wxTHREAD_JOINABLE)
{
	db = psrc;
	data = pdst;			//calling routine needs to create this with a FreeImage_Clone(psrc)
	startrow = pstartrow;
	increment = pincrement;
}

ThreadedWxConvert::~ThreadedWxConvert() 
{ 

}

wxThread::ExitCode ThreadedWxConvert::Entry()
{
	unsigned x, y;
	BYTE *bits = NULL;
	long pos;

	unsigned dpitch = FreeImage_GetPitch(db);
	void * dstbits = FreeImage_GetBits(db);
	unsigned h = FreeImage_GetHeight(db);
	unsigned w = FreeImage_GetWidth(db);
	int bytespp = FreeImage_GetLine(db) / w;


	for(y = startrow; y < h; y+=increment) {
		bits = (BYTE *) dstbits + dpitch*y;
		pos = ((h-y-1) * w * 3);
		for(x = 0; x < w; x++) {
			//pos = ((h-y-1) * w + x) * 3;  //old pos computation
			data[pos]   = bits[FI_RGBA_RED]; 
			data[pos+1] = bits[FI_RGBA_GREEN]; 
			data[pos+2] = bits[FI_RGBA_BLUE];
			bits += bytespp;
			pos += 3;
		}
	}

	return (wxThread::ExitCode)0;
}
