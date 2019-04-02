#include "CameraData.h"
#include "gimage/strutil.h"
#include <vector>
#include <stdio.h>


CameraData::CameraData(std::string filename)
{
	char buf[256];
	FILE *f = fopen(filename.c_str(), "r");
	while (!feof(f)) {
		while(!contains(buf,"void") | !contains(buf,"CLASS") | !contains(buf,"adobe_coeff")) fgets(buf, 255, f);
		while(!contains(buf,"table[]")) fgets(buf, 255, f);
		fgets(buf, 255, f); 
		while(!contains(buf,"};")) {
			if (contains(buf, "{")) {
				struct cameradata c;
				std::vector<std::string> t = split(std::string(buf), ",");
				std::string makemodel = split(t[0], "\"")[1];
				c.black = t[1];
				c.maximum = t[2];
				fgets(buf, 255, f); 
				c.trans = split(split(std::string(buf), "{ ")[1], " }")[0];
				camdat[makemodel] = c;
			}
			fgets(buf, 255, f); 
		}
		break;
	}
}

std::string CameraData::getBlack(std::string makemodel)
{
	if (camdat.find(makemodel) != camdat.end())
		return camdat[makemodel].black;
	return "";
}

std::string CameraData::getMaximum(std::string makemodel)
{
	if (camdat.find(makemodel) != camdat.end())
		return camdat[makemodel].maximum;
	return "";
}

std::string CameraData::getTrans(std::string makemodel)
{
	if (camdat.find(makemodel) != camdat.end())
		return camdat[makemodel].trans;
	return "";
}

struct cameradata CameraData::getData(std::string makemodel)
{
	struct cameradata cd;
	if (camdat.find(makemodel) != camdat.end())
		cd = camdat[makemodel];
	return cd;
}


