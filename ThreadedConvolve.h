#ifndef __THREADEDCONVOLVE_H__
#define __THREADEDCONVOLVE_H__


#include <wx/thread.h>
#include "FreeImage.h"


class ThreadedConvolve : public wxThread
{
public:
	ThreadedConvolve(FIBITMAP *src, FIBITMAP *dst, unsigned startrow, unsigned increment, double kernel[3][3]);
	~ThreadedConvolve();
protected:
	virtual ExitCode Entry();

	FIBITMAP *src;
	FIBITMAP *dst;
	unsigned startrow;
	unsigned increment;
	double kernel[3][3];

};


#endif