
#include "fileutil.h"
#include <sys/stat.h>

#ifdef _WIN32
	#define WINVER 0x0600
#if __GNUC__ < 6 //mxe.cc (gcc 5.3) doesn't want this...
	#define _WIN32_WINNT 0x0600
#endif
	#include <windows.h>
	#include <shlobj.h>
	#include <direct.h>
	#include <io.h> 
	#include <initguid.h>
	#include <knownfolders.h>
	#define GetCurrentDir _getcwd
	#define access    _access_s
#else
	#include <limits.h>
	#include <unistd.h>
	#define GetCurrentDir getcwd
#endif

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
#ifdef _WIN32
	while ((pos = str.find_first_of('/')) != std::string::npos) str.replace(pos,1,"\\");
	if (str[str.size()-1] != '\\') str.push_back('\\');
#else
	while ((pos = str.find_first_of('\\')) != std::string::npos) str.replace(pos,1,"/");
	if (str[str.size()-1] != '/') str.push_back('/');
#endif
	return str;
}


//ToDo: add MacOS code for getAppConfigDir() and getExeDir()

std::string getAppConfigDir(std::string filename)
{
	std::string dir;

#ifdef _WIN32
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


std::string getExeDir(std::string filename)
{
	std::string dir;

#ifdef _WIN32
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
#ifdef _WIN32
	return std::string(cwdpath)+"\\rawproc.conf";
#else
	return std::string(cwdpath) + "/rawproc.conf";
#endif
}


bool file_exists(const std::string& filename) 
{
  struct stat buffer;   
  return (stat (filename.c_str(), &buffer) == 0); 
}


