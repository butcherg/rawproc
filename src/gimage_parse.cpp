
#include "gimage_parse.h"
#include "gimage/strutil.h"
#include "myConfig.h"
#include "elapsedtime.h"
#include "cJSON.h"

#include <algorithm>

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

void paramprint(std::map<std::string,std::string> params)
{
	printf("parmstring: %ld elements:\n",params.size()); fflush(stdout);
	if (params.empty()) return;
	
	for (std::map<std::string,std::string>::iterator it=params.begin(); it!=params.end(); ++it) {
		printf("\t%s: %s\n",it->first.c_str(), it->second.c_str()); 
		fflush(stdout);
	}
}


//add
//:file,<filename> - adds the specified image file to the image, pixel-for-pixel
//:<sfloat> - add the specified value to the image
std::map<std::string,std::string> parse_add(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (psize < 1) {
			pmap["error"] = "add:ParseError - Need some parameters"; 
			return pmap;
		}

		if (p[0] == "rgb" | p[0] == "red" | p[0] == "green" | p[0] == "blue") {
			pmap["mode"] = "value";
			pmap["cmdlabel"] = string_format("add:%s,value",p[0].c_str());
			pmap["channel"] = p[0];
			if (psize >= 2) {
				if (isFloat(p[1])) {
					pmap["value"] = p[1];
				}
				else {
					pmap["error"] = string_format("add:ParseError - Not a float: %s", p[1].c_str()); 
					return pmap;
				}
			}
			else {
				pmap["error"] = "add:ParseError - Needs a float value"; 
				return pmap;
			}
		}
		else if (isFloat(p[0])) {
			pmap["mode"] = "value";
			pmap["cmdlabel"] = "add:rgb,value";
			pmap["value"] = p[0];
			pmap["channel"] = "rgb";
		}
		else if (p[0] == "file") {
			pmap["mode"] = "file";
			pmap["cmdlabel"] = "add:file";
			if (psize >= 2) {
				pmap["filename"] = p[1];
			}
			else {
					pmap["error"] = string_format("add:ParseError - no file name."); 
					return pmap;
			}
			if (psize >= 3) {
				pmap["position"] = p[2];
			}
			else {
					pmap["error"] = string_format("add:ParseError - no position."); 
					return pmap;
			}
		}
		else { //filename?
			pmap["mode"] = "file";
			pmap["cmdlabel"] = "add:file";
			pmap["filename"] = p[0];
		}

	}
	return pmap;
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

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (paramstring == std::string() | paramstring == "auto") {  //NULL string, default processing
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
					if (!isFloat(p[1])) {
						pmap["error"] = string_format("blackwhitepoint:ParseError - invalid float: %s",p[1].c_str());
						return pmap;
					}
					pmap["mode"] = "values";
					pmap["black"] = p[1];
					if (psize >= 3) {
						if (!isFloat(p[2])) {
							pmap["error"] = string_format("blackwhitepoint:ParseError - invalid float: %s",p[2].c_str());
							return pmap;
						}
						pmap["white"] = p[2];
					}
					else {
						if (atof(p[1].c_str()) < 1.0)
							pmap["white"] = "1.0";
						else
							pmap["white"] = "255";
					}
				}
			}
			else pmap["mode"] = "auto";
		}
		else if (p[0] == "data") {
			pmap["mode"] = "data";
			pmap["channel"] = "rgb";
			if (psize >= 2 && p[1] == "minwhite") pmap["minwhite"] = "true";
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
			else {
				pmap["error"] = "blackwhitepoint:ParseError - norm requires two values.";
				return pmap;
			}
		}
		else if (psize == 2) {  //just two values
			if (!isFloat(p[0])) {
				pmap["error"] = string_format("blackwhitepoint:ParseError - invalid float: %s",p[0].c_str());
				return pmap;
			}
			if (!isFloat(p[1])) {
				pmap["error"] = string_format("blackwhitepoint:ParseError - invalid float: %s",p[1].c_str());
				return pmap;
			}
			pmap["mode"] = "values";
			pmap["channel"] = "rgb";
			pmap["black"] = p[0];
			pmap["white"] = p[1];
		}
		else {
			pmap["error"] = string_format("blackwhitepoint:ParseError - unrecognized positional parameter string: %s",p[0].c_str());
			return pmap;
		}
		pmap["cmdlabel"] = string_format("blackwhitepoint:%s",pmap["mode"].c_str());
	}
	return pmap;
}



//colorspace
//file		:<profile_filename>[,assign|convert][,absolute_colorimetric|relative_colorimetric|perceptual|saturation][,bpc] - open profile file and use to assign|convert
//camera	:camera[,assign|convert][,absolute_colorimetric|relative_colorimetric|perceptual|saturation][,bpc] - find camera primaries in dcraw.c|camconst.json|libraw, make a d65 profile and use to assign|convert
//primaries	:<int>,<int>,<int>,<int>,<int>,<int>,<int>,<int>,<int>[,assign|convert][,absolute_colorimetric|relative_colorimetric|perceptual|saturation][,bpc] - use 9 ints to make a d65 profile and use to assign|convert 
//built-in	:srgb|wide|adobe|prophoto|identity[,assign|convert][,absolute_colorimetric|relative_colorimetric|perceptual|saturation][,bpc]

