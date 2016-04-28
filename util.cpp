#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

#ifdef WIN32
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <wx/fileconf.h>
#include <wx/tokenzr.h>
#include <vector>
#include "FreeImage.h"
#include "ThreadedWxConvert.h"

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

//cross-platform duration:

#ifdef WIN32
timeval s,f;
timeval diff(timeval start, timeval end)
{
	timeval temp;
	if ((end.tv_usec-start.tv_usec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_usec = 1000000+end.tv_usec-start.tv_usec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_usec = end.tv_usec-start.tv_usec;
	}
	return temp;
}

void mark ()
{
	gettimeofday(&s, NULL);
}

wxString duration ()
{
	gettimeofday(&f, NULL);
	timeval d = diff(s,f);
	return wxString::Format("%ld.%ldsec", d.tv_sec, d.tv_usec);
}
#else
timespec s,f;
timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

void mark ()
{
	clock_gettime(CLOCK_MONOTONIC,&s);
}

wxString duration ()
{
	clock_gettime(CLOCK_MONOTONIC,&f);
	timespec d = diff(s,f);
	return wxString::Format("%ld.%ldsec", d.tv_sec, d.tv_nsec);
}
#endif

// end of cross-platform duration

//File logging:
void log(wxString msg)
{
	wxString logfile = wxConfigBase::Get()->Read("log.filename","");
	if (logfile == "") return;
	FILE * f = fopen(logfile.c_str(), "a");
	if (f) {
		fputs(wxNow().c_str(), f);
		fputs(" - ",f);
		fputs(msg,f);
		fputs("\n",f);
		fclose(f);
	}
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



wxBitmap HistogramFrom(wxImage img, int width, int height) 
{
	int hdata[256];
	int hmax = 0;
	wxBitmap bmp(width, height);  //, outbmp(width, height);
	for (int i=0; i<256; i++) hdata[i]=0;
	int iw = img.GetWidth();
	int ih = img.GetHeight();
	for (int i=0; i<256; i++) hdata[i] = 0;
	int gray;
	long pos;
	unsigned char *data = img.GetData();
	for (int x=0; x<iw; x++) {
		for (int y=0; y<ih; y++) {
			pos = (y * iw + x) * 3;
			gray = (data[pos]+data[pos+1]+data[pos+2]) / 3;
			hdata[gray]++;
			if (hdata[gray] > hmax) hmax = hdata[gray];
		}
	}
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

wxImage ThreadedFreeImage2wxImage(FIBITMAP* dib)
{
	mark();
	FIBITMAP *db = FreeImage_ConvertTo24Bits(dib);
	if (db == NULL) return NULL;
	wxImage img(FreeImage_GetWidth(db), FreeImage_GetHeight(db));
	unsigned char *data = img.GetData();

	std::vector<ThreadedWxConvert *> t;
	int threadcount = 1;
	wxConfigBase::Get()->Read("display.wxconvert.cores",&threadcount,0);
	if (threadcount == 0) threadcount = (long) wxThread::GetCPUCount();

	for (int i=0; i<threadcount; i++) {
		t.push_back(new ThreadedWxConvert(db, data, i,threadcount));
		t.back()->Run();
	}
	while (!t.empty()) {
		t.back()->Wait(wxTHREAD_WAIT_BLOCK);
		t.pop_back();
	}

	FreeImage_Unload(db);
	wxString d = duration();
	if (wxConfigBase::Get()->Read("display.wxconvert.log","0") == "1")
		log(wxString::Format("tool=wxconvert,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

	return img;
}


wxImage FreeImage2wxImage(FIBITMAP* dib)
{
mark();
int threadcount = 1;
	unsigned x, y;
	BYTE *bits = NULL;
	long pos;

	FIBITMAP *db = FreeImage_ConvertTo24Bits(dib);
	if (db == NULL) return NULL;
	unsigned h = FreeImage_GetHeight(db);
	unsigned w = FreeImage_GetWidth(db);
	wxImage img(w, h);
	unsigned char *data = img.GetData();
	int bytespp = FreeImage_GetLine(db) / FreeImage_GetWidth(db);

	unsigned dpitch = FreeImage_GetPitch(db);
	void * dstbits = FreeImage_GetBits(db);

	for(y = 0; y < h; y++) {
		bits = (BYTE *) dstbits + dpitch*y;
		pos = ((h-y-1) * w * 3);
		for(x = 0; x<w; x++) {
			//pos = ((h-y-1) * w + x) * 3;  //old pos computation
			data[pos]   = bits[FI_RGBA_RED]; 
			data[pos+1] = bits[FI_RGBA_GREEN]; 
			data[pos+2] = bits[FI_RGBA_BLUE];
			bits += bytespp;
			pos += 3;
		}
	}
	FreeImage_Unload(db);
wxString d = duration();
if (wxConfigBase::Get()->Read("display.wxconvert.log","0") == "1")
	log(wxString::Format("tool=wxconvert(old-noscanline3),imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

	return img;
}

unsigned hdata[256];
unsigned hmax;

wxImage FreeImage2wxImageAndHistogram(FIBITMAP* dib)
{
mark();
int threadcount = 1;
	unsigned x, y;
	hmax = 0;
	BYTE *bits = NULL;
	for (int i=0; i<=255; i++) hdata[i]=0;
	FIBITMAP *db = FreeImage_ConvertTo24Bits(dib);
	if (db == NULL) return NULL;
	unsigned h = FreeImage_GetHeight(db);
	unsigned w = FreeImage_GetWidth(db);
	wxImage img(w, h);
	long pos;
	unsigned char *data = img.GetData();
	int bytespp = FreeImage_GetLine(db) / FreeImage_GetWidth(db);
	int tmp; unsigned int gray;
	for(y = 0; y < h; y++) {
		bits =  FreeImage_GetScanLine(db, y);
		for(x = 0; x<w; x++) {
			pos = ((h-y-1) * w + x) * 3;
			data[pos] = bits[FI_RGBA_RED]; data[pos+1] = bits[FI_RGBA_GREEN]; data[pos+2] = bits[FI_RGBA_BLUE];
			gray = (BYTE) ((bits[FI_RGBA_RED]+bits[FI_RGBA_GREEN]+bits[FI_RGBA_BLUE])/3);
			hdata[gray]++;
			if (hdata[gray] > hmax) hmax = hdata[gray];
			bits += bytespp;
		}
	}
	FreeImage_Unload(db);
wxString d = duration();
if (wxConfigBase::Get()->Read("tool.wxconvert.log","0") == "1")
	log(wxString::Format("tool=wxconvert(old),imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

	return img;
}

wxBitmap HistogramFromData(int width, int height) 
{
	wxBitmap bmp(width, height);  //, outbmp(width, height);
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
