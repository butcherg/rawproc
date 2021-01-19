#include "myConfig.h"

#include <fstream>
#include <vector>
#include "gimage/strutil.h"

std::map<std::string, std::string> myConfig::defaultconfig;
std::map<std::string, std::map<std::string,std::string> > myConfig::sectionconfig;

myConfig config;

myConfig::myConfig(std::string conffile)
{
	temp = false;
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
			if (nameval.size() < 2) nameval.push_back("");
			if (nameval[0] != std::string()) {
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

std::string myConfig::replace_variables(std::string str)
{
	if (variables.size() == 0) return str;
	std::string strng = str;
	for (std::map<std::string, std::string>::iterator it=variables.begin(); it!=variables.end(); ++it) {
		std::string var = "$(";
		var.append(it->first);
		var.append(")");
		replace_all(strng, var, it->second);
	}
	return strng;
}

void myConfig::clearVariables()
{
	variables.clear();
}

void myConfig::setVariable(std::string name, std::string value)
{
	variables[name] = value;
}

std::string myConfig::getValue(std::string name)
{
	if (temp) {
		if (tempconfig.count(name) != 0)
			return replace_variables(tempconfig[name]);
	}
	if (exists(name))
		return replace_variables(defaultconfig[name]);
	else
		return "";
}

std::string myConfig::getValue(std::string section, std::string name)
{
	if (exists(section,name))
		return replace_variables(sectionconfig[section][name]);
	else
		return "";
}

std::string myConfig::getValueOrDefault(std::string name, std::string defaultval)
{
	std::string defval = defaultval;
	if (temp) {
		if (tempconfig.count(name) != 0)
			return replace_variables(tempconfig[name]);
	}
	if (exists(name)) {
		if (replace_variables(defaultconfig[name]).empty())
			return defval;
		else
			return replace_variables(defaultconfig[name]);
	}
	else {
		return defval;
	}
}

std::string myConfig::getValueOrDefault(std::string section, std::string name, std::string defaultval)
{
	std::string defval = defaultval;
	if (exists(section,name)) {
		if (replace_variables(sectionconfig[section][name]).empty()) 
			return defval;
		else
			return replace_variables(sectionconfig[section][name]);
	}
	else {
		return defval;
	}
}

std::map<std::string, std::string> myConfig::getSubset(std::string spec)
{
	std::map<std::string, std::string> s;
	std::map<std::string, std::string> c = myConfig::getConfig().getDefault();
	for (std::map<std::string, std::string>::iterator it=c.begin(); it!=c.end(); ++it) {
		std::string name = it->first.c_str();
		std::string val =  it->second.c_str();
		if (name.find(spec) != std::string::npos) {
			name.erase(name.find(spec), spec.size());
			s[name] = val;
		}
	}
	return s;
}

std::map<std::string, std::string> myConfig::getSubset(std::string section, std::string spec)
{
	std::map<std::string, std::string> s;
	std::map<std::string, std::string> c = myConfig::getConfig().getSection(section);
	for (std::map<std::string, std::string>::iterator it=c.begin(); it!=c.end(); ++it) {
		std::string name = it->first.c_str();
		std::string val =  it->second.c_str();
		if (name.find(spec) != std::string::npos) {
			name.erase(name.find(spec), spec.size());
			s[name] = val;
		}
	}
	return s;
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

//wildcard match, property names must be of the form 'part1.part2.part3...' :
std::string myConfig::match_name(std::string section, std::string name)
{	
	std::vector<std::string> match = split(name, ".");
	std::map<std::string, std::string> temps = getSection(section);
	for (std::map<std::string, std::string>::iterator it=temps.begin(); it!=temps.end(); ++it) {
		std::vector<std::string> prop = split(it->first, ".");
		if (prop.size() != match.size()) continue;
		bool found = true;
		for (unsigned i = 0; i< prop.size(); i++) {
			if (prop[i] == "*") continue;
			if (match[i] != prop[i]) {found = false; break; }
		}
		if (found) return it->second;
	}
	return std::string();
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



