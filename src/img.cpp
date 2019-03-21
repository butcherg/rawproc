
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <vector>
#include <string>
#include <iostream>
#include <dirent.h>
#include <math.h> 

#include "gimage/gimage.h"
#include "elapsedtime.h"
#include "gimage/strutil.h"
#include "myConfig.h"

std::string param_string(std::string filter)
{
	std::string paramstr, name, val;

	std::map<std::string, std::string> c = myConfig::getConfig().getDefault();
	for (std::map<std::string, std::string>::iterator it=c.begin(); it!=c.end(); ++it) {
		name = it->first;
		val =  it->second;
		if (name.find(filter) != std::string::npos) {
			val = myConfig::getConfig().getValue(name);
			name = name.substr(name.find_last_of(".")+1, name.size());
			if (val != "") paramstr.append(string_format("%s=%s;",name.c_str(), val.c_str()));
		}
	}

	return paramstr;
}


//matchspec: takes a file name and returns the variant part, specified by the file specification
std::string matchspec(std::string fname, std::string fspec)
{
	if (fname.size() < fspec.size()) return "";
	std::string specf, specb;
	std::string fnamef, fnameb;
	std::string variant;
	int wp = fspec.find_first_of("*");
	specf = fspec.substr(0,wp);
	specb = fspec.substr(wp+1,fspec.size()-1);
	fnamef = fname.substr(0,wp);
	fnameb = fname.substr(fname.size()-specb.size(), fname.size()-1);
	variant = fname.substr(wp,fname.size()-fspec.size()+1);
	if ((specf.compare(fnamef) == 0) & (specb.compare(fnameb) == 0)) 
		return variant;
	return "";
}

//makename: returns a file name constructed using a file specification and the variant string to insert
std::string makename(std::string variant, std::string fspec)
{
	char e[1024];
	char *a, *b;
	strncpy(e,fspec.c_str(), 1023);
	if (e[0] == '*') {
		b = NULL;
		a = e+1;
	}
	else {
		b = strtok(e,"*");
		a = strtok(NULL,"*");
	}
	std::string m;
	if (b) m.append(b);
	m.append(variant);
	if (a) m.append(a);
	return m;
}

int countchar(std::string s, char c)
{
	int count = 0;
	for (int i=0; i<s.size(); i++) {
		if (s[i] == c) count++;
	}
	return count;
}

//gets the file out of a path/file string
std::string getfile(std::string path)
{
	std::size_t found = path.find_last_of("/\\");
	if (found == std::string::npos) return path;
	return path.substr(found+1);
}

//gets the path out of a path/file string
std::string getpath(std::string path)
{
	std::size_t found = path.find_last_of("/\\");
	if (found == std::string::npos) return ".";
	return path.substr(0,found);
}

std::vector<std::string> fileglob(std::string dspec)
{
	std::vector<std::string> glob;
	struct dirent *dp;
	DIR *dirp = opendir(".");
	while ((dp = readdir(dirp)) != NULL)
		if (matchspec(std::string(dp->d_name), dspec) != "")
			glob.push_back(std::string(dp->d_name));
	(void)closedir(dirp);
	return glob;
}


int file_exists (const char *filename)
{
	struct stat   buffer;   
	return (stat (filename, &buffer) == 0);
}

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

void strappend(char* s, char c)
{
        int len = strlen(s);
        s[len] = c;
        s[len+1] = '\0';
}

int _CRT_glob = 0;
bool force = false;

bool saveFile (gImage &savedib, std::string outfilename, std::string params, std::string commandstring)
{

	gImage dib = savedib;
	GIMAGE_FILETYPE filetype = gImage::getFileNameType(outfilename.c_str());
	
	std::string profilepath = myConfig::getConfig().getValueOrDefault("cms.profilepath","");
	if (profilepath[profilepath.length()-1] != '/') profilepath.push_back('/');
	
	std::string intentstr;
	cmsUInt32Number intent = INTENT_PERCEPTUAL;

	if (filetype == FILETYPE_JPEG) {
		profilepath.append(myConfig::getConfig().getValueOrDefault("output.jpeg.cms.profile",""));
		intentstr = myConfig::getConfig().getValueOrDefault("output.jpeg.cms.renderingintent","perceptual");
	}
	else if (filetype == FILETYPE_TIFF) {
		profilepath.append(myConfig::getConfig().getValueOrDefault("output.tiff.cms.profile",""));
		intentstr = myConfig::getConfig().getValueOrDefault("output.tiff.cms.renderingintent","perceptual");
	}
	else if (filetype == FILETYPE_PNG) {
		profilepath.append(myConfig::getConfig().getValueOrDefault("output.png.cms.profile",""));
		intentstr = myConfig::getConfig().getValueOrDefault("output.png.cms.renderingintent","perceptual");
	}
				
	if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
	if (intentstr == "saturation") intent = INTENT_SATURATION;
	if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
	if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

	_mark();
	//printf("Saving file %s %s... ",outfile[0].c_str(), outfile[1].c_str());
	printf("Saving file %s %s... ",outfilename.c_str(), params.c_str());
	dib.setInfo("Software","img 0.1");
	dib.setInfo("ImageDescription", commandstring);
	
	cmsHPROFILE profile = cmsOpenProfileFromFile(profilepath.c_str(), "r");
	if (dib.getProfile() && profile)
		dib.saveImageFile(outfilename.c_str(), params.c_str(), profile, intent);
	else
		dib.saveImageFile(outfilename.c_str(), params.c_str());

	if (dib.getLastError() == GIMAGE_OK) {
		printf("done. (%fsec)\n",_duration());
		return true;
	}
	else {
		printf("Error: %s: %s\n",dib.getLastErrorMessage().c_str(), outfilename.c_str());
		return false;
	}
}

//used to contain a list of corresponding input and output file names,
//constructed from the input and output file specifications
struct fnames {
	std::string infile, outfile, variant;
};

std::string commandstring;

