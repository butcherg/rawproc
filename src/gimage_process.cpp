
#include "gimage_process.h"
#include "gimage_parse.h" //for paramexists()
#include "gimage/strutil.h"
#include "gimage/curve.h"
#include "fileutil.h"
#include "myConfig.h"
#include "elapsedtime.h"
#include "CameraData.h"
#include "cJSON.h"
#include <math.h> 

int getThreadCount(int threadcount) { 
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	return threadcount;
}

std::map<std::string,std::string> process_blackwhitepoint(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;
	char buffer[4096];
	double blk=0.0, wht=255.0;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "blackwhitepoint:ProcessError - no mode";
	}
	//nominal processing:
	else {
		//parm tool.*.cores: Sets the number of processing cores used by the tool.  0=use all available, -N=use available minus n.  Default=0);
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);

		//tool-specific setup:

		GIMAGE_CHANNEL channel = CHANNEL_RGB;
		if (paramexists(params, "channel")) {
			if (params["channel"] == "rgb")   channel = CHANNEL_RGB;
			if (params["channel"] == "red")   channel = CHANNEL_RED;
			if (params["channel"] == "green") channel = CHANNEL_GREEN;
			if (params["channel"] == "blue")  channel = CHANNEL_BLUE;
		}
		else channel = CHANNEL_RGB;

		//tool-specific logic:
		if (params["mode"] == "auto"){
			double blkthresh = 
				atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blackthreshold","0.001").c_str());
			double whtthresh = 
				atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitethreshold","0.001").c_str());
			//int blklimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blacklimit","128").c_str());
			//int whtlimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitelimit","128").c_str()); 
			long whtinitial = 
				atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whiteinitialvalue","255").c_str());

			std::vector<double> bwpts = 
				dib.CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial, params["channel"]);	
			blk = bwpts[0];
			wht = bwpts[1];
			result["treelabel"] = string_format("blackwhitepoint:%s,%s",params["channel"].c_str(),params["mode"].c_str());
		}
		else if (params["mode"] == "values"){
			blk = atof(params["black"].c_str());
			wht = atof(params["white"].c_str());
			//result["treelabel"] = string_format("blackwhitepoint:%s",params["mode"].c_str());
			//result["treelabel"] = "blackwhitepoint";
			result["treelabel"] = string_format("blackwhitepoint:%s,%s",params["channel"].c_str(),params["mode"].c_str());
		}
		else if (params["mode"] == "data"){
			std::map<std::string,float> s = dib.StatsMap();
			//if (channel == CHANNEL_RGB) {
				blk = std::min(std::min(s["rmin"],s["gmin"]),s["bmin"]);
				wht = std::max(std::max(s["rmax"],s["gmax"]),s["bmax"]);
				if (paramexists(params, "minwhite")) {
					if (params["minwhite"] == "true") wht = std::min(std::min(s["rmax"],s["gmax"]),s["bmax"]);
				}
			//}
			//else if (channel == CHANNEL_RED) {
			//	blk = s["rmin"];
			//	wht = s["rmax"];
			//}
			//else if (channel == CHANNEL_GREEN) {
			//	blk = s["gmin"];
			//	wht = s["gmax"];
			//}
			//else if (channel == CHANNEL_BLUE) {
			//	blk = s["bmin"];
			//	wht = s["bmax"];
			//}
			result["treelabel"] = string_format("blackwhitepoint:%s",params["mode"].c_str());
		}
		else if (params["mode"] == "norm"){
			blk = atof(params["black"].c_str());
			wht = atof(params["white"].c_str());
			result["treelabel"] = string_format("blackwhitepoint:%s",params["mode"].c_str());
		}
		else if (params["mode"] == "camera"){
			if (paramexists(dib.getInfo(), "Libraw.Black"))
				blk = atoi(dib.getInfoValue("Libraw.Black").c_str()) / 65536.0;
			else 
				blk = 0.0;   //not raw, do no harm...
			if (paramexists(dib.getInfo(), "Libraw.Maximum"))
				wht = atoi(dib.getInfoValue("Libraw.Maximum").c_str()) / 65536.0;
			else 
				wht = 255.0; //not raw, do no harm...
			result["treelabel"] = string_format("blackwhitepoint:%s",params["mode"].c_str());
		}

		_mark();
		if (params["mode"] == "norm")
			dib.ApplyNormalization(blk, wht, threadcount);
		else
			dib.ApplyToneLine(blk, wht, channel, threadcount);
		result["duration"] = std::to_string(_duration());
		result["black"] = tostr(blk);
		result["white"] = tostr(wht);
		//result["treelabel"] = string_format("blackwhitepoint:%s",params["mode"].c_str());
		//if (params["minwhite"] == "true") result["treelabel"] += ",minwhite";
	}

	return result;

}

