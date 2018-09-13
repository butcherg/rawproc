
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

//used to contain a list of corresponding input and output file names,
//constructed from the input and output file specifications
struct fnames {
	std::string infile, outfile;
};


std::string commandstring;

void do_cmd(gImage &dib, std::string commandstr)
{
		char c[256];
		strncpy(c, commandstr.c_str(), 255);
		char* cmd = strtok(c,":");
		
		if (strcmp(cmd,"colorspace") == 0) { 
			char *profstr = strtok(NULL, ",");
			char *opstr = strtok(NULL, ",");
			char *istr = strtok(NULL, ",");
			char *bpstr = strtok(NULL, " ");
			
			std::string profile = "";
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
			std::string bpcomp = "";
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
		
		//#bright:[-100 - 100] default: 0 (no-bright)
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

		//#blackwhitepoint[:rgb|red|green|blue][;][0-127,128-255] default: auto blackwhitepoint determination. The calculated points will be used in the metafile entry.
		else if (strcmp(cmd,"blackwhitepoint") == 0) {   
			char *c, *b, *w;
			GIMAGE_CHANNEL channel = CHANNEL_RGB;
			std::string chan = "";
			double blk=0.0, wht=255.0;
			double blkthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blackthreshold","0.05").c_str());
			double whtthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitethreshold","0.05").c_str());
			int blklimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blacklimit","128").c_str());
			int whtlimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitelimit","128").c_str()); 
			long whtinitial = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whiteinitialvalue","255").c_str());
		
			long hmax = 0;
			int maxpos;
			c = strtok(NULL,", ");
			if (c) { //first token is a channel
				chan = std::string(c);
				if (chan == "rgb" | chan == "red" |chan == "green" | chan == "blue") {
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

		//#contrast:[-100 - 100] default: 0 (no-contrast)
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

		//#gamma:[0.0-5.0] default: 1.0 (linear, or no-gamma)
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


		//#resize:w[,h] If either w or h is 0, resize preserves aspect of that dimension.  If only one number is present, the image is resized to that number along the longest dimension, preserving aspect.  
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
				sprintf(cs, "%s:%d,%d,lanczos3 ",cmd, w, h);
				commandstring += std::string(cs);
			}
		}

		//#rotate:[-45.0 - 45.0] default: 0 (no-rotate)
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

		//#sharpen:[0 - 10, default: 0 (no-sharpen)
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

		//#saturation:[0 - 5.0] default=1.0, no change
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

		//#denoise:[0 - 100.0],[1-10],[1-10], default=0.0,1,3
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

		//#tint:[r,g,b] default: 0,0,0
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
		
		//#whitebalance:[rmult,gmult,bmult] default: automatic, based on "gray world"
		else if (strcmp(cmd,"whitebalance") == 0) {  
			std::string op = "";
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
					printf("whitebalance: %s (%d threads)... ",op,threadcount);
					_mark();
					dib.ApplyWhiteBalance(threadcount);
					printf("done (%fsec).\n",_duration());
					sprintf(cs, "%s:%s",cmd, op.c_str());
				}
				else if (parm[0] == "patch") {
					op = "patch";
					patchx   = atoi(parm[1].c_str());
					patchy   = atoi(parm[2].c_str());
					patchrad = atof(parm[3].c_str());
					printf("whitebalance: %s,%d,%d,%0.1f (%d threads)... ",op,patchx,patchy,patchrad,threadcount);
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
						printf("whitebalance: %s,%0.2f,%0.2f,%0.2f (%d threads)... ",op,redmult,greenmult,bluemult,threadcount);
						_mark();
						dib.ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
						printf("done (%fsec).\n",_duration());
						sprintf(cs, "%s:%s",cmd, op.c_str());
					}
				}
				else { //parms are just three multipliers
					redmult   = atof(parm[0].c_str());
					greenmult = atof(parm[1].c_str());
					bluemult  = atof(parm[2].c_str());
					printf("whitebalance: %0.1f,%0.1f,%0.1f (%d threads)... ",redmult, greenmult, bluemult,threadcount);
					_mark();
					dib.ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
					printf("done (%fsec).\n",_duration());
					sprintf(cs, "%s:%0.1f,%0.1f,%0.1f ",cmd, op.c_str(), redmult, greenmult, bluemult);
				}
				commandstring += std::string(cs);
			}
		}