std::map<std::string,std::string> parse_colorspace(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		unsigned token = 0;

		pmap["bpc"] = "false";

		if (p[0] == "camera") {
			pmap["mode"] = "camera";
			token = 1;
		}

		else if (p[0] == "srgb" | 
				p[0] == "wide" | 
				p[0] == "adobe" | 
				p[0] == "prophoto" | 
				p[0] == "identity" |
				p[0] == "aces2065-1-v4-g10" |
				p[0] == "adobergb-v4-g10" |
				p[0] == "bt709-v4-g10" |
				p[0] == "prophoto-v4-g10" |
				p[0] == "rec2020-v4-g10" |
				p[0] == "srgb-v4-g10" |
				p[0] == "srgb-v2-g22" |
				p[0] == "srgb-output") 
				{
			pmap["mode"] = "built-in";
			pmap["icc"] = p[0];
			token = 1;
		}

		//else if (std::count(p[0].begin(), p[0].end(), ',') == 8) {
		else if (isInt(p[0])) {
			pmap["mode"] = "primaries";
			pmap["icc"] = 
				string_format("%s,%s,%s,%s,%s,%s,%s,%s,%s",p[0].c_str(),p[1].c_str(),p[2].c_str(),p[3].c_str(),p[4].c_str(),p[5].c_str(),p[6].c_str(),p[7].c_str(),p[8].c_str());
			token = 9;
		}
		else {  //treat whatever is in p[0] as a file name
			pmap["mode"] = "file";
			pmap["icc"] = p[0];
			token = 1;
		}

		for (unsigned i=token; i<psize; i++) {  //op, rendering intent, and/or bpc in any order
			if (p[i] == "convert" | p[i] == "assign") {
				pmap["op"] = p[i];
			}
			else if (p[i] == "absolute_colorimetric" | 
				 p[i] == "relative_colorimetric" | 
				 p[i] == "perceptual" | 
				 p[i] == "saturation") {
					pmap["rendering_intent"] = p[i];
			}
			else if (p[i] == "bpc") {
				pmap["bpc"] = true;
			}				
			else {
				pmap["error"] = string_format("colorspace:ParseError - unrecognized positional parameter: %s",p[i].c_str());
				return pmap;
			}
		}
		pmap["cmdlabel"] = string_format("colorspace:%s,%s",pmap["mode"].c_str(),pmap["op"].c_str());

	}
	return pmap;
}

//crop
//default	:<x1>,<y1>,<x2>,<y2> - extract subimage at top,left,bottom,right bounds and make the new image.  can be either int coords or 0.0-1.0 proportions to w|h
std::map<std::string,std::string> parse_crop(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		
		if (psize < 4) {
			pmap["error"] = string_format("crop:ParseError - not enough crop parameters.");
			return pmap;
		}
		pmap["mode"] = "default";
		pmap["cmdlabel"] = "crop";
		pmap["x"] = p[0];
		pmap["y"] = p[1];
		pmap["w"] = p[2];
		pmap["h"] = p[3];
		
	}
	return pmap;
}

std::map<std::string,std::string> parse_cropspectrum(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		
		if (psize >= 1) {
			pmap["bound"] = p[0];
		}
		else {
			pmap["error"] = string_format("cropspectrum:ParseError - not enough parameters.");
			return pmap;
		}
			
		if (psize >= 2) {
			pmap["threshold"] = p[1];
		}
		else {
			pmap["error"] = string_format("cropspectrum:ParseError - not enough parameters.");
			return pmap;
		}
		pmap["mode"] = "default";
		pmap["cmdlabel"] = "cropspectrum";
	}
	return pmap;
}

//curve
//default	:rgb|red|green|blue,<x1>,<y1>,...,<xn>,<yn> - apply curve to the designated channel defined by the x,y coordinates, 0-255
std::map<std::string,std::string> parse_curve(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		
		if (p[0] == "rgb" | p[0] == "red" | p[0] == "green" | p[0] == "blue") {  
			pmap["channel"] =  p[0];
			if (isFloat(p[1])) 
				pmap["curvecoords"] = p[1];
			else {
				pmap["error"] = string_format("curve:ParseError - not a float: %s.",p[1].c_str());
				return pmap;
			}
			for (unsigned i=2; i<psize; i++) {
				if (isFloat(p[i])) {
					pmap["curvecoords"] += std::string(",") + p[i];
				}
				else {
					pmap["error"] = string_format("curve:ParseError - not a float: %s.",p[i].c_str());
					return pmap;
				}
			}
		}
		else if (isFloat(p[0])) {
			pmap["channel"] =  "rgb";
			for (unsigned i=0; i<psize; i++) {
				if (isFloat(p[i])) {
					pmap["curvecoords"] += std::string(",") + p[i];
				}
				else {
					pmap["error"] = string_format("curve:ParseError - not a float: %s.",p[i].c_str());
					return pmap;
				}
			}
		}
		else {
			pmap["error"] = string_format("curve:ParseError - Unrecognized token: %s.",p[0].c_str());
			return pmap;
		}
		pmap["mode"] = "default";
		pmap["cmdlabel"] = string_format("curve:%s",pmap["channel"]);
	}
	return pmap;
}


