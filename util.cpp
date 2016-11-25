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

#include <gimage.h>

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

	#pragma omp parallel num_threads(threadcount)
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

wxImage gImage2wxImage(gImage &dib)
{
	int threadcount;
	unsigned h = dib.getHeight();
	unsigned w =  dib.getWidth();
	unsigned char *img = (unsigned char *) dib.getImageData(BPP_8);
	wxImage image(w, h);
	unsigned char *data = image.GetData();
	//ToDo: gImage2wxImage Optimize?  not so sure...
	//ToDo: gImage2wxImage logging
	//unsigned s = w*h*3;
	//#pragma omp parallel for num_threads(4)
	//for (int i = 0; i<s; i++) {
	//	data[i] = img[i];
	//}
	memcpy(data,img, w*h*3);
	delete img;
	return image;
}



unsigned hdata[256];
unsigned hmax;

void FillHistogram(unsigned *histogram)
{
	for (int i=0; i<256; i++) histogram[i] = hdata[i];
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


bool ImageContainsRawprocCommand(wxString fname)
{
	std::map<std::string, std::string> p = gImage::getInfo(fname.c_str());
	if (p.find("ImageDescription") != p.end())
		if (p["ImageDescription"].find("rawproc") != std::string::npos)
			return true;
	return false;
}


/*
wxString RawFlags2Command(int flags)
{
	wxString cmd = "";

	if ((flags & RAW_DEFAULT) == RAW_DEFAULT)    cmd.Append("demosaic");
	else if ((flags & RAW_PREVIEW) == RAW_PREVIEW)  cmd.Append("preview");
	else if ((flags & RAW_DISPLAY) == RAW_DISPLAY) cmd.Append("display");
	else if ((flags & RAW_HALFSIZE) == RAW_HALFSIZE) cmd.Append("half");
	else if ((flags & RAW_UNPROCESSED) == RAW_UNPROCESSED) cmd.Append("unprocessed");

#ifdef RAW_COLOR_RAW
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

#ifdef RAW_COLOR_RAW
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
*/


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
