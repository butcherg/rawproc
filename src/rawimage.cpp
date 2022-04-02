#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <string>
#include <map>
#include <vector>
#include "gimage/strutil.h"

#ifndef USE_DCRAW
#include <libraw/libraw.h>
#else
#include <unistd.h>
#endif

#include <lcms2.h>
#include "gimage/gimage.h"
#include "nikonlensid.h"

//use for reading rgb(a) from libraw
struct uspix { unsigned short r, g, b; };
struct uspixa { unsigned short r, g, b, a; };

void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    puts("");
}


#ifdef USE_DCRAW
//char dcrawversion[128] = "dcraw";
std::string dcrawpath="dcraw";
void setdcrawpath(std::string path) {dcrawpath = path;}
std::string getdcrawpath() {return dcrawpath;}
#endif


#ifdef USE_DCRAW
std::string librawVersion()
{
	return "dcraw";
}
#else
std::string librawVersion()
{

	return std::string(LibRaw::version());
}
#endif  //USE_DCRAW


#ifdef USE_DCRAW
bool _checkRAW(const char *filename)
{
	return true;
}
#else
bool _checkRAW(const char *filename)
{

	LibRaw RawProcessor;
	if (RawProcessor.open_file(filename) == LIBRAW_SUCCESS) return true;
	return false;
}
#endif

#ifdef USE_DCRAW
bool _loadRAWInfo(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info)
{
	std::string exif;

	std::string cmd = dcrawpath;
	cmd.append(" -i -v ");
	cmd.append(filename);

#ifdef WIN32
	FILE* pipe = _popen(cmd.c_str(), "rb");
#else
	FILE* pipe = popen(cmd.c_str(), "r");
#endif
	if (!pipe) return false;

	while (!feof(pipe)) {
		char c = (char) fgetc(pipe);
		if (c == '\r') continue;
		exif.append(&c);
	}
	pclose(pipe);

	std::vector<std::string> lines = split(exif,"\n");
	//for (std::vector<std::string>::iterator it = lines.begin(); it != lines.end(); ++it) {
	for (unsigned i=0; i<lines.size(); i++) {
printf("line: %s\n",lines[i].c_str());
		std::vector<std::string> nvpair = split(lines[i], ":");
		info[nvpair[0]] = nvpair[1]; 
	}
	return true;
}
#else
bool _loadRAWInfo(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info)
{
	int w, h, c, b;
	LibRaw RawProcessor;

#define P1 RawProcessor.imgdata.idata
#define S RawProcessor.imgdata.sizes
#define C RawProcessor.imgdata.color
#define T RawProcessor.imgdata.thumbnail
#define P2 RawProcessor.imgdata.other
// #define OUT RawProcessor.imgdata.params

	if (RawProcessor.open_file(filename) == LIBRAW_SUCCESS) {
		//RawProcessor.unpack();

		info["ISOSpeedRatings"] = tostr(P2.iso_speed);  
		info["ExposureTime"] = tostr(P2.shutter);  
		info["FNumber"] = tostr(P2.aperture);  
		info["FocalLength"] = tostr(P2.focal_len);  
		info["ImageDescription"] = P2.desc;  
		info["Artist"] = P2.artist; 
		info["Make"] = P1.make;  
		info["Model"] = P1.model;  
		info["Orientation"] = tostr((unsigned short) S.flip);

		time_t rawtime = P2.timestamp;
		struct tm * timeinfo;
		char buffer [80];
		timeinfo = localtime (&rawtime);
		strftime (buffer,80,"%Y:%m:%d %H:%M:%S",timeinfo);
		info["DateTime"] = buffer;  
		RawProcessor.recycle();
		return true;
	}
	else {
		RawProcessor.recycle();
		return false;
	}

}
#endif //USE_DCRAW


