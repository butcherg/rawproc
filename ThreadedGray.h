#ifndef __THREADEDSGRAY_H__
#define __THREADEDGRAY_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif


#include <wx/thread.h>
#include "FreeImage.h"

class ThreadedGray : public wxThread
{
public:
	ThreadedGray(FIBITMAP *psrc, FIBITMAP *pdst, unsigned pstartrow, unsigned pincrement, double Rpct, double Gpct, double Bpct);
	~ThreadedGray();
	static void ApplyGray(FIBITMAP *src, FIBITMAP *dst, double redpct, double greenpct, double bluepct, int threadcount);


protected:
	virtual ExitCode Entry();

	FIBITMAP *src;
	FIBITMAP *dst;
	unsigned startrow;
	unsigned increment;
	double Rpct, Gpct, Bpct;

};


#endif