//demosaic (each algo is a mode...)
//:color - color the unmosaiced image with the pattern colors
//:half|half_resize|vng|rcd|igv|ahd|xtran_fast - demosaic the image
//:dcb[,<iterations>][,dcb_enhance] - demosaic the image with the supplied parameters
//:amaze[,<initgain>][,<border>] - demosaic the image with the supplied parameters
//:lmmse[,<iterations] - demosaic the image with the supplied parameters
//:xtran_markesteijn[,<passes>][,usecielab] - demosaic the image with the supplied parameters
//:proof - xtran_fast or half, depending on image type (xtran|bayer)
std::map<std::string,std::string> parse_demosaic(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (p[0] == "half" |p[0] == "half_resize" |
#ifdef USE_LIBRTPROCESS
			p[0] == "vng" |p[0] == "rcd" |p[0] == "igv" |p[0] == "ahd" |p[0] == "xtran_fast" |p[0] == "dcb" |p[0] == "amaze" |p[0] == "lmmse" |p[0] == "xtran_markesteijn" |
#endif
			p[0] == "proof" | p[0] == "color") {
				pmap["mode"] = p[0];
				pmap["cmdlabel"] = string_format("demosaic:%s",p[0].c_str());
				
		}
		else {
			pmap["error"] = string_format("demosaic:ParseError - Unrecognized demosaic option: %s.",p[0].c_str());
			return pmap;
		}

#ifdef USE_LIBRTPROCESS
		if (pmap["mode"] == "dcb") {
			if (psize >= 2) 
				pmap["iterations"] = p[1];
			else
				pmap["iterations"] = "1";
			if (psize >= 3) {
				if (p[2] == "dcb_enhance" | p[2] == "1") 
					pmap["dcb_enhance"] = "true";
				else {
					pmap["dcb_enhance"] = "false";
				}
			}
		}
		if (pmap["mode"] == "amaze") {
			if (psize >= 2) pmap["initgain"] = p[1];
			if (psize >= 3) pmap["border"] = p[2]; 
		}
		if (pmap["mode"] == "lmmse") {
			if (psize >= 2) 
				pmap["iterations"] = p[1];
			else
				pmap["iterations"] = "1";
		}
		if (pmap["mode"] == "xtran_markesteijn") {
			if (psize >= 2) 
				pmap["passes"] = p[1];
			else
				pmap["passes"] = "1";
			if (psize >= 3) {
				if (p[2] == "usecielab") 
					pmap["usecielab"] = "true";
				else {
					pmap["error"] = string_format("demosaic:ParseError - parameter should be 'usecielab', not %s.",p[2].c_str());
					return pmap;
				}
			}
		}
#endif
	}
	return pmap;
}

//denoise (each algo is a mode...)
//:nlmeans[,<sigma>][,<local>][,<patch>][,<threshold>] - apply nlmeans denoise
//:wavelet[,<threshold>] - apply wavelet denoise
std::map<std::string,std::string> parse_denoise(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if ( p[0] == "nlmeans") {
			pmap["mode"] = p[0];
			pmap["cmdlabel"] = "denoise:nlmeans";
			pmap["sigma"]	  = myConfig::getConfig().getValueOrDefault("tool.denoise.initialvalue",
				myConfig::getConfig().getValueOrDefault("tool.denoise.initialvalue","0"));
			pmap["local"]	  = myConfig::getConfig().getValueOrDefault("tool.denoise.local","3");
			pmap["patch"]	  = myConfig::getConfig().getValueOrDefault("tool.denoise.patch","1");
			if (psize >= 2) pmap["sigma"] = p[1];
			if (psize >= 3) pmap["local"] = p[2]; 
			if (psize >= 4) pmap["patch"] = p[3]; 
			if (psize >= 5) pmap["nlmeansthreshold"] = p[4]; 
		}
		else if (p[0] == "wavelet") {
			pmap["mode"] = p[0];
			pmap["cmdlabel"] = "denoise:wavelet";
			pmap["threshold"] = myConfig::getConfig().getValueOrDefault("tool.denoise.threshold","0.0");
			if (psize >= 2) pmap["threshold"] = p[1];
		}
		else {
			pmap["error"] = string_format("denoise:ParseError - Unrecognized denoise option: %s.",p[0].c_str());
			return pmap;
		}
		
		
	}
	return pmap;
}

//exposure
//:<ev> - apply the ev to the image (img and rawproc)
//:patch,x,y,r,ev0
std::map<std::string,std::string> parse_exposure(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (p[0] == "patch") {
			if (psize < 5) {
				pmap["error"] = "exposure:ParseError - patch mode needs x, y, radius and ev0.";
				return pmap;
			}
			pmap["mode"] = "patch";
			pmap["cmdlabel"] = "exposure:patch";
			pmap["x"] = p[1];
			pmap["y"] = p[2]; 
			pmap["radius"] = p[3]; 
			pmap["ev0"] = p[4]; 
		}
		else {
			if (isFloat(p[0])) {
				pmap["ev"] = p[0];
			}
			else {
				pmap["error"] = string_format("exposure:ParseError - Not numeric: %s.",p[0].c_str());
				return pmap;
			}
			pmap["mode"] = "ev";
			pmap["cmdlabel"] = "exposure:ev";
		}

	}
	return pmap;
}