#ifdef USE_DCRAW
char * _loadRAW(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info, 
			std::string params=std::string(),
			char ** icc_m=NULL, 
			unsigned  *icclength=0)
{

	//int w, h, c, b;
	char * img;
	char imgdata[4096];
	
	int rawdata = 0;

	std::map<std::string,std::string> p = parseparams(params);

	char m[20];
	size_t result;
	unsigned char * image;
	unsigned maxval;
	//unsigned width, height, bpp, colors, icclength;
	unsigned w, h, b, c, iccl;
	BPP bits;
	//std::map<std::string,std::string> imgdata;
	//*numcolors = 3;

	if (access (dcrawpath.c_str(), X_OK)) return NULL;

	std::string cmd = dcrawpath;
	cmd.append(" -c ");
	//$ <li><b>dcrawparams</b>: command line parameters for dcraw-based (--enable-dcraw) raw file input.  Spaces need to be specified by underscores, e.g., -o_3_-g_1_1_-W</li>
	if (p.find("params") != p.end()) 
		cmd.append(de_underscore(p["params"])); 
	cmd.append(" ");
	cmd.append(filename);

#ifdef WIN32
	FILE* pipe = _popen(cmd.c_str(), "rb");
#else
	FILE* pipe = popen(cmd.c_str(), "r");
#endif
	if (!pipe) return NULL;
	result = fscanf(pipe, "%s", m);
	result = fscanf(pipe, "%d", &w);
	result = fscanf(pipe, "%d", &h);
	result = fscanf(pipe, "%d", &maxval);

	std::string magic = std::string(m);
	if (magic == "P5")
		*numcolors = 1;
	else
		*numcolors = 3;

	fgetc(pipe);
	if (maxval < 256) { 
		bits = BPP_8;
		*numbits = 8;
		image = new unsigned char[w*h*3];
		result = fread(image, 1, w*h*3, pipe);
	}
	else {
		bits = BPP_16;
		*numbits = 16;
		image = new unsigned char[w*h*3*2];
		result = fread(image, 2, w*h*3, pipe);
		unsigned short * img = (unsigned short *) image;
		for (int i=0; i< (w*h*3); i++) img[i] = ((img[i] & 0x00ff)<<8)|((img[i] & 0xff00)>>8);
	}
	pclose(pipe);

	*width = w;
	*height = h;

//exif:
	
	std::string exif;

	cmd = dcrawpath;
	cmd.append(" -i -v ");
	cmd.append(filename);

#ifdef WIN32
	FILE* epipe = _popen(cmd.c_str(), "rb");
#else
	FILE* epipe = popen(cmd.c_str(), "r");
#endif
	if (!epipe) return NULL;

	while (!feof(epipe)) {
		char charac = (char) fgetc(epipe);
		if (charac == '\r') continue;
		exif.append(&charac,1);
	}
	pclose(epipe);

	std::vector<std::string> lines = split(exif,"\n");
	
	for (unsigned i=0; i<lines.size(); i++) {
		std::vector<std::string> nvpair = bifurcate(lines[i], ':');
		if (nvpair.size() < 2) continue;
		if (nvpair[1][0] == ' ') nvpair[1].erase(0,1);
		if (nvpair[0] == "Camera") {
			std::vector<std::string> makemodel = bifurcate(nvpair[1],' ');
			if (makemodel.size() == 2) {
				info["Make"] = makemodel[0];
				info["Model"] = makemodel[1];
			}
		}
		if (nvpair[0] == "ISO speed") {
			info["ISOSpeedRatings"] = nvpair[1];
		}
		if (nvpair[0] == "Owner") {
			info["Artist"] = nvpair[1];
		}
		if (nvpair[0] == "Focal length") {
			std::vector<std::string> focal = bifurcate(nvpair[1],' '); //remove 'mm'
			if (focal.size() == 2) info["FocalLength"] = focal[0];
		}
		if (nvpair[0] == "Aperture") {
			std::vector<std::string> aperture = bifurcate(nvpair[1],'/'); //remove 'f/'
			if (aperture.size() == 2) info["FNumber"] = aperture[1];
		}
		if (nvpair[0] == "Shutter") {
			std::vector<std::string> ss = bifurcate(nvpair[1],' '); //remove 'sec'
			std::vector<std::string> sr = split(ss[0],"/"); //parse fraction
			if (sr.size() >= 2) 
				info["ExposureTime"] = tostr(1.0/atof(sr[1].c_str()));
			else
				info["ExposureTime"] = sr[0];
		}
		if (nvpair[0] == "Filter pattern") {
			nvpair[1].erase(nvpair[1].find("/"),1);
			info["Libraw.CFAPattern"] =  nvpair[1];
		}
		if (nvpair[0] == "Timestamp") {
			std::vector<std::string> ts = split(nvpair[1]," "); //parse dcraw timestamp
			if (ts.size() >= 5) {
				std::string month;
				if (ts[1] == "Jan") month = "01";
				if (ts[1] == "Feb") month = "02";
				if (ts[1] == "Mar") month = "03";
				if (ts[1] == "Apr") month = "04";
				if (ts[1] == "May") month = "05";
				if (ts[1] == "Jun") month = "06";
				if (ts[1] == "Jul") month = "07";
				if (ts[1] == "Aug") month = "08";
				if (ts[1] == "Sep") month = "09";
				if (ts[1] == "Oct") month = "10";
				if (ts[1] == "Nov") month = "11";
				if (ts[1] == "Dec") month = "12";
				info["DateTime"].append(ts[4]);
				info["DateTime"].append(":");
				info["DateTime"].append(month);
				info["DateTime"].append(":");
				info["DateTime"].append(ts[2]);
				info["DateTime"].append(" ");
				info["DateTime"].append(ts[3]);
			}
			else info["dcrawTimestamp"] = nvpair[1];
		}
		if (nvpair[0] == "Daylight multipliers") {
			//for (unsigned i=0; i<nvpair[1].size(); i++) if (nvpair[1][i] == ' ') nvpair[1][i] = ',';
			//info["Libraw.WhiteBalance"] = nvpair[1];
			std::vector<std::string> mults = split(nvpair[1], " ");
			if (mults.size() >=3) {
				double rmult = atof(mults[0].c_str());
				double gmult = atof(mults[1].c_str());
				double bmult = atof(mults[2].c_str());
				rmult /= gmult;
				bmult /= gmult;
				gmult = 1.0;
				info["Libraw.WhiteBalance"] = tostr(rmult);
				info["Libraw.WhiteBalance"].append(",");
				info["Libraw.WhiteBalance"].append(tostr(gmult));
				info["Libraw.WhiteBalance"].append(",");
				info["Libraw.WhiteBalance"].append(tostr(bmult));
			}
		}
		info["Orientation"] = "1";
	}

//end exif.

	cmsHPROFILE profile = NULL;
	cmsUInt32Number size;
	if (p.find("cameraprofile") != p.end()) {
		if (p["cameraprofile"] != std::string()) {
			if (p["cameraprofile"].find_first_of(",") != std::string::npos) 
				profile = gImage::makeLCMSAdobeCoeffProfile(p["cameraprofile"]);
			else
				profile = gImage::myCmsOpenProfileFromFile((gImage::getProfilePath()+p["cameraprofile"]));
		}
	}
	if (profile) {
		//gImage::makeICCProfile(profile, icc_m, size);
		//delete if the above works: 
		cmsSaveProfileToMem(profile, NULL, &size);
		*icclength = size;
		*icc_m = new char[size];
		cmsSaveProfileToMem(profile, *icc_m, &size);
	}
	else {
		*icc_m=NULL;
		*icclength = 0;
	}


	return (char *) image;
}
#else

