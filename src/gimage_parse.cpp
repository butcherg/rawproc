
#include "gimage_parse.h"
#include "gimage/strutil.h"
#include "myConfig.h"
#include "elapsedtime.h"
#include "cJSON.h"

bool paramexists (std::map<std::string,std::string> m, std::string k)
{
	return (m.find(k) != m.end());
}

std::map<std::string,std::string> parse_JSONparams(std::string JSONstring)
{
	std::map<std::string,std::string> pmap;
	cJSON *json = cJSON_Parse(JSONstring.c_str());
	cJSON *iterator;
	cJSON_ArrayForEach(iterator, json) {
		pmap[std::string(iterator->string)] = std::string(iterator->valuestring);
	}
	return pmap;
}

int getThreadCount(int threadcount) {
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	return threadcount;
}


void paramprint(std::map<std::string,std::string> params)
{
	printf("parmstring: %ld elements:\n",params.size()); fflush(stdout);
	if (params.empty()) return;
	
	for (std::map<std::string,std::string>::iterator it=params.begin(); it!=params.end(); ++it) {
		printf("%s: %s\n",it->first.c_str(), it->second.c_str()); 
		fflush(stdout);
	}
}


//blackwhitepoint
//auto:		rgb|red|green|blue - do auto on channel
//values:	rgb|red|green|blue,<nbr>[,<nbr>] - specific b/w on channel, if only one number, b=<nbr>, w=255
//data:		rgb|red|green|blue,data - b/w on data limits, b=smallest r|g|b, w=largest r|g|b
//data:		rgb|red|green|blue,data,minwhite - b/w on data limits w=smallest r|g|b
//camera:	camera - b/w on rgb using camera exif limits
//values:	<nbr>,[<nbr>] - specific b/w on rgb, if only one number, b=<nbr>, w=255
//auto:		NULL - do auto on rgb

std::map<std::string,std::string> parse_blackwhitepoint(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (paramstring == std::string()) {  //NULL string, default processing
			pmap["channel"] = "rgb";
			pmap["mode"] = "auto";
		}
		else if (p[0] == "rgb" | p[0] == "red" | p[0] == "green" | p[0] == "blue") {    // | p[0] == "min") {   ??
			pmap["channel"] = p[0];
			if (psize >= 2) {
				if (p[1] == "data") { //bound on min/max of image data
					pmap["mode"] = "data";
					if (psize >= 3 && p[2] == "minwhite") pmap["minwhite"] = "true";
				}
				else if (p[1] == "auto") {
					pmap["mode"] = "auto";
					pmap["channel"] = p[0];
				}
				else { 
					pmap["mode"] = "values";
					pmap["black"] = p[1];
					if (psize >= 3) 
						pmap["white"] = p[2];
					else
						pmap["white"] = "255";
				}
			}
			else pmap["mode"] = "auto";
		}
		else if (p[0] == "camera") {
			pmap["mode"] = "camera";
			pmap["channel"] = "rgb";
		}
		else if (p[0] == "norm") {
			pmap["mode"] = "norm";
			pmap["channel"] = "rgb";
			pmap["black"] = "0.0";
			pmap["white"] = "1.0";
			if (psize == 3) {
				pmap["black"] = p[1];
				pmap["white"] = p[2];
			}
		}
		else if (psize == 2) {  //just two values
			pmap["mode"] = "values";
			pmap["channel"] = "rgb";
			pmap["black"] = p[0];
			pmap["white"] = p[1];
		}
		else {
			pmap["error"] = "Error - unrecognized positional parameter string";
		}
	}
	return pmap;
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

