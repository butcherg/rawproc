#include "gimage/gimage.h"
#include <string>

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <math.h> 

#include "elapsedtime.h"
#include "gimage/strutil.h"
#include "myConfig.h"
#include "CameraData.h"


std::string do_cmd(gImage &dib, std::string commandstr, std::string outfile, bool print)
{
		std::string commandstring = std::string();
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
				return std::string();
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

			char cs[256];

			if (profile == "camera") {
				std::string dcrawfile = "";
				if (myConfig::getConfig().exists("tool.colorspace.dcrawpath")) {
					dcrawfile = myConfig::getConfig().getValue("tool.colorspace.dcrawpath");
					if (access(dcrawfile.c_str(), 0 ) != 0) dcrawfile == "";
				}

				if (dcrawfile == "") {
					dcrawfile = getExeDir("dcraw.c");
					if (access(dcrawfile.c_str(), 0 ) != 0) dcrawfile == "";
				}

				if (dcrawfile == "") {
					dcrawfile = getAppConfigDir("dcraw.c");
					if (access(dcrawfile.c_str(), 0 ) != 0) dcrawfile == "";
				}

				if (dcrawfile == "") {
					if (print) printf("Error: dcraw.c not found in either the tool.colorspace.dcrawpath, the executable's directory, or the app config directory.\n");
					return std::string();
				}

				std::string makemodel;
				if (dib.getInfoValue("Make") != "" & dib.getInfoValue("Model") != "") {
					makemodel = dib.getInfoValue("Make");
					makemodel.append(" ");
					makemodel.append(dib.getInfoValue("Model"));
				}
				else {
					if (print) printf("Error: camera make and/or model not found.\n");
					return std::string();
				}

				std::string dcrawpath;
				std::string camconstpath;
				std::string cam;

				CameraData c;
				dcrawpath = c.findFile("dcraw.c","tool.colorspace.dcrawpath");
				camconstpath = c.findFile("camconst.json","tool.colorspace.camconstpath");
				if (file_exists(dcrawpath)) c.parseDcraw(dcrawpath);
				if (file_exists(camconstpath)) c.parseCamconst(camconstpath);
				cam = c.getItem(makemodel, "dcraw_matrix");


				if (cam != "") {
					if (print) printf("colorspace: %s, %s (%s) (%d threads)... ",profile.c_str(),opstr,makemodel.c_str(),threadcount); fflush(stdout);
					_mark();
					if (operation == "convert") {
						if (dib.ApplyColorspace(cam, intent, bp, threadcount) != GIMAGE_OK) printf("Error: %s\n", dib.getLastErrorMessage().c_str());
					}
					else if (operation == "assign") {
						if (dib.AssignColorspace(cam)!= GIMAGE_OK) printf("Error: %s\n", dib.getLastErrorMessage().c_str());
					} 
					else if (print) printf("Error: unrecognized operator %s ", operation.c_str());
					sprintf(cs, "%s:%s,%s ",cmd,profile.c_str(),opstr);
				}
			}
			else {
				if (print) printf("colorspace: %s, %s, %s, %s (%d threads)... ",profile.c_str(),opstr,istr,bpstr,threadcount); fflush(stdout);
				_mark();
				if (operation == "convert") {
					if (dib.ApplyColorspace(profilepath+profile, intent, bp, threadcount) != GIMAGE_OK) printf("Error: %s (%s)\n", dib.getLastErrorMessage().c_str(), profilepath.append(profile).c_str());
					sprintf(cs, "%s:%s,%s,%s,%s ",cmd,profile.c_str(),opstr,istr,bpstr);
				}
				else if (operation == "assign") {
					dib.AssignColorspace(profilepath+profile);
					sprintf(cs, "%s:%s,%s ",cmd,profile.c_str(),opstr);
				}
				else if (print) printf("Error: unrecognized operator -%s- ", operation.c_str());

			}

			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("bright: %0.2f (%d threads)... ",bright,threadcount); fflush(stdout);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("demosaic: %s (%d threads)... ",demosaic.c_str(),threadcount); fflush(stdout);

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
			else {
				if (print) printf("Error: unrecognized algorithm");
				return std::string();
			}
			dib.setInfo("LibrawMosaiced","0");
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			char cs[256];
			sprintf(cs, "%s:%s ",cmd, demosaic.c_str());
			commandstring += std::string(cs);
		}

		//img <li>addexif:tagname,value - tagname must be valid EXIF tag for it to survive the file save...</li>
		else if (strcmp(cmd,"addexif") == 0) {  
			char *name = strtok(NULL, ",");
			char *value = strtok(NULL, " ");
			
			if (print) printf("addexif: %s=%s ... ",name,value); fflush(stdout);

			dib.setInfo(std::string(name),std::string(value));
		}

		//img <li>blackwhitepoint[:rgb|red|green|blue][,0-127,128-255] default: auto blackwhitepoint determination. The calculated points will be used in the metafile entry.</li>
		else if (strcmp(cmd,"blackwhitepoint") == 0) {   
			char *c, *b, *w;
			GIMAGE_CHANNEL channel = CHANNEL_RGB;
			std::string chan;
			double blk=0.0, wht=255.0;
			double blkthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blackthreshold","0.001").c_str());
			double whtthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitethreshold","0.001").c_str());
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
				chan = "rgb";
				std::vector<double> bwpts = dib.CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial);
				blk = bwpts[0];
				wht = bwpts[1];
			}
			
			if (chan == "rgb")   channel = CHANNEL_RGB;
			if (chan == "red")   channel = CHANNEL_RED;
			if (chan == "green") channel = CHANNEL_GREEN;
			if (chan == "blue")  channel = CHANNEL_BLUE;

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			if (print) printf("blackwhitepoint: %s,%0.2f,%0.2f (%d threads)... ",chan.c_str(),blk,wht,threadcount); fflush(stdout);

			_mark();
			dib.ApplyToneLine(blk, wht, channel, threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			char cs[256];
			sprintf(cs, "%s:%s,%0.0f,%0.0f ",cmd, chan.c_str(), blk, wht);
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
			if (print) printf("contrast: %0.2f (%d threads)... ",contrast,threadcount); fflush(stdout);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("gamma: %0.2f (%d threads)... ",gamma,threadcount); fflush(stdout);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
				if (print) printf("Error: resize needs at least one parameter.\n");
				return std::string();
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
				if (print) printf("resize: %dx%d,%s (%d threads)... ",w,h,algo.c_str(), threadcount); fflush(stdout);

				_mark();
				dib.ApplyResize(w,h, filter, threadcount);
				if (print) printf("done (%fsec).\n", _duration()); fflush(stdout);
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
			if (print) printf("rotate: %0.2f (%d threads)... ",angle,threadcount); fflush(stdout);

			_mark();
			dib.ApplyRotate(angle, false, threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("sharp: %0.2f (%d threads)... ",sharp, threadcount); fflush(stdout);

			_mark();
			dib.ApplySharpen(sharp, threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("crop: %d,%d %dx%d (%d threads)... ",x,y,width,height,threadcount); fflush(stdout);

			_mark();
			dib.ApplyCrop(x,y,width,height,threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("saturate: %0.2f (%d threads)... ",saturation,threadcount); fflush(stdout);

			_mark();
			dib.ApplySaturate(saturation, threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("denoise: %0.2f (%d threads)... ",sigma,threadcount); fflush(stdout);

			_mark();
			dib.ApplyNLMeans(sigma, local, patch, threadcount);  
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("tint: %0.2f,%0.2f,%0.2f (%d threads)... ",red,green,blue,threadcount); fflush(stdout);

			_mark();
			dib.ApplyTint(red,green,blue, threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
				if (dib.getInfoValue("LibrawMosaiced") == "0") {
					if (parm[0] == "auto") {
						op = "auto";
						if (print) printf("whitebalance: %s (%d threads)... ",op.c_str(),threadcount); fflush(stdout);
						_mark();
						dib.ApplyWhiteBalance(threadcount);
						if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
						sprintf(cs, "%s:%s ",cmd, op.c_str());
					}
					else if (parm[0] == "patch") {
						op = "patch";
						patchx   = atoi(parm[1].c_str());
						patchy   = atoi(parm[2].c_str());
						patchrad = atof(parm[3].c_str());
						if (print) printf("whitebalance: %s,%d,%d,%0.1f (%d threads)... ",op.c_str(),patchx,patchy,patchrad,threadcount); fflush(stdout);
						_mark();
						dib.ApplyWhiteBalance((unsigned) patchx, (unsigned) patchy, patchrad, threadcount);
						if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
							if (print) printf("whitebalance: %s,%0.2f,%0.2f,%0.2f (%d threads)... ",op.c_str(),redmult,greenmult,bluemult,threadcount); fflush(stdout);
							_mark();
							dib.ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
							if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
							sprintf(cs, "%s:%s ",cmd, op.c_str());
						}
					}
					else { // parameters are just three multipliers
						redmult   = atof(parm[0].c_str());
						greenmult = atof(parm[1].c_str());
						bluemult  = atof(parm[2].c_str());
						if (print) printf("whitebalance: %0.1f,%0.1f,%0.1f (%d threads)... ",redmult, greenmult, bluemult,threadcount); fflush(stdout);
						_mark();
						dib.ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
						if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
						sprintf(cs, "%s:%0.1f,%0.1f,%0.1f ",cmd, redmult, greenmult, bluemult);
					}
				}
				else {
					if (parm[0] == "camera") {
						op = "camera";
						std::string cameraWB = dib.getInfoValue("LibrawWhiteBalance");
						if (cameraWB != "") {
							std::vector<std::string> m = split(cameraWB,",");
							redmult   = atof(m[0].c_str());
							greenmult = atof(m[1].c_str());
							bluemult  = atof(m[2].c_str());
							if (print) printf("whitebalance: %s,%0.2f,%0.2f,%0.2f (%d threads)... ",op.c_str(),redmult,greenmult,bluemult,threadcount); fflush(stdout);
							_mark();
							dib.ApplyCameraWhiteBalance(redmult, greenmult, bluemult, threadcount);
							if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
							sprintf(cs, "%s:%s ",cmd, op.c_str());
						}
					}
					else {
						if (print) printf("whitebalance: Error: only camera white balance can be applied to pre-demosaiced images.\n");
						return std::string();
					}
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
			if (print) printf("gray: %0.2f,%0.2f,%0.2f (%d threads)... ",red,green,blue,threadcount); fflush(stdout);

			_mark();
			dib.ApplyGray(red,green,blue, threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
					if (print) printf("redeye (%d threads)... ", threadcount); fflush(stdout);
					_mark();
					dib.ApplyRedeye(pts, threshold, limit, false, 1.0,  threadcount);
					if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
				}
				else if (print) printf("redeye: bad y coord\n");
			}
			else if (print) printf("redeye: bad x coord\n");
		
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
			if (print) printf("curve: %s (%d threads)... ",p,threadcount); fflush(stdout);
			_mark();
			dib.ApplyToneCurve(ctrlpts, channel, threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("exposure: %0.2f (%d threads)... ",ev,threadcount); fflush(stdout);

			_mark();
			dib.ApplyExposureCompensation(ev, threadcount);  //local and patch hard-coded, for now...
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			char cs[256];
			sprintf(cs, "%s:%0.1f ",cmd, ev);
			commandstring += std::string(cs);
		}

		//img <li>subtract:val|camera|file,filename  val=a float value, "camera" retrieves the camera </li>
		else if (strcmp(cmd,"subtract") == 0) {
			double subtract;
			char cs[256];
			char filename[256];
			char *v = strtok(NULL,", ");
			char *f = strtok(NULL," ");

			if (strcmp(v,"camera") == 0) {
				subtract = atof(dib.getInfoValue("LibrawBlack").c_str()) / 65536.0;
				sprintf(cs, "%s:camera ",cmd);
			}
			else if (strcmp(v,"file") == 0) {
				strcpy(filename, f);
				sprintf(cs, "%s:file,%s ",cmd, filename);
			}
			else {
				subtract = atof(v);
				sprintf(cs, "%s:%0.1f ",cmd, subtract);
			}

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.subtract.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			if (print) printf("subtract: %f (%d threads)... ",subtract,threadcount); fflush(stdout);

			_mark();
			if (strcmp(v,"file") == 0)
				dib.ApplySubtract(std::string(filename), threadcount);  
			else
				dib.ApplySubtract(subtract, threadcount);  
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);


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
			if (print) printf("highlight: %0.2f,%0.2f (%d threads)... ",highlight,threshold,threadcount); fflush(stdout);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("shadow: %0.2f,%0.2f (%d threads)... ",shadow,threshold,threadcount); fflush(stdout);

			_mark();
			dib.ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			char cs[256];
			sprintf(cs, "%s:%0.0f,%0.0f ",cmd, shadow,threshold);
			commandstring += std::string(cs);
		}
		
		//img <li>rotate90 - rotate 90 degrees clockwise</li>
		else if (strcmp(cmd,"rotate90") == 0) {
			int threadcount = gImage::ThreadCount();
			if (print) printf("rotate90 (%d threads)... ", threadcount); fflush(stdout);
			_mark();
			dib.ApplyRotate90(threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			char cs[256];
			sprintf(cs, "rotate:90 ");
			commandstring += std::string(cs);
		}

		//img <li>rotate180 - rotate 180 degrees</li>
		else if (strcmp(cmd,"rotate180") == 0) {
			int threadcount = gImage::ThreadCount();
			if (print) printf("rotate180 (%d threads)... ", threadcount); fflush(stdout);
			_mark();
			dib.ApplyRotate180(threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			char cs[256];
			sprintf(cs, "rotate:180 ");
			commandstring += std::string(cs);
		}

		//img <li>rotate270 - rotate 270 degrees clockwise</li>
		else if (strcmp(cmd,"rotate270") == 0) {
			int threadcount = gImage::ThreadCount();
			if (print) printf("rotate270 (%d threads)... ", threadcount); fflush(stdout);
			_mark();
			dib.ApplyRotate270(threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			char cs[256];
			sprintf(cs, "rotate:270 ");
			commandstring += std::string(cs);
		}

		//these don't have rawproc equivalents, so they're not added to the metadata-embedded command
		//img <li>hmirror - flip horizontal</li>
		else if (strcmp(cmd,"hmirror") == 0) {
			int threadcount = gImage::ThreadCount();
			if (print) printf("mirror horizontal (%d threads)... ", threadcount); fflush(stdout);
			_mark();
			dib.ApplyHorizontalMirror(threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
		}

		//img <li>vmirror - flip upside down</li>	
		else if (strcmp(cmd,"vmirror") == 0) {
			int threadcount = gImage::ThreadCount();
			if (print) printf("mirror vertical (%d threads)... ", threadcount); fflush(stdout);
			_mark();
			dib.ApplyVerticalMirror(threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("tonescale: %0.2f,%0.2f,%0.2f (%d threads)... ",red,green,blue,threadcount); fflush(stdout);
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
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
			if (print) printf("blur, 2Dkernel: %s (%d threads)... ",k, threadcount); fflush(stdout);
			_mark();
			dib.Apply2DConvolutionKernel(kdata, 5, threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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

			if (print) printf("blur: sigma=%0.1f, kernelsize=%d (%d threads)... ", sigma, kernelsize, threadcount); fflush(stdout);
			_mark();
			dib.ApplyGaussianBlur(sigma, kernelsize, threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
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
/*
		else if (strcmp(cmd,"save") == 0) {
			char *of = strtok(NULL, ", ");
			char *params = strtok(NULL, ", ");

			std::string outfilename = std::string(of);
			if (countchar(outfilename,'*') == 1) outfilename = makename(outfile, outfilename);

			if (!force && file_exists(outfilename.c_str())) {
				if (print) printf("save: file %s exists, skipping...\n",outfilename.c_str()); fflush(stdout);
			}
			else {

				if (params)
					saveFile (dib, outfilename, std::string(params), std::string(commandstring));
				else
					saveFile (dib, outfilename, "", std::string(commandstring));
			} 
		}
*/

		return commandstring;
		//else printf("Unrecognized command: %s.  Continuing...\n",cmd); fflush(stdout);

}

