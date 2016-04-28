#ifndef __THREADEDSSATURATE_H__
#define __THREADEDSATURATE_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif


#include <wx/thread.h>
#include "FreeImage.h"

#define  Pr  .299
#define  Pg  .587
#define  Pb  .114

class ThreadedSaturate : public wxThread
{
public:
	ThreadedSaturate(FIBITMAP *psrc, FIBITMAP *pdst, unsigned pstartrow, unsigned pincrement, double psaturate);
	~ThreadedSaturate();

protected:
	virtual ExitCode Entry();

	FIBITMAP *src;
	FIBITMAP *dst;
	unsigned startrow;
	unsigned increment;
	double percent;

};


#endif