std::map<std::string,std::string> process_colorspace(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "colorspace:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.colorspace.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);

		//tool-specific setup:
		GIMAGE_ERROR ret;
		std::string icc;

		std::string profilepath = filepath_normalize(myConfig::getConfig().getValueOrDefault("cms.profilepath",""));
		
	
		cmsUInt32Number intent = INTENT_PERCEPTUAL;
		if (params["rendering_intent"] == "perceptual") intent = INTENT_PERCEPTUAL;
		if (params["rendering_intent"] == "saturation") intent = INTENT_SATURATION;
		if (params["rendering_intent"] == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
		if (params["rendering_intent"] == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

		bool bpc = false;
		if (params["bpc"] == "true") bpc = true;


		//tool-specific logic:
		if (params["mode"] == "camera") {
			std::string makemodel = dib.getInfoValue("Make");
			makemodel.append(" ");
			makemodel.append(dib.getInfoValue("Model"));

			std::string dcrawpath;
			std::string camconstpath;
			std::string dcraw_primaries, primary_source;

			if (dcraw_primaries.empty()) { //Look in dcraw.c and camconst.json
				CameraData c;
				dcrawpath = c.findFile("dcraw.c","tool.colorspace.dcrawpath");
				camconstpath = c.findFile("camconst.json","tool.colorspace.camconstpath");
				if (file_exists(dcrawpath)) c.parseDcraw(dcrawpath);
				if (file_exists(camconstpath)) c.parseCamconst(camconstpath);
				params["icc"] = c.getItem(makemodel, "dcraw_matrix");
				result["dcraw_source"] = c.getItem(makemodel, "primary_source");
				result["dcraw_primaries"] = params["icc"];
			}
			
			if (dcraw_primaries.empty()) { //Last resort, look in the LibRaw metadata
				std::string libraw_primaries = dib.getInfoValue("Libraw.CamXYZ");
				std::vector<std::string> primaries = split(libraw_primaries, ",");
				if (primaries.size() >= 9 & atof(primaries[0].c_str()) != 0.0) {
					params["icc"] = string_format("%d,%d,%d,%d,%d,%d,%d,%d,%d",
						int(atof(primaries[0].c_str()) * 10000),
						int(atof(primaries[1].c_str()) * 10000),
						int(atof(primaries[2].c_str()) * 10000),
						int(atof(primaries[3].c_str()) * 10000),
						int(atof(primaries[4].c_str()) * 10000),
						int(atof(primaries[5].c_str()) * 10000),
						int(atof(primaries[6].c_str()) * 10000),
						int(atof(primaries[7].c_str()) * 10000),
						int(atof(primaries[8].c_str()) * 10000));
					result["dcraw_source"] = "LibRaw";
					result["dcraw_primaries"] = params["icc"];
				}
				
			}
			if (!params["icc"].empty()) {
				result["treelabel"] = "colorspace:camera";
				icc = params["icc"];
			}
			else {
				result["error"] = "colorspace:ProcessError - camera primaries not found.";
				return result;
			}
		}
		else if (params["mode"] == "primaries") {
			result["treelabel"] = "colorspace:primaries";
			icc = params["icc"];
		}
		else if (params["mode"] == "built-in") {
			result["treelabel"] = "colorspace:" + params["icc"];
			icc = params["icc"];
		}
		else if (params["mode"] == "file") {
			if (params["icc"] != "(none)") {
				if (!file_exists(profilepath+params["icc"])) {
					result["error"] = string_format("colorspace:ProcessError - file not found: %s",params["icc"].c_str());
					return result;
				}
				else {
					icc = profilepath+params["icc"];
				}
			}
			result["treelabel"] = "colorspace:file";
		}
		else {
			 result["error"] = string_format("colorspace:ProcessError - unrecognized mode: %s",params["mode"].c_str());
			 return result;
		}

		_mark();
		if (params["op"] == "convert") {
			ret = dib.ApplyColorspace(icc, intent, bpc, threadcount);
		}
		else if (params["op"] == "assign") {
			ret = dib.AssignColorspace(icc);
		}

		switch (ret) {
			case GIMAGE_OK:
				result["duration"] = std::to_string(_duration());
				result["treelabel"] += std::string(",") + params["op"];
				break;
			case GIMAGE_APPLYCOLORSPACE_BADPROFILE:
				result["error"] = "colorspace:ProcessError - no input profile in image.";
				break;
			case GIMAGE_APPLYCOLORSPACE_BADINTENT_INPUT:
				result["error"] = "colorspace:ProcessError - input profile doesn't support rendering intent.";
				break;
			case GIMAGE_APPLYCOLORSPACE_BADINTENT_OUTPUT:
				result["error"] = "colorspace:ProcessError - output profile doesn't support rendering intent.";
				break;
			case GIMAGE_APPLYCOLORSPACE_BADTRANSFORM:
				result["error"] = "colorspace:ProcessError - colorspace transform creation failed.";
				break;
		}

	}
	return result;
}

std::map<std::string,std::string> process_crop(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "crop:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);
		
		//tool-specific setup:
		float x = atof(params["x"].c_str());
		float y = atof(params["y"].c_str());
		float w = atof(params["w"].c_str());
		float h = atof(params["h"].c_str());
		
		//tool-specific logic:
		if (params["mode"] == "default") {
			_mark();
			dib.ApplyRatioCrop(x, y, w, h, threadcount);
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = "crop";
			
		}
		
	}
	return result;
}

