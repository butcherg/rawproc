
#include "gimage_process.h"
#include "gimage_parse.h" //for paramexists()
#include "gimage/strutil.h"
#include "myConfig.h"
#include "elapsedtime.h"
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
	
		cmsUInt32Number intent = INTENT_PERCEPTUAL;
		if (params["rendering_intent"] == "perceptual") intent = INTENT_PERCEPTUAL;
		if (params["rendering_intent"] == "saturation") intent = INTENT_SATURATION;
		if (params["rendering_intent"] == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
		if (params["rendering_intent"] == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

		bool bpc = false;
		if (params["bpc"] == "true") bpc = true;


		//tool-specific logic:
		if (params["mode"] == "camera") {

		}
		else if (params["mode"] == "primaries") {
			if (params["op"] == "convert") {
				_mark();
				if (dib.ApplyColorspace(params["primaries"], intent, bpc, threadcount) != GIMAGE_OK) {
					result["error"] = "Error - ColorSpace convert failed.";
				}
				else params["treelabel"] = "colorspace:primaries,convert";
					//m_tree->SetItemText(id, wxString::Format(_("colorspace:%s,convert"),wxString(cp[0])));
				result["duration"] = std::to_string(_duration());
			}
			else if (params["op"] == "assign") {
				_mark();
				if (dib.AssignColorspace(params["primaries"]) != GIMAGE_OK) {
					result["error"] = "Error - ColorSpace assign failed.";
				}
				else params["treelabel"] = "colorspace:primaries,assign";
 					//m_tree->SetItemText(id, wxString::Format(_("colorspace:%s,assign"),wxString(cp[0])));
				result["duration"] = std::to_string(_duration());
			}
		}
		else if (params["mode"] == "built-in") {
			if (params["op"] == "convert") {
				_mark();
				if (dib.ApplyColorspace(params["profilename"], intent, bpc, threadcount) != GIMAGE_OK) {
					result["error"] = "Error - ColorSpace convert failed.";
				}
				else params["treelabel"] = string_format("colorspace:%s,convert",params["profilename"]);
					//m_tree->SetItemText(id, wxString::Format(_("colorspace:%s,convert"),wxString(cp[0])));
				result["duration"] = std::to_string(_duration());
			}
			else if (params["op"] == "assign") {
				_mark();
				if (dib.AssignColorspace(params["profilename"]) != GIMAGE_OK) {
					result["error"] = "Error - ColorSpace assign failed.";
				}
				else params["treelabel"] = string_format("colorspace:%s,assign",params["profilename"]);
 					//m_tree->SetItemText(id, wxString::Format(_("colorspace:%s,assign"),wxString(cp[0])));
				result["duration"] = std::to_string(_duration());
			}
		}
		else if (params["mode"] == "file") {

		}
		else result["error"] = string_format("Error - unrecognized mode: %s",params["mode"]);



	}
	return result;
}