//gray
//:<float>,<float>,<float> - grayscale the image using the specified RGB multipliers
//:NULL - grayscale the image using the default multipliers
std::map<std::string,std::string> parse_gray(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		
		std::string mult;

		if (paramstring.size() == 0) {
			mult = myConfig::getConfig().getValueOrDefault("tool.gray.r","0.21");
			if (isFloat(mult)) { 
				pmap["red"]   = mult; 
			}
			else { 
				pmap["error"] = string_format("gray:ParseError - invalid float from tool.gray.r: %s.",mult.c_str()); 
				return pmap; 
			}
			mult = myConfig::getConfig().getValueOrDefault("tool.gray.g","0.72");
			if (isFloat(mult)) {
				pmap["green"] = mult; 
			}
			else { 
				pmap["error"] = string_format("gray:ParseError - invalid float from tool.gray.g: %s.",mult.c_str()); 
				return pmap; 
			}
			mult = myConfig::getConfig().getValueOrDefault("tool.gray.b","0.07");
			if (isFloat(mult)) {
				pmap["blue"]  = mult; 
			}
			else { 
				pmap["error"] = string_format("gray:ParseError - invalid float from tool.gray.b: %s.",mult.c_str()); 
				return pmap; 
			}
		}
		else if (psize == 3) {
			if (isFloat(p[0])) pmap["red"]   = p[0]; else { pmap["error"] = string_format("gray:ParseError - invalid float: %s.",p[0].c_str()); return pmap; }
			if (isFloat(p[1])) pmap["green"] = p[1]; else { pmap["error"] = string_format("gray:ParseError - invalid float: %s.",p[1].c_str()); return pmap; }
			if (isFloat(p[2])) pmap["blue"]  = p[2]; else { pmap["error"] = string_format("gray:ParseError - invalid float: %s.",p[2].c_str()); return pmap; }
		}
		else {
			pmap["error"] = string_format("gray:ParseError -invalid parameters: %s.",paramstring.c_str()); 
			return pmap;
		}

		pmap["mode"] = "gray";
		pmap["cmdlabel"] = "gray";
		
	}
	return pmap;
}

#ifdef USE_LENSFUN
//laundrylist - lenscorrection:([ca][,vig][,dist][,autocrop])[,nearest|bilinear|lanczos3],[reticlinear|fisheye|panoramic|equirectangular|orthographic|stereographic|equisolid|thoby)
//nameval - lenscorrection:ops=<op1>...[;algo=nearest|bilinear|lanczos3][,geometry=reticlinear|fisheye|panoramic|equirectangular|orthographic|stereographic|equisolid|thoby - Apply the specified lens corrections (ca,vig,dist,autocrop) using the lensfun data for that lens. algo applies to dist and ca
std::map<std::string,std::string> parse_lenscorrection(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	// no positional parameters for lenscorrection, use "laundry list" ...
	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		
		for (unsigned i=0; i<psize; i++) { //ca,vig,dist,autocrop
			if (p[i] == "ca") pmap["ops"] += "ca,";
			else if (p[i] == "vig") pmap["ops"] += "vig,";
			else if (p[i] == "dist") pmap["ops"] += "dist,";
			else if (p[i] == "autocrop") pmap["ops"] += "autocrop,";
			
			else if (p[i] == "nearest") pmap["algo"] = "nearest,";
			else if (p[i] == "bilinear") pmap["algo"] = "bilinear,";
			else if (p[i] == "lanczos3") pmap["algo"] = "lanczos3,";
			
			else if (p[i] == "reticlinear") pmap["geometry"] = "reticlinear";
			else if (p[i] == "fisheye") pmap["geometry"] = "fisheye";
			else if (p[i] == "panoramic") pmap["geometry"] = "panoramic,";
			else if (p[i] == "equirectangular") pmap["geometry"] = "equirectangula";
			else if (p[i] == "orthographic") pmap["geometry"] = "orthographic";
			else if (p[i] == "stereographic") pmap["geometry"] = "stereographic";
			else if (p[i] == "equisolid") pmap["geometry"] = "equisolid";
			else if (p[i] == "thoby") pmap["geometry"] = "thoby,";
			else {
				pmap["error"] = string_format("lenscorrection:ParseError - unrecognized parameter: %s",p[i].c_str()); 
				return pmap;
			}
		}
		size_t opssize = pmap["ops"].size();
		if (opssize > 0) pmap["ops"].erase(opssize-1);  //get rid of the last comma
 		
	}

	pmap["mode"] = "default";
	pmap["cmdlabel"] = "lenscorrection";
	return pmap;
}
#endif