std::map<std::string,std::string> process_curve(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "curve:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);
		
		GIMAGE_CHANNEL channel = CHANNEL_RGB;
		if (paramexists(params, "channel")) {
			if (params["channel"] == "rgb")   channel = CHANNEL_RGB;
			if (params["channel"] == "red")   channel = CHANNEL_RED;
			if (params["channel"] == "green") channel = CHANNEL_GREEN;
			if (params["channel"] == "blue")  channel = CHANNEL_BLUE;
		}
		else params["channel"] = "rgb";
		
		Curve crv;
		std::vector<std::string> cpts = split(params["curvecoords"], ",");
		for (unsigned i=0; i<cpts.size()-1; i+=2) {
			crv.insertpoint(atof(cpts[i].c_str()), atof(cpts[i+1].c_str()));
		}
		std::vector<cp> ctrlpts = crv.getControlPoints();
		
		if (params["mode"] == "default") {
			_mark();
			dib.ApplyToneCurve(ctrlpts, channel, threadcount);
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = string_format("%s:%s",params["mode"].c_str(),params["channel"].c_str());
		}
		
	}
	return result;
}

std::map<std::string,std::string> process_demosaic(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;
	bool ret = false;

#ifdef USE_LIBRTPROCESS
	LIBRTPROCESS_PREPOST prepost = LIBRTPROCESS_DEMOSAIC;  //from back when I thought pre- and post-demosaic should also be in the respective demosaic chains...
#endif

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "demosaic:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.demosaic.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);

		unsigned xtarray[6][6];
		if (params["mode"] == "proof") {
			if (dib.xtranArray(xtarray))
				params["mode"] = "xtran_fast";
			else
				params["mode"] = "half";
		}

		if (params["mode"] == "color") {
			_mark();
			ret = dib.ApplyMosaicColor(threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s",params["mode"].c_str());
		}
		else if (params["mode"] == "half") {
			_mark();
			ret = dib.ApplyDemosaicHalf(false, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s",params["mode"].c_str());
		}
		else if (params["mode"] == "half_resize") {
			_mark();
			ret = dib.ApplyDemosaicHalf(true, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s",params["mode"].c_str());
		}
#ifdef USE_LIBRTPROCESS
		else if (params["mode"] == "vng") {
			_mark();
			ret = dib.ApplyDemosaicVNG(prepost, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s",params["mode"].c_str());
		}
		else if (params["mode"] == "rcd") {
			_mark();
			ret = dib.ApplyDemosaicRCD(prepost, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s",params["mode"].c_str());
		}
		else if (params["mode"] == "dcb") {
			int iterations = atoi(params["iterations"].c_str());
			bool dcb_enhance = false;
			if (paramexists(params,"dcb_enhance") && params["dcb_enhance"] == "true") dcb_enhance = true;
			_mark();
			ret = dib.ApplyDemosaicDCB(prepost, iterations, dcb_enhance, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s,%d",params["mode"].c_str(),iterations);
			if (dcb_enhance) result["commandstring"] += ",dcb_enhance";
		}
		else if (params["mode"] == "amaze") {
			double initGain = 1.0;
			if (paramexists(params,"initgain")) initGain = atof(params["initgain"].c_str());
			int border = 0;
			if (paramexists(params,"border")) initGain = atoi(params["border"].c_str());
			float inputScale = 1.0;
			float outputScale = 1.0;
			_mark();
			ret = dib.ApplyDemosaicAMAZE(prepost, initGain, border, inputScale, outputScale, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s,%f,%d",params["mode"].c_str(),initGain,border);
		}
		else if (params["mode"] == "igv") {
			_mark();
			ret = dib.ApplyDemosaicIGV(prepost, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s",params["mode"].c_str());
		}
		else if (params["mode"] == "ahd") {
			_mark();
			ret = dib.ApplyDemosaicAHD(prepost, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s",params["mode"].c_str());
		}
		else if (params["mode"] == "lmmse") {
			int iterations = atoi(params["iterations"].c_str());
			_mark();
			ret = dib.ApplyDemosaicLMMSE(prepost, iterations, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s,%d",params["mode"].c_str(),iterations);
		}
		else if (params["mode"] == "xtran_fast") {
			_mark();
			ret = dib.ApplyDemosaicXTRANSFAST(prepost, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s",params["mode"].c_str());
		}
		else if (params["mode"] == "xtran_markesteijn") { 
			int passes = atoi(params["passes"].c_str());
			bool useCieLab = false;
			if (paramexists(params,"usecielab") && params["usecielab"] == "true") useCieLab = true;
			_mark();
			ret = dib.ApplyDemosaicXTRANSMARKESTEIJN(prepost, passes, useCieLab, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("demosaic:%s,%d",params["mode"].c_str(),passes);
			if (useCieLab) result["commandstring"] += ",usecielab";
		}
#endif
		else {
			result["error"] = string_format("demosaic:ProcessError - Unrecognized demosaic option: %s.",params["mode"].c_str());
			return result;
		}

		if (ret == false) {
			result["error"] = "demosaic:ProcessError - Demosaic failed, wrong image type (bayer vs xtran).";
			return result;
		}
		
		result["treelabel"] = string_format("demosaic:%s",params["mode"].c_str());
		result["mode"] = params["mode"];

	}
	return result;
}

std::map<std::string,std::string> process_denoise(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "curve:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);
		
		if (params["mode"] == "nlmeans") {
			_mark();
			int sigma = atoi(params["sigma"].c_str());
			int local = atoi(params["local"].c_str());
			int patch = atoi(params["patch"].c_str());
			if (sigma != 0) dib.ApplyNLMeans(sigma,local, patch, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("denoise:nlmeans,%d,%d,%d",sigma,local,patch);
		}
		else if (params["mode"] == "wavelet") {
			float threshold = atof(params["threshold"].c_str());
			_mark();
			if (threshold != 0.0) dib.ApplyWaveletDenoise(threshold, threadcount);
			result["duration"] = std::to_string(_duration());
			result["commandstring"] = string_format("denoise:wavelet,%f",threshold);
		}
		else {
			result["error"] = string_format("denoise:ProcessError - Unrecognized denoise option: %s.",params["mode"].c_str());
			return result;
		}
		
		result["treelabel"] = string_format("denoise:%s",params["mode"].c_str());
		
		
	}
	return result;
}

std::map<std::string,std::string> process_exposure(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "curve:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);
		
		if (params["mode"] == "patch") {
			int x = atoi(params["x"].c_str());
			int y = atoi(params["y"].c_str());
			float radius = atof(params["radius"].c_str());
			float ev0 = atof(params["ev0"].c_str());
			_mark();
			float stops = dib.ApplyExposureCompensation(x, y, radius, ev0, threadcount);
			result["duration"] = std::to_string(_duration());
			result["stops"] = tostr(stops);
			result["commandstring"] = string_format("exposure:patch,%d,%d,%0.1f,%0.2f",x,y,radius,ev0);
			result["treelabel"] = "exposure:patch";
		}
		else if (params["mode"] == "ev") {
			float ev = atof(params["ev"].c_str());
			_mark();
			dib.ApplyExposureCompensation(ev, threadcount);
			result["duration"] = std::to_string(_duration());
			result["stops"] = tostr(ev);
			result["commandstring"] = string_format("exposure:%0.2f",ev);
			result["treelabel"] = "exposure:ev";
		}
		else {
			result["error"] = string_format("exposure:ProcessError - Unrecognized exposure option: %s.",params["mode"].c_str());
			return result;
		}
	}
	return result;
}

std::map<std::string,std::string> process_gray(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "curve:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);
		
		double red   = atof(params["red"].c_str());
		double green = atof(params["green"].c_str());
		double blue  = atof(params["blue"].c_str());
		
		_mark();
		dib.ApplyGray(red,green,blue, threadcount);
		result["duration"] = std::to_string(_duration());
		result["commandstring"] = string_format("gray:%0.2f,%0.2f,%0.2f",red,green,blue);
		//result["treelabel"] = "gray";
	}
	return result;
}

std::map<std::string,std::string> process_lenscorrection(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "curve:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);
		
	}
	return result;
}

std::map<std::string,std::string> process_redeye(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "curve:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);
		
		std::vector<coord> points;
		std::vector<std::string> p = split(params["coords"], ",");
		for (unsigned i=0; i<p.size()-1; i++) {
			points.push_back({atoi(p[i].c_str()),atoi(p[i+1].c_str())});
		}
		double threshold = atof(params["threshold"].c_str());
		unsigned limit = atoi(params["limit"].c_str());
		bool desat = false;
		double desatpct=0.0;
		if (params["desat"] == "true") {
			desat = true;
			desatpct = atof(params["desatpct"].c_str());
		}
				
		_mark();
		dib.ApplyRedeye(points, threshold, limit, desat, desatpct, threadcount);
		result["duration"] = std::to_string(_duration());
		result["commandstring"] = string_format("redeye:%s",params["paramstring"].c_str());
		result["treelabel"] = "redeye";
		
	}
	return result;
}

std::map<std::string,std::string> process_resize(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "curve:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);
		
		int w=0, h=0, dw=dib.getWidth(), dh=dib.getHeight();
		if (params["mode"] == "asspecified") {
			w = atoi(params["width"].c_str());
			h = atoi(params["height"].c_str());
		}
		else if (params["mode"] == "largestside") {
			if (dh > dw) {
				h = atoi(params["width"].c_str());
			}
			else {
				w = atoi(params["width"].c_str());
			}
		}
		else {
			result["error"] = string_format("resize:ProcessError - Unrecognized mode: %s.",params["mode"].c_str());
			return result;
		}

		if (h ==  0) h = dh * ((float)w/(float)dw);
		if (w == 0)  w = dw * ((float)h/(float)dh); 
		
		RESIZE_FILTER filter = FILTER_CATMULLROM;  //must be same as default if tool.resize.algorithm is not specified
		if (params["algorithm"] == "box") filter = FILTER_BOX;
		if (params["algorithm"] == "bilinear") filter = FILTER_BILINEAR;
		if (params["algorithm"] == "bspline") filter = FILTER_BSPLINE;
		if (params["algorithm"] == "bicubic") filter = FILTER_BICUBIC;
		if (params["algorithm"] == "catmullrom") filter = FILTER_CATMULLROM;
		if (params["algorithm"] == "lanczos3") filter = FILTER_LANCZOS3;
		
		_mark();
		dib.ApplyResize(w,h, filter, threadcount);
		result["duration"] = std::to_string(_duration());
		result["commandstring"] = string_format("resize:%s",params["paramstring"].c_str());
		result["treelabel"] = "resize";
		
	}
	return result;
}

std::map<std::string,std::string> process_rotate(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "curve:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);

		float angle = atof(params["angle"].c_str());;

		bool autocrop = false, hmirror = false, vmirror = false;
		if (params.find("autocrop") != params.end()) autocrop = true;
		if (params.find("hmirror") != params.end()) hmirror = true;
		if (params.find("vmirror") != params.end()) vmirror = true;

		_mark();
		if (hmirror) dib.ApplyHorizontalMirror(threadcount);
		else if (vmirror) dib.ApplyVerticalMirror(threadcount);
		else if ((int) angle == 270) dib.ApplyRotate270(threadcount);
		else if ((int) angle == 180) dib.ApplyRotate180(threadcount);
		else if ((int) angle == 90) dib.ApplyRotate90(threadcount);
		else dib.ApplyRotate(angle, autocrop, threadcount);
		result["duration"] = std::to_string(_duration());
		result["commandstring"] = string_format("rotate:%s",params["paramstring"].c_str());
		result["treelabel"] = "rotate";
	}
	return result;
}

std::map<std::string,std::string> process_saturation(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "saturation:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.saturation.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);

		float saturation = atof(params["saturation"].c_str());;

		_mark();
		dib.ApplySaturate(saturation, threadcount);
		result["duration"] = std::to_string(_duration());
		result["commandstring"] = string_format("saturation:%s",params["paramstring"].c_str());
		result["treelabel"] = "saturation";
	}
	return result;
}

std::map<std::string,std::string> process_sharpen(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "sharpen:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.sharpen.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);
		
		result["duration"] = "0";
		if (params["mode"] == "convolution") {
			float strength = atof(params["strength"].c_str());
			if (strength != 0.0) {
				_mark();
				dib.ApplySharpen(strength, threadcount);
				result["duration"] = std::to_string(_duration());
				result["treelabel"] = "sharpen:convolution";
			}
		}
		else if (params["mode"] == "usm") {
			float sigma = atof(params["sigma"].c_str());
			float radius = atof(params["radius"].c_str());
			if (sigma != 0.0) {
				_mark();
				gImage blur = gImage(dib);
				blur.ApplyGaussianBlur(sigma, (int) (radius*2.0), threadcount);
				gImage mask = gImage(dib);
				mask.ApplySubtract(blur,threadcount);
				dib.ApplyAdd(mask,threadcount);
				result["duration"] = std::to_string(_duration());
				result["treelabel"] = "sharpen:usm";
			}
		}
		else {
			result["error"] = string_format("sharpen:ProcessError - Unrecognized mode: %s.",params["mode"].c_str());
		}
		result["commandstring"] = string_format("sharpen:%s",params["paramstring"].c_str());
	}
	return result;
}

std::map<std::string,std::string> process_subtract(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "subtract:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.subtract.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);

		if (params["mode"] == "value") {
			float subtract = atof(params["value"].c_str());
			_mark();
			if (params["channel"] == "rgb") {
				dib.ApplySubtract(subtract, subtract, subtract, subtract, true, threadcount);
			}
			else if (params["channel"] == "red") {
				dib.ApplySubtract(subtract, 0.0, 0.0, 0.0, true, threadcount);
			}
			else if (params["channel"] == "green") {
				dib.ApplySubtract(0.0, subtract, 0.0, 0.0, true, threadcount);
			}
			else if (params["channel"] == "blue") {
				dib.ApplySubtract(0.0, 0.0, subtract, true, threadcount);
			}
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = string_format("subtract:%s,value",params["channel"].c_str());
		}
		else if (params["mode"] == "camera") {
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
				result["duration"] = std::to_string(_duration());
				result["treelabel"] = "subtract:camera(cfa)";
			}
			else if (info.find("Libraw.PerChannelBlack") != info.end()) {
				float subr=0.0, subg1=0.0, subg2=0.0, subb=0.0;
				std::vector<std::string> s = split(info["Libraw.PerChannelBlack"],",");
				if (s.size() >= 4) {
					subr  = atof(s[0].c_str());
					subg1 = atof(s[1].c_str());
					subb  = atof(s[2].c_str());
					subg2 = atof(s[3].c_str());
					dib.ApplySubtract(subr, subg1, subg2, subb, true, threadcount);
					result["duration"] = std::to_string(_duration());
				
				}
				result["treelabel"] = "subtract:camera(chan)";
			}
			else if (info.find("Libraw.Black") != info.end()) {
				int subval = atoi(info["Libraw.Black"].c_str());
				float subtract = (float) subval / 65536.0;
				dib.ApplySubtract(subtract, subtract, subtract, subtract, true, threadcount);
				result["duration"] = std::to_string(_duration());
				result["treelabel"] = "subtract:camera";
			}
		}
		else if (params["mode"] == "file") {
			if (!file_exists(params["filename"])) {
				result["error"] = string_format("subtract:ProcessError - file %s doesn't exist.",params["filename"].c_str());
			}
			_mark();
			dib.ApplySubtract(params["filename"], threadcount);  
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = "subtract:file";
		}


	}
	return result;
}

std::map<std::string,std::string> process_tone(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "subtract:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.subtract.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);

		if (params["mode"] == "gamma") {
			float gamma = atof(params["value"].c_str());
			_mark();
			dib.ApplyToneMapGamma(gamma, threadcount);
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = "tone:gamma";
		}
		else if (params["mode"] == "reinhard") {
			bool channel = true;
			if (params["reinhardmode"] == "luminance") channel = false;
			bool norm = false;
			if (params["norm"] == "true") norm = true;
			_mark();
			dib.ApplyToneMapReinhard(channel, norm, threadcount);
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = "tone:reinhard";
		}
		else if (params["mode"] == "log2") {
			_mark();
			dib.ApplyToneMapLog2(threadcount);
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = "tone:log2";
		}
		else if (params["mode"] == "loggamma") {
			_mark();
			dib.ApplyToneMapLogGamma(threadcount);
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = "tone:loggamma";
		}
		else if (params["mode"] == "filmic") {
			float A = atof(params["A"].c_str());
			float B = atof(params["B"].c_str());
			float C = atof(params["C"].c_str());
			float D = atof(params["D"].c_str());
			float power = atof(params["power"].c_str());
			bool norm = (params["norm"] == "true") ? true : false;
			_mark();
			dib.ApplyToneMapFilmic(A, B, C, D, power, norm, threadcount);
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = "tone:filmic";
		}

	}
 	return result;
}

