#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

#include "elapsedtime.h"
#include <omp.h>
#include <lcms2.h>

#ifdef WIN32
#include <sys/time.h>
#else
#include <time.h>
#endif

#include <wx/fileconf.h>
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



void mark ()
{
	_mark();
}

wxString duration ()
{
	return wxString::Format("%fsec", _duration());
}



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

wxBitmap ThreadedHistogramFrom(wxImage img, int width, int height) 
{
	mark();
	unsigned hdata[256];
	int hmax = 0;
	wxBitmap bmp(width, height);  //, outbmp(width, height);
	for (int i=0; i<256; i++) hdata[i]=0;
	int iw = img.GetWidth();
	int ih = img.GetHeight();
	for (int i=0; i<256; i++) hdata[i] = 0;
	int gray;
	long pos;
	unsigned char *data = img.GetData();

	int threadcount = 1;
	wxConfigBase::Get()->Read("display.wxhistogram.cores",&threadcount,0);
	if (threadcount == 0)
#if defined(_OPENMP)
		threadcount = (long) omp_get_max_threads();
#else
		threadcount = 1;
#endif

	#pragma omp parallel
	{
		unsigned pdata[256] = {0};
		#pragma omp for
		for(unsigned y = 0; y < ih; y++) {
			for(unsigned x = 0; x < iw; x++) {
				long pos = (y * iw + x) * 3;
				int gray = (data[pos]+data[pos+1]+data[pos+2]) / 3;
				pdata[gray]++;
			}
		}

		#pragma omp critical 
		{
			for (unsigned i=0; i<256; i++) {
				hdata[i] += pdata[i];
			}
		}

	}


	for (int i=0; i<256; i++) if (hdata[i]>hmax) hmax = hdata[i];

	wxMemoryDC dc;
	dc.SelectObject(bmp);
	dc.Clear();
	dc.SetUserScale((double) width / 256.0, (double) height/ (double) hmax);
	dc.SetPen(wxPen(wxColour(128,128,128),1));
	for(int x=0; x<256; x++) {
		dc.DrawLine(x,dc.DeviceToLogicalY(height),x,dc.DeviceToLogicalY(height)-hdata[x]);
	}

	dc.SelectObject(wxNullBitmap);
	wxString d = duration();
	if ((wxConfigBase::Get()->Read("display.all.log","0") == "1") || (wxConfigBase::Get()->Read("display.wxhistogram.log","0") == "1"))
		log(wxString::Format("tool=wxhistogram,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",iw, ih,24,threadcount,d));

	return bmp;
}

wxBitmap HistogramFrom(wxImage img, int width, int height) 
{
	mark();
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
	wxString d = duration();
	if ((wxConfigBase::Get()->Read("display.all.log","0") == "1") || (wxConfigBase::Get()->Read("display.wxhistogram.log","0") == "1"))
		log(wxString::Format("tool=wxhistogram(old),imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",iw, ih,24,1,d));

	return bmp;
}

