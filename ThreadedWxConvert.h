#ifndef __THREADEDWXCONVERT_H__
#define __THREADEDWXCONVERT_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif


#include <wx/thread.h>
#include "FreeImage.h"

class ThreadedWxConvert : public wxThread
{
public:
	ThreadedWxConvert(FIBITMAP *psrc, unsigned char *pdst, unsigned pstartrow, unsigned pincrement);
	~ThreadedWxConvert();

protected:
	virtual ExitCode Entry();

	FIBITMAP *db;
	unsigned char *data;
	unsigned startrow;
	unsigned increment;

};


#endif