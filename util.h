#ifndef __UTIL_H__
#define __UTIL_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

#include <vector>
#include "FreeImage.h"

wxArrayString split(wxString str, wxString delim);

wxBitmap HistogramFrom(wxImage img, int width, int height);

wxBitmap HistogramFromVec(std::vector<int> hdata, int hmax, int width, int height);

wxString MetadataString(const char *sectionTitle, FIBITMAP *dib, FREE_IMAGE_MDMODEL model);

wxString FreeImage_Information(FIBITMAP *dib);

#endif