/*
		//#whitebalance:[rmult,gmult,bmult] default: automatic, based on "gray world"
		else if (strcmp(cmd,"whitebalance") == 0) {  
			//no properties yet, no whitebalance in rawproc...
			double redmult=1.0; 
			double greenmult = 1.0; 
			double bluemult = 1.0;
			char *rm = strtok(NULL,", ");
			char *gm = strtok(NULL,", ");
			char *bm = strtok(NULL," ");
			if (rm && strcmp(rm, "auto") != 0) {
				redmult = atof(rm);
				if (gm) greenmult = atof(gm);
				if (bm) bluemult = atof(bm);
			}
			else { 
				std::vector<double> rgbmeans = dib.CalculateChannelMeans();
				redmult = rgbmeans[0] / rgbmeans[1];
				bluemult = rgbmeans[2] / rgbmeans[1];
			}

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.whitebalance.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);

			printf("whitebalance: %0.2f,%0.2f,%0.2f (%d threads)... ",redmult,greenmult,bluemult,threadcount);

			_mark();
			dib.ApplyWhiteBalance(redmult,greenmult,bluemult, threadcount);
			printf("done (%fsec).\n",_duration());
			char cs[256];
			if (rm)
				sprintf(cs, "%s:%0.1f,%0.1f,%0.1f ",cmd, redmult, greenmult, bluemult);
			else
				sprintf(cs, "auto");
			commandstring += std::string(cs);
		}
*/

		//#gray:[r,g,b] default: 0.21,0.72,0.07 
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
		
		//#exposure:ev default: 1.0
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
		
		//these don't have rawproc equivalents, so they're not added to the metadata-embedded command
		//#rotate90 - rotate 90 degrees clockwise
		else if (strcmp(cmd,"rotate90") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("rotate90 (%d threads)... ", threadcount);
			_mark();
			dib.ApplyRotate90(threadcount);
			printf("done (%fsec).\n",_duration());
		}

		//#rotate180 - rotate 180 degrees
		else if (strcmp(cmd,"rotate180") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("rotate180 (%d threads)... ", threadcount);
			_mark();
			dib.ApplyRotate180(threadcount);
			printf("done (%fsec).\n",_duration());
			
		}

		//#rotate270 - rotate 270 degrees clockwise
		else if (strcmp(cmd,"rotate270") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("rotate270 (%d threads)... ", threadcount);
			_mark();
			dib.ApplyRotate270(threadcount);
			printf("done (%fsec).\n",_duration());
		}

		//#hmirror - flip horizontal	
		else if (strcmp(cmd,"hmirror") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("mirror horizontal (%d threads)... ", threadcount);
			_mark();
			dib.ApplyHorizontalMirror(threadcount);
			printf("done (%fsec).\n",_duration());
		}

		//#vmirror - flip upside down		
		else if (strcmp(cmd,"vmirror") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("mirror vertical (%d threads)... ", threadcount);
			_mark();
			dib.ApplyVerticalMirror(threadcount);
			printf("done (%fsec).\n",_duration());
		}
		
		else if (strcmp(cmd,"demosaic") == 0) {
			int threadcount = gImage::ThreadCount();
			printf("demosaic (%d threads)... ", threadcount);
			_mark();
			dib.ApplyDemosaic(DEMOSAIC_HALF, threadcount);
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


		else printf("Unrecognized command: %s.  Continuing...\n",cmd);

}

