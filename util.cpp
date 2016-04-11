#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

#include <wx/tokenzr.h>
#include <vector>
#include "FreeImage.h"

wxString hstr="";

wxArrayString split(wxString str, wxString delim)
{
	wxArrayString a;
	wxStringTokenizer tokenizer(str, delim);
	while ( tokenizer.HasMoreTokens() ) {
		wxString token = tokenizer.GetNextToken();
		a.Add(token);
	}
	return a;
}


wxBitmap HistogramFrom(wxImage img, int width, int height) 

{
	int hdata[256];
	int hmax = 10000;
	wxBitmap bmp(width, height);  //, outbmp(width, height);
	for (int i=0; i<256; i++) hdata[i]=0;
	int iw = img.GetWidth();
	int ih = img.GetHeight();
	for (int i=0; i<256; i++) hdata[i] = 0;
	for (int x=0; x<iw; x++) {
		for (int y=0; y<ih; y++) {
			int gray = floor(0.21 * (double) img.GetRed(x,y) + 0.72 * (double) img.GetGreen(x,y) + 0.07 * (double) img.GetBlue(x,y))+0.5;
			hdata[gray]++;
			if (hdata[gray] > hmax) hmax = hdata[gray];
		}
	}
	//for (int i=0; i<256; i++) hstr.Append(wxString::Format("%d,",hdata[i]));
	wxMemoryDC dc;
	dc.SelectObject(bmp);
	dc.Clear();
	dc.SetUserScale((double) width / 256.0, (double) height/ (double) hmax);
	dc.SetPen(wxPen(wxColour(128,128,128),1));
	for(int x=0; x<256; x++) {
		dc.DrawLine(x,dc.DeviceToLogicalY(height),x,dc.DeviceToLogicalY(height)-hdata[x]);
	}

	dc.SelectObject(wxNullBitmap);
	return bmp;
}


wxBitmap HistogramFromVec(std::vector<int> hdata, int hmax, int width, int height) 
{
	wxBitmap bmp(width, height); 
	wxMemoryDC dc;
	dc.SelectObject(bmp);
	dc.Clear();
	dc.SetUserScale((double) width / (double) hdata.size(), (double) height/ (double) hmax);
	dc.SetPen(wxPen(wxColour(128,128,128),1));
	for(int x=0; x<hdata.size(); x++) {
		dc.DrawLine(x,dc.DeviceToLogicalY(height),x,dc.DeviceToLogicalY(height)-hdata[x]);
	}

	dc.SelectObject(wxNullBitmap);
	return bmp;
}

wxString MetadataString(const char *sectionTitle, FIBITMAP *dib, FREE_IMAGE_MDMODEL model) {
	FITAG *tag = NULL;
	FIMETADATA *mdhandle = NULL;
	const char * binary = "(binary)";
	wxString exif = "";

	mdhandle = FreeImage_FindFirstMetadata(model, dib, &tag);
	if(mdhandle) {
		do {
			// convert the tag value to a string
			const char *value = FreeImage_TagToString(model, tag);

			if ( FreeImage_GetTagType(tag) == FIDT_UNDEFINED) 
				if ( ! (strcmp(FreeImage_GetTagKey(tag),"UserComment") == 0))
					value = binary;
			//exif.Append(wxString::Format("%s: %s\n",FreeImage_GetTagKey(tag), value));
			exif.Append(wxString::Format("%s:%s\n",FreeImage_GetTagKey(tag), value));

		} while(FreeImage_FindNextMetadata(mdhandle, &tag));
	}

	FreeImage_FindCloseMetadata(mdhandle);
	return exif;
}

