#include "CameraData.h"
#include "gimage/strutil.h"
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
				camdat[makemodel]["black"] = t[1];
				camdat[makemodel]["maximum"] = t[2];
				fgets(buf, 255, f); 
				camdat[makemodel]["dcraw_matrix"] = split(split(std::string(buf), "{ ")[1], " }")[0];
			}
			fgets(buf, 255, f); 
		}
		break;
	}
}


void CameraData::parseCamconst(std::string filename)
{

}

/*
std::string CameraData::getBlack(std::string makemodel)
{
	if (camdat.find(makemodel) != camdat.end())
		//return camdat[makemodel].black;
		return camdat[makemodel]["black"];
	return "";
}

std::string CameraData::getMaximum(std::string makemodel)
{
	if (camdat.find(makemodel) != camdat.end())
		//return camdat[makemodel].maximum;
		return camdat[makemodel]["maximum"];
	return "";
}

std::string CameraData::getTrans(std::string makemodel)
{
	if (camdat.find(makemodel) != camdat.end())
		//return camdat[makemodel].trans;
		return camdat[makemodel]["trans"];
	return "";
}

struct cameradata CameraData::getData(std::string makemodel)
{
	struct cameradata cd;
	if (camdat.find(makemodel) != camdat.end())
		cd = camdat[makemodel];
	return cd;
}

std::string CameraData::getItem(std::string makemodel, std::string itemname)
{
	if (itemname == "black") return camdat[makemodel].black;
	if (itemname == "maximum") return camdat[makemodel].maximum;
	if (itemname == "dcraw_matrix") return camdat[makemodel].trans;
	return "";
}
*/

std::string CameraData::getItem(std::string makemodel, std::string itemname)
{
	return camdat[makemodel][itemname];
}

