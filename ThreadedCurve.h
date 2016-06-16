#ifndef __THREADEDCURVE_H__
#define __THREADEDCURVE_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include <vector>
#include <wx/thread.h>
#include "Curve.h"
#include "FreeImage.h"


class ThreadedCurve : public wxThread
{
public:
	ThreadedCurve(FIBITMAP *psrc, FIBITMAP *pdst, unsigned pstartrow, unsigned pincrement, void *pLUT);
	~ThreadedCurve();
	static void ApplyCurve(FIBITMAP *psrc, FIBITMAP *pdst, std::vector<cp> ctpts, int threadcount);
	static void ApplyLUT(FIBITMAP *src, FIBITMAP *dst, void * pLUT, int threadcount);

protected:
	virtual ExitCode Entry();

	FIBITMAP *src;
	FIBITMAP *dst;
	unsigned startrow;
	unsigned increment;
	void *LUT;

};


#endif