#ifndef _CameraData_h_
#define _CameraData_h_

#include <string>
#include <map>

#define USECONFIG

class CameraData
{
	public:
		CameraData();
		CameraData(std::string filename);

		void parseDcraw(std::string filename);
		void parseCamconst(std::string filename);

		std::string getItem(std::string makemodel, std::string itemname);

#ifdef USECONFIG
		static std::string findFile(std::string filename, std::string propertypath);
#endif
		

	private:
		std::map<std::string, std::map<std::string, std::string>> camdat;

};

#endif
