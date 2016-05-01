#ifndef __THREADEDCURVE_H__
#define __THREADEDCURVE_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif


#include <wx/thread.h>
#include "FreeImage.h"


class ThreadedCurve : public wxThread
{
public:
	ThreadedCurve(FIBITMAP *psrc, FIBITMAP *pdst, unsigned pstartrow, unsigned pincrement, void *pLUT);
	~ThreadedCurve();

protected:
	virtual ExitCode Entry();

	FIBITMAP *src;
	FIBITMAP *dst;
	unsigned startrow;
	unsigned increment;
	void *LUT;

};


#endif