wxImage ThreadedFreeImage2wxImage(FIBITMAP* dib)
{
	mark();
	FIBITMAP *db = FreeImage_ConvertTo24Bits(dib);
	if (db == NULL) return NULL;
	wxImage img(FreeImage_GetWidth(db), FreeImage_GetHeight(db));
	unsigned char *data = img.GetData();

//	std::vector<ThreadedWxConvert *> t;
	int threadcount = 1;
	wxConfigBase::Get()->Read("display.wxconvert.cores",&threadcount,0);
	if (threadcount == 0) 
#if defined(_OPENMP)
		threadcount = (long) omp_get_max_threads();
#else
		threadcount = 1;
#endif

	unsigned dpitch = FreeImage_GetPitch(db);
	void * dstbits = FreeImage_GetBits(db);
	unsigned h = FreeImage_GetHeight(db);
	unsigned w = FreeImage_GetWidth(db);
	int bytespp = FreeImage_GetLine(db) / w;

	#pragma omp parallel for
	for(unsigned y = 0; y < h; y++) {
		BYTE *bits = (BYTE *) dstbits + dpitch*y;
		long pos = ((h-y-1) * w * 3);
		for(unsigned x = 0; x < w; x++) {
			data[pos]   = bits[FI_RGBA_RED]; 
			data[pos+1] = bits[FI_RGBA_GREEN]; 
			data[pos+2] = bits[FI_RGBA_BLUE];
			bits += bytespp;
			pos += 3;
		}
	}

	FreeImage_Unload(db);
	wxString d = duration();
	if ((wxConfigBase::Get()->Read("display.all.log","0") == "1") || (wxConfigBase::Get()->Read("display.wxconvert.log","0") == "1"))
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
	if ((wxConfigBase::Get()->Read("display.all.log","0") == "1") || (wxConfigBase::Get()->Read("display.wxconvert.log","0") == "1"))
		log(wxString::Format("tool=wxconvert,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),1,d));

	return img;
}

unsigned hdata[256];
unsigned hmax;

void FillHistogram(unsigned *histogram)
{
	for (int i=0; i<256; i++) histogram[i] = hdata[i];
}

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
	if ((wxConfigBase::Get()->Read("display.all.log","0") == "1") || (wxConfigBase::Get()->Read("display.wxconvert.log","0") == "1"))
	log(wxString::Format("tool=wxconvert,imagesize=%dx%d,imagebpp=%d,threads=%d,time=%s",FreeImage_GetWidth(dib), FreeImage_GetHeight(dib),FreeImage_GetBPP(dib),threadcount,d));

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

bool ImageContainsRawprocCommand(wxString fname)
{
	FIBITMAP *srcdib;
	FREE_IMAGE_FORMAT fif;
	fif = FreeImage_GetFileType(fname, 0);
	if(fif != FIF_UNKNOWN) {
		srcdib = FreeImage_Load(fif, fname, FIF_LOAD_NOPIXELS);
		FITAG *tagSource = NULL;
		if (fif == FIF_TIFF)
			FreeImage_GetMetadata(FIMD_EXIF_MAIN, srcdib, "ImageDescription", &tagSource);
		else
			FreeImage_GetMetadata(FIMD_COMMENTS, srcdib, "Comment", &tagSource);
		if(tagSource != NULL) {
			wxString script = (char *) FreeImage_GetTagValue(tagSource);
			wxArrayString token = split(script, " ");
			if (token[0].Find("rawproc") != wxNOT_FOUND) return true;
		}
	}
	return false;
}

wxString RawFlags2Command(int flags)
{
	wxString cmd = "";

	if ((flags & RAW_DEFAULT) == RAW_DEFAULT)    cmd.Append("demosaic");
	else if ((flags & RAW_PREVIEW) == RAW_PREVIEW)  cmd.Append("preview");
	else if ((flags & RAW_DISPLAY) == RAW_DISPLAY) cmd.Append("display");
	else if ((flags & RAW_HALFSIZE) == RAW_HALFSIZE) cmd.Append("half");
	else if ((flags & RAW_UNPROCESSED) == RAW_UNPROCESSED) cmd.Append("unprocessed");

#ifdef CUSTOM_FREEIMAGE
	if ((flags & RAW_COLOR_RAW) == RAW_COLOR_RAW)    cmd.Append(",raw");
	else if ((flags & RAW_COLOR_SRGB) == RAW_COLOR_SRGB)  cmd.Append(",srgb");
	else if ((flags & RAW_COLOR_ADOBE) == RAW_COLOR_ADOBE) cmd.Append(",adobe");
	else if ((flags & RAW_COLOR_WIDE) == RAW_COLOR_WIDE) cmd.Append(",wide");
	else if ((flags & RAW_COLOR_PROPHOTO) == RAW_COLOR_PROPHOTO) cmd.Append(",prophoto");
	else if ((flags & RAW_COLOR_XYZ) == RAW_COLOR_XYZ) cmd.Append(",xyz");

	if ((flags & RAW_QUAL_LINEAR) == RAW_QUAL_LINEAR)    cmd.Append(",linear");
	else if ((flags & RAW_QUAL_VNG) == RAW_QUAL_VNG)  cmd.Append(",vng");
	else if ((flags & RAW_QUAL_PPG) == RAW_QUAL_PPG) cmd.Append(",ppg");
	else if ((flags & RAW_QUAL_AHD) == RAW_QUAL_AHD) cmd.Append(",ahd");
#endif

	return cmd;
}

int Command2RawFlags(wxString cmd)
{
	int flags = 0;
	wxArrayString commands = split(cmd,",");

	if (commands[0].Cmp("demosaic")==0) flags = flags | RAW_DEFAULT;
	if (commands[0].Cmp("preview")==0) flags = flags | RAW_PREVIEW;
	if (commands[0].Cmp("half")==0) flags = flags | RAW_HALFSIZE;
	if (commands[0].Cmp("display")==0) flags = flags | RAW_DISPLAY;
	if (commands[0].Cmp("unprocessed")==0) flags = flags | RAW_UNPROCESSED;
	if (commands.GetCount() == 1) return flags;

#ifdef CUSTOM_FREEIMAGE
	if (commands[1].Cmp("raw")==0) flags = flags | RAW_COLOR_RAW;
	if (commands[1].Cmp("srgb")==0) flags = flags | RAW_COLOR_SRGB;
	if (commands[1].Cmp("adobe")==0) flags = flags | RAW_COLOR_ADOBE;
	if (commands[1].Cmp("wide")==0) flags = flags | RAW_COLOR_WIDE;
	if (commands[1].Cmp("prophoto")==0) flags = flags | RAW_COLOR_PROPHOTO;
	if (commands[1].Cmp("xyz")==0) flags = flags | RAW_COLOR_XYZ;
	if (commands.GetCount() == 2) return flags;

	if (commands[2].Cmp("linear")==0) flags = flags | RAW_QUAL_LINEAR;
	if (commands[2].Cmp("vng")==0) flags = flags | RAW_QUAL_VNG;
	if (commands[2].Cmp("ppg")==0) flags = flags | RAW_QUAL_PPG;
	if (commands[2].Cmp("ahd")==0) flags = flags | RAW_QUAL_AHD;
	return flags;
#endif

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
