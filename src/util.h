#ifndef __UTIL_H__
#define __UTIL_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

#include <vector>
#include "gimage/gimage.h"


void FillHistogram(unsigned *histogram);

bool ImageContainsRawprocCommand(wxString fname);

wxArrayString split(wxString str, wxString delim);
wxString paramString(wxString filter);
wxString rawParamString(wxString filter);
wxArrayString paramList(wxString filter);

void paramAppend(wxString name, wxString value, wxString &paramstring);
wxArrayString paramSplit(wxString paramstring);
void opAppend(wxString name, wxString &opstring);

wxBitmap HistogramFrom(wxImage img, int width, int height);

wxBitmap HistogramFromVec(std::vector<int> hdata, int hmax, int width, int height);

//wxString MetadataString(const char *sectionTitle, FIBITMAP *dib, FREE_IMAGE_MDMODEL model);

//wxString FreeImage_Information(FIBITMAP *dib);

//wxImage FreeImage2wxImage(FIBITMAP* dib);

wxBitmap ThreadedHistogramFrom(wxImage img, int width, int height);
//wxImage ThreadedFreeImage2wxImage(FIBITMAP* dib);

//use these together:
//wxImage FreeImage2wxImageAndHistogram(FIBITMAP* dib);
wxBitmap HistogramFromData(int width, int height);

wxImage gImage2wxImage(gImage &dib);
wxImage gImage2wxImage(gImage &dib, int oob);

//cross-platform duration:
void mark ();
wxString duration ();


//Logging to file:
void log(wxString msg);

//RAW flag/command translation:
//int Command2RawFlags(wxString cmd);
//wxString RawFlags2Command(int flags);

#endif


