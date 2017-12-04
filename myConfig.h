#ifndef MYCONFIG_H
#define MYCONFIG_H

#include <string>
#include <map>


class myConfig
{
public:
	myConfig(std::string conffile);
	myConfig() { }

	//use to set and use a global configuration:
	static void loadConfig(std::string conffile);
	static myConfig & getConfig();

	void flush();
	bool exists(std::string name);
	bool exists(std::string section, std::string name);
	std::string getValue(std::string name);
	std::string getValue(std::string section, std::string name);
	std::string getValueOrDefault(std::string name, std::string defaultval);
	std::string getValueOrDefault(std::string section, std::string name, std::string defaultval);
	void setValue(std::string section, std::string name, std::string value);
	void setValue(std::string name, std::string value);
	std::map<std::string, std::string> getDefault();
	std::map<std::string, std::string> getSection(std::string section);

private:
	std::string configfile;
	static std::map<std::string, std::string> defaultconfig;
	static std::map<std::string, std::map<std::string,std::string> > sectionconfig;


};


extern myConfig config;


#endif