wxString FreeImage_Information(FIBITMAP *dib)
{
	FREE_IMAGE_TYPE t = FreeImage_GetImageType(dib);
	wxString info = "";
    switch (t) {
        case FIT_UNKNOWN:
            info.Append("FIT_UNKNOWN: Unknown format (returned value only, never use it as input value)\n");
            break;
        case FIT_BITMAP:
            info.Append("FIT_BITMAP: Standard image: 1-, 4-, 8-, 16-, 24-, 32-bit\n");
            break;
        case FIT_UINT16:
            info.Append("FIT_UINT16: Array of unsigned short: unsigned 16-bit\n");
            break;
        case FIT_INT16:
            info.Append("FIT_INT16: Array of short: signed 16-bit\n");
            break;
        case FIT_UINT32:
            info.Append("FIT_UINT32: Array of unsigned long: unsigned 32-bit\n");
            break;
        case FIT_INT32:
            info.Append("FIT_INT32: Array of long: signed 32-bit\n");
            break;
        case FIT_FLOAT:
            info.Append("FIT_FLOAT: Array of float: 32-bit IEEE floating point\n");
            break;
        case FIT_DOUBLE:
            info.Append("FIT_DOUBLE Array of double: 64-bit IEEE floating point\n");
            break;
        case FIT_COMPLEX:
            info.Append("FIT_COMPLEX Array of FICOMPLEX: 2 x 64-bit IEEE floating point\n");
            break;
        case FIT_RGB16:
            info.Append("FIT_RGB16 48-bit RGB image: 3 x unsigned 16-bit\n");
            break;
        case FIT_RGBA16:
            info.Append("FIT_RGBA16 64-bit RGBA image: 4 x unsigned 16-bit\n");
            break;
        case FIT_RGBF:
            info.Append("FIT_RGBF 96-bit RGB float image: 3 x 32-bit IEEE floating point\n");
            break;
        case FIT_RGBAF:
            info.Append("FIT_RGBAF 128-bit RGBA float image: 4 x 32-bit IEEE floating point\n");
            break;
    }
	info.Append(wxString::Format("Width: %d\nHeight %d\nBits Per Pixel: %d\n", FreeImage_GetWidth(dib), FreeImage_GetHeight(dib), FreeImage_GetBPP(dib)));

	wxString exif = "";
	exif.Append(MetadataString("Exif-Main", dib, FIMD_EXIF_MAIN));
	exif.Append(MetadataString("Exif-Advanced", dib, FIMD_EXIF_EXIF));
	wxArrayString exifarray = split(exif, "\n");
	for (int i=0; i<exifarray.size(); i++) {
		wxArrayString exifline = split(exifarray[i], ":");
		if (exifline[0] == "DateTime") info.Append(wxString::Format("Date/Time: %s-%s-%s:%s:%s\n", exifline[1],exifline[2],exifline[3],exifline[4],exifline[5]));
		if (exifline[0] == "ExposureTime") {
			wxArrayString foo = split(exifline[1], " ");
			wxArrayString bar = split(foo[0], "/");
			info.Append(wxString::Format("Shutter Speed: 1/%d\n", atoi(bar[1])/atoi(bar[0])));
		}
		if (exifline[0] == "FNumber") info.Append(wxString::Format("Aperture: %s\n", exifline[1]));
		if (exifline[0] == "ISOSpeedRatings") info.Append(wxString::Format("ISO: %s\n", exifline[1]));
		if (exifline[0] == "FocalLength") info.Append(wxString::Format("Focal Length: %s\n", exifline[1]));

	}
	return info;
}


/*
Comment:rawproc-0.1 DSG_3890.NEF gamma:2.2  curve:16.0,0.0,64.0,109.0,131.0,194.0,220.0,256.0  saturation:17  
Artist:Glenn Butcher                       
DateTime:2015:10:23 22:05:11
ImageDescription:
Make:Nikon
Model:D7000
Orientation:top, left side
PlanarConfiguration:1
ResolutionUnit:inches
Software:dcraw v9.24
XResolution:300
YResolution:300
ExposureTime:2857/1000000 sec
FNumber:F11.0
FocalLength:44.0 mm
ISOSpeedRatings:200
*/
