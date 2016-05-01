
#include "ThreadedWxHistogram.h"
//#include "util.h"


ThreadedWxHistogram::ThreadedWxHistogram(unsigned char* pdata, unsigned pstartrow, unsigned pincrement, unsigned pw, unsigned ph)
: wxThread(wxTHREAD_JOINABLE)
{
	data = pdata;
	w = pw;
	h = ph;	
	startrow = pstartrow;
	increment = pincrement;
	for (int i=0; i<256; i++) hdata[i] = 0;
}

ThreadedWxHistogram::~ThreadedWxHistogram() 
{ 

}

wxThread::ExitCode ThreadedWxHistogram::Entry()
{
	unsigned x, y;
	long pos;
	unsigned gray;

	for(y = startrow; y < h; y+=increment) {
		for(x = 0; x < w; x++) {
			pos = (y * w + x) * 3;
			gray = (data[pos]+data[pos+1]+data[pos+2]) / 3;
			hdata[gray]++;
		}
	}

	return (wxThread::ExitCode)0;
}

void ThreadedWxHistogram::addData(unsigned * histogramdata)
{
	for (int i=0; i<256; i++)
		histogramdata[i] += hdata[i];
}
