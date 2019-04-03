#include "CameraData.h"
#include "gimage/strutil.h"
#include "cJSON.h"
#include <vector>
#include <stdio.h>


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
	FILE *f = fopen(filename.c_str(), "r");
	while (!feof(f)) {
		while(!contains(buf,"void") | !contains(buf,"CLASS") | !contains(buf,"adobe_coeff")) fgets(buf, 255, f);
		while(!contains(buf,"table[]")) fgets(buf, 255, f);
		fgets(buf, 255, f); 
		while(!contains(buf,"};")) {
			if (contains(buf, "{")) {
				std::vector<std::string> t = split(std::string(buf), ",");
				std::string makemodel = split(t[0], "\"")[1];
				if (camdat.find(makemodel) != camdat.end()) d++;
				camdat[makemodel]["black"] = t[1];
				camdat[makemodel]["maximum"] = t[2];
				fgets(buf, 255, f); 
				camdat[makemodel]["dcraw_matrix"] = split(split(std::string(buf), "{ ")[1], " }")[0];
				i++;
			}
			fgets(buf, 255, f); 
		}
		break;
	}
	printf("CameraData: %d dcraw.c additions (%d duplicates), %d entries total...\n",i,d,camdat.size()); fflush(stdout);
}

void CameraData::parseCamconst(std::string filename)
{
	int i = 0, d = 0;;
	FILE *f = NULL;
	long len = 0;
	char *data = NULL;

	f = fopen(filename.c_str(), "rb");
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);

	data = (char *) malloc(len+1);

	fread(data, 1, len, f);
	data[len] = '\0';
	fclose(f);

	cJSON_Minify(data);
	cJSON * json = cJSON_Parse(data);

	if (!json) return;
        free(data);

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


	if (json) cJSON_Delete(json);

	printf("CameraData: %d camconst.json additions (%d duplicates), %d entries total...\n",i,d,camdat.size()); fflush(stdout);
}

std::string CameraData::getItem(std::string makemodel, std::string itemname)
{
	return camdat[makemodel][itemname];
}

