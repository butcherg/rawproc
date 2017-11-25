#include "myConfig.h"

#include <fstream>
#include <vector>

std::map<std::string, std::string> myConfig::defaultconfig;
std::map<std::string, std::map<std::string,std::string> > myConfig::sectionconfig;

myConfig config;

std::vector<std::string> split(std::string strg, char c = ' ')
{
	const char * str = strg.c_str();
	std::vector<std::string> result;
	do
	{
		const char *begin = str;
		while(*str != c && *str)
			str++;
		result.push_back(std::string(begin, str));
	} while (0 != *str++);

	return result;
}

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
			parameter = split(str,'#');
			parm = parameter[0];
		}
		else
			parm = str;


		if (str.find_first_of("=") != std::string::npos) {
			nameval = split(parm, '=');
			if (section == "default")
				defaultconfig[nameval[0]] = nameval[1];
			else
				sectionconfig[section][nameval[0]] = nameval[1];
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

void myConfig::flush()
{
	std::ofstream file(configfile.c_str());
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
}


std::string myConfig::getValue(std::string name)
{
	return defaultconfig[name];
}

std::string myConfig::getValue(std::string section, std::string name)
{
	return sectionconfig[section][name];
}

std::map<std::string, std::string> myConfig::getSection(std::string section)
{
	return sectionconfig[section];
}

void myConfig::setValue(std::string section, std::string name, std::string value)
{
	sectionconfig[section][name] = value;
}

void myConfig::setValue(std::string name, std::string value)
{
	defaultconfig[name] = value;
}

bool myConfig::exists(std::string name)
{
	if (defaultconfig.find(name) == defaultconfig.end()) return false;
	return true;
}

bool myConfig::exists(std::string section, std::string name)
{
	if (sectionconfig.find(section) != sectionconfig.end() ) 
		if (sectionconfig[section].find(name) != sectionconfig[section].end())
			return true;
	return false;
}




