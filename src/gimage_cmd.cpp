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
#include "fileutil.h"
#include "myConfig.h"
#include "CameraData.h"
#include "gimage_parse.h"
#include "gimage_process.h"


std::string buildcommand(std::string cmd, std::map<std::string,std::string> params)
{
	std::string c = cmd;
	if (!params.empty()) {
		c.append(":");
		for (std::map<std::string,std::string>::iterator it=params.begin(); it!=params.end(); ++it) {
			c.append(it->first);
			c.append("=");
			c.append(it->second);
			c.append(";");
		}
	}
	c.append(" ");
	return c;
}

std::string do_cmd(gImage &dib, std::string commandstr, std::string outfile, bool print)
{
		std::string commandstring = std::string();
		char c[256];
		strncpy(c, commandstr.c_str(), 255);
		char* cmd = strtok(c,":");
		
		if (strcmp(cmd,"blackwhitepoint") == 0) {   
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_blackwhitepoint(std::string(pstr));
			else
				params = parse_blackwhitepoint("rgb,auto");
			
			//error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];    //ToDo: return an error message...
			}

			//processing
			std::map<std::string,std::string> result =  process_blackwhitepoint(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf("blackwhitepoint:%s,%s,%s (%s threads, %ssec)\n",
				params["mode"].c_str(),
				result["black"].c_str(),
				result["white"].c_str(),
				result["threadcount"].c_str(),
				result["duration"].c_str()
			); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			commandstring += std::string(cmd) + ":" + pstr + " ";
		}

		//img <li>colorspace:profilefile[,convert|assign][,renderingintent][,bpc]</li>
		if (strcmp(cmd,"colorspace") == 0) {   
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_colorspace(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}

			//processing
			std::map<std::string,std::string> result =  process_colorspace(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf("colorspace:%s,%s (%s threads, %ssec)\n",
				params["mode"].c_str(),
				params["op"].c_str(),
				result["threadcount"].c_str(),
				result["duration"].c_str()
			); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			commandstring += std::string(cmd) + ":" + pstr + " ";
		}

		//img <li>crop:x,y,w,y  no defaults</li>
		if (strcmp(cmd,"crop") == 0) {   
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_crop(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}

			//processing
			std::map<std::string,std::string> result =  process_crop(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf("crop:%s (%s threads, %ssec)\n",
				params["mode"].c_str(),
				result["threadcount"].c_str(),
				result["duration"].c_str()
			); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			commandstring += std::string(cmd) + ":" + pstr + " ";
		}
		
		//img <li>curve:[rgb,|red,|green,|blue,]x1,y1,x2,y2,...xn,yn  Default channel: rgb</li>
		else if (strcmp(cmd,"curve") == 0) {
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_curve(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}

			//processing
			std::map<std::string,std::string> result =  process_curve(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf("curve:%s,%s (%s threads, %ssec)\n",
				params["mode"].c_str(),
				params["op"].c_str(),
				result["threadcount"].c_str(),
				result["duration"].c_str()
			); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			commandstring += std::string(cmd) + ":" + pstr + " ";
			
		}

		//img <li>demosaic:[half|half_resize|color|vng|amaze|dcb|rcd|igv|lmmse|ahd][,p1[,p2..]][,cacorrect][,hlrecovery] default: ahd<br>Alternatively, name=val pairs: algorithm=half|half_resize|color|vng|amaze|dcb|rcd|igv|lmmse|ahd,cacorrect,hlrecovery,passes=#,iterations=#,dcb_enhance,usecielab,initgain</li>
		else if (strcmp(cmd,"demosaic") == 0) {  
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_demosaic(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}

			//processing
			std::map<std::string,std::string> result =  process_demosaic(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			dib.setInfo("Libraw.Mosaiced","0");

			if (print) printf("demosaic:%s (%s threads, %ssec)\n",
				result["mode"].c_str(), //using result instead of params, gimage_process may have changed params (proof = half|xtran_fast)
				result["threadcount"].c_str(),
				result["duration"].c_str()
			); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			if (paramexists(result, "commandstring"))
				commandstring += result["commandstring"] + " ";
			else
				commandstring += std::string(cmd) + ":" + pstr + " ";

		}

		//img <li>denoise:[0 - 100.0],[1-10],[1-10], default=0.0,1,3</li>
		else if (strcmp(cmd,"denoise") == 0) {  
			
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_denoise(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}
			
			if (print) printf("denoise:%s... ",params["mode"].c_str()); 
			fflush(stdout);

			//processing
			std::map<std::string,std::string> result =  process_denoise(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf(" (%s threads, %ssec)\n",result["threadcount"].c_str(),result["duration"].c_str()); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			if (paramexists(result, "commandstring"))
				commandstring += result["commandstring"] + " ";
			else
				commandstring += std::string(cmd) + ":" + pstr + " ";
		
		}

		//img <li>exposure:ev default: 1.0</li>
		else if (strcmp(cmd,"exposure") == 0) {
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_exposure(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}
			
			if (print) printf("exposure:%s... ",params["mode"].c_str()); 
			fflush(stdout);

			//processing
			std::map<std::string,std::string> result =  process_exposure(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf(" (%s threads, %ssec)\n",result["threadcount"].c_str(),result["duration"].c_str()); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			if (paramexists(result, "commandstring"))
				commandstring += result["commandstring"] + " ";
			else
				commandstring += std::string(cmd) + ":" + pstr + " ";
		}

		//img <li>gray:[r,g,b] default: 0.21,0.72,0.07</li> 
		else if (strcmp(cmd,"gray") == 0) {  
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_gray(std::string(pstr));
			else
				params = parse_gray(std::string());
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}
			
			if (print) printf("gray:%s,%s,%s... ",params["red"].c_str(),params["green"].c_str(),params["blue"].c_str()); 
			fflush(stdout);

			//processing
			std::map<std::string,std::string> result =  process_gray(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf(" (%s threads, %ssec)\n",result["threadcount"].c_str(),result["duration"].c_str()); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			if (paramexists(result, "commandstring"))
				commandstring += result["commandstring"] + " ";
			else
				commandstring += std::string(cmd) + ":" + pstr + " ";		
		
		}

		else if (strcmp(cmd,"redeye") == 0) {  
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_redeye(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}
			
			if (print) printf("redeye... "); 
			fflush(stdout);

			//processing
			std::map<std::string,std::string> result =  process_redeye(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf(" (%s threads, %ssec)\n",result["threadcount"].c_str(),result["duration"].c_str()); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			if (paramexists(result, "commandstring"))
				commandstring += result["commandstring"] + " ";
			else
				commandstring += std::string(cmd) + ":" + pstr + " ";		
		
		}
		
		else if (strcmp(cmd,"resize") == 0) {  
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_resize(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}
			
			if (print) printf("resize... "); 
			fflush(stdout);

			//processing
			std::map<std::string,std::string> result =  process_resize(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf(" (%s threads, %ssec)\n",result["threadcount"].c_str(),result["duration"].c_str()); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			if (paramexists(result, "commandstring"))
				commandstring += result["commandstring"] + " ";
			else
				commandstring += std::string(cmd) + ":" + pstr + " ";		
		
		}

		else if (strcmp(cmd,"rotate") == 0) {  
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_rotate(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}
			
			if (print) printf("rotate... "); 
			fflush(stdout);

			//processing
			std::map<std::string,std::string> result =  process_rotate(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf(" (%s threads, %ssec)\n",result["threadcount"].c_str(),result["duration"].c_str()); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			if (paramexists(result, "commandstring"))
				commandstring += result["commandstring"] + " ";
			else
				commandstring += std::string(cmd) + ":" + pstr + " ";		
		
		}

		else if (strcmp(cmd,"saturation") == 0) {  
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_saturation(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}
			
			if (print) printf("saturation... "); 
			fflush(stdout);

			//processing
			std::map<std::string,std::string> result =  process_saturation(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf(" (%s threads, %ssec)\n",result["threadcount"].c_str(),result["duration"].c_str()); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			if (paramexists(result, "commandstring"))
				commandstring += result["commandstring"] + " ";
			else
				commandstring += std::string(cmd) + ":" + pstr + " ";		
		
		}

		else if (strcmp(cmd,"sharpen") == 0) {  
			//parsing:
			std::map<std::string,std::string> params;
			char * pstr = strtok(NULL, " ");
			if (pstr)
				params = parse_sharpen(std::string(pstr));
			
			//parse error-catching:
			if (params.find("error") != params.end()) {
				return params["error"];  
			}
			
			if (print) printf("sharpen... "); 
			fflush(stdout);

			//processing
			std::map<std::string,std::string> result =  process_sharpen(dib, params);
			
			//process error catching:
			if (result.find("error") != result.end()) {
				return result["error"];  
			}

			if (print) printf(" (%s threads, %ssec)\n",result["threadcount"].c_str(),result["duration"].c_str()); 
			fflush(stdout);

			//commandstring += buildcommand(cmd, params);
			if (paramexists(result, "commandstring"))
				commandstring += result["commandstring"] + " ";
			else
				commandstring += std::string(cmd) + ":" + pstr + " ";		
		
		}



//old ops: ********************************************************************

		//img <li>tone:operator,[param...]</li>
		else if (strcmp(cmd,"tone") == 0) {
			//tone:filmic,6.20,0.50,1.70,0.06,1.00,norm 
			char cs[4096];
			cs[0] = '\0';
			char *c = strtok(NULL, " ");
			std::vector<std::string> p = split(std::string(c), ",");

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.tone.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			if (print) printf("tone: %s (%d threads)... ",p[0].c_str(),threadcount); fflush(stdout);

			_mark();
			if (p[0] == "gamma") {
				double gamma = 1.0;
				if (p.size() >= 2) gamma = atof(p[1].c_str());
				dib.ApplyToneMapGamma(gamma, threadcount);
				sprintf(cs, "tone:gamma,%0.2f ",gamma);
			}
			else if (p[0] == "reinhard") {
				bool channel = true;
				std::string cmdstr = "reinhard";
				if (p.size() >= 2) {
					cmdstr.append(":"+p[1]);
					if (p[1] == "luminance") channel = false;
				}
				bool norm = false;
				if (p.size() >= 3) {
					cmdstr.append(","+p[2]);
					if (p[2] == "norm") norm = true;
				}
				cmdstr.append(" ");
				dib.ApplyToneMapReinhard(channel, norm, threadcount);
				strncpy(cs,cmdstr.c_str(),4096);
			}
			else if (p[0] == "log2") {
				dib.ApplyToneMapLog2(threadcount);
				sprintf(cs, "tone:log2 ");
			}
			else if (p[0] == "loggamma") {
				dib.ApplyToneMapLogGamma(threadcount);
				sprintf(cs, "tone:loggamma ");
			}
			else if (p[0] == "filmic") {
				double filmicA = atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.A","6.2").c_str());
				double filmicB = atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.B","0.5").c_str());
				double filmicC = atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.C","1.7").c_str());
				double filmicD = atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.D","0.06").c_str());
				double power =   atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.power","1.0").c_str());;
				bool norm = myConfig::getConfig().getValueOrDefault("tool.tone.filmic.norm","1") == "1" ? true : false;
				if (p.size() >= 2) filmicA = atof(p[1].c_str());
				if (p.size() >= 3) filmicB = atof(p[2].c_str());
				if (p.size() >= 4) filmicC = atof(p[3].c_str());
				if (p.size() >= 5) filmicD = atof(p[4].c_str());
				if (p.size() >= 6) power = atof(p[5].c_str());
				if (p.size() >= 7 && p[6] == "norm") norm = true;
				dib.ApplyToneMapFilmic(filmicA, filmicB, filmicC, filmicD, power, norm, threadcount);
				if (norm)
					sprintf(cs, "tone:filmic,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,norm ",filmicA,filmicB,filmicC,filmicD,power);
				else
					sprintf(cs, "tone:filmic,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f ",filmicA,filmicB,filmicC,filmicD,power);
			}
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);

			//char cs[256];
			//sprintf(cs, "tone:%s ",c);
			commandstring += std::string(cs);
		}
		
		//img <li>group:command;command;...</li>
		else if (strcmp(cmd,"group") == 0) {
			char cs[4096];
			char *c = strtok(NULL, " ");
			std::vector<std::string> cmdlist = split(std::string(c), ";");
			for (int i=0; i<cmdlist.size(); i++) {
				do_cmd(dib, cmdlist[i], std::string(), true);
			}
			sprintf(cs, "group:%s ",c);
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



#ifdef USE_LIBRTPROCESS
		//img <li>cacorrect - Correct chromatic abberation. Use only before demosaic.</li>
		else if (strcmp(cmd,"cacorrect") == 0) {  
			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.clcorrect.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			if (print) printf("clcorrect: (%d threads)... ",threadcount); fflush(stdout);

			_mark();
			dib.ApplyCACorrect(threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			char cs[256];
			sprintf(cs, "%s ",cmd);
			commandstring += std::string(cs);
		}

		//img <li>hlrecover - Use unclipped channels to reconstruct highlights.  Use only after demosaic</li>
		else if (strcmp(cmd,"hlrecover") == 0) {  
			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.hlrecover.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			if (print) printf("hlrecover: (%d threads)... ",threadcount); fflush(stdout);

			_mark();
			dib.ApplyHLRecover(threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			char cs[256];
			sprintf(cs, "%s ",cmd);
			commandstring += std::string(cs);
		}
#endif

		//img <li>addexif:tagname,value - tagname must be valid EXIF tag for it to survive the file save...</li>
		else if (strcmp(cmd,"addexif") == 0) {  
			char *name = strtok(NULL, ",");
			char *value = strtok(NULL, " ");
			
			if (print) printf("addexif: %s=%s ... ",name,value); fflush(stdout);

			dib.setInfo(std::string(name),std::string(value));
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
				if (dib.getInfoValue("Libraw.Mosaiced") == "0") {
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
						std::string cameraWB = dib.getInfoValue("Libraw.WhiteBalance");
						if (cameraWB != "") {


							std::vector<std::string> m = split(cameraWB,",");
							redmult   = atof(m[0].c_str());
							greenmult = atof(m[1].c_str());
							bluemult  = atof(m[2].c_str());
							if (print) printf("whitebalance: %s,%0.2f,%0.2f,%0.2f (%d threads)... ",op.c_str(),redmult,greenmult,bluemult,threadcount); fflush(stdout);
							_mark();
							if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
								if (dib.getInfoValue("Libraw.CFAPattern") != "")
									dib.ApplyCameraWhiteBalance(redmult, greenmult, bluemult, threadcount);
							}
							else {
								dib.ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
							}
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
						std::string cameraWB = dib.getInfoValue("Libraw.WhiteBalance");
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
					else { // parameters are just three multipliers
						redmult   = atof(parm[0].c_str());
						greenmult = atof(parm[1].c_str());
						bluemult  = atof(parm[2].c_str());
						if (print) printf("whitebalance: %0.1f,%0.1f,%0.1f (%d threads)... ",redmult, greenmult, bluemult,threadcount); fflush(stdout);
						_mark();
						dib.ApplyCameraWhiteBalance(redmult, greenmult, bluemult, threadcount);
						if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
						sprintf(cs, "%s:%0.1f,%0.1f,%0.1f ",cmd, redmult, greenmult, bluemult);
					}
/*
					else {
						if (print) printf("whitebalance: Error: only camera white balance or multipliers can be applied to pre-demosaiced images.\n");
						return std::string();
					}
*/
				}
				commandstring += std::string(cs);
			}
		}




		
		//img <li>normalize:newmin,newmax default: 0.0,1.0</li>
		else if (strcmp(cmd,"normalize") == 0) {
			double min = atof(myConfig::getConfig().getValueOrDefault("tool.normalize.min","0.0").c_str());
			double max = atof(myConfig::getConfig().getValueOrDefault("tool.normalize.max","1.0").c_str());
			char *mn = strtok(NULL,", ");
			char *mx = strtok(NULL," ");
			if (mn) min = atof(mn);
			if (mx) max = atof(mx);

			int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.exposure.cores","0").c_str());
			if (threadcount == 0) 
				threadcount = gImage::ThreadCount();
			else if (threadcount < 0) 
				threadcount = std::max(gImage::ThreadCount() + threadcount,0);
			if (print) printf("normalization: %f,%f (%d threads)... ",min,max,threadcount); fflush(stdout);

			_mark();
			dib.ApplyNormalization(min, max, threadcount); 
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			char cs[256];
			sprintf(cs, "%s:%f,%f ",cmd, min, max);
			commandstring += std::string(cs);
		}


		//img <li>subtract:val|camera|file,filename  val=a float value, "camera" retrieves the camera </li>
		else if (strcmp(cmd,"subtract") == 0) {
			double subtract;
			char cs[1024];
			char filename[256];
			char *v = strtok(NULL,", ");
			char *f = strtok(NULL," ");

			if (strcmp(v,"camera") == 0) {
				subtract = atof(dib.getInfoValue("Libraw.Black").c_str()) / 65536.0;
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
			if (strcmp(v,"rgb") == 0) {
				dib.ApplySubtract(subtract, subtract, subtract, subtract, true, threadcount);
			}
			else if (strcmp(v,"red") == 0) {
				dib.ApplySubtract(subtract, 0.0, 0.0, 0.0, true, threadcount);
			}
			else if (strcmp(v,"green") == 0) {
				dib.ApplySubtract(0.0, subtract, 0.0, 0.0, true, threadcount);
			}
			else if (strcmp(v,"blue") == 0) {
				dib.ApplySubtract(0.0, 0.0, subtract, true, threadcount);
			}
			else if (strcmp(v,"camera") == 0) {
				std::map<std::string,std::string> info = dib.getInfo();
				if (info.find("Libraw.CFABlack") != info.end()) {
					float sub[6][6];
					std::string blackstr = info["Libraw.CFABlack"];
					std::vector<std::string> blackvec = split(blackstr,",");
					unsigned blackdim = sqrt(blackvec.size());
					for (unsigned r=0; r< blackdim; r++)
						for (unsigned c=0; c< blackdim; c++)
							sub[r][c] = atof(blackvec[c + r*blackdim].c_str()) / 65536.0;
					dib.ApplyCFASubtract(sub, true, threadcount);
				}
				else if (info.find("Libraw.PerChannelBlack") != info.end()) {
					float subr=0.0, subg1=0.0, subg2=0.0, subb=0.0;
					std::vector<std::string> s = split(info["Libraw.PerChannelBlack"],",");
					if (s.size() >= 4) {
						subr = atof(s[0].c_str());
						subg1 = atof(s[0].c_str());
						subb = atof(s[0].c_str());
						subg2 = atof(s[0].c_str());
						dib.ApplySubtract(subr, subg1, subg2, subb, true, threadcount);
					}
				}
				else if (info.find("Libraw.Black") != info.end()) {
					int subval = atoi(info["Libraw.Black"].c_str());
					float subtract = (float) subval / 65536.0;
					dib.ApplySubtract(subtract, subtract, subtract, subtract, true, threadcount);
				}
			}
			else if (strcmp(v,"file") == 0) {
				dib.ApplySubtract(std::string(filename), threadcount);  
			}
			else
				dib.ApplySubtract(subtract, CHANNEL_RGB, true, threadcount);  
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

		else if (strcmp(cmd,"matmultiply") == 0) { 
			char cs[4096];
			char *c = strtok(NULL, " ");
			std::vector<std::string> mat = split(std::string(c), ",");
			if (mat.size() < 9) {
				if (print) printf("Error: Insufficient number of values in matrix (<9).\n");
				return std::string();
			}

			double matrix[3][3];

			for (unsigned r=0; r<3; r++) {
				for (unsigned c=0; c<3; c++) {
					unsigned i = c + r*3;
					float m = atof(mat[i].c_str());
					if (m > 10.0)
						matrix[r][c] = m /10000.0;
					else
						matrix[r][c] = m;
				}
			}

for (unsigned r=0; r<3; r++) {
	for (unsigned c=0; c<3; c++) {
		printf("%f ",matrix[r][c]);
	}
	printf("\n");
}

			int threadcount = gImage::ThreadCount();

			if (print) printf("matmultiply: %s (%d threads)... ", c, threadcount); fflush(stdout);
			_mark();
			dib.ApplyMatrixMultiply(matrix, threadcount);
			if (print) printf("done (%fsec).\n",_duration()); fflush(stdout);
			sprintf(cs, "%s:%s ",cmd,c);
			commandstring += std::string(cs);
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

