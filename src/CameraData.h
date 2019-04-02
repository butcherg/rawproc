#ifndef _CameraData_h_
#define _CameraData_h_

#include <string>
#include <map>


struct cameradata {
	std::string black, maximum, trans;
};

class CameraData
{
	public:
		CameraData(std::string filename);
		std::string getBlack(std::string makemodel);
		std::string getMaximum(std::string makemodel);
		std::string getTrans(std::string makemodel);
		struct cameradata getData(std::string makemodel);
		

	private:
		std::map<std::string, struct cameradata> camdat;

};

#endif