void do_cmd(gImage &dib, std::string commandstr, std::string outfile)
{
		char c[256];
		strncpy(c, commandstr.c_str(), 255);
		char* cmd = strtok(c,":");
		
		//img <li>colorspace:profilefile[,convert|assign][,renderingintent][,bpc]</li>
		if (strcmp(cmd,"colorspace") == 0) { 
			char *profstr = strtok(NULL, ",");
			char *opstr = strtok(NULL, ",");
			char *istr = strtok(NULL, ",");
			char *bpstr = strtok(NULL, " ");
			
			std::string profile;
			std::string profilepath =  gImage::getProfilePath(); //myConfig::getConfig().getValueOrDefault("cms.profilepath","").c_str();
			if (profstr == NULL) {
				printf("colorspace: no profile.\n");
				return;
			}
			else profile = std::string(profstr);
			
			std::string operation = "convert";
			if (opstr) operation = std::string(opstr);
			
			cmsUInt32Number intent = INTENT_RELATIVE_COLORIMETRIC;
			if (istr) {
				std::string intentstr = std::string(istr);
				if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
				if (intentstr == "saturation") intent = INTENT_SATURATION;
				if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
				if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;
			}
			
			bool bp = false;
			std::string bpcomp;
			if (bpstr != NULL) bpcomp = std::string(bpstr);
			if (bpcomp == "bpc") bp = true;
			
			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.colorspace.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);

			printf("colorspace: %s, %s, %s, %s (%d threads)... ",profile.c_str(),opstr,istr,bpstr,threadcount);
			_mark();
			if (operation == "convert")
				if (dib.ApplyColorspace(profilepath+profile, intent, bp, threadcount) != GIMAGE_OK) printf("Error: %s\n", dib.getLastErrorMessage().c_str());
			else
				dib.AssignColorspace(profilepath+profile);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%s,%s,%s,%s ",cmd,profile.c_str(),opstr,istr,bpstr);
			commandstring += std::string(cs);
		}
		
		//img <li>bright:[-100 - 100] default: 0 (no-bright)</li>
		else if (strcmp(cmd,"bright") == 0) {  
			double bright = atof(myConfig::getConfig().getValueOrDefault("tool.bright.initialvalue","0").c_str());
			char *b = strtok(NULL," ");
			if (b) bright = atof(b);

			Curve ctrlpts;
			ctrlpts.insertpoint(0,0);
			if (bright < 0)
				ctrlpts.insertpoint(255,255+bright);
			else
				ctrlpts.insertpoint(255-bright,255);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.bright.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("bright: %0.2f (%d threads)... ",bright,threadcount);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.0f ",cmd, bright);
			commandstring += std::string(cs);
		}

		//img <li>demosaic:[half|half_resize|color|vng|amaze|dcb|rcd|igv|lmmse|ahd] default: ahd</li>
		else if (strcmp(cmd,"demosaic") == 0) {  
			std::string demosaic = myConfig::getConfig().getValueOrDefault("tool.demosaic.default","ahd").c_str();
			char *d = strtok(NULL," ");
			if (d) demosaic = d;

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.demosaic.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("demosaic: %s (%d threads)... ",demosaic.c_str(),threadcount);

			_mark();
			if (demosaic == "color")
				dib.ApplyMosaicColor(threadcount);
			else if (demosaic == "half")
				dib.ApplyDemosaicHalf(false, threadcount);
			else if (demosaic == "half_resize")
				dib.ApplyDemosaicHalf(true, threadcount);
#ifdef USE_LIBRTPROCESS
			else if (demosaic == "vng")
				dib.ApplyDemosaicVNG(threadcount);
			else if (demosaic == "amaze")
				dib.ApplyDemosaicAMAZE(1.0, 0, 1.0, 1.0, threadcount);
			else if (demosaic == "dcb")
				dib.ApplyDemosaicDCB(1, false, threadcount);
			else if (demosaic == "rcd")
				dib.ApplyDemosaicRCD(threadcount);
			else if (demosaic == "igv")
				dib.ApplyDemosaicIGV(threadcount);
			else if (demosaic == "lmmse")
				dib.ApplyDemosaicLMMSE(1, threadcount);
			else if (demosaic == "ahd")
				dib.ApplyDemosaicAHD(threadcount);
			else if (demosaic == "xtran_fast") 
				dib.ApplyDemosaicXTRANSFAST(threadcount);
			else if (demosaic == "xtran_markesteijn") 
				dib.ApplyDemosaicXTRANSMARKESTEIJN(1, false, threadcount);
#endif
			else printf("no-op... ");
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%s ",cmd, demosaic.c_str());
			commandstring += std::string(cs);
		}

		//img <li>addexif:tagname,value - tagname must be valid EXIF tag for it to survive the file save...</li>
		else if (strcmp(cmd,"addexif") == 0) {  
			char *name = strtok(NULL, ",");
			char *value = strtok(NULL, " ");
			
			printf("addexif: %s=%s ... ",name,value);

			dib.setInfo(std::string(name),std::string(value));
		}

		//img <li>blackwhitepoint[:rgb|red|green|blue][,0-127,128-255] default: auto blackwhitepoint determination. The calculated points will be used in the metafile entry.</li>
		else if (strcmp(cmd,"blackwhitepoint") == 0) {   
			char *c, *b, *w;
			GIMAGE_CHANNEL channel = CHANNEL_RGB;
			std::string chan;
			double blk=0.0, wht=255.0;
			double blkthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blackthreshold","0.05").c_str());
			double whtthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitethreshold","0.05").c_str());
			int blklimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blacklimit","128").c_str());
			int whtlimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitelimit","128").c_str()); 
			long whtinitial = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whiteinitialvalue","255").c_str());
		
			long hmax = 0;
			int maxpos;
			c = strtok(NULL,", ");
			if (c) { //first token is a channel, or min 
				chan = std::string(c);
				if (chan == "rgb" | chan == "red" |chan == "green" | chan == "blue" | chan == "min") {
					b = strtok(NULL,", ");
					w = strtok(NULL,", ");
					wht = 255; blk = 0;
					if (!b) { //no black/white, compute using channel:
						std::vector<double> bwpts = dib.CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial, chan);
						blk = bwpts[0];
						wht = bwpts[1];

					}
				}
				else { //no channel, just black and white:
					b = c;
					w = strtok(NULL,", ");
					if (w) wht = atof(w);
					if (b) blk = atof(b);
				}
			}
			else { //no tokens, do auto rgb:
				std::vector<double> bwpts = dib.CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial);
				blk = bwpts[0];
				wht = bwpts[1];
			}
			
			if (chan == "rgb")   channel = CHANNEL_RGB;
			if (chan == "red")   channel = CHANNEL_RED;
			if (chan == "green") channel = CHANNEL_GREEN;
			if (chan == "blue")  channel = CHANNEL_BLUE;

			Curve ctrlpts;
			ctrlpts.insertpoint(blk,0);
			ctrlpts.insertpoint(wht,255);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("blackwhitepoint: %s,%0.2f,%0.2f (%d threads)... ",chan.c_str(),blk,wht,threadcount);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), channel, threadcount);
			//dib.ApplyToneLine(blk, wht, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.0f,%0.0f ",cmd, blk, wht);
			commandstring += std::string(cs);
		}

		//img <li>contrast:[-100 - 100] default: 0 (no-contrast)</li>
		else if (strcmp(cmd,"contrast") == 0) {  
			double contrast=atof(myConfig::getConfig().getValueOrDefault("tool.contrast.initialvalue","0").c_str());
			char *c = strtok(NULL," ");
			if (c) contrast = atof(c);

			Curve ctrlpts;
			if (contrast < 0) {
				ctrlpts.insertpoint(0,-contrast);
				ctrlpts.insertpoint(255,255+contrast);
			}
			else {
				ctrlpts.insertpoint(contrast,0);
				ctrlpts.insertpoint(255-contrast,255);
			}

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.contrast.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("contrast: %0.2f (%d threads)... ",contrast,threadcount);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.0f ",cmd, contrast);
			commandstring += std::string(cs);
		}

		//img <li>gamma:[0.0-5.0] default: 1.0 (linear, or no-gamma)</li>
		else if (strcmp(cmd,"gamma") == 0) {  
			double gamma=atof(myConfig::getConfig().getValueOrDefault("tool.gamma.initialvalue","2.2").c_str());
			char *g = strtok(NULL," ");
			if (g) gamma = atof(g);

			Curve ctrlpts;
			double exponent = 1 / gamma;
			double v = 255.0 * (double)pow((double)255, -exponent);
			for (int i = 0; i< 256; i+=1) {
				double color = (double)pow((double)i, exponent) * v;
				if (color > 255.0) color = 255.0;
				ctrlpts.insertpoint((double) i, color);
			}	

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.gamma.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("gamma: %0.2f (%d threads)... ",gamma,threadcount);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.1f ",cmd, gamma);
			commandstring += std::string(cs);
		}


		//img <li>resize:w[,h] If either w or h is 0, resize preserves aspect of that dimension.  If only one number is present, the image is resized to that number along the longest dimension, preserving aspect.</li>
		else if (strcmp(cmd,"resize") == 0) {  
			unsigned w, h;  //don't read defaults from properties
			std::string algo = myConfig::getConfig().getValueOrDefault("tool.resize.algorithm","catmullrom");
			char *wstr = strtok(NULL,", ");
			char *hstr = strtok(NULL,", ");
			char *astr = strtok(NULL," ");
			if (wstr == NULL) {
				printf("Error: resize needs at least one parameter.\n");
			}
			else {
				if (wstr) w = atoi(wstr);
				if (hstr) h = atoi(hstr);
				if (astr) algo = std::string(astr);
				unsigned dw = dib.getWidth();
				unsigned dh = dib.getHeight();

				//if only one number is provided, put it in the largest dimension, set the other to 0
				if (hstr == NULL) {
					if (dh > dw) {
						h = w;
						w = 0;
					}
					else {
						h =0;
					}
				}

				if (h ==  0) h = dh * ((float)w/(float)dw);
				if (w == 0)  w = dw * ((float)h/(float)dh); 

				RESIZE_FILTER filter = FILTER_CATMULLROM;  //must be same as default if tool.resize.algorithm is not specified
				if (algo == "box") filter = FILTER_BOX;
				if (algo == "bilinear") filter = FILTER_BILINEAR;
				if (algo == "bspline") filter = FILTER_BSPLINE;
				if (algo == "bicubic") filter = FILTER_BICUBIC;
				if (algo == "catmullrom") filter = FILTER_CATMULLROM;
				if (algo == "lanczos3") filter = FILTER_LANCZOS3;

				int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.resize.cores","0").c_str());
				if (threadcount == 0) 
					threadcount = gImage::ThreadCount();
				else if (threadcount < 0) 
					threadcount = std::max(gImage::ThreadCount() + threadcount,0);
				printf("resize: %dx%d,%s (%d threads)... ",w,h,algo.c_str(), threadcount);

				_mark();
				dib.ApplyResize(w,h, filter, threadcount);
				printf("done (%fsec).\n", _duration());
				char cs[256];
				if (wstr)
					if (hstr)
						if (astr)
							sprintf(cs, "%s:%s,%s,%s ",cmd, wstr, hstr, astr);
						else
							sprintf(cs, "%s:%s,%s ",cmd, wstr, hstr);
					else
						sprintf(cs, "%s:%s ",cmd, wstr);
				commandstring += std::string(cs);
			}
		}

		//img <li>rotate:[-45.0 - 45.0] default: 0 (no-rotate)</li>
		else if (strcmp(cmd,"rotate") == 0) {  
			double angle= atof(myConfig::getConfig().getValueOrDefault("tool.rotate.initialvalue","0.0").c_str());
			char *s = strtok(NULL," ");
			if (s) angle = atof(s);
			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.rotate.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("rotate: %0.2f (%d threads)... ",angle,threadcount);

			_mark();
			dib.ApplyRotate(angle, false, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.1f ",cmd, angle);
			commandstring += std::string(cs);
		}

		//img <li>sharpen:[0 - 10, default: 0 (no-sharpen)</li>
		else if (strcmp(cmd,"sharpen") == 0) {  
			double sharp= atof(myConfig::getConfig().getValueOrDefault("tool.sharpen.initialvalue","0").c_str());
			char *s = strtok(NULL," ");
			if (s) sharp = atof(s);
			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.sharpen.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("sharp: %0.2f (%d threads)... ",sharp, threadcount);

			_mark();
			dib.ApplySharpen(sharp, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.0f ",cmd, sharp);
			commandstring += std::string(cs);
		}

		//img <li>crop:x,y,w,y  no defaults</li>
		else if (strcmp(cmd,"crop") == 0) {  //#crop:x,y,w,h      
			unsigned x=0, y=0, width=0, height=0;
			char *xstr = strtok(NULL,", ");
			char *ystr = strtok(NULL,", ");
			char *wstr = strtok(NULL,", ");
			char *hstr = strtok(NULL," ");
			if (xstr) x = atoi(xstr);
			if (ystr) y = atoi(ystr);
			if (wstr) width = atoi(wstr);
			if (hstr) height = atoi(hstr);
			width += x;
			height += y;
			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("crop: %d,%d %dx%d (%d threads)... ",x,y,width,height,threadcount);

			_mark();
			dib.ApplyCrop(x,y,width,height,threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%d,%d,%d,%d ",cmd, x, y, width, height);
			commandstring += std::string(cs);
		}

		//img <li>saturation:[0 - 5.0] default=1.0, no change</li>
		else if (strcmp(cmd,"saturation") == 0) {  
			double saturation= atof(myConfig::getConfig().getValueOrDefault("tool.saturate.initialvalue","1.0").c_str());
			char *s = strtok(NULL," ");
			if (s) saturation = atof(s);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.saturation.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("saturate: %0.2f (%d threads)... ",saturation,threadcount);

			_mark();
			dib.ApplySaturate(saturation, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.1f ",cmd, saturation);
			commandstring += std::string(cs);
		}

		//img <li>denoise:[0 - 100.0],[1-10],[1-10], default=0.0,1,3</li>
		else if (strcmp(cmd,"denoise") == 0) {  
			double sigma= atof(myConfig::getConfig().getValueOrDefault("tool.denoise.initialvalue","0").c_str());
			int local = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.local","3").c_str());
			int patch = atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.patch","1").c_str());
			char *s = strtok(NULL,", ");
			char *l = strtok(NULL,", ");
			char *p = strtok(NULL," ");
			if (s) sigma = atof(s);
			if (l) local = atoi(l);
			if (p) patch = atoi(p);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.denoise.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("denoise: %0.2f (%d threads)... ",sigma,threadcount);

			_mark();
			dib.ApplyNLMeans(sigma, local, patch, threadcount);  
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.1f,%d,%d ",cmd, sigma, 3, 1);
			commandstring += std::string(cs);
		}

		//img <li>tint:[r,g,b] default: 0,0,0 (doesn't have a corresponding tool in rawproc)</li>
		else if (strcmp(cmd,"tint") == 0) {  
			double red=0.0; double green=0.0; double blue = 0.0;
			char *r = strtok(NULL,", ");
			char *g = strtok(NULL,", ");
			char *b = strtok(NULL," ");
			if (r) red = atof(r);
			if (g) green = atof(g);
			if (b) blue = atof(b);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.tint.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("tint: %0.2f,%0.2f,%0.2f (%d threads)... ",red,green,blue,threadcount);

			_mark();
			dib.ApplyTint(red,green,blue, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.1f,%0.1f,%0.1f ",cmd, red, green, blue);
			commandstring += std::string(cs);
		}
		
		//img <li>whitebalance:[auto]|[patch]|[camera]|[rmult,gmult,bmult] default: auto, based on "gray world"</li>
		else if (strcmp(cmd,"whitebalance") == 0) {  
			std::string op;
			double redmult=1.0; 
			double greenmult = 1.0; 
			double bluemult = 1.0;
			int patchx, patchy; double patchrad;
			char cs[256];
			
			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.whitebalance.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);

			char *p = strtok(NULL, " ");
			if (p) {
				std::vector<std::string> parm = split(std::string(p),",");
				if (parm[0] == "auto") {
					op = "auto";
					printf("whitebalance: %s (%d threads)... ",op.c_str(),threadcount);
					_mark();
					dib.ApplyWhiteBalance(threadcount);
					printf("done (%fsec).\n",_duration());
					sprintf(cs, "%s:%s ",cmd, op.c_str());
				}
				else if (parm[0] == "patch") {
					op = "patch";
					patchx   = atoi(parm[1].c_str());
					patchy   = atoi(parm[2].c_str());
					patchrad = atof(parm[3].c_str());
					printf("whitebalance: %s,%d,%d,%0.1f (%d threads)... ",op.c_str(),patchx,patchy,patchrad,threadcount);
					_mark();
					dib.ApplyWhiteBalance((unsigned) patchx, (unsigned) patchy, patchrad, threadcount);
					printf("done (%fsec).\n",_duration());
					sprintf(cs, "%s:%s,%d,%d,%0.1f ",cmd, op.c_str(), patchx, patchy, patchrad);
				}
				else if (parm[0] == "camera") {
					op = "camera";
					std::string cameraWB = dib.getInfoValue("LibrawWhiteBalance");
					if (cameraWB != "") {
						std::vector<std::string> m = split(cameraWB,",");
						redmult   = atof(m[0].c_str());
						greenmult = atof(m[1].c_str());
						bluemult  = atof(m[2].c_str());
						printf("whitebalance: %s,%0.2f,%0.2f,%0.2f (%d threads)... ",op.c_str(),redmult,greenmult,bluemult,threadcount);
						_mark();
						dib.ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
						printf("done (%fsec).\n",_duration());
						sprintf(cs, "%s:%s ",cmd, op.c_str());
					}
				}
				else { // parameters are just three multipliers
					redmult   = atof(parm[0].c_str());
					greenmult = atof(parm[1].c_str());
					bluemult  = atof(parm[2].c_str());
					printf("whitebalance: %0.1f,%0.1f,%0.1f (%d threads)... ",redmult, greenmult, bluemult,threadcount);
					_mark();
					dib.ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
					printf("done (%fsec).\n",_duration());
					sprintf(cs, "%s:%0.1f,%0.1f,%0.1f ",cmd, redmult, greenmult, bluemult);
				}
				commandstring += std::string(cs);
			}
		}

		//img <li>gray:[r,g,b] default: 0.21,0.72,0.07</li> 
		else if (strcmp(cmd,"gray") == 0) {  
			double red   = atof(myConfig::getConfig().getValueOrDefault("tool.gray.r","0.21").c_str()); 
			double green = atof(myConfig::getConfig().getValueOrDefault("tool.gray.g","0.72").c_str()); 
			double blue  = atof(myConfig::getConfig().getValueOrDefault("tool.gray.b","0.07").c_str());
			char *r = strtok(NULL,", ");
			char *g = strtok(NULL,", ");
			char *b = strtok(NULL," ");
			if (r) red = atof(r);
			if (g) green = atof(g);
			if (b) blue = atof(b);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.gray.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("gray: %0.2f,%0.2f,%0.2f (%d threads)... ",red,green,blue,threadcount);

			_mark();
			dib.ApplyGray(red,green,blue, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.1f,%0.1f,%0.1f ",cmd, red, green, blue);
			commandstring += std::string(cs);
		}

		else if (strcmp(cmd,"redeye") == 0) {  //not documented, for testing only
			int limit = atoi(myConfig::getConfig().getValueOrDefault("tool.redeye.radius","50").c_str()); 
			double threshold = atof(myConfig::getConfig().getValueOrDefault("tool.redeye.threshold","1.5").c_str());
			char *sx = strtok(NULL,", ");
			char *sy = strtok(NULL,", ");
			char *st = strtok(NULL,", ");
			char *sl = strtok(NULL,", ");
			if (sx) {
				int x = atoi(sx);
				if (sy) {
					if (st) threshold = atof(st);
					if (sl) limit = atoi(sl);
					int y = atoi(sy);
					std::vector<coord> pts;
					struct coord pt; pt.x = x; pt.y = y;
					pts.push_back(pt);
					int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.redeye.cores","0").c_str());
					if (threadcount == 0) 
						threadcount = gImage::ThreadCount();
					else if (threadcount < 0) 
						threadcount = std::max(gImage::ThreadCount() + threadcount,0);
					printf("redeye (%d threads)... ", threadcount);
					_mark();
					dib.ApplyRedeye(pts, threshold, limit, false, 1.0,  threadcount);
					printf("done (%fsec).\n",_duration());
				}
				else printf("redeye: bad y coord\n");
			}
			else printf("redeye: bad x coord\n");
		
		}

		//img <li>curve:[rgb,|red,|green,|blue,]x1,y1,x2,y2,...xn,yn  Default channel: rgb</li>
		else if (strcmp(cmd,"curve") == 0) {
			Curve crv;
			int ctstart;
			GIMAGE_CHANNEL channel;
			std::vector<cp> ctrlpts;
			char *p = strtok(NULL," ");
			std::vector<std::string> cpts = split(std::string(p), ",");
			ctstart = 1;
			if      (cpts[0] == "rgb") 	channel = CHANNEL_RGB;
			else if (cpts[0] == "red")	channel = CHANNEL_RED;
			else if (cpts[0] == "green")	channel = CHANNEL_GREEN;
			else if (cpts[0] == "blue")	channel = CHANNEL_BLUE;
			else {
				channel = CHANNEL_RGB;
				ctstart = 0;
			}
			for (int i=ctstart; i<cpts.size()-1; i+=2) {
				crv.insertpoint(atof(cpts[i].c_str()), atof(cpts[i+1].c_str()));
			}
			ctrlpts = crv.getControlPoints();
			
			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.curve.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("curve: %s (%d threads)... ",p,threadcount);
			_mark();
			dib.ApplyToneCurve(ctrlpts, channel, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%s ",cmd, p);
			commandstring += std::string(cs);
			
		}
		
		//img <li>exposure:ev default: 1.0</li>
		else if (strcmp(cmd,"exposure") == 0) {
			double ev = atof(myConfig::getConfig().getValueOrDefault("tool.exposure.initialvalue","0.0").c_str());
			char *s = strtok(NULL," ");
			if (s) ev = atof(s);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.exposure.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("exposure: %0.2f (%d threads)... ",ev,threadcount);

			_mark();
			dib.ApplyExposureCompensation(ev, threadcount);  //local and patch hard-coded, for now...
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.1f ",cmd, ev);
			commandstring += std::string(cs);
		}

		//img <li>subtract:val|camera|file,filename  val=a float value, "camera" retrieves the camera </li>
		else if (strcmp(cmd,"subtract") == 0) {
			double subtract;
			char filename[256];
			char *v = strtok(NULL,", ");
			char *f = strtok(NULL," ");

			if (strcmp(v,"camera") == 0) {
				subtract = atof(dib.getInfoValue("LibrawBlack").c_str()) / 65536.0;
			}
			else if (strcmp(v,"file") == 0) {
				strcpy(filename, f);
			}
			else {
				subtract = atof(v);
			}

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.subtract.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("subtract: %f (%d threads)... ",subtract,threadcount);

			_mark();
			if (strcmp(v,"file") == 0)
				dib.ApplySubtract(std::string(filename), threadcount);  
			else
				dib.ApplySubtract(subtract, threadcount);  
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.1f ",cmd, subtract);
			commandstring += std::string(cs);
		}

		//img <li>highlight:1-10</li>
		else if (strcmp(cmd,"highlight") == 0) {
			double highlight = atof(myConfig::getConfig().getValueOrDefault("tool.highlight.level","0").c_str());
			double threshold = atof(myConfig::getConfig().getValueOrDefault("tool.highlight.threshold","192").c_str());
			char *h = strtok(NULL,", ");
			char *t = strtok(NULL," ");
			if (h) highlight = atof(h);
			if (t) threshold = atof(t);

			Curve ctrlpts;
			ctrlpts.insertpoint(0,0);
			ctrlpts.insertpoint(threshold-20,threshold-20);
			ctrlpts.insertpoint(threshold,threshold);
			ctrlpts.insertpoint((threshold+threshold/2)-highlight,(threshold+threshold/2)+highlight);
			ctrlpts.insertpoint(255,255);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.highlight.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("highlight: %0.2f,%0.2f (%d threads)... ",highlight,threshold,threadcount);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.0f,%0.0f ",cmd, highlight,threshold);
			commandstring += std::string(cs);
		}

		//img <li>shadow:1-10</li>
		else if (strcmp(cmd,"shadow") == 0) {
			double shadow = atof(myConfig::getConfig().getValueOrDefault("tool.shadow.level","0").c_str());
			double threshold = atof(myConfig::getConfig().getValueOrDefault("tool.shadow.threshold","64").c_str());
			char *s = strtok(NULL,", ");
			char *t = strtok(NULL," ");
			if (s) shadow = atof(s);
			if (t) threshold = atof(t);

			Curve ctrlpts;
			ctrlpts.insertpoint(0,0);
			ctrlpts.insertpoint((threshold/2)-shadow,(threshold/2)+shadow);
			ctrlpts.insertpoint(threshold,threshold);
			ctrlpts.insertpoint(threshold+20,threshold+20);
			ctrlpts.insertpoint(255,255);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.shadow.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("shadow: %0.2f,%0.2f (%d threads)... ",shadow,threshold,threadcount);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.0f,%0.0f ",cmd, shadow,threshold);
			commandstring += std::string(cs);
		}
		
		//img <li>rotate90 - rotate 90 degrees clockwise</li>
		else if (strcmp(cmd,"rotate90") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("rotate90 (%d threads)... ", threadcount);
			_mark();
			dib.ApplyRotate90(threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "rotate:90 ");
			commandstring += std::string(cs);
		}

		//img <li>rotate180 - rotate 180 degrees</li>
		else if (strcmp(cmd,"rotate180") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("rotate180 (%d threads)... ", threadcount);
			_mark();
			dib.ApplyRotate180(threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "rotate:180 ");
			commandstring += std::string(cs);
		}

		//img <li>rotate270 - rotate 270 degrees clockwise</li>
		else if (strcmp(cmd,"rotate270") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("rotate270 (%d threads)... ", threadcount);
			_mark();
			dib.ApplyRotate270(threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "rotate:270 ");
			commandstring += std::string(cs);
		}

		//these don't have rawproc equivalents, so they're not added to the metadata-embedded command
		//img <li>hmirror - flip horizontal</li>
		else if (strcmp(cmd,"hmirror") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("mirror horizontal (%d threads)... ", threadcount);
			_mark();
			dib.ApplyHorizontalMirror(threadcount);
			printf("done (%fsec).\n",_duration());
		}

		//img <li>vmirror - flip upside down</li>	
		else if (strcmp(cmd,"vmirror") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("mirror vertical (%d threads)... ", threadcount);
			_mark();
			dib.ApplyVerticalMirror(threadcount);
			printf("done (%fsec).\n",_duration());
		}

		else if (strcmp(cmd,"tonescale") == 0) {  
			double red   = atof(myConfig::getConfig().getValueOrDefault("tool.tonescale.r","0.21").c_str()); 
			double green = atof(myConfig::getConfig().getValueOrDefault("tool.tonescale.g","0.72").c_str()); 
			double blue  = atof(myConfig::getConfig().getValueOrDefault("tool.tonescale.b","0.07").c_str());
			char *r = strtok(NULL,", ");
			char *g = strtok(NULL,", ");
			char *b = strtok(NULL," ");
			if (r) red = atof(r);
			if (g) green = atof(g);
			if (b) blue = atof(b);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.tonescale.cores","0").c_str());
			if (threadcount == 0) 
                                threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			printf("tonescale: %0.2f,%0.2f,%0.2f (%d threads)... ",red,green,blue,threadcount);
			_mark();

			gImage mask = gImage(dib);
			mask.ApplyGray(red, green, blue, threadcount);
			std::vector<pix>& m = mask.getImageData();

			std::vector<pix>& image = dib.getImageData();
			unsigned w = dib.getWidth();
			unsigned h = dib.getHeight();

			#pragma omp parallel for num_threads(threadcount)
			for (unsigned x=0; x<w; x++) {
				for (unsigned y=0; y<h; y++) {
					unsigned pos = x + y*w;
					double pct = ((image[pos].r*0.21)+(image[pos].g*0.72)+(image[pos].b*0.07)) / m[pos].r;
					image[pos].r *= pct;
					image[pos].g *= pct;
					image[pos].b *= pct;
				}
			}




			//dib.ApplyToneGrayMask(red,green,blue, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			sprintf(cs, "%s:%0.1f,%0.1f,%0.1f ",cmd, red, green, blue);
			commandstring += std::string(cs);
		}

/*
		else if (strcmp(cmd,"blur") == 0) {  
			double kernel1[5][5] = 
			{
				0.003765,	0.015019,	0.023792,	0.015019,	0.003765,
				0.015019,	0.059912,	0.094907,	0.059912,	0.015019,
				0.023792,	0.094907,	0.150342,	0.094907,	0.023792,
				0.015019,	0.059912,	0.094907,	0.059912,	0.015019,
				0.003765,	0.015019,	0.023792,	0.015019,	0.003765
			};
			double kernel3[5][5] = 
			{
				0.031827,	0.037541,	0.039665,	0.037541,	0.031827,
				0.037541,	0.044281,	0.046787,	0.044281,	0.037541,
				0.039665,	0.046787,	0.049434,	0.046787,	0.039665,
				0.037541,	0.044281,	0.046787,	0.044281,	0.037541,
				0.031827,	0.037541,	0.039665,	0.037541,	0.031827
			};

			double * karray = (double *) kernel3;  //default
			char * k = strtok(NULL, " ");
			if (k)
				if (strcmp(k,"1") == 0)
					karray = (double *) kernel1;

			std::vector<double> kdata;
			for (unsigned i=0; i<25; i++) kdata.push_back(karray[i]);

			int threadcount = gImage::ThreadCount();
			printf("blur, 2Dkernel: %s (%d threads)... ",k, threadcount);
			_mark();
			dib.Apply2DConvolutionKernel(kdata, 5, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			if (k)
				sprintf(cs, "%s:%s ",cmd, k);
			else
				sprintf(cs, "%s ",cmd);
			//commandstring += std::string(cs);  //uncomment when rawproc supports blur
		}
*/
		else if (strcmp(cmd,"blur") == 0) { 
			unsigned kernelsize = 3; 
			double sigma = 1.0;
			char * s = strtok(NULL, ", ");
			char * k = strtok(NULL, ", ");
			
			if (s) sigma = atof(s);
			if (k) kernelsize = atoi(k);

			int threadcount = gImage::ThreadCount();

			printf("blur: sigma=%0.1f, kernelsize=%d (%d threads)... ", sigma, kernelsize, threadcount);
			_mark();
			dib.ApplyGaussianBlur(sigma, kernelsize, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			if (s)
				if (k)
					sprintf(cs, "%s:%0.1f,%d ",cmd, sigma, kernelsize);
				else
					sprintf(cs, "%s:%0.1f ",cmd, sigma);
			else
				sprintf(cs, "%s ",cmd);
			//commandstring += std::string(cs);  //uncomment when rawproc supports blur
		}

		else if (strcmp(cmd,"save") == 0) {
			char *of = strtok(NULL, ", ");
			char *params = strtok(NULL, ", ");

			std::string outfilename = std::string(of);
			if (countchar(outfilename,'*') == 1) outfilename = makename(outfile, outfilename);

			if (!force && file_exists(outfilename.c_str())) {
				printf("save: file %s exists, skipping...\n",outfilename.c_str());
			}
			else {

				if (params)
					saveFile (dib, outfilename, std::string(params), std::string(commandstring));
				else
					saveFile (dib, outfilename, "", std::string(commandstring));
			} 
		}

		else printf("Unrecognized command: %s.  Continuing...\n",cmd);

}


//https://github.com/ccxvii/asstools/blob/master/getopt.c
/*
 * This is a version of the public domain getopt implementation by
 * Henry Spencer originally posted to net.sources.
 *
 * This file is in the public domain.
 */


#define getopt xgetopt
#define optarg xoptarg
#define optind xoptind

char *optarg; /* Global argument pointer. */
int optind = 0; /* Global argv index. */

static char *scan = NULL; /* Private scan pointer. */

int
getopt(int argc, char *argv[], char *optstring)
{
	char c;
	char *place;

	optarg = NULL;

	if (!scan || *scan == '\0') {
		if (optind == 0)
			optind++;

		if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
			return EOF;
		if (argv[optind][1] == '-' && argv[optind][2] == '\0') {
			optind++;
			return EOF;
		}

		scan = argv[optind]+1;
		optind++;
	}

	c = *scan++;
	place = strchr(optstring, c);

	if (!place || c == ':') {
		fprintf(stderr, "%s: unknown option -%c\n", argv[0], c);
		return '?';
	}

	place++;
	if (*place == ':') {
		if (*scan != '\0') {
			optarg = scan;
			scan = NULL;
		} else if( optind < argc ) {
			optarg = argv[optind];
			optind++;
		} else {
			fprintf(stderr, "%s: option requires argument -%c\n", argv[0], c);
			return ':';
		}
	}

	return c;
}

//end getopt.c

std::string getFileType(std::string filename)
{
	size_t pos = filename.find_last_of(".");
	if (pos = std::string::npos) return "";
	std::string ext = filename.substr(pos);
	if (ext == "jpg" | ext == "jpeg" | ext == "JPG" | ext == "JPEG") return "jpeg";
	if (ext == "tif" | ext == "tiff" | ext == "TIF" | ext == "TIFF") return "tiff";
	if (ext == "png" | ext == "PNG") return "png";
	return "raw";
}



int main (int argc, char **argv) 
{
	char * filename;

	std::string conffile;
	bool noconf = false;
	int f;
	//opterr = 0;

	while ((f = getopt(argc, argv, (char *) "fnc:")) != -1)
		switch(f) {
			case 'f':  //force processing even if output file exists
				force = true;
				break;
			case 'n': //no config file is used
				noconf = true;
				break;
			case 'c': //use the specified config file
				conffile = std::string(optarg);
				break;
			case '?':
				exit(-1);
			default:
				exit(-1);

		}
	
	//the corresponding input and output file names 
	std::vector<fnames> files;
	
	int c;
	int flags;
	//gImage dib;

	std::string conf_cwd = getCwdConfigFilePath();
	std::string conf_configd = getAppConfigFilePath();

	if (!noconf) {
		if (conffile != "") {
			if (access(conffile.c_str(), 0 ) == 0) {
				myConfig::loadConfig(conffile);
				printf("configuration file: %s\n", conffile.c_str());
			}
			else printf("configuration file %s not found.\n", conffile.c_str());
		}
		else if (access( conf_cwd.c_str(), 0 ) == 0) {
			myConfig::loadConfig(conf_cwd);
			printf("configuration file: %s\n", conf_cwd.c_str());
		}
		else if (access( conf_configd.c_str(), 0 ) == 0) {
			myConfig::loadConfig(conf_configd);
			printf("configuration file: %s\n", conf_configd.c_str());
		}
	}

	gImage::setProfilePath(filepath_normalize(myConfig::getConfig().getValueOrDefault("cms.profilepath",getCwd())));

	#ifdef USE_DCRAW
	gImage::setdcrawPath(filepath_normalize(myConfig::getConfig().getValueOrDefault("input.raw.dcraw.path","dcraw")));
	#endif

	if (argc < 2) {
		//printf("Error: No input file specified.\n");
		#ifdef VERSION
		printf("img version: %s build date: %s\n",VERSION, BUILDDATE);
		#else
		printf("img build date: %s\n", BUILDDATE);
		#endif
		printf("Usage: img inputfile [command[:parameters] ...] outputfile\n\n");
		printf("inputfile and output file can have one wildcard each, '*', to process \n");
		printf("multiple files, e.g., 'img *.NEF gamma:2.2 blackwhitepoint *.tif' will\n");
		printf("open all the .NEFs in the current directory, apply gamma and auto black/white \n");
		printf("point correction to each,and save each as the corresponding filename.tif.\n\n");
		printf("Available commands:\n");

		printf("\tcolorspace:profilefile[,convert|assign][,renderingintent][,bpc]\n");
		printf("\tbright:[-100 - 100] default: 0 (no-bright)\n");
		printf("\tdemosaic:[half|half_resize|color|vng|amaze|dcb|rcd|igv|lmmse|ahd|\n\t\txtran_fast|xtran_markesteijn] default: ahd\n");
		printf("\taddexif:tagname,value - tagname must be valid EXIF tag for it\n\t\t to survive the file save...\n");
		printf("\tblackwhitepoint[:rgb|red|green|blue][,0-127,128-255] \n\t\tdefault: auto blackwhitepoint determination. The \n\t\tcalculated points will be used in the metafile entry.\n");
		printf("\tcontrast:[-100 - 100] default: 0 (no-contrast)\n");
		printf("\tgamma:[0.0-5.0] default: 1.0 (linear, or no-gamma)\n");
		printf("\tresize:w[,h] If either w or h is 0, resize preserves aspect of\n\t\t that dimension.  If only one number is present, the image is\n\t\t resized to that number along the longest dimension,\n\t\tpreserving aspect.\n");
		printf("\trotate:[-45.0 - 45.0] default: 0 (no-rotate)\n");
		printf("\tsharpen:[0 - 10, default: 0 (no-sharpen)\n");
		printf("\tcrop:x,y,w,y  no defaults\n");
		printf("\tsaturation:[0 - 5.0] default=1.0, no change\n");
		printf("\tdenoise:[0 - 100.0],[1-10],[1-10], default=0.0,1,3\n");
		printf("\ttint:[r,g,b] default: 0,0,0 (doesn't have a corresponding\n\t\t tool in rawproc)\n");
		printf("\twhitebalance:[auto]|[patch]|[camera]|[rmult,gmult,bmult] \n\t\tdefault: auto, based on 'gray world'\n");
		printf("\tgray:[r,g,b] default: 0.21,0.72,0.07\n"); 
		printf("\tcurve:[rgb,|red,|green,|blue,]x1,y1,x2,y2,...xn,yn  Default channel: rgb\n");
		printf("\texposure:ev default: 1.0\n");
		printf("\thighlight:1-10\n");
		printf("\tshadow:1-10\n");
		printf("\trotate90 - rotate 90 degrees clockwise\n");
		printf("\trotate180 - rotate 180 degrees\n");
		printf("\trotate270 - rotate 270 degrees clockwise\n");
		printf("\thmirror - flip horizontal\n");
		printf("\tvmirror - flip upside down\n");

		exit(1);
	}

	if (argc < 3) {
		printf("Error: No output file specified.\n");
		exit(1);
	}

	if (countchar(std::string(argv[optind]),'*') > 1) {
		printf("Error: Too many wildcards in input filespec.\n");
		exit(1);
	}
	if (countchar(std::string(argv[argc-1]),'*') > 1) {
		printf("Error: Too many wildcards in output filespec.\n");
		exit(1);
	}



	//separates the parameters from the input and output file strings
	std::vector<std::string> infile;
	std::string infilename = std::string(argv[optind]);

#ifdef WIN32
	if (infilename.find_first_of(':') == 1) { 	
		if (std::count(infilename.begin(), infilename.end(), ':') > 1)
			infile = bifurcate(infilename, ':', true);
		else 
			infile.push_back(infilename); 
	}
	else { 
		infile = bifurcate(infilename, ':');
	}
#else
	infile = bifurcate(infilename, ':');
#endif

	if (infile.size() < 2) infile.push_back("");
	optind++;


	std::vector<std::string> outfile;
	std::string outfilename = std::string(std::string(argv[argc-1]));

#ifdef WIN32
	if (outfilename.find_first_of(':') == 1) { 	
		if (std::count(outfilename.begin(), outfilename.end(), ':') > 1)
			outfile = bifurcate(outfilename, ':', true);
		else 
			outfile.push_back(outfilename); 
	}
	else { 
		outfile = bifurcate(outfilename, ':');
	}
#else
	outfile = bifurcate(outfilename, ':');
#endif

	if (outfile.size() < 2) outfile.push_back("");



	std::string filetype = getFileType(infile[0]);

	//construct input parameters from input.filetype.parameters, input.raw.libraw.* if raw, 
	//and the command line parameters in that order.  If raw and rawdata=1, paramlist is
	//truncted to rawdata and cameraprofile
	std::map<std::string,std::string> inputparams;
	if (myConfig::getConfig().exists("input."+filetype+".parameters"))
		parseparams(inputparams, myConfig::getConfig().getValue("input."+filetype+".parameters"));
	if (filetype == "raw")
		parseparams(inputparams, param_string("input.raw.libraw."));
	if (infile[1] != "")
		parseparams(inputparams, infile[1]);
	if (filetype == "raw") {
		if (inputparams.find("rawdata") != inputparams.end()) {
			if (inputparams["rawdata"] == "1") {
				infile[1] = "rawdata=1";
				if (inputparams.find("cameraprofile") != inputparams.end())
					infile[1].append(";cameraprofile="+inputparams["cameraprofile"]);
			}
			else infile[1] = paramstring(inputparams);
		}
	}
	else 
		infile[1] = paramstring(inputparams);


	if (countchar(infile[0],'*') == 1) {
		if (countchar(outfile[0],'*') == 1) {
			std::vector<std::string> flist = fileglob(infile[0]);
			for (int i=0; i<flist.size(); i++) {
				std::string variant = matchspec(flist[i], infile[0]);
				if (variant == "") continue;
				fnames f;
				f.infile = flist[i];
				f.outfile = makename(variant,outfile[0]);
				f.variant = variant;
				files.push_back(f);
			}
				
		}
		else {
			printf("Error: If input file has a wildcard spec, the output file should have one also.\n");
			exit(1);
		}

	}
	else {
		fnames f;
		f.infile = infile[0];
		f.outfile = outfile[0];
		files.push_back(f);
	}
	

//list of commands to apply to each input file
std::vector<std::string> commands;
for (int i = optind; i<argc-1; i++) {
	commands.push_back(std::string(argv[i]));
}

int count = 0;

for (int f=0; f<files.size(); f++)
{

	char iname[256];
	strncpy(iname, files[f].infile.c_str(), 255);
	
	if (!force && file_exists(files[f].outfile.c_str())) {
		printf("Output file %s exists, skipping %s...\n",files[f].outfile.c_str(), iname);
		continue;
	}
	
	if (!file_exists(iname)) {
		printf("Input file %s doesn't exist...\n", iname);
		continue;
	}

	count++;

	commandstring = "rawproc-img ";

	printf("%d: Loading file %s %s... ",count, iname, infile[1].c_str());
	_mark();
	gImage dib = gImage::loadImageFile(iname, infile[1]);
	if (dib.getWidth() == 0 | dib.getHeight() == 0) {
		printf("error: (%fsec) - Image not loaded correctly\n",_duration());
		continue;
	}
	printf("done. (%fsec)\nImage size: %dx%d\n",_duration(), dib.getWidth(),dib.getHeight());

	commandstring += std::string(iname);
	if (infile[1] != "") commandstring += ":" + infile[1];
	commandstring += " ";


	
	//process commands:
	for (int i=0; i<commands.size(); i++) {
		if (commands[i] == "input.raw.default") {
			std::vector<std::string> cmdlist = split(myConfig::getConfig().getValue("input.raw.default"), " ");
			for (unsigned i=0; i<cmdlist.size(); i++) do_cmd(dib, cmdlist[i], files[f].variant);
		}
		else do_cmd(dib, commands[i], files[f].variant);
	}

	int orientation = atoi(dib.getInfoValue("Orientation").c_str());
	//printf("Orientation: %d\n", orientation);
	if (orientation != 1) {
		printf("Normalizing image orientation from %d...",orientation);
		_mark();
		dib.NormalizeRotation();
		printf("done. (%fsec)\n",_duration());
	}

	char outfilename[256];
	strncpy(outfilename, files[f].outfile.c_str(), 255);


	if (strcmp(outfilename, "info") == 0) {
		std::map<std::string,std::string> imginfo = dib.getInfo();
		for (std::map<std::string,std::string>::iterator it=imginfo.begin(); it!=imginfo.end(); ++it) {
			if (it->first == "ExposureTime") {
				if (atof(it->second.c_str()) < 1.0) {
					printf("%s: 1/%d\n",it->first.c_str(), int(round(1.0/atof(it->second.c_str()))));
				}
			}
			else
				printf("%s: %s\n",it->first.c_str(), it->second.c_str());
		}
		printf("\n");
		exit(0);
	}

	else if (strcmp(outfilename,"histogram") == 0) {
		std::vector<long> h = dib.Histogram();
		printf("%ld",h[0]);
		for (int i = 1; i < h.size(); i++)
			printf(",%ld",h[i]);
		printf("\n");
		exit(0);
	}

	else if (strcmp(outfilename,"cdf") == 0) {
		long prev = 0;
		std::vector<long> h = dib.Histogram();
		printf("%ld",h[0]);
		for (int i = 1; i < h.size(); i++) {
			prev += h[i];
			printf(",%ld",prev);
		}
		printf("\n");
		exit(0);
	}


	saveFile (dib, std::string(outfilename), outfile[1], std::string(commandstring));
	printf("\n");

}


	return 0;
}
