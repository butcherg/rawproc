

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream> 

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <sys/stat.h>

#ifdef WIN32
	#include <direct.h>
	#include <io.h> 
	#define GetCurrentDir _getcwd

	#define access    _access_s
#else
	#include <unistd.h>
	#define GetCurrentDir getcwd
#endif


std::string tostr(double t)
{ 
   std::ostringstream os; 
   os<<t; 
   return os.str(); 
} 

std::string tostr(unsigned short t)
{ 
   std::ostringstream os; 
   os<<t; 
   return os.str(); 
} 

std::string underscore(std::string str)
{
	std::string s = str;
	for (int i=0; i<s.size(); i++)
		if (s[i] == ' ') s[i] = '_';
	return s;
}

std::string de_underscore(std::string str)
{
	std::string s = str;
	for (int i=0; i<s.size(); i++)
		if (s[i] == '_') s[i] = ' ';
	return s;
}

std::string filepath_normalize(std::string str)
{
	int pos;
#ifdef WIN32
	while ((pos = str.find_first_of('/')) != std::string::npos) str.replace(pos,1,"\\");
	if (str[str.size()-1] != '\\') str.push_back('\\');
#else
	while ((pos = str.find_first_of('\\')) != std::string::npos) str.replace(pos,1,"/");
	if (str[str.size()-1] != '/') str.push_back('/');
#endif
	return str;
}

	
std::string getAppConfigFilePath()
{
	std::string dir;
	char *d;
#ifdef WIN32
	d = getenv("APPDATA");
	if (d) dir = std::string(d) + "\\rawproc\\rawproc.conf";
#else
	d = getenv("HOME");
	if (d) dir = std::string(d) + "/.rawproc/rawproc.conf";
#endif

	if (access( dir.c_str(), 0 ) == 0)
		return dir;
	else
		return "";
}

std::string getCwdConfigFilePath()
{
	char cwdpath[FILENAME_MAX];

	if (!GetCurrentDir(cwdpath, sizeof(cwdpath))) {
		return "";
	}
#ifdef WIN32
	return std::string(cwdpath)+"\\rawproc.conf";
#else
	return std::string(cwdpath) + "/rawproc.conf";
#endif
}



std::vector<std::string> split(std::string s, std::string delim)
{
	std::vector<std::string> v;
	size_t pos=0;
	size_t start;
	while (pos < s.length()) {
		start = pos;
		pos = s.find(delim,pos);
		if (pos == s.npos) {
			v.push_back(s.substr(start,s.length()-start));
			return v;
		}
		v.push_back(s.substr(start, pos-start));
		pos += delim.length();
	}
	return v;
}

std::vector<std::string> bifurcate(std::string strg, char c = ' ')
{
	std::vector<std::string> result;
	std::size_t eq = strg.find_first_of(c);
	result.push_back(strg.substr(0,eq));
	result.push_back(strg.substr(eq+1));
	return result;
}


std::map<std::string, std::string> parseparams(std::string params)
{
	std::map<std::string, std::string> p;
	std::vector<std::string> l = split(params,";");
	for (std::vector<std::string>::iterator it=l.begin(); it!=l.end(); ++it) {
		std::string name, val;
		std::vector<std::string> nameval = split(*it,"=");
		if (nameval.size() == 2)
			p[nameval[0]] = nameval[1];
		else
			p[nameval[0]] = "1";
	}
	return p;
}

//https://stackoverflow.com/questions/5343190/how-do-i-replace-all-instances-of-a-string-with-another-string/
void replace_all(std::string& str, const std::string& from, const std::string& to) 
{
    if(from.empty())
        return;
    std::string wsRet;
    wsRet.reserve(str.length());
    size_t start_pos = 0, pos;
    while((pos = str.find(from, start_pos)) != std::string::npos) {
        wsRet += str.substr(start_pos, pos - start_pos);
        wsRet += to;
        pos += from.length();
        start_pos = pos;
    }
    wsRet += str.substr(start_pos);
    str.swap(wsRet); // faster than str = wsRet;
}


std::string string_format(const std::string fmt, ...) 
{
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) {     // Maximum two passes on a POSIX system...
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) {  // Everything worked
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
            size = n + 1;   // For null char
        else
            size *= 2;      // Guess at a larger size (OS specific)
    }
    return str;
}

std::string nexttoken(std::string &strng, std::string delims)
{
	int pos = strng.find_first_of(delims);
	std::string token = strng.substr(0,pos);
	strng.erase(0,pos);
	while (strng.find_first_of(delims) == 0) strng.erase(0,1);
	return token;
}

bool file_exists(const std::string& filename) 
{
  struct stat buffer;   
  return (stat (filename.c_str(), &buffer) == 0); 
}




