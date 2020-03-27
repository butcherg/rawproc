
#include "gimage_process.h"
#include "gimage_parse.h" //for paramexists()
#include "gimage/strutil.h"
#include "fileutil.h"
#include "myConfig.h"
#include "elapsedtime.h"
#include "CameraData.h"
#include "cJSON.h"

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
		result["error"] = "Error - no mode";
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
		else params["channel"] = "rgb";

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
		}
		else if (params["mode"] == "values"){
			blk = atof(params["black"].c_str());
			wht = atof(params["white"].c_str());
		}
		else if (params["mode"] == "data"){
			std::map<std::string,float> s = dib.StatsMap();
			if (channel == CHANNEL_RGB) {
				blk = std::min(std::min(s["rmin"],s["gmin"]),s["bmin"]);
				wht = std::max(std::max(s["rmax"],s["gmax"]),s["bmax"]);
				if (paramexists(params, "minwhite")) {
					if (params["minwhite"] == "true") wht = std::min(std::min(s["rmax"],s["gmax"]),s["bmax"]);
				}
			}
			else if (channel == CHANNEL_RED) {
				blk = s["rmin"];
				wht = s["rmax"];
			}
			else if (channel == CHANNEL_GREEN) {
				blk = s["gmin"];
				wht = s["gmax"];
			}
			else if (channel == CHANNEL_BLUE) {
				blk = s["bmin"];
				wht = s["bmax"];
			}
		}
		else if (params["mode"] == "norm"){
			blk = atof(params["black"].c_str());
			wht = atof(params["white"].c_str());
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
		}

		_mark();
		if (params["mode"] == "norm")
			dib.ApplyNormalization(blk, wht, threadcount);
		else
			dib.ApplyToneLine(blk, wht, channel, threadcount);
		result["duration"] = std::to_string(_duration());
		result["black"] = params["black"];
		result["white"] = params["white"];
		result["treelabel"] = string_format("blackwhitepoint:%s",params["mode"].c_str());
		if (params["minwhite"] == "true") result["treelabel"] += ",minwhite";
	}

	return result;

}

std::map<std::string,std::string> process_colorspace(gImage &dib, std::map<std::string,std::string> params)
{
	std::map<std::string,std::string> result;

	//error-catching:
	if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		result["error"] = "Error - no mode";
	}
	//nominal processing:
	else {
		int threadcount = getThreadCount(atoi(myConfig::getConfig().getValueOrDefault("tool.colorspace.cores","0").c_str()));
		result["threadcount"] = std::to_string(threadcount);

		//tool-specific setup:
		GIMAGE_ERROR ret;

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
				params["source"] = c.getItem(makemodel, "primary_source");
				//((ColorspacePanel *) toolpanel)->setCamdatStatus(wxString(c.getStatus()));
			}
			
			if (dcraw_primaries.empty()) { //Last resort, look in the LibRaw metadata
				std::string libraw_primaries = dib.getInfoValue("Libraw.CamXYZ");
				std::vector<std::string> primaries = split(libraw_primaries, ",");
				if (primaries.size() >= 9 & atof(primaries[0].c_str()) != 0.0) {
					params["icc"] = string_format("%d,%d,%d,%d,%d,%d,%dfs,%d,%d",
						int(atof(primaries[0].c_str()) * 10000),
						int(atof(primaries[1].c_str()) * 10000),
						int(atof(primaries[2].c_str()) * 10000),
						int(atof(primaries[3].c_str()) * 10000),
						int(atof(primaries[4].c_str()) * 10000),
						int(atof(primaries[5].c_str()) * 10000),
						int(atof(primaries[6].c_str()) * 10000),
						int(atof(primaries[7].c_str()) * 10000),
						int(atof(primaries[8].c_str()) * 10000));
					params["source"] = "LibRaw";
				}
				result["treelabel"] = "colorspace:camera";
			}
		}
		else if (params["mode"] == "primaries") {
			result["treelabel"] = "colorspace:primaries";
		}
		else if (params["mode"] == "built-in") {
			result["treelabel"] = "colorspace:" + params["icc"];
		}
		else if (params["mode"] == "file") {
			if (!file_exists(profilepath+params["icc"])) {
				result["error"] = string_format("Error - file not found: %s",params["icc"].c_str());
				return result;
			}
			result["treelabel"] = "colorspace:file";
		}
		else {
			 result["error"] = string_format("Error - unrecognized mode: %s",params["mode"].c_str());
			 return result;
		}

		if (params["op"] == "convert") {
			_mark();
			if ((ret = dib.ApplyColorspace(params["icc"], intent, bpc, threadcount)) == GIMAGE_OK)
			result["duration"] = std::to_string(_duration());
			result["treelabel"] += std::string(",") + std::string("convert");
		}
		else if (params["op"] == "assign") {
			_mark();
			if ((ret = dib.AssignColorspace(params["icc"])) == GIMAGE_OK) 
			result["duration"] = std::to_string(_duration());
			result["treelabel"] += std::string(",") + std::string("assign");
		}

		switch (ret) {
			case GIMAGE_APPLYCOLORSPACE_BADPROFILE:
				result["error"] = "ColorSpace apply: no input profile in image.";
				break;
			case GIMAGE_APPLYCOLORSPACE_BADINTENT_INPUT:
				result["error"] = "ColorSpace apply: input profile doesn't support rendering intent.";
				break;
			case GIMAGE_APPLYCOLORSPACE_BADINTENT_OUTPUT:
				result["error"] = "ColorSpace apply: output profile doesn't support rendering intent.";
				break;
			case GIMAGE_APPLYCOLORSPACE_BADTRANSFORM:
				result["error"] = "ColorSpace apply: colorspace transform creation failed.";
				break;
		}

	}
	return result;
}

