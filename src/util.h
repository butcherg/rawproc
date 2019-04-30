#ifndef __UTIL_H__
#define __UTIL_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

#include <vector>
#include "gimage/gimage.h"

#ifdef WIN32
#define TEXTCTRLHEIGHT 20
#else
#define TEXTCTRLHEIGHT 30
#endif


bool ImageContainsRawprocCommand(wxString fname);

wxArrayString split(wxString str, wxString delim);
wxString paramString(wxString filter);
wxString rawParamString(wxString filter);
wxArrayString paramList(wxString filter);


void paramAppend(wxString name, wxString value, wxString &paramstring);
wxArrayString paramSplit(wxString paramstring);
void opAppend(wxString name, wxString &opstring);


void FillHistogram(unsigned *histogram);
wxBitmap HistogramFrom(wxImage img, int width, int height);
wxBitmap HistogramFromVec(std::vector<int> hdata, int hmax, int width, int height);
wxBitmap ThreadedHistogramFrom(wxImage img, int width, int height);
wxBitmap HistogramFromData(int width, int height);

wxImage gImage2wxImage(gImage &dib);
wxImage gImage2wxImage(gImage &dib, int oob);
wxImage gImage2wxImage(gImage &dib, cmsHTRANSFORM transform, int oob);

wxColour wxString2wxColour(wxString s);

//cross-platform duration:
void mark ();
wxString duration ();
float durationf();


//Logging to file:
void log(wxString msg);

//RAW flag/command translation:
//int Command2RawFlags(wxString cmd);
//wxString RawFlags2Command(int flags);

#endif


