#ifndef _CameraData_h_
#define _CameraData_h_

#include <string>
#include <map>


class CameraData
{
	public:
		CameraData();
		CameraData(std::string filename);
		void parseDcraw(std::string filename);
		void parseCamconst(std::string filename);
		//std::string getBlack(std::string makemodel);
		//std::string getMaximum(std::string makemodel);
		//std::string getTrans(std::string makemodel);
		//struct cameradata getData(std::string makemodel);
		std::string getItem(std::string makemodel, std::string itemname);
		

	private:
		std::map<std::string, std::map<std::string, std::string>> camdat;

};

#endif