std::map<std::string,std::string> process_whitebalance(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "whitebalance:ProcessError - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.subtract.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);

		if (params["mode"] == "camera") {
			std::string cameraWB = dib.getInfoValue("Libraw.WhiteBalance");
			if (cameraWB != "") {
				std::vector<std::string> m = split(cameraWB,",");
				float redmult   = atof(m[0].c_str());
				float greenmult = atof(m[1].c_str());
				float bluemult  = atof(m[2].c_str());
				_mark();
				if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
					if (dib.getInfoValue("Libraw.CFAPattern") != "")
						dib.ApplyCameraWhiteBalance(redmult, greenmult, bluemult, threadcount);
				}
				else {
					dib.ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
				}
				result["duration"] = std::to_string(_duration());
				result["treelabel"] = "whitebalance:camera";
			}
			else {
				result["error"] = "whitebalance:ProcessError - no camera multipliers found";
				return result;
			}
			
		}
		else if (params["mode"] == "auto") {
			_mark();
			dib.ApplyWhiteBalance(threadcount);
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = "whitebalance:auto";
		}
		else if (params["mode"] == "patch") {
			unsigned patchx = atoi(params["patchx"].c_str());
			unsigned patchy = atoi(params["patchy"].c_str());
			float patchrad = atof(params["patchradius"].c_str());
			_mark();
			dib.ApplyWhiteBalance((unsigned) patchx, (unsigned) patchy, patchrad, threadcount);
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = "whitebalance:patch";
		}
		else if (params["mode"] == "multipliers") {
			float redmult = atof(params["redmultiplier"].c_str());
			float greenmult = atof(params["greenmultiplier"].c_str());
			float bluemult = atof(params["bluemultiplier"].c_str());
			_mark();
			dib.ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
			result["duration"] = std::to_string(_duration());
			result["treelabel"] = "whitebalance:multipliers";
		}

	}
 	return result;
}