//redeye
//:<xint1>,<yint1>[,<xint2>,<yint2>...],<tint>,<lint> - Apply redeye correction at the points specified by xn,yn with the specified threshold and limit (limit is a radius) (img can only specify one x,y)
//also, coord pairs can be ";" separated...
//src/img -f /home/glenn/SDCard/Pictures/DSG_6304-redeyetest.jpg "redeye:1.50,30,0,1.00;3014,1076;3349,1048;" foo.jpg
//src/img -f /home/glenn/SDCard/Pictures/DSG_6304-redeyetest.jpg redeye:1.50,30,0,1.00,3014,1076,3349,1048 foo.jpg
std::map<std::string,std::string> parse_redeye(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional


		//parse parameters, 4 of 'em:


		if (paramstring.find(";") == std::string::npos) { //all parameters and coordinates separated by ","
			std::vector<std::string> p = split (paramstring, ",");
			int psize = p.size();

			if (psize < 4) {
				pmap["error"] = "redeye:ParseError - not enough parameters."; 
				return pmap;
			}

			if (isFloat(p[0])) {
				pmap["threshold"] = p[0];
			}
			else {
				pmap["error"] = string_format("redeye:ParseError - threshold is not a float: %s.",p[0].c_str()); 
				return pmap;
			}

			if (isUnsignedInt(p[1])) {
				pmap["limit"] = p[1];
			}
			else {
				pmap["error"] = string_format("redeye:ParseError - limit is not an unsigned integer: %s.",p[1].c_str()); 
				return pmap;
			}

			if (p[2] == "1" | p[2] == "desat") {
				pmap["desat"] = "true";
			}
			else if (p[2] == "0") {
				pmap["desat"] = "false";
			}
			else {
				pmap["error"] = string_format("redeye:ParseError - should be \"desat\" or \"1|0\", instead of %s.",p[2].c_str()); 
				return pmap;
			}

			if (isFloat(p[3])) {
				pmap["desatpct"] = p[3];
			}
			else {
				pmap["error"] = string_format("redeye:ParseError - threshold is not a float: %s.",p[3].c_str()); 
				return pmap;
			}

			if (((psize - 4) % 2) != 0) { //remaining number of parameters is not even
				pmap["error"] = "redeye:ParseError - number of coord parameters is not even."; 
				return pmap;
			} 

			if (isUnsignedInt(p[4])) pmap["coords"] = p[4];
			for (unsigned i=4; i < psize; i++) {
				if (isUnsignedInt(p[i])) pmap["coords"].append(","+p[i]);
			}
		}
		else { // semicolon-delimited params and coordinates

			std::vector<std::string> s = split(paramstring, ";");
			int ssize = s.size();

			std::vector<std::string> p = split (s[0], ",");
			int psize = p.size();

			if (psize < 4) {
				pmap["error"] = "redeye:ParseError - not enough parameters."; 
				return pmap;
			}

			if (isFloat(p[0])) {
				pmap["threshold"] = p[0];
			}
			else {
				pmap["error"] = string_format("redeye:ParseError - threshold is not a float: %s.",p[0].c_str()); 
				return pmap;
			}

			if (isUnsignedInt(p[1])) {
				pmap["limit"] = p[1];
			}
			else {
				pmap["error"] = string_format("redeye:ParseError - limit is not an unsigned integer: %s.",p[1].c_str()); 
				return pmap;
			}

			if (p[2] == "1" | p[2] == "desat") {
				pmap["desat"] = "true";
			}
			else if (p[2] == "0") {
				pmap["desat"] = "false";
			}
			else {
				pmap["error"] = string_format("redeye:ParseError - should be \"desat\" or \"1|0\", instead of %s.",p[2].c_str()); 
				return pmap;
			}

			if (isFloat(p[3])) {
				pmap["desatpct"] = p[3];
			}
			else {
				pmap["error"] = string_format("redeye:ParseError - threshold is not a float: %s.",p[3].c_str()); 
				return pmap;
			}

			//parse x,y coordinates
			for (unsigned i=1; i < s.size(); i++) {
				std::vector<std::string> c = split(s[i], ",");
				if (c.size() != 2) {
					pmap["error"] = string_format("redeye:ParseError - not a coordinate pair: %s.",s[i].c_str()); 
					return pmap;
				}
				if (i == s.size() - 1)
					pmap["coords"].append(c[0]+","+c[1]);
				else
					pmap["coords"].append(c[0]+","+c[1]+",");
			}
		}
		pmap["mode"] = "default";
		pmap["cmdlabel"] = "redeye";
		pmap["paramstring"] = paramstring;

	}
	return pmap;
}

//resize
//:<wint>[,<hint>][,box|bilinear|bspline|bicubic|catmullrom|lanczos3] 
//	- resize the image to the specified width and height, using the specified interpolation algorithm.  
//	If only one number is provided, use it for the largest dimension and compute the other to preserve 
//	the aspect ratio (0 is passed to the Apply function and it does the aspect computation).  
std::map<std::string,std::string> parse_resize(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		
		if (isUnsignedInt(p[0])) {
			pmap["width"] = p[0];
		}
		else {
			pmap["error"] = string_format("resize:ParseError - first parameter is not an unsigned integer: %s.",p[0].c_str()); 
			return pmap;
		}
		
		if (psize < 2) {
			pmap["mode"] = "largestside";
			pmap["cmdlabel"] = "resize";
			return pmap;
		}
		else if (isUnsignedInt(p[1])) {
			pmap["height"] = p[1];
			pmap["mode"] = "asspecified";
			pmap["cmdlabel"] = "resize";
		}
		else if (p[1] == "box" | p[1] == "bilinear" | p[1] == "bspline" | p[1] == "bicubic" | p[1] == "catmullrom" | p[1] == "lanczos3") {
			pmap["algorithm"] = p[1];
		}
		else {
			pmap["error"] = string_format("resize:ParseError - Unrecognized parameter: %s.",p[1].c_str()); 
			return pmap;
		}
		
		if (psize >= 3 && (p[2] == "box" | p[2] == "bilinear" | p[2] == "bspline" | p[2] == "bicubic" | p[2] == "catmullrom" | p[2] == "lanczos3")) {
			pmap["algorithm"] = p[1];
		}
		else {
			pmap["error"] = string_format("resize:ParseError - Unrecognized algorithm: %s.",p[2].c_str()); 
			return pmap;
		}

	}
	return pmap;
}

