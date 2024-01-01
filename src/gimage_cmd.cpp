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

std::string do_cmd(gImage &dib, std::string commandstr, std::string outfile, bool verbose)
{
	std::string commandstring = std::string();
	
	std::vector<std::string> cs = bifurcate(commandstr,':');
	std::string command = cs[0];
	std::string parms;
	if (cs.size() >= 2) parms = cs[1]; 
	
	std::map<std::string,std::string> result;

	//parsing:
	std::map<std::string,std::string> params;

	if (command == "blackwhitepoint") params = parse_blackwhitepoint(parms);
	else if (command == "add") params = parse_add(parms);
	else if (command == "colorspace") params = parse_colorspace(parms);
	else if (command == "crop") params = parse_crop(parms);
	else if (command == "cropspectrum") params = parse_cropspectrum(parms);
	else if (command == "curve") params = parse_curve(parms);
	else if (command == "demosaic") params = parse_demosaic(parms);
	else if (command == "denoise") params = parse_denoise(parms);
	else if (command == "exposure") params = parse_exposure(parms);
#ifdef USE_GMIC
	else if (command == "gmic") params = parse_gmic(parms);
#endif
	else if (command == "gray") params = parse_gray(parms);
	else if (command == "hlrecover") params = parse_hlrecover(parms);
	else if (command == "lenscorrection") params = parse_lenscorrection(parms);
	else if (command == "redeye") params = parse_redeye(parms);
	else if (command == "resize") params = parse_resize(parms);
	else if (command == "rotate") params = parse_rotate(parms);
	else if (command == "saturation") params = parse_saturation(parms);
	else if (command == "sharpen") params = parse_sharpen(parms);
	else if (command == "spot") params = parse_spot(parms);
	else if (command == "subtract") params = parse_subtract(parms);
	else if (command == "tone") params = parse_tone(parms);
	else if (command == "whitebalance") params = parse_whitebalance(parms);
	else if (command == "group"){ 
		params["cmdlabel"] = "group"; 
		params["mode"] = "default";
		params["cmdstring"] = parms;
	}
	else return string_format("Error - Unrecognized command: %s",command.c_str());
	
	params["paramstring"] = parms; 
			
	//parse error-catching:
	if (params.find("error") != params.end()) {
		return params["error"];  
	}
			
	//if (print) printf("-%s-... ",params["cmdlabel"].c_str()); 
	if (verbose) printf("%s: ",command.c_str()); 
	fflush(stdout);

	//processing
	if (command == "blackwhitepoint") result =  process_blackwhitepoint(dib, params);
	else if (command == "add") result =  process_add(dib, params);
	else if (command == "colorspace") result =  process_colorspace(dib, params);
	else if (command == "crop") result =  process_crop(dib, params);
	else if (command == "cropspectrum") result =  process_cropspectrum(dib, params);
	else if (command == "curve") result =  process_curve(dib, params);
	else if (command == "demosaic") result =  process_demosaic(dib, params);
	else if (command == "denoise") result =  process_denoise(dib, params);
	else if (command == "exposure") result =  process_exposure(dib, params);
#ifdef USE_GMIC
	else if (command == "gmic") result =  process_exposure(dib, params);
#endif
	else if (command == "gray") result =  process_gray(dib, params);
	else if (command == "hlrecover") result =  process_hlrecover(dib, params);
	else if (command == "lenscorrection") result =  process_lenscorrection(dib, params);
	else if (command == "redeye") result =  process_redeye(dib, params);
	else if (command == "resize") result =  process_resize(dib, params);
	else if (command == "rotate") result =  process_rotate(dib, params);
	else if (command == "saturation") result =  process_saturation(dib, params);
	else if (command == "sharpen") result =  process_sharpen(dib, params);
	else if (command == "spot") result =  process_spot(dib, params);
	else if (command == "subtract") result =  process_subtract(dib, params);
	else if (command == "tone") result =  process_tone(dib, params);
	else if (command == "whitebalance") result =  process_whitebalance(dib, params);
	else if (command == "group") result = process_group(dib, params, verbose);
	
			
	//process error catching:
	if (result.find("error") != result.end()) {
		return result["error"];  
	}

	if (verbose) {
		if (command != "group") {
			//printf(" (%s threads, %ssec)\n",result["threadcount"].c_str(),result["duration"].c_str()); 
			printf("%s [%d]\n",result["imgmsg"].c_str(), verbose);
			fflush(stdout);
		}
	}

	//commandstring += buildcommand(cmd, params);
	if (paramexists(result, "commandstring"))
		commandstring += result["commandstring"] + " ";
	else
		commandstring += command + ":" + parms + " ";		

	return commandstring;
}


/*
//old ops: ********************************************************************
		
		//img <li>group:command;command;...</li>
		if (strcmp(cmd,"group") == 0) {
			char cs[4096];
			char *c = strtok(NULL, " ");
			std::vector<std::string> cmdlist = split(std::string(c), ";");
			for (int i=0; i<cmdlist.size(); i++) {
				do_cmd(dib, cmdlist[i], std::string(), true);
			}
			sprintf(cs, "group:%s ",c);
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
		
