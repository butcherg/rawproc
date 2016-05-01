#ifndef __THREADEDWXHISTOGRAM_H__
#define __THREADEDWXHISTOGRAM_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif


#include <wx/thread.h>
//#include "FreeImage.h"


class ThreadedWxHistogram : public wxThread
{
public:
	ThreadedWxHistogram(unsigned char* pdata, unsigned pstartrow, unsigned pincrement, unsigned pw, unsigned ph);
	~ThreadedWxHistogram();
	void addData(unsigned * histogramdata);

protected:
	virtual ExitCode Entry();
	
	unsigned w, h;
	unsigned char *data;
	unsigned startrow;
	unsigned increment;
	unsigned hdata[256];
	unsigned hmax;

};


#endif