std::map<std::string,std::string> parse_rotate(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (isFloat(p[0])) {
			pmap["angle"] = p[0];
		}
		else if (p[0] == "hmirror") {
			pmap["hmirror"] == "true";
		}
		else if (p[0] == "vmirror") {
			pmap["vmirror"] == "true";
		}
		else {
			pmap["error"] = string_format("rotate:ParseError - Not a float: %s.",p[0].c_str()); 
			return pmap;
		}

		if (psize >= 2) {
			if (p[1] == "autocrop") {
				pmap["autocrop"] == "true";
			}
		}
		pmap["mode"] = "default";
		pmap["cmdlabel"] = "rotate";
	}
	return pmap;
}

//saturation
//:[rgb|red|green|blue],<sfloat>[,<tfloat>] - apply HSL saturation to the image in the amount specified. If channel is specified, restrict application to it; if threshold is specified (not used for rgb), restrict application to only values above it. 
std::map<std::string,std::string> parse_saturation(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (psize < 1) {
			pmap["error"] = "saturation:ParseError - Need a value"; 
			return pmap;
		}
		
		if (p[0] == "rgb" | p[0] == "red" | p[0] == "green" | p[0] == "blue") { 
			pmap["channel"] = p[0];
			if (psize >= 2) {
				if (isFloat(p[1])) {
					pmap["saturation"] = p[1];
				}
				else {
					pmap["error"] = string_format("saturation:ParseError - Not a float value: %s", p[1].c_str()); 
					return pmap;
				}
			}
			if (psize >= 3) {
				if (isFloat(p[2])) {
					pmap["threshold"] = p[2];
				}
			}
			else {
				pmap["threshold"] = "0.0";
			}
		}
		else if (isFloat(p[0])) {
			pmap["channel"] = "rgb";
			pmap["saturation"] = p[0];
			pmap["threshold"] = "0.0";
		}
		else {
			pmap["error"] = string_format("saturation:ParseError - Not a channel or float value: %s",p[0].c_str()); 
			return pmap;
		}
		
		pmap["mode"] = "default";
		pmap["cmdlabel"] = "saturation";
	}
	return pmap;
}

//sharpen
//:usm[,<sfloat>][,<rfloat>] - apply usm sharpen with the specified sigma and radius
//:convolution[,<afloat>] - apply convolution sharpen with the specified amount
//:<afloat> - apply convolution sharpening with the specified amount
std::map<std::string,std::string> parse_sharpen(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();
		
		if (psize < 1) {
			pmap["error"] = "sharpen:ParseError - Need some parameters"; 
			return pmap;
		}
		if (isFloat(p[0])) {
			pmap["mode"] = "convolution";
			pmap["cmdlabel"] = "sharpen:convolution";
			pmap["strength"] = p[0];
		}
		else if (p[0] == "convolution") {
			pmap["mode"] = "convolution";
			pmap["cmdlabel"] = "sharpen:convolution";
			if (psize >= 2) {
				if (isFloat(p[1])) {
					pmap["strength"] = p[1];
				}
				else {
					pmap["error"] = string_format("sharpen:ParseError - Convolution strength is not a float: %s", p[1].c_str()); 
					return pmap;
				}
			}
			else {
				pmap["error"] = "sharpen:ParseError - Convolution needs a float strength"; 
				return pmap;
			}
			
		}
		else if (p[0] == "usm") {
			pmap["mode"] = "usm";
			pmap["cmdlabel"] = "sharpen:usm";
			if (psize >= 2) {
				if (isFloat(p[1])) {
					pmap["sigma"] = p[1];
				}
				else {
					pmap["error"] = string_format("sharpen:ParseError - USM sigma is not a float: %s", p[1].c_str()); 
					return pmap;
				}
			}
			else {
				pmap["error"] = "sharpen:ParseError - USM needs a float sigma"; 
				return pmap;
			}
			if (psize >= 3) {
				if (isFloat(p[2])) {
					pmap["radius"] = p[2];
				}
				else {
					pmap["error"] = string_format("sharpen:ParseError - USM radius is not a float: %s", p[2].c_str()); 
					return pmap;
				}
			}
			else {
				pmap["error"] = "sharpen:ParseError - USM needs a float radius"; 
				return pmap;
			}
		}
		else {
			pmap["error"] = string_format("sharpen:ParseError - Unrecognized parameter: %s",p[0].c_str()); 
			return pmap;
		}
	}
	return pmap;
}

