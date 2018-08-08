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

	bool flush();
	bool exists(std::string name);
	bool exists(std::string section, std::string name);
	std::string getValue(std::string name);
	std::string getValue(std::string section, std::string name);
	std::string getValueOrDefault(std::string name, std::string defaultval);
	std::string getValueOrDefault(std::string section, std::string name, std::string defaultval);
	void setValue(std::string section, std::string name, std::string value);
	void setValue(std::string name, std::string value);
	void deleteValue(std::string section, std::string name);
	void deleteValue(std::string name);
	std::map<std::string, std::string> getDefault();
	std::map<std::string, std::string> getSection(std::string section);
	
	//builds a list of variables to replace in property values where $(name) is found:
	void set_variable(std::string name, std::string value);

	void enableTempConfig(bool e);
	bool getTempConfig();

private:
	std::string configfile;
	static std::map<std::string, std::string> defaultconfig;
	static std::map<std::string, std::map<std::string,std::string> > sectionconfig;
	
	std::string replace_variables(std::string str);
	std::map<std::string, std::string> variables;

	bool temp;
	std::map<std::string, std::string> tempconfig;


};


extern myConfig config;


#endif
