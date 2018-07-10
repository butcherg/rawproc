#include "myConfig.h"

#include <fstream>
#include <vector>
#include "gimage/strutil.h"

std::map<std::string, std::string> myConfig::defaultconfig;
std::map<std::string, std::map<std::string,std::string> > myConfig::sectionconfig;

myConfig config;

myConfig::myConfig(std::string conffile)
{
	std::vector<std::string> parameter;
	std::vector<std::string> nameval;
	std::string parm;
	configfile = conffile;
	std::ifstream file(configfile.c_str());
	std::string str; 
	std::string section = "default";

	while (std::getline(file, str))
	{
		if (str == "") continue;
		unsigned last = str.size()-1;
		unsigned first = 0;
		if ((str[first] == '[') & (str[last] == ']')) {
			section = str.erase(last,1).erase(first,1);
			continue;
		}

		if (str.find_first_of("#") != std::string::npos) {
			parameter = bifurcate(str,'#');
			parm = parameter[0];
		}
		else
			parm = str;


		if (str.find_first_of("=") != std::string::npos) {
			nameval = bifurcate(parm, '=');
			if (nameval[0] != "") {
				if (section == "default")
					defaultconfig[nameval[0]] = nameval[1];
				else
					sectionconfig[section][nameval[0]] = nameval[1];
			}
		}
	}
	file.close();
}


void myConfig::loadConfig(std::string conffile)
{
	config = myConfig(conffile);
}

myConfig & myConfig::getConfig()
{
	return config;
}

bool myConfig::flush()
{
	std::ofstream file(configfile.c_str());
	if (!file.good()) return false;
	for (std::map<std::string, std::string>::iterator it=defaultconfig.begin(); it!=defaultconfig.end(); ++it)
		file << it->first << "=" << it->second << '\n';

	file << '\n';

	for (std::map<std::string, std::map<std::string, std::string> >::iterator sit=sectionconfig.begin(); sit!=sectionconfig.end(); ++sit) {
		file << '[' << sit->first << "]\n";
		for (std::map<std::string, std::string>::iterator it=sit->second.begin(); it!=sit->second.end(); ++it)
			file << it->first << "=" << it->second << '\n';
		file << '\n';
	}

	file.close();
	return true;
}


std::string myConfig::getValue(std::string name)
{
	if (temp) {
		if (tempconfig.count(name) != 0)
			return tempconfig[name];
	}
	if (exists(name))
		return defaultconfig[name];
	else
		return "";
}

std::string myConfig::getValue(std::string section, std::string name)
{
	if (exists(section,name))
		return sectionconfig[section][name];
	else
		return "";
}

std::string myConfig::getValueOrDefault(std::string name, std::string defaultval)
{
	std::string defval = defaultval;
	if (temp) {
		if (tempconfig.count(name) != 0)
			return tempconfig[name];
	}
	if (exists(name))
		return defaultconfig[name];
	else
		return defval;
}

std::string myConfig::getValueOrDefault(std::string section, std::string name, std::string defaultval)
{
	std::string defval = defaultval;
	if (exists(section,name))
		return sectionconfig[section][name];
	else
		return defval;
}

void myConfig::deleteValue(std::string section, std::string name)
{
	if (exists(section,name))
		sectionconfig[section].erase(name);
}

void myConfig::deleteValue(std::string name)
{
	if (temp) {
		if (tempconfig.count(name) != 0)
			tempconfig.erase(name);
		return;
	}
	if (exists(name))
		defaultconfig.erase(name);
}

std::map<std::string, std::string> myConfig::getSection(std::string section)
{
	return sectionconfig[section];
}

std::map<std::string, std::string> myConfig::getDefault()
{
	return defaultconfig;
}

void myConfig::setValue(std::string section, std::string name, std::string value)
{
	sectionconfig[section][name] = value;
}

void myConfig::setValue(std::string name, std::string value)
{
	if (temp) {
		tempconfig[name] = value;
		return;
	}
	defaultconfig[name] = value;
}

bool myConfig::exists(std::string name)
{
	std::string n = name;
	if (temp)
		if (tempconfig.count(n) != 0) return true;
	if (defaultconfig.empty()) return false; 
	if (defaultconfig.count(n) == 0) return false;
	return true;
}

bool myConfig::exists(std::string section, std::string name)
{
	if (sectionconfig.find(section) != sectionconfig.end() ) 
		if (sectionconfig[section].find(name) != sectionconfig[section].end())
			return true;
	return false;
}


void myConfig::enableTempConfig(bool e)
{
	if (e) {
		temp = true;
	}
	else {
		temp = false;
		tempconfig.clear();
	}
}

bool myConfig::getTempConfig()
{
	return temp;
}