//subtract
//:camera - subtract the metadata-extracted black value, global (img and rawproc), perchannel (rawproc), or cfa (rawproc)
//:file,<filename> - subtracts the specified image file from the image, pixel-for-pixel
//:<sfloat> - subtract the specified value from the image
std::map<std::string,std::string> parse_subtract(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (psize < 1) {
			pmap["error"] = "subtract:ParseError - Need some parameters"; 
			return pmap;
		}

		if (p[0] == "rgb" | p[0] == "red" | p[0] == "green" | p[0] == "blue") {
			pmap["mode"] = "value";
			pmap["cmdlabel"] = string_format("subtract:%s,value",p[0].c_str());
			pmap["channel"] = p[0];
			if (psize >= 2) {
				if (isFloat(p[1])) {
					pmap["value"] = p[1];
				}
				else {
					pmap["error"] = string_format("subtract:ParseError - Not a float: %s", p[1].c_str()); 
					return pmap;
				}
			}
			else {
				pmap["error"] = "subtract:ParseError - Needs a float value"; 
				return pmap;
			}
		}
		else if (isFloat(p[0])) {
			pmap["mode"] = "value";
			pmap["cmdlabel"] = "subtract:rgb,value";
			pmap["value"] = p[0];
			pmap["channel"] = "rgb";
		}
		else if (p[0] == "camera") {
			pmap["mode"] = "camera";
			pmap["cmdlabel"] = "subtract:camera";
		}
		else if (p[0] == "file") {
			pmap["mode"] = "file";
			pmap["cmdlabel"] = "subtract:file";
			pmap["filename"] = p[1];
		}
		else { //filename?
			pmap["mode"] = "file";
			pmap["cmdlabel"] = "subtract:file";
			pmap["filename"] = p[0];
		}

	}
	return pmap;
}


//tone
//:gamma[,<gfloat>]
//:reinhard,channel|luminance[,norm]
//:log2
//:loggamma
//:filmic[,<Afloat>,<Bfloat>,<Cfloat>,<Dfloat>][,<pfloat>][,norm]
std::map<std::string,std::string> parse_tone(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (psize < 1) {
			 p.push_back("filmic");  //ToDo: use tool.tone.defaultmode instead...
		}

		if (p[0] == "gamma") {
			if (psize >=2) {
				if (isFloat(p[0])) {
					pmap["mode"] = "gamma";
					pmap["cmdlabel"] = "tone:gamma";
					pmap["value"] = p[1];
				}
				else {
					pmap["error"] = string_format("tone:ParseError - Not a float: %s", p[1].c_str()); 
					return pmap;
				}
			}
			else {
				pmap["error"] = "tone:ParseError - gamma needs a float power"; 
				return pmap;
			}
		}
		else if (p[0] == "reinhard") {
			pmap["mode"] = "reinhard";
			pmap["cmdlabel"] = "tone:reinhard";
			pmap["reinhardmode"] = "channel";  //ToDo: use value from tool.tone.reinard.mode instead...
			if (psize >=2) {
				if (p[1] == "luminance") {
					pmap["reinhardmode"] = "luminance";
				}
				else if (p[1] == "norm") {
					pmap["norm"] = "true";
				}
				else if (p[1] != "channel") {
					pmap["error"] = string_format("tone:ParseError - Unknown token: %s", p[1].c_str()); 
					return pmap;
				}
			}
		}
		else if (p[0] == "log2") {
			pmap["mode"] = "log2";
			pmap["cmdlabel"] = "tone:log2";
		}
		else if (p[0] == "loggamma") {
			pmap["mode"] = "loggamma";
			pmap["cmdlabel"] = "tone:loggamma";
		}
		else if (p[0] == "filmic") {
			pmap["mode"] = "filmic";
			pmap["cmdlabel"] = "tone:filmic";
			pmap["power"] = "1.0";
			pmap["A"] = myConfig::getConfig().getValueOrDefault("tool.tone.filmic.A","6.2");
			pmap["B"] = myConfig::getConfig().getValueOrDefault("tool.tone.filmic.B","0.5");
			pmap["C"] = myConfig::getConfig().getValueOrDefault("tool.tone.filmic.C","1.7");
			pmap["D"] = myConfig::getConfig().getValueOrDefault("tool.tone.filmic.D","0.6");
			pmap["power"] = myConfig::getConfig().getValueOrDefault("tool.tone.filmic.power","1.0");
			pmap["norm"] = (myConfig::getConfig().getValueOrDefault("tool.tone.filmic.norm","0") == "1") ? "true" : "false";
			if (psize >= 5) {
				if (isFloat(p[1])) {
					pmap["A"] = p[1]; 
				}
				else {
					pmap["error"] = string_format("tone:ParseError - Expected float A, instead got: %s", p[1].c_str()); 
					return pmap;
				}
				if (isFloat(p[2])) {
					pmap["B"] = p[2];
				} 
				else {
					pmap["error"] = string_format("tone:ParseError - Expected float B, instead got: %s", p[2].c_str()); 
					return pmap;
				}
				if (isFloat(p[3])) {
					pmap["C"] = p[3]; 
				} 
				else {
					pmap["error"] = string_format("tone:ParseError - Expected float C, instead got: %s", p[3].c_str()); 
					return pmap;
				}
				if (isFloat(p[4])) {
					pmap["D"] = p[4];
				} 
				else {
					pmap["error"] = string_format("tone:ParseError - Expected float D, instead got: %s", p[4].c_str()); 
					return pmap;
				}
			}
			if (psize >= 6) {
				if (isFloat(p[5])) {
					pmap["power"] = p[5];
				}
				else if (p[5] == "norm") {
					pmap["norm"] = "true";
				}
				else {
					pmap["error"] = string_format("tone:ParseError - Unrecognized token: %s", p[5].c_str()); 
					return pmap;
				}
			}
			if (psize >= 7) {
				if (isFloat(p[5])) {
					pmap["power"] = p[5];
				}
				else {
					pmap["error"] = string_format("tone:ParseError - Expected float power, instead got: %s", p[1].c_str()); 
					return pmap;
				}
				if (p[6] == "norm") {
					pmap["norm"] = "true";
				}
				else {
					pmap["error"] = string_format("tone:ParseError - Unrecognized token: %s", p[6].c_str()); 
					return pmap;
				}
			}
		}
		else {
			pmap["error"] = string_format("tone:ParseError - Unrecognized mode: %s", p[0].c_str()); 
			return pmap;
		}


	}
	return pmap;
}


