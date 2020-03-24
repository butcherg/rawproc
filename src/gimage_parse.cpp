
#include "gimage/strutil.h"
#include "cJSON.h"


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

//std::string paramprint(std::map<std::string,std::string> params)
void paramprint(std::map<std::string,std::string> params)
{
	std::string s;
	printf("params ");
	printf("%d elements):\n",params.size()); 
	fflush(stdout);
	if (params.empty()) return;
	
	for (std::map<std::string,std::string>::iterator it=params.begin(); it!=params.end(); ++it) {
		//s.append(it->first + ": " + it->second) + "\r\n";
		printf("%s: %s\n",it->first.c_str(), it->second.c_str()); 
		fflush(stdout);
	}
	//return s;
}


//blackwhitepoint
//:rgb|red|green|blue - do Auto on channel
//:rgb|red|green|blue,<nbr>[,<nbr>] - specific b/w on channel, if only one number, b=<nbr>, w=255
//:rgb|red|green|blue,data - b/w on data limits, b=smallest r|g|b, w=largest r|g|b
//:rgb|red|green|blue,data,minwhite - b/w on data limits w=smallest r|g|b
//:camera - b/w on camera data limits, b=smallest r|g|b, w=largest r|g|b
//:<nbr>,[<nbr>] - specific b/w, if only one number, b=<nbr>, w=255
//:NULL - do Auto on rgb

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
		//if (paramstring == std::string()) {  //NULL string, default processing
		//	pmap["channel"] = "rgb";
		//	pmap["mode"] = "auto";
		//}
		//else 
		if (p[0] == "rgb" | p[0] == "red" | p[0] == "green" | p[0] == "blue") {    // | p[0] == "min") {   ??
			pmap["channel"] = p[0];
			if (p.size() >= 2) {
				if (p[1] == "data") { //bound on min/max of image data
					pmap["mode"] == "data";
					if (p.size() >= 3 && p[2] == "minwhite") pmap["minwhite"] = "true";
				}
				else if (p[1] == "auto") {
					pmap["channel"] = p[0];
					pmap["mode"] = "auto";
				}
				else { 
					pmap["black"] = p[1];
					if (p.size() >= 3) 
						pmap["white"] = p[2];
					else
						pmap["white"] = "255";
				}
			}
			else pmap["mode"] = "auto";
		}
		else if (p[0] == "camera") {
			pmap["channel"] = "rgb";
			pmap["mode"] = "camera";
		}
		else {
			pmap["channel"] = "rgb";
			pmap["mode"] = "auto";
		}
	}
	printf("in parse_blackwhitepoint: %d elements):\n",pmap.size()); 
	for (std::map<std::string,std::string>::iterator it=pmap.begin(); it!=pmap.end(); ++it) {
		printf("\t%s: %s\n",it->first.c_str(), it->second.c_str()); 
		fflush(stdout);
	}
	return pmap;
}



