#include "CameraData.h"
#include "gimage/strutil.h"
#include "cJSON.h"
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#ifdef USECONFIG
#include "myConfig.h"
#include <unistd.h>
#endif

CameraData::CameraData()
{

}

CameraData::CameraData(std::string filename)
{
	parseDcraw(filename);
}

void CameraData::parseDcraw(std::string filename)
{
	int i = 0, d = 0;
	char buf[256];
	char * result;
	FILE *f = fopen(filename.c_str(), "r");
	while (!feof(f)) {
		while(!contains(buf,"void") | !contains(buf,"CLASS") | !contains(buf,"adobe_coeff")) result = fgets(buf, 255, f);
		while(!contains(buf,"table[]")) result = fgets(buf, 255, f);
		result = fgets(buf, 255, f); 
		while(!contains(buf,"};")) {
			if (contains(buf, "{")) {
				std::vector<std::string> t = split(std::string(buf), ",");
				std::string makemodel = split(t[0], "\"")[1];
				if (camdat.find(makemodel) != camdat.end()) d++;
				camdat[makemodel]["black"] = t[1];
				camdat[makemodel]["maximum"] = t[2];
				result = fgets(buf, 255, f); 
				camdat[makemodel]["dcraw_matrix"] = split(split(std::string(buf), "{ ")[1], " }")[0];
				i++;
			}
			result = fgets(buf, 255, f); 
		}
		break;
	}

	fileorder.push_back("dcraw.c");
	camdat_status["dcraw.c"].append(string_format("<li>load: %d additions (%d replacements), %ld entries total</li>\n",i,d,camdat.size())); 

}

void CameraData::parseCamconst(std::string filename)
{
	int i = 0, d = 0;;
	FILE *f = NULL;
	long len = 0;
	char *data = NULL;
	size_t result;

	f = fopen(filename.c_str(), "rb");
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	data = (char *) malloc(len+1);
	result = fread(data, 1, len, f);
	data[len] = '\0';
	fclose(f);

	cJSON_Minify(data);
	cJSON * json = cJSON_Parse(data);
        free(data);
	if (!json) return;

	std::string makemodel, name, value;

	cJSON * camlist = cJSON_GetObjectItemCaseSensitive(json,"camera_constants");
	if (camlist) {
		cJSON *camera, *item;
		cJSON_ArrayForEach(camera,camlist) {
			cJSON * makemodellist = cJSON_GetObjectItemCaseSensitive(camera,"make_model");
			if (cJSON_IsArray(makemodellist))
				makemodel = std::string(cJSON_GetArrayItem(makemodellist, 0)->valuestring);
			else 
				makemodel = std::string(makemodellist->valuestring);
			if (camdat.find(makemodel) != camdat.end()) d++;
			cJSON_ArrayForEach(item,camera) {
				if (std::string(item->string) == "make_model") continue;
				if (std::string(item->string) != "dcraw_matrix") continue;  //for now, just collect dcraw_matrices...
				name = std::string(item->string);
				if (cJSON_IsArray(item) | cJSON_IsObject(item)) {
					char * valstr = cJSON_PrintUnformatted(item);
					value = std::string(valstr);
					free(valstr);
				}
				if (cJSON_IsString(item)) value = std::string(item->valuestring);

				camdat[makemodel][name] =  value;
				i++;

			}
		}
	}

	cJSON_Delete(json);

	fileorder.push_back("camconst.json");
	camdat_status["camconst.json"].append(string_format("<li>load: %d additions (%d replacements), %ld entries total</li>\n",i,d,camdat.size())); 
}

std::string CameraData::getItem(std::string makemodel, std::string itemname)
{
	if (camdat.find(makemodel) != camdat.end())
		if (camdat[makemodel].find(itemname) != camdat[makemodel].end())
			return camdat[makemodel][itemname];
	return std::string();
}

std::string CameraData::getStatus()
{
	std::string msg;
	//for (std::map<std::string, std::string>::iterator it=camdat_status.begin(); it!=camdat_status.end(); ++it) {
	for (std::vector<std::string>::iterator it=fileorder.begin(); it!=fileorder.end(); ++it) {
		//msg.append("<p><b>" + it->first + ":</b><ul>" + it->second + "</ul></p>");
		msg.append("<p><b>" + *it + ":</b><ul>" + camdat_status[*it] + "</ul></p>");
	}
		
	return msg;
}

#ifdef USECONFIG
//searches for filename in 
//   1) property-specified full pathname, if the property exists, 
//   2) the directory containing the executable, 
//   3) the OS-specific application configuration
//If not found, returns an empty string
std::string CameraData::findFile(std::string filename, std::string propertypath)
{
	std::string foundfile;

	if (!propertypath.empty()) {
		if (myConfig::getConfig().exists(propertypath)) {
			foundfile = myConfig::getConfig().getValue(propertypath);
			if (!file_exists(foundfile.c_str())) foundfile = "";
		}
	}

	if (foundfile.empty()) {
		foundfile = getExeDir(filename);
		if (!file_exists(foundfile.c_str())) foundfile.clear();
	}

	if (foundfile.empty()) {
		foundfile = getAppConfigDir(filename);
		if (!file_exists(foundfile.c_str())) foundfile.clear();
	}

	if (foundfile.empty())
		camdat_status[filename].append("<li>path: file not found.</li>\n");

	else
		camdat_status[filename].append("<li>path: " + foundfile + "</li>\n");

	return foundfile;

}
#endif