//whitebalance
//:camera - apply white balance from the camera metadata multipliers, "as-shot"
//:auto - apply auto (gray world) white balance
//:patch,<xint>,<yint>,<rfloat> - apply white balance calculated from the patch at x,y of radius r
//:<rfloat>,<gfloat>,<bfloat> - apply white balance with the specified RGB multipliers
std::map<std::string,std::string> parse_whitebalance(std::string paramstring)
{
	std::map<std::string,std::string> pmap;
	//collect all defaults into pmap:

	if (paramstring.size() != 0 && paramstring.at(0) == '{') {  //if string is a JSON map, parse it into pmap;
		pmap = parse_JSONparams(paramstring);
	}

	//if string has name=val;name=val.., pairs, just parse them into pmap:
	else if (paramstring.find("=") != std::string::npos) {  //name=val pairs
		pmap = parseparams(paramstring);  //from gimage/strutil.h
	}

	else { //positional
		std::vector<std::string> p = split(paramstring, ",");
		int psize = p.size();

		if (isFloat(p[0])) {
			pmap["mode"] = "multipliers";
			pmap["cmdlabel"] = "whitebalance:multipliers";
			if (psize < 3) {
				pmap["error"] = "whitebalance:ParseError - Insufficient number of multipliers"; 
				return pmap;
			}
			
			if (isFloat(p[0])) {
				pmap["redmultiplier"] = p[0];
			}
			else {
				pmap["error"] = string_format("whitebalance:ParseError - Red multiplier expected to be float: %s", p[0].c_str()); 
				return pmap;
			}
			if (isFloat(p[1])) {
				pmap["greenmultiplier"] = p[1];
			}
			else {
				pmap["error"] = string_format("whitebalance:ParseError - Green multiplier expected to be float: %s", p[1].c_str()); 
				return pmap;
			}
			if (isFloat(p[2])) {
				pmap["bluemultiplier"] = p[2];
			}
			else {
				pmap["error"] = string_format("whitebalance:ParseError - Blue multiplier expected to be float: %s", p[2].c_str()); 
				return pmap;
			}
		}
		else if (p[0] == "camera") {
			pmap["mode"] = "camera";
			pmap["cmdlabel"] = "whitebalance:camera";

		}
		else if (p[0] == "auto") {
			pmap["mode"] = "auto";
			pmap["cmdlabel"] = "whitebalance:auto";
			
		}
		else if (p[0] == "patch") {
			pmap["mode"] = "patch";
			pmap["cmdlabel"] = "whitebalance:patch";
			if (psize < 4) {
				pmap["error"] = "whitebalance:ParseError - Insufficient number of patch parameters"; 
				return pmap;
			}
			if (isUnsignedInt(p[1])) {
				pmap["patchx"] = p[1];
			}
			else {
				pmap["error"] = string_format("whitebalance:ParseError - Patch x expected to be unsigned integer: %s", p[1].c_str()); 
				return pmap;
			}
			if (isUnsignedInt(p[2])) {
				pmap["patchy"] = p[2];
			}
			else {
				pmap["error"] = string_format("whitebalance:ParseError - Patch y expected to be unsigned integer: %s", p[2].c_str()); 
				return pmap;
			}
			if (isFloat(p[3])) {
				pmap["patchradius"] = p[3];
			}
			else {
				pmap["error"] = string_format("whitebalance:ParseError - Patch radius expected to be float: %s", p[3].c_str()); 
				return pmap;
			}

			pmap["patchy"] = p[2];
			pmap["patchradius"] = p[3];
			
		}
		else if (p[0] == "bluethreshold") {
			pmap["mode"] = "bluethreshold";
			if (isFloat(p[1])) {
				pmap["redmultiplier"] = p[1];
			}
			else {
				pmap["error"] = string_format("whitebalance:ParseError - Red multiplier expected to be float: %s", p[0].c_str()); 
				return pmap;
			}
			if (isFloat(p[2])) {
				pmap["greenmultiplier"] = p[2];
			}
			else {
				pmap["error"] = string_format("whitebalance:ParseError - Green multiplier expected to be float: %s", p[1].c_str()); 
				return pmap;
			}
			if (isFloat(p[3])) {
				pmap["bluemultiplier"] = p[3];
			}
			else {
				pmap["error"] = string_format("whitebalance:ParseError - Blue multiplier expected to be float: %s", p[2].c_str()); 
				return pmap;
			}
			if (isFloat(p[4])) {
				pmap["bluethreshold"] = p[4];
			}
			else {
				pmap["error"] = string_format("whitebalance:ParseError - Blue threshold expected to be float: %s", p[2].c_str()); 
				return pmap;
			}
		}
		else {
			pmap["error"] = string_format("whitebalance:ParseError - Unrecognized token: %s", p[0].c_str()); 
			return pmap;
		}

	}
	return pmap;
}