int main (int argc, char **argv) 
{
	char * filename;
	
	//the corresponding input and output file names 
	std::vector<fnames> files;
	
	
	int c;
	int flags;
	//gImage dib;
	

	std::string conf_cwd = getCwdConfigFilePath();
	std::string conf_configd = getAppConfigFilePath();

	if (access( conf_cwd.c_str(), 0 ) == 0) {
		myConfig::loadConfig(conf_cwd);
		printf("configuration file: %s\n", conf_cwd.c_str());
	}
	else if (access( conf_configd.c_str(), 0 ) == 0) {
		myConfig::loadConfig(conf_configd);
		printf("configuration file: %s\n", conf_configd.c_str());
	}
	
	gImage::setProfilePath(filepath_normalize(myConfig::getConfig().getValue("cms.profilepath")));

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
		printf("\tbright:[-100 - 100, default=0]\n");
		printf("\tblackwhitepoint[:0-127,128-255 default=auto]\n");
		printf("\tcontrast:[-100 - 100, default=0]\n");
		printf("\tcrop:x,y,w,h\n");
		printf("\tdenoise:[0 - 100.0],[1-10],[1-10], default=0.0,1,3\n");
		printf("\tgamma:[0.0 - 5.0, default=1.0]\n");
		printf("\tgray:r,g,b - values are the fractional (0.x) proportions with which to calculate the neutral value.\n");
		printf("\tresize:[width],[height],[box|bilinear|bspline|bicubic|catmullrom|\n");
		printf("\t\tlanczos3 (default)]\n");
		printf("\trotate:[0 - 45, default=0]\n");
		printf("\tsharpen:[0 - 10, default=0]\n");
		printf("\tsaturation:[0 - 5.0, default=1.0, no change]\n");
		printf("\ttint:r,g,b - add/subtract value from each channel\n\n");
		printf("\twhitebalance:rmult,gmult,bmult\n");
		
		exit(1);
	}

	if (argc < 3) {
		printf("Error: No output file specified.\n");
		exit(1);
	}

	if (countchar(std::string(argv[1]),'*') > 1) {
		printf("Error: Too many wildcards in input filespec.\n");
		exit(1);
	}
	if (countchar(std::string(argv[argc-1]),'*') > 1) {
		printf("Error: Too many wildcards in output filespec.\n");
		exit(1);
	}

	//separates the parameters from the input and output file strings
	std::vector<std::string> infile = split(std::string(argv[1]),":");
	if (infile.size() < 2) infile.push_back("");
	std::vector<std::string> outfile = split(std::string(argv[argc-1]),":");
	if (outfile.size() < 2) outfile.push_back("");
	
	
	//sets input parameter string from a input.*.parameters property:
	if (infile[1].find(".") != std::string::npos)
		if (infile[1].find("input") != std::string::npos)
			if (infile[1].find("parameters") != std::string::npos)
				if (myConfig::getConfig().exists(infile[1]))
					infile[1] = myConfig::getConfig().getValue(infile[1]);
				else {
					printf("Error: property %s not found.\n", infile[1].c_str());
					exit(0);
				}

	//builds input parameter string from input.raw.libraw.* parameters
	else if (infile[1].find(".") != std::string::npos)
		if (infile[1].find("input") != std::string::npos)
			if (infile[1].find("raw") != std::string::npos)
				if (infile[1].find("libraw") != std::string::npos)
					infile[1] = param_string(infile[1]+".");


	if (countchar(infile[0],'*') == 1) {
		if (countchar(outfile[0],'*') == 1) {
			std::vector<std::string> flist = fileglob(infile[0]);
			for (int i=0; i<flist.size(); i++) {
				std::string variant = matchspec(flist[i], infile[0]);
				if (variant == "") continue;
				fnames f;
				f.infile = flist[i];
				f.outfile = makename(variant,outfile[0]);
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
for (int i = 2; i<argc-1; i++) {
	commands.push_back(std::string(argv[i]));
}

int count = 0;


for (int f=0; f<files.size(); f++)
{

	char iname[256];
	strncpy(iname, files[f].infile.c_str(), 255);
	
	if (file_exists(files[f].outfile.c_str())) {
		printf("Output file %s exists, skipping %s...\n",files[f].outfile.c_str(), iname);
		continue;
	}
	
	count++;

	commandstring = "rawproc-img ";

	printf("%d: Loading file %s %s... ",count, iname, infile[1].c_str());
	_mark();
	gImage dib = gImage::loadImageFile(iname, infile[1]);
	printf("done. (%fsec)\nImage size: %dx%d\n",_duration(), dib.getWidth(),dib.getHeight());

	commandstring += std::string(iname);
	if (infile[1] != "") commandstring += ":" + infile[1];
	commandstring += " ";

	int orientation = atoi(dib.getInfoValue("Orientation").c_str());
	//printf("Orientation: %d\n", orientation);
	if (orientation != 0) {
		printf("Normalizing image orientation from %d...",orientation);
		_mark();
		if (orientation == 2) dib.ApplyHorizontalMirror(); 
		if (orientation == 3) dib.ApplyRotate180(); 
		if (orientation == 4) dib.ApplyVerticalMirror(); 
		if (orientation == 5) {dib.ApplyRotate90(); dib.ApplyHorizontalMirror(); }
		if (orientation == 6) {dib.ApplyRotate90(); }
		if (orientation == 7) {dib.ApplyRotate270(); dib.ApplyHorizontalMirror(); }
		if (orientation == 8) dib.ApplyRotate270();
		dib.setInfo("Orientation","0");
		printf("done. (%fsec)\n",_duration());
	}
	
	//process commands:
	for (int i=0; i<commands.size(); i++) {
		if (commands[i] == "input.raw.default") {
			std::vector<std::string> cmdlist = split(myConfig::getConfig().getValue("input.raw.default"), " ");
			for (unsigned i=0; i<cmdlist.size(); i++) do_cmd(dib, cmdlist[i]);
		}
		else do_cmd(dib, commands[i]);
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
	
	GIMAGE_FILETYPE filetype = gImage::getFileNameType(outfilename);
	
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
	printf("Saving file %s %s... ",outfilename, outfile[1].c_str());
	dib.setInfo("Software","img 0.1");
	dib.setInfo("ImageDescription", commandstring);
	
	cmsHPROFILE profile = cmsOpenProfileFromFile(profilepath.c_str(), "r");
	if (dib.getProfile() && profile)
		dib.saveImageFile(outfilename, outfile[1].c_str(), profile, intent);
	else
		dib.saveImageFile(outfilename, outfile[1].c_str());

	if (dib.getLastError() == GIMAGE_OK) 
		printf("done. (%fsec)\n\n",_duration());
	else
		printf("Error: %s: %s\n\n",dib.getLastErrorMessage().c_str(), outfile[0].c_str());

}


	return 0;
}
