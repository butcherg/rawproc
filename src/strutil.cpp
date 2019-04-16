

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sstream> 

#include <locale>
#include <codecvt>

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> 
#include <sys/stat.h>

#ifdef WIN32
	#include <direct.h>
	#include <io.h> 
	#include <initguid.h>
	#include <KnownFolders.h>
	#include <shlobj.h>
	#define GetCurrentDir _getcwd

	#define access    _access_s
#else
	#include <limits.h>
	#include <unistd.h>
	#define GetCurrentDir getcwd
#endif

std::string to_utf8(std::wstring wstr)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(wstr);
}


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

int countchar(std::string s, char c)
{
	int count = 0;
	for (int i=0; i<s.size(); i++) {
		if (s[i] == c) count++;
	}
	return count;
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


//ToDo: add MacOS code for getAppConfigDir() and getExeDir()

std::string getAppConfigDir(std::string filename="")
{
	std::string dir;

#ifdef WIN32
	wchar_t *roamingAppData = NULL;
	SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &roamingAppData);
	char strng[128];
	wcstombs(strng, roamingAppData, 128);
	dir = std::string(strng) + "\\rawproc\\";
#else
	char *d = getenv("HOME");
	if (d) dir = std::string(d) + "/.rawproc/";
#endif
	if (filename != "") dir.append(filename);
	return dir;
}


std::string getExeDir(std::string filename="")
{
	std::string dir;

#ifdef WIN32
	TCHAR exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH) ;
	dir = std::string(exePath);
	dir.erase(dir.find_last_of('\\'));
	if (filename != "") dir.append("\\"+filename);
#else
	char exePath[PATH_MAX];
	size_t len = readlink("/proc/self/exe", exePath, sizeof(exePath));
	if (len == -1 || len == sizeof(exePath))
	        len = 0;
	exePath[len] = '\0';
	dir = std::string(exePath);
	dir.erase(dir.find_last_of('/'));
	if (filename != "") dir.append("/"+filename);
#endif

	return dir;
}



std::string getAppConfigFilePath()
{
	std::string path = getAppConfigDir();
	path = path + "rawproc.conf";
	return path;
}

std::string getCwd() 
{
	char buff[4096];

	char * result = getcwd( buff, 4096 );
	std::string cwd( buff );

	return cwd;
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


bool contains(const char * buf, const char * str)
{
	if (std::string(buf).find(str) != std::string::npos) return true;
	return false;
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

std::vector<std::string> bifurcate(std::string strg, char c = ' ', bool fromback=false)
{
	std::vector<std::string> result;
	if (countchar(strg, c) == 0) {
		result.push_back(strg);
	}
	else {
		std::size_t eq;
		if (fromback)
			eq = strg.find_last_of(c);
		else
			eq = strg.find_first_of(c);
		result.push_back(strg.substr(0,eq));
		result.push_back(strg.substr(eq+1));
	}
	return result;
}

//string of the form "name=val;name=val;..."
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

void parseparams(std::map<std::string, std::string> &p, std::string params)
{
	std::vector<std::string> l = split(params,";");
	for (std::vector<std::string>::iterator it=l.begin(); it!=l.end(); ++it) {
		std::string name, val;
		std::vector<std::string> nameval = split(*it,"=");
		if (nameval.size() == 2)
			p[nameval[0]] = nameval[1];
		else
			p[nameval[0]] = "1";
	}
}

std::string paramstring(std::map<std::string, std::string> &p)
{
	std::string s;
	bool first = true;
	for (std::map<std::string, std::string>::iterator it=p.begin(); it!=p.end(); ++it) {
		if (!first) s.append(";");
		s.append(it->first+"="+it->second);
		first = false;
	}
	return s;
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