char * _loadRAW(const char *filename, 
			unsigned *width, 
			unsigned *height, 
			unsigned *numcolors, 
			unsigned *numbits, 
			std::map<std::string,std::string> &info, 
			std::string params=std::string(),
			char ** icc_m=NULL, 
			unsigned  *icclength=0)
{

	int w, h, c, b;
	LibRaw RawProcessor;
	char * img;
	char imgdata[4096];
	
	int rawdata = 0;

	std::map<std::string,std::string> p = parseparams(params);

	//for (std::map<std::string,std::string>::iterator it=p.begin(); it!=p.end(); ++it)
	//	std::cout << it->first << ": -" << it->second << "-" << std::endl;
	//printf("\n");
	
#define P1 RawProcessor.imgdata.idata
#define S RawProcessor.imgdata.sizes
#define C RawProcessor.imgdata.color
#define T RawProcessor.imgdata.thumbnail
#define P2 RawProcessor.imgdata.other
// #define OUT RawProcessor.imgdata.params

	//remove all use of shot_select, libraw changed its location and that vexes compilation in all situations...
	//RawProcessor.imgdata.rawparams.shot_select = 0;
	RawProcessor.imgdata.params.use_camera_wb = 1;
	RawProcessor.imgdata.params.output_color = 1;	//sRGB
	RawProcessor.imgdata.params.user_qual = 3;	//AHD
	RawProcessor.imgdata.params.no_auto_bright = 1;

	RawProcessor.imgdata.params.output_bps = 16;

	RawProcessor.imgdata.params.gamm[0] = 1/2.4;  //1/1.0
	RawProcessor.imgdata.params.gamm[1] = 12.92; //1.0


	//raw <li><b>rawdata</b>=0|1|crop - Preempts all other parameters, if 1 loads unprocessed raw data as a one-color grayscale array of the individual R, G, and B measurements; well, subject to camera shenanigans. If crop, also crop the array to the visible image.  Default=0 (empty property value is the same as 0).</li>

	if (p.find("rawdata") != p.end() && !p["rawdata"].empty() && p["rawdata"] != "0") 
		rawdata = 1; // = atoi(p["rawdata"].c_str());
	
	//raw <li><b>Output Colorspace</b>:<ul><li>output_color=0|1|2|3|4|5 - Output color space, dcraw: -o [0-6].  Default=1 (srgb)</li><li>colorspace=raw|srgb|adobe|wide|prophoto|xyz - Alias of output_color, with mnemonic values. default=srgb</li></ul></li>
	//template input.raw.libraw.colorspace=raw|srgb|adobe|wide|prophoto|xyz
	if (p.find("colorspace") != p.end()) {
		if (p["colorspace"].compare("raw") == 0) 
			RawProcessor.imgdata.params.output_color = 0;
		if (p["colorspace"].compare("srgb") == 0) 
			RawProcessor.imgdata.params.output_color = 1;
		if (p["colorspace"].compare("adobe") == 0) 
			RawProcessor.imgdata.params.output_color = 2;
		if (p["colorspace"].compare("wide") == 0) 
			RawProcessor.imgdata.params.output_color = 3;
		if (p["colorspace"].compare("prophoto") == 0) 
			RawProcessor.imgdata.params.output_color = 4;
		if (p["colorspace"].compare("xyz") == 0) 
			RawProcessor.imgdata.params.output_color = 5;
	}

	if (p.find("output_color") != p.end()) 
		RawProcessor.imgdata.params.output_color = atoi(p["output_color"].c_str());


	//raw <li><b>Demosaic:</b><ul><li>user_qual=0|1|2|3|4 - Demosaic algorithm, dcraw: -q [0-4].  Default=3 (ahd)</li><li>demosaic=linear|vng|ppg|ahd|dcb - Alias of user_qual, with mnemonic values. Default=ahd</li></ul></li>
	//template input.raw.libraw.demosaic=linear|vng|ppg|ahd|dcb
	// these algorithms not available in LibRaw 0.19.x: |modahd|afd|vcd|vcdahd|lmmse|amaze|dht|moddht
	if (p.find("demosaic") != p.end()) {
		if (p["demosaic"].compare("linear") == 0) 
			RawProcessor.imgdata.params.user_qual = 0;
		if (p["demosaic"].compare("vng") == 0) 
			RawProcessor.imgdata.params.user_qual = 1;
		if (p["demosaic"].compare("ppg") == 0) 
			RawProcessor.imgdata.params.user_qual = 2;
		if (p["demosaic"].compare("ahd") == 0) 
			RawProcessor.imgdata.params.user_qual = 3;
		if (p["demosaic"].compare("dcb") == 0) 
			RawProcessor.imgdata.params.user_qual = 4;

/*
		if (p["demosaic"].compare("modahd") == 0) 
			RawProcessor.imgdata.params.user_qual = 5;
		if (p["demosaic"].compare("afd") == 0) 
			RawProcessor.imgdata.params.user_qual = 6;
		if (p["demosaic"].compare("vcd") == 0) 
			RawProcessor.imgdata.params.user_qual = 7;
		if (p["demosaic"].compare("vcdahd") == 0) 
			RawProcessor.imgdata.params.user_qual = 8;
		if (p["demosaic"].compare("lmmse") == 0) 
			RawProcessor.imgdata.params.user_qual = 9;
		if (p["demosaic"].compare("amaze") == 0) 
			RawProcessor.imgdata.params.user_qual = 10;
		if (p["demosaic"].compare("dht") == 0) 
			RawProcessor.imgdata.params.user_qual = 11;
		if (p["demosaic"].compare("moddht") == 0) 
			RawProcessor.imgdata.params.user_qual = 12;
*/
	}
	if (p.find("user_qual") != p.end()) RawProcessor.imgdata.params.user_qual = atoi(p["user_qual"].c_str());


	//raw <li><b>output_bps</b>=8|16 - bits per sample, dcraw: -4 (16-bit).  Default=16</li>

	if (p.find("output_bps") != p.end()) 
		if (p["output_bps"] == "8")
			RawProcessor.imgdata.params.output_bps = 8;
	


	//raw <li><b>Gamma</b>:  three ways to set it:<ol><li>gamma=preset, where preset=bt709|srgb|linear|prophoto.  In slope/toe, bt709 is 2.2/4.5, srgb is 2.4/4.5, prophoto is 1.8/0, and linear is 1.0/1.0</li><li>gammaval=2.222;gammatoe=4.5, where the slope and toe are specified in val and toe, respectively. Libraw alias.</li><li>gamm=2.222,4.5, where slope and toe are specified with a comma delimiter. dcraw: -g &lt;p ts&gt;</li></ol></li>
	//template input.raw.libraw.gamma=bt709|srgb|linear|prophoto

	if (p.find("gamma") != p.end()) {
		if (p["gamma"].compare("bt709") == 0) {
			RawProcessor.imgdata.params.gamm[0] = 1/2.222;
			RawProcessor.imgdata.params.gamm[1] = 4.5;
		}
		if (p["gamma"].compare("srgb") == 0) {
			RawProcessor.imgdata.params.gamm[0] = 1/2.4;
			RawProcessor.imgdata.params.gamm[1] = 12.92;
		}
		if (p["gamma"].compare("linear") == 0) {
			RawProcessor.imgdata.params.gamm[0] = 1/1.0;
			RawProcessor.imgdata.params.gamm[1] = 1.0;
		}
		if (p["gamma"].compare("prophoto") == 0) {
			RawProcessor.imgdata.params.gamm[0] = 1/1.8;
			RawProcessor.imgdata.params.gamm[1] = 0.0;
		}
	}


	if (p.find("gammaval") != p.end()) 
		RawProcessor.imgdata.params.gamm[0] = 1.0/atof(p["gammaval"].c_str());
	if (p.find("gammatoe") != p.end()) 
		RawProcessor.imgdata.params.gamm[0] = atof(p["gammatoe"].c_str());

	if (p.find("gamm") != p.end()) {
		std::vector<std::string> g = split(p["gamm"],",");
		RawProcessor.imgdata.params.gamm[0] = 1.0/atof(g[0].c_str());
		RawProcessor.imgdata.params.gamm[1] = atof(g[1].c_str());
	}

	//raw <li><b>autobright</b>=0|1 -  - Use dcraw automatic brightness (note the difference from libraw's no_auto_bright). Default=0, don't brighten image.  This is a scaling operation, expands the image histogram to the black/white limits. dcraw: the opposite of -W</li>
	if (p.find("autobright") != p.end()) {
		if (p["autobright"] == "1")
			RawProcessor.imgdata.params.no_auto_bright = 0; 
	}

	//raw <li><b>greybox</b>=x,y,w,h - coordinates of a rectangle used for white balance. dcraw: -A &lt;x y w h&gt;</li>
	if (p.find("greybox") != p.end()) {
		std::vector<std::string> g = split(p["greybox"],",");
		if (g.size() == 4) {
			RawProcessor.imgdata.params.greybox[0] = atoi(g[0].c_str());
			RawProcessor.imgdata.params.greybox[1] = atoi(g[1].c_str());
			RawProcessor.imgdata.params.greybox[2] = atoi(g[2].c_str());
			RawProcessor.imgdata.params.greybox[3] = atoi(g[3].c_str());
		}
	}

	//raw <li><b>cropbox</b>=x,y,w,h - coordinates of a rectangle used for crop. </li>
	if (p.find("cropbox") != p.end()) {
		std::vector<std::string> c = split(p["cropbox"],",");
		if (c.size() == 4) {
			RawProcessor.imgdata.params.cropbox[0] = atoi(c[0].c_str());
			RawProcessor.imgdata.params.cropbox[1] = atoi(c[1].c_str());
			RawProcessor.imgdata.params.cropbox[2] = atoi(c[2].c_str());
			RawProcessor.imgdata.params.cropbox[3] = atoi(c[3].c_str());
		}
	}

	//raw <li><b>aber</b>=rmult,gmult - chromatic aberration correction. dcraw: -C &lt;r b&gt;</li>
	if (p.find("aber") != p.end()) {
		std::vector<std::string> c = split(p["aber"],",");
		if (c.size() == 2) {
			RawProcessor.imgdata.params.aber[0] = atoi(c[0].c_str());
			RawProcessor.imgdata.params.aber[2] = atoi(c[1].c_str());
		}
	}

	//raw <li><b>user_mul</b>=mul0,mul1,mul2,mul3 - user white balance multipliers, r,g,b,g. dcraw: -r &lt;r g b g&gt;</li>
	if (p.find("user_mul") != p.end()) {
		std::vector<std::string> c = split(p["user_mul"],",");
		if (c.size() == 4) {
			RawProcessor.imgdata.params.user_mul[0] = atoi(c[0].c_str());
			RawProcessor.imgdata.params.user_mul[1] = atoi(c[1].c_str());
			RawProcessor.imgdata.params.user_mul[2] = atoi(c[2].c_str());
			RawProcessor.imgdata.params.user_mul[3] = atoi(c[3].c_str());
		}
	}

	//deprecated <li><b>shot_select</b>=n - select image number for processing.  dcraw: -s [0..N-1]</li>
	//remove all use of shot_select, libraw changed its location and that vexes compilation in all situations...
	//if (p.find("shot_select") != p.end()) 
	//	RawProcessor.imgdata.rawparams.shot_select = atoi(p["shot_select"].c_str());

	//raw <li><b>bright</b>=1.0 - brighten image. Default=1.0, no brighten.  dcraw: -b &lt;num&gt;</li>
	if (p.find("bright") != p.end()) 
		RawProcessor.imgdata.params.bright = atof(p["bright"].c_str());

	//raw <li><b>Wavelet Denoise:</b><ul><li>threshold=100-1000 - wavelet denoising threshold. dcraw: -n &lt;num&gt;</li><li>wavelet_denoise - alias for threshold</li></ul>From the dcraw manpage: "Use wavelets to erase noise while preserving real detail. The best threshold should be somewhere between 100 and 1000."</li>
	if (p.find("threshold") != p.end()) 
		RawProcessor.imgdata.params.threshold = atof(p["threshold"].c_str());

	if (p.find("wavelet_denoise") != p.end()) 
		RawProcessor.imgdata.params.threshold = atof(p["wavelet_denoise"].c_str());

	//raw <li><b>half_size</b>=0|1 - demosaic with the 'half' algorithm, which just produces a single pixel from each quad. dcraw: -h</li>
	if (p.find("half_size") != p.end()) 
		RawProcessor.imgdata.params.half_size = atoi(p["half_size"].c_str());

	//raw <li><b>four_color_rgb</b>=0|1 - separate interpolations for the two green components. dcraw: -f</li>
	if (p.find("four_color_rgb") != p.end()) 
		RawProcessor.imgdata.params.four_color_rgb = atoi(p["four_color_rgb"].c_str());

	//raw <li><b>highlight</b>=0|1|2|3+ - deal with image highlights, clip=0, 1=unclip, 2=blend, 3+=rebuild. dcraw: -H [0-9]</li>
	if (p.find("highlight") != p.end()) 
		RawProcessor.imgdata.params.highlight = atoi(p["highlight"].c_str());

	//raw <li><b>use_auto_wb</b>=0|1 - use auto white balance, averaging over entire image. dcraw: -a</li>
	if (p.find("use_auto_wb") != p.end()) 
		RawProcessor.imgdata.params.use_auto_wb = atoi(p["use_auto_wb"].c_str());

	//raw <li><b>use_camera_wb</b>=0|1 - use camera white balance, if available. dcraw: -w.  Note: If no white balance option is specified, this is the default.  Further, you need to set it to 0 to use any of the others.</li>
	if (p.find("use_camera_wb") != p.end()) 
		RawProcessor.imgdata.params.use_camera_wb = atoi(p["use_camera_wb"].c_str());

	//raw <li><b>use_camera_matrix</b>=0|1 - use camera color matrix. dcraw: +M/-M.  From the libraw docs:<ul><li>0: do not use embedded color profile<li>1 (default): use embedded color profile (if present) for DNG files (always); for other files only if use_camera_wb is set;<li>3: use embedded color data (if present) regardless of white balance setting.</ul></li>
	if (p.find("use_camera_matrix") != p.end()) 
		RawProcessor.imgdata.params.use_camera_matrix = atoi(p["use_camera_matrix"].c_str());

	//raw <li><b>output_profile</b>=filepath - use ICC profile from the file. dcraw: -o &lt;file&gt;</li>
	if (p.find("output_profile") != p.end()) 
		RawProcessor.imgdata.params.output_profile = (char *) p["output_profile"].c_str();

	//raw <li><b>camera_profile</b>=filepath|embed - use ICC profile from the file. dcraw: -p &lt;file&gt; Note: use cameraprofile instead.</li>
	if (p.find("camera_profile") != p.end()) 
		RawProcessor.imgdata.params.camera_profile = (char *) p["camera_profile"].c_str();

	//raw <li><b>bad_pixels</b>=filepath - use bad pixel map. dcraw: -P &lt;file&gt;</li>
	if (p.find("bad_pixels") != p.end()) 
		RawProcessor.imgdata.params.bad_pixels = (char *) p["bad_pixels"].c_str();

	//raw <li><b>dark_frame</b>=filepath - use dark frame map, 16-bit PGM format. dcraw: -K &lt;file&gt;</li>
	if (p.find("dark_frame") != p.end()) 
		RawProcessor.imgdata.params.dark_frame = (char *) p["dark_frame"].c_str();

	//raw <li><b>output_tiff</b>=0|1 - 0=output PPM, 1=output TIFF. Default=0.  dcraw: -T.  gImage actually doesn't do anything with this parameter.</li>
	if (p.find("output_tiff") != p.end()) 
		RawProcessor.imgdata.params.output_tiff = atoi(p["output_tiff"].c_str());

/* Getting confused with rawproc operations
	//hide <li><b>user_flip</b>=0|1 - Flip image (0=none, 3=180, 5=90CCW, 6=90CW). dcraw: -t [0-7] Note: Let rawproc deal with this, output.orient=1...</li>
	if (p.find("user_flip") != p.end()) 
		RawProcessor.imgdata.params.user_flip = atoi(p["user_flip"].c_str());
*/
	//raw <li><b>user_black</b>=n - User black level. dcraw: -k &lt;num&gt;</li>
	if (p.find("user_black") != p.end()) 
		RawProcessor.imgdata.params.user_black = atoi(p["user_black"].c_str());

	//raw <li><b>user_cblack</b>=rb,gb,bb,gb - user per-channel black levels, r,g,b,g</li>
	if (p.find("user_cblack") != p.end()) {
		std::vector<std::string> c = split(p["user_cblack"],",");
		if (c.size() == 4) {
			RawProcessor.imgdata.params.user_cblack[0] = atoi(c[0].c_str());
			RawProcessor.imgdata.params.user_cblack[1] = atoi(c[1].c_str());
			RawProcessor.imgdata.params.user_cblack[2] = atoi(c[2].c_str());
			RawProcessor.imgdata.params.user_cblack[3] = atoi(c[3].c_str());
		}
	}

	//
	// sony_arw2_hack=0|1 - Turn on/off division by four for Sony ARW
	//doesn't seem to be in libraw-17.2.0
	//if (p.find("sony_arw2_hack") != p.end()) 
	//	RawProcessor.imgdata.params.sony_arw2_hack = atoi(p["sony_arw2_hack"].c_str());


	//raw <li><b>user_sat</b>=n - saturation. dcraw: -S &lt;num&gt;</li>
	if (p.find("user_sat") != p.end()) 
		RawProcessor.imgdata.params.user_sat = atoi(p["user_sat"].c_str());

	//raw <li><b>med_passes</b>=n - number of median filter passes. dcraw: -m &lt;num&gt;</li>
	if (p.find("med_passes") != p.end()) 
		RawProcessor.imgdata.params.med_passes = atoi(p["med_passes"].c_str());

	//raw <li><b>auto_bright_thr</b>=0.01 - portion of clipped pixel with autobright.</li>
	if (p.find("auto_bright_thr") != p.end()) 
		RawProcessor.imgdata.params.auto_bright_thr = atof(p["auto_bright_thr"].c_str());

	//raw <li><b>adjust_maximum_thr</b>=0.01 - portion of clipped pixel with autobright</li>
	if (p.find("adjust_maximum_thr") != p.end()) 
		RawProcessor.imgdata.params.adjust_maximum_thr = atof(p["adjust_maximum_thr"].c_str());

	//raw <li><b>use_fuji_rotate</b>=0|1 - rotation for cameras with Fuji sensor. dcraw: -j</li>
	if (p.find("use_fuji_rotate") != p.end()) 
		RawProcessor.imgdata.params.use_fuji_rotate = atoi(p["use_fuji_rotate"].c_str());

	//raw <li><b>green_matching</b>=0|1 - fix green channel disbalance</li>
	if (p.find("green_matching") != p.end()) 
		RawProcessor.imgdata.params.green_matching = atoi(p["green_matching"].c_str());

	//raw <li><b>dcb_iterations</b>=n - number of DCB correction passes, -1 is no correction, and is also the default.</li>
	if (p.find("dcb_iterations") != p.end()) 
		RawProcessor.imgdata.params.dcb_iterations = atoi(p["dcb_iterations"].c_str());

	//raw <li><b>dcb_enhance_fl</b>=n - DCB interpolation with enhanced interpolated colors.</li>
	//#
	if (p.find("dcb_enhance_fl") != p.end()) 
		RawProcessor.imgdata.params.dcb_enhance_fl = atoi(p["dcb_enhance_fl"].c_str());

	//raw <li><b>FBDD Denoise:</b><ul><li>fbdd_noiserd=n - FBDD noise reduction, before demosaic<li>fbdd_denoise - alias for fbdd_noiserd</li></ul></li>
	if (p.find("fbdd_noiserd") != p.end()) 
		RawProcessor.imgdata.params.fbdd_noiserd = atoi(p["fbdd_noiserd"].c_str());
	if (p.find("fbdd_denoise") != p.end()) 
		RawProcessor.imgdata.params.fbdd_noiserd = atoi(p["fbdd_denoise"].c_str());

	//raw <li><b>Exposure Compensation:</b><ul><li>exp_correc=0|1 - Turns on exposure correction before demosaic</li><li>exp_shift=1.0 - From 0.25 (2 stops darken) to 8.0 (3 stops lighten), default 1.0=no shift</li><li>exp_preser=0.0 to 1.0 - Preserve hightlights when lightening the image</li></ul></li>
	if (p.find("exp_correc") != p.end()) 
		RawProcessor.imgdata.params.exp_correc = atoi(p["exp_correc"].c_str());
	if (p.find("exp_shift") != p.end()) 
		RawProcessor.imgdata.params.exp_shift = atof(p["exp_shift"].c_str());
	if (p.find("exp_preser") != p.end()) 
		RawProcessor.imgdata.params.exp_preser = atof(p["exp_preser"].c_str());

	//raw <li><b>no_auto_scale</b>=0|1 - Disables pixel scaling, dcraw: -D.  Default=0. Note: Behavior is not the same as dcraw; image is 3-color.</li>
	if (p.find("no_auto_scale") != p.end()) 
		RawProcessor.imgdata.params.no_auto_scale = atoi(p["no_auto_scale"].c_str());

	//raw <li><b>no_interpolation</b>=0|1 - Disables demosaic. dcraw: -d  Default=0. Note: Behavior is not the same as dcraw; image is 3-color.  Use input.raw.libraw.rawdata=1 for unprocessed raw array.</li>
	if (p.find("no_interpolation") != p.end()) 
		RawProcessor.imgdata.params.no_interpolation = atoi(p["no_interpolation"].c_str());

	//raw <li><b>whitebalance</b>=none|auto|camera|n,n,n,n - whitebalance overrides all other dcraw/libraw white balance parameters, provides a more intuitive way to specify white balance.  Default=camera (no value also equals 'camera').</li>
	//RawProcessor.imgdata.params.no_auto_scale=1;  //turn it all off

	//default to camera if whitebalance is not specified:
	RawProcessor.imgdata.params.no_auto_scale=0;
	RawProcessor.imgdata.params.use_camera_wb = 1;
	RawProcessor.imgdata.params.use_auto_wb = 0;

	if (p.find("whitebalance") != p.end()) {
		if (p["whitebalance"] == "none") {
			RawProcessor.imgdata.params.no_auto_scale=1;
			RawProcessor.imgdata.params.use_camera_wb = 0;
			RawProcessor.imgdata.params.use_auto_wb = 0;
			RawProcessor.imgdata.params.user_mul[0] = 1;
			RawProcessor.imgdata.params.user_mul[1] = 1;
			RawProcessor.imgdata.params.user_mul[2] = 1;
			RawProcessor.imgdata.params.user_mul[3] = 1;
		}
		else if (p["whitebalance"] == "auto" ) {
			RawProcessor.imgdata.params.no_auto_scale=0;
			RawProcessor.imgdata.params.use_camera_wb = 0;
			RawProcessor.imgdata.params.use_auto_wb = 1;
		}
		else if (p["whitebalance"] == "camera" | p["whitebalance"].empty()) {  //default if property exists but is empty
			RawProcessor.imgdata.params.no_auto_scale=0;
			RawProcessor.imgdata.params.use_camera_wb = 1;
			RawProcessor.imgdata.params.use_auto_wb = 0;
		}
		else { //should be four comma-separated multipliers
			std::vector<std::string> c = split(p["whitebalance"],",");
			if (c.size() == 4) {
				RawProcessor.imgdata.params.no_auto_scale=0;
				RawProcessor.imgdata.params.use_camera_wb = 0;
				RawProcessor.imgdata.params.use_auto_wb = 0;
				RawProcessor.imgdata.params.user_mul[0] = atof(c[0].c_str());
				RawProcessor.imgdata.params.user_mul[1] = atof(c[1].c_str());
				RawProcessor.imgdata.params.user_mul[2] = atof(c[2].c_str());
				RawProcessor.imgdata.params.user_mul[3] = atof(c[3].c_str());
			}
		}
	}

#ifdef OLD_LIBRAW
	//These processing parameters were deleted when LibRaw removed the demosaic packs.  '&' flags the doc extracts.

	//&
	//& eeci_refine=n - non-zero for ECCI refine for VCD interpolation
	//&
	if (p.find("eeci_refine") != p.end()) 
		RawProcessor.imgdata.params.eeci_refine = atoi(p["eeci_refine"].c_str());

	//&
	//& es_med_passes=n - number of edge-sensitive median filter passes after VCD+AHD demosaic
	//&
	if (p.find("es_med_passes") != p.end()) 
		RawProcessor.imgdata.params.es_med_passes = atoi(p["es_med_passes"].c_str());

	//&
	//& ca_correc=0|1 - chromatic aberration suppression
	//&
	if (p.find("ca_correc") != p.end()) 
		RawProcessor.imgdata.params.ca_correc = atoi(p["ca_correc"].c_str());

	//&
	//& cared=0.01 - chromatic aberration suppression red correction
	//&
	if (p.find("cared") != p.end()) 
		RawProcessor.imgdata.params.cared = atof(p["cared"].c_str());

	//&
	//& cablue=0.01 - chromatic aberration suppression blue correction
	//&
	if (p.find("cablue") != p.end()) 
		RawProcessor.imgdata.params.cablue = atof(p["cablue"].c_str());

	//&
	//& cfaline=0|1 - banding reduction
	//&
	if (p.find("cfaline") != p.end()) 
		RawProcessor.imgdata.params.cfaline = atoi(p["cfaline"].c_str());

	//&
	//& linenoise=0.01 - banding reduction amount
	//&
	if (p.find("linenoise") != p.end()) 
		RawProcessor.imgdata.params.linenoise = atof(p["linenoise"].c_str());

	//&
	//& cfa_clean=0|1 - Turns on impulse noise and Gaussian high frequency reduction
	//& lclean=0.005 to 0.05 - Amount of luminance reduction, 0.01 is a common value
	//& cclean=0.005 to 0.05 - Amount of color reduction, 0.01 is a common value
	//&
	if (p.find("cfa_clean") != p.end()) 
		RawProcessor.imgdata.params.cfa_clean = atoi(p["cfa_clean"].c_str());
	if (p.find("lclean") != p.end()) 
		RawProcessor.imgdata.params.lclean = atof(p["lclean"].c_str());
	if (p.find("cclean") != p.end()) 
		RawProcessor.imgdata.params.cclean = atof(p["cclean"].c_str());

	//&
	//& cfa_green=0|1 - Turns on reduction of maze artifacts produced by bad balance of green channels
	//& green_thresh=0.01 to 0.1 - Max difference between channels allowed for equalization 
	//&
	if (p.find("cfa_green") != p.end()) 
		RawProcessor.imgdata.params.cfa_green = atoi(p["cfa_green"].c_str());
	if (p.find("green_thresh") != p.end()) 
		RawProcessor.imgdata.params.green_thresh = atof(p["green_thresh"].c_str());

	//&
	//& wf_debanding=0|1 - Turns on banding suppression
	//& wf_deband_threshold=tr,tg,tb,tg - Per-channel debanding thresholds
	//&
	if (p.find("wf_debanding") != p.end()) 
		RawProcessor.imgdata.params.wf_debanding = atoi(p["wf_debanding"].c_str());
	if (p.find("wf_deband_treshold") != p.end()) {
		std::vector<std::string> c = split(p["wf_deband_treshold"],",");
		if (c.size() == 4) {
			RawProcessor.imgdata.params.wf_deband_treshold[0] = atoi(c[0].c_str());
			RawProcessor.imgdata.params.wf_deband_treshold[1] = atoi(c[1].c_str());
			RawProcessor.imgdata.params.wf_deband_treshold[2] = atoi(c[2].c_str());
			RawProcessor.imgdata.params.wf_deband_treshold[3] = atoi(c[3].c_str());
		}
	}

#endif






	if (RawProcessor.open_file(filename) != LIBRAW_SUCCESS) {
		printf("loadRAW: RawProcessor.open_file failed...\n"); 
		return NULL;
	}
	RawProcessor.unpack();
	
	*icclength = 0;

	info["ISOSpeedRatings"] = tostr(P2.iso_speed);  
	info["ExposureTime"] = tostr(P2.shutter);  
	info["FNumber"] = tostr(P2.aperture);  
	info["FocalLength"] = tostr(P2.focal_len);  
	info["ImageDescription"] = P2.desc;  
	info["Artist"] = P2.artist; 
	info["Make"] = P1.make;  
	info["Model"] = P1.model;  

	//used for subsequent metadata collection:
	char buffer  [4096];
	char cfarray [4096];
	const char ndesc[4] = {'0', '1', '2', '3'};

	//Normalized libraw white balance:
	snprintf(buffer, 4096, "%f,%f,%f", C.cam_mul[0]/C.cam_mul[1], C.cam_mul[1]/C.cam_mul[1], C.cam_mul[2]/C.cam_mul[1]);
	info["Libraw.WhiteBalance"] = buffer;
	snprintf(buffer, 4096, "%f,%f,%f", C.cam_mul[0], C.cam_mul[1], C.cam_mul[2]);
	info["Libraw.CamMult"] = buffer;


	//Black level, for subtraction
	if (C.black != 0) {
		snprintf(buffer, 4096, "%d", C.black);
		info["Libraw.Black"] = buffer;
	}

	//per-channel black subtraction:
	//if (C.cblack[0] != 0 | C.cblack[1] != 0 | C.cblack[2] != 0 | C.cblack[3] != 0) { //libraw 0.19.5 isn't encoding this correctly...
	else if (C.cblack[0] > 1 | C.cblack[1] > 1 | C.cblack[2] > 1 | C.cblack[3] > 1) {
		snprintf(buffer, 4096, "%d,%d,%d,%d",C.cblack[0],C.cblack[1],C.cblack[2],C.cblack[3]);
		info["Libraw.PerChannelBlack"] = buffer;
	}

	//color matrix black subtraction
	else if (C.cblack[6] != 0) {
		unsigned cblackdim_col = C.cblack[4];
		unsigned cblackdim_row = C.cblack[5];
		std::string cblackarray;
		snprintf(buffer, 4096, "%d",C.cblack[6]);
		cblackarray.append(buffer);
		for (unsigned i=7; i< 7+cblackdim_col*cblackdim_row-1; i++) {
			snprintf(buffer, 4096, ",%d",C.cblack[i]);
			cblackarray.append(buffer);
		}
		info["Libraw.CFABlack"] = cblackarray;
	}

	//Maximum pixel value:
	snprintf(buffer, 4096, "%d", C.maximum);
	info["Libraw.Maximum"] = buffer;

	//cam_xyz matrix:
	snprintf(buffer, 4096, "%f,%f,%f,%f,%f,%f,%f,%f,%f", C.cam_xyz[0][0],C.cam_xyz[0][1],C.cam_xyz[0][2],
		C.cam_xyz[1][0],C.cam_xyz[1][1],C.cam_xyz[1][2],
		C.cam_xyz[2][0],C.cam_xyz[2][1],C.cam_xyz[2][2]);
	info["Libraw.CamXYZ"] = buffer;

	//rgb_cam matrix:
	snprintf(buffer, 4096, "%f,%f,%f,%f,%f,%f,%f,%f,%f", C.rgb_cam[0][0],C.rgb_cam[0][1],C.rgb_cam[0][2],
		C.rgb_cam[1][0],C.rgb_cam[1][1],C.rgb_cam[1][2],
		C.rgb_cam[2][0],C.rgb_cam[2][1],C.rgb_cam[2][2]);
	info["Libraw.RGBCam"] = buffer;

	//camera matrix:
	snprintf(buffer, 4096, "%f,%f,%f,%f,%f,%f,%f,%f,%f", C.cmatrix[0][0],C.cmatrix[0][1],C.cmatrix[0][2],
		C.cmatrix[1][0],C.cmatrix[1][1],C.cmatrix[1][2],
		C.cmatrix[2][0],C.cmatrix[2][1],C.cmatrix[2][2]);
	info["Libraw.CameraMatrix"] = buffer;

	int cfadim = 2;			 //bayer
	if (P1.filters == 9) cfadim = 6; //fuji x-trans

	unsigned pos;
	for (unsigned r=0; r<cfadim; r++) {
		for (unsigned c=0; c<cfadim; c++) {
			pos = c + r*cfadim;
			buffer[pos]  = P1.cdesc[RawProcessor.COLOR(r,c)];
			cfarray[pos] = ndesc[RawProcessor.COLOR(r,c)];
		}
	}
	buffer[pos+1]  = '\0';
	cfarray[pos+1] = '\0';

	info["Libraw.CFAPattern"] =  buffer;
	info["Libraw.CFAArray"] =  cfarray;

	//Lens nomenclature for LensFun:
	if (strlen(RawProcessor.imgdata.lens.makernotes.Lens) > 0)
		info["LensModel"] = RawProcessor.imgdata.lens.makernotes.Lens;
	else if (strlen(RawProcessor.imgdata.lens.Lens) > 0)
		info["LensModel"] = RawProcessor.imgdata.lens.Lens;
	else
		info["LensModel"] = lens_lookup(RawProcessor.imgdata.lens.makernotes.LensID);
	

	//Normalize libraw orientation for EXIF:

	if (RawProcessor.imgdata.params.user_flip == -1) {  //take the orientation from flip
		info["Orientation"] = tostr((unsigned short) S.flip); //dcraw left the orientation alone, use the metadata
		if (S.flip == 0)  info["Orientation"] = "1";
		if (S.flip == 3)  info["Orientation"] = "3";
		if (S.flip == 5)  info["Orientation"] = "8";
		if (S.flip == 6)  info["Orientation"] = "6";
	}
	else {
		info["Orientation"] = "1"; //dcraw flipped the image per the user's instruction (3, 5, 6) or the raw file specification (-1), so don't specify an orientation transform
	}


	time_t rawtime = P2.timestamp;
	struct tm * timeinfo;
	timeinfo = localtime (&rawtime);
	strftime (buffer,80,"%Y:%m:%d %H:%M:%S",timeinfo);
	info["DateTimeOriginal"] = buffer;  

	
	cmsHPROFILE profile = NULL;
	cmsUInt32Number size;
	*icc_m=NULL;
	*icclength = 0;
	
	/* keep for debugging float images...
	if (RawProcessor.imgdata.rawdata.raw_alloc) printf("raw_alloc\n");
	if (RawProcessor.imgdata.rawdata.raw_image) printf("raw_image\n");
	if (RawProcessor.imgdata.rawdata.color3_image) printf("color3_image\n");
	if (RawProcessor.imgdata.rawdata.color4_image) printf("color4_image\n");
	if (RawProcessor.imgdata.rawdata.float_image) printf("float_image\n");
	if (RawProcessor.imgdata.rawdata.float3_image) printf("float3_image\n");
	if (RawProcessor.imgdata.rawdata.float4_image) printf("float4_image\n");
	fflush(stdout);
	*/
	
	
	if (rawdata) {
		info["Libraw.raw_width"] = tostr(S.raw_width);
		info["Libraw.raw_height"] = tostr(S.raw_height);
		info["Libraw.width"] = tostr(S.width);
		info["Libraw.height"] = tostr(S.height);
		info["Libraw.top_margin"] = tostr(S.top_margin);
		info["Libraw.left_margin"] = tostr(S.left_margin);
		
		if (p["rawdata"].compare("crop") == 0) {
/*			//old code (prior to commit 6a3ab0) in case I don't like raw2image...  :D
			*width = S.width;
			*height = S.height;

			unsigned short *raw = RawProcessor.imgdata.rawdata.raw_image;
			//unsigned short *img = new unsigned short[S.width*S.height];
			img = new char[S.raw_width*S.raw_height*2];

			unsigned short *src = raw;
			unsigned short *dst = (unsigned short *) img;

			unsigned cnt = 0;
			for (unsigned row=0; row < S.height; row++) {
				for (unsigned col=0; col < S.width; col++) {
					unsigned spos = ((row+S.top_margin)*S.raw_width)+(col+S.left_margin);
					unsigned dpos = row*S.width+col;
					dst[dpos] = src[spos];
					cnt++;
				}
			}
*/

			//new code uses raw2image, because I don't think I can trust S.top_margin and S.left_margin...
			*width = S.width;
			*height = S.height;
			
			int result = RawProcessor.raw2image();
			img = new char[S.width*S.height*2];
			unsigned short *dst = (unsigned short *) img;

			for (unsigned r = 0; r < S.height; r++) { 
				for (unsigned c = 0; c < S.width; c++) {
					unsigned pos = r*S.width+c;
					dst[pos] = RawProcessor.imgdata.image[r*S.width+c][RawProcessor.COLOR(r,c)];
				}
			}
			*numcolors = 1;
			*numbits = 16;
			info["Libraw.Mosaiced"] = "1";
			info["PhotometricInterpretation"] = "32803"; //Color Filter Array;
		}
		else {
			*width = S.raw_width;
			*height = S.raw_height;
			
			if (RawProcessor.imgdata.rawdata.raw_image) {
				img = new char[S.raw_width*S.raw_height*2];
				memcpy(img, (char *) RawProcessor.imgdata.rawdata.raw_image, S.raw_width*S.raw_height*2);
				*numcolors = 1;
				*numbits = 16;
				info["Libraw.Mosaiced"] = "1";
				info["PhotometricInterpretation"] = "32803"; //Color Filter Array;
			}
			else if (RawProcessor.imgdata.rawdata.color3_image) {
				img = new char[S.raw_width*S.raw_height*(2*3)];
				memcpy(img, (char *) RawProcessor.imgdata.rawdata.color3_image, S.raw_width*S.raw_height*(2*3));
				*numcolors = 3;
				*numbits = 16;
				info["Libraw.Mosaiced"] = "0";
				info["PhotometricInterpretation"] = "2"; //RGB;
			}
			else if (RawProcessor.imgdata.rawdata.color4_image) {
				img = new char[S.raw_width*S.raw_height*(2*3)];
				uspix * timg = (uspix *) img;
				uspixa * limg = (uspixa *)RawProcessor.imgdata.rawdata.color4_image;
				//memcpy(img, (char *) RawProcessor.imgdata.rawdata.color3_image, S.raw_width*S.raw_height*(2*3));
				for (unsigned i=0; i< S.raw_width*S.raw_height; i++) {
					timg[i].r = limg[i].r;
					timg[i].g = limg[i].g;
					timg[i].b = limg[i].b;
				}
				*numcolors = 3;
				*numbits = 16;
				info["Libraw.Mosaiced"] = "0";
				info["PhotometricInterpretation"] = "2"; //RGB;
			}
			else { //return empty image
				return NULL;
			}
			
		}
	
		RawProcessor.imgdata.params.output_color = 0;


		if (p.find("cameraprofile") != p.end()) {
			if (p["cameraprofile"] != std::string()) {
					if (p["cameraprofile"].find_first_of(",") != std::string::npos) 
						profile = gImage::makeLCMSAdobeCoeffProfile(p["cameraprofile"]);
					else
						profile = gImage::myCmsOpenProfileFromFile((gImage::getProfilePath()+p["cameraprofile"]));
			}
		}
		if (profile) {
			//gImage::makeICCProfile(profile, icc_m, size);
			//delete if the above works: 
			cmsSaveProfileToMem(profile, NULL, &size);
			*icclength = size;
			*icc_m = new char[size];
			cmsSaveProfileToMem(profile, *icc_m, &size);
		}
		else {
			*icc_m=NULL;
			*icclength = 0;
		}
	}
	else {
	
		int result = RawProcessor.dcraw_process();
		//printf("%s\n",LibRaw::strerror(result));
		if (RawProcessor.imgdata.process_warnings & LIBRAW_WARN_FALLBACK_TO_AHD) {
			info["Notice"] = "Selected demosaic algorithm not supported, AHD used";
		}
		RawProcessor.get_mem_image_format(&w, &h, &c, &b);
		*width = w;
		*height = h;
		*numcolors = c;
		*numbits = b;
		img = new char[w*h*c*(b/8)];
	
		libraw_processed_image_t *image = RawProcessor.dcraw_make_mem_image();
		memcpy(img, image->data, image->data_size);
		LibRaw::dcraw_clear_mem(image);


		if (C.profile) {
			*icc_m = new char[C.profile_length];
			memcpy(*icc_m, C.profile, C.profile_length);
			*icclength = C.profile_length;
		}
		else {  //because apparently libraw doesn't pass along the dcraw-generated profiles
			float gamma = 1.0/RawProcessor.imgdata.params.gamm[0];
			if (RawProcessor.imgdata.params.output_color == 0) {  // raw image, check for cameraprofile and assign if found

				//raw <li><b>cameraprofile</b>=iccfile|adobe_coeff - If (and only if) colorspace=raw, this parameter assigns the camera profile to the image.  Unlike input.raw.cms.profile in rawproc, this parameter will provide a record of its application in the command string, so it is the preferred method for assigning camera profiles.  If the parameter is present but blank in Properties, it will be ignored.  Trick: Instead of a filename, paste a comma-delimited set of dcraw-style (adobe_coeff) primaries here and a linear gamma D65 whitepoint profile will be built and assigned to the raw image.</li>
				//template input.raw.libraw.cameraprofile=iccfile
				if (p.find("cameraprofile") != p.end()) {
					if (p["cameraprofile"] != std::string()) {
						if (p["cameraprofile"].find_first_of(",") != std::string::npos) 
							profile = gImage::makeLCMSAdobeCoeffProfile(p["cameraprofile"]);
						else
							profile = gImage::myCmsOpenProfileFromFile((gImage::getProfilePath()+p["cameraprofile"]));
					}
				}
			}
		
			if (profile) {
				//gImage::makeICCProfile(profile, icc_m, size);
				//delete if the above works: 
				cmsSaveProfileToMem(profile, NULL, &size);
				*icclength = size;
				*icc_m = new char[size];
				cmsSaveProfileToMem(profile, *icc_m, &size);
			}
			else if ((p.find("colorspace") != p.end()) | (p.find("output_color") != p.end())) {
				if (RawProcessor.imgdata.params.output_color == 1) {
					profile = gImage::makeLCMSdcrawProfile("srgb", gamma);
					cmsSaveProfileToMem(profile, NULL, &size);
					*icclength = size;
					*icc_m = new char[size];
					cmsSaveProfileToMem(profile, *icc_m, &size);
				}
				else if (RawProcessor.imgdata.params.output_color == 2) {
					profile = gImage::makeLCMSdcrawProfile("adobe", gamma);
					cmsSaveProfileToMem(profile, NULL, &size);
					*icclength = size;
					*icc_m = new char[size];
					cmsSaveProfileToMem(profile, *icc_m, &size);
				}
				else if (RawProcessor.imgdata.params.output_color == 3) {
					profile = gImage::makeLCMSdcrawProfile("wide", gamma);
					cmsSaveProfileToMem(profile, NULL, &size);
					*icclength = size;
					*icc_m = new char[size];
					cmsSaveProfileToMem(profile, *icc_m, &size);
				}
				else if (RawProcessor.imgdata.params.output_color == 4) {
					profile = gImage::makeLCMSdcrawProfile("prophoto", gamma);
					cmsSaveProfileToMem(profile, NULL, &size);
					*icclength = size;
					*icc_m = new char[size];
					cmsSaveProfileToMem(profile, *icc_m, &size);
				}
				else if (RawProcessor.imgdata.params.output_color == 5) {
					profile = cmsCreateXYZProfile();
					cmsSaveProfileToMem(profile, NULL, &size);
					*icclength = size;
					*icc_m = new char[size];
					cmsSaveProfileToMem(profile, *icc_m, &size);
				}
				else {
					*icc_m=NULL;
					*icclength = 0;
				}
			}
		}
	}

	RawProcessor.recycle();

	return img;

}
#endif //USE_DCRAW



