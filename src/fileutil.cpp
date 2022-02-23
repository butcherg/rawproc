
#include "fileutil.h"
#include <sys/stat.h>
#include <stdio.h>
#include <fstream>

#ifdef _WIN32
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



std::string getRawprocConfPath(std::string conf_cmdline)
{
	//config file search order: 
	//	1) path specified in conf_cmdline, usually from a command line parameter; 
	//	2) current working directory (conf_cwd); 
	//	3) executable directory (conf_exe); 
	//	4) OS-defined user data directory (conf_configd) 
	
	std::string conf_cwd = getCwdConfigFilePath();
	std::string conf_exe = getExeDir("rawproc.conf");
	std::string conf_configd = getAppConfigFilePath();
	
	if (access( conf_cmdline.c_str(), 0 ) == 0) 
		return conf_cmdline;
	else if (access( conf_cwd.c_str(), 0 ) == 0) 
		return conf_cwd;
	else if (access( conf_exe.c_str(), 0 ) == 0) 
		return conf_exe;
	else if (access( conf_configd.c_str(), 0 ) == 0) 
		return conf_configd;
	else return "(none)";
}


bool file_exists(const std::string& filename) 
{
  struct stat buffer;   
  return (stat (filename.c_str(), &buffer) == 0); 
}

void file_copy(std::string frompath, std::string topath)
{
	std::ifstream  src(frompath, std::ios::binary);
    std::ofstream  dst(topath,   std::ios::binary);
    dst << src.rdbuf();
}

bool file_delete(std::string filepath)
{
	int i =  remove(filepath.c_str());
	if (i == 0) return false; else return true;
}

std::map<std::string, std::string> file_parts(std::string filepath)
{
	std::map<std::string, std::string> fileparts;
	char dirsep;
#ifdef _WIN32
	dirsep = '\\';
#else
	dirsep = '/';
#endif
	size_t dirpos = filepath.find_last_of(dirsep);
	if (dirpos != std::string::npos) fileparts["dir"] = filepath.substr(0,dirpos+1);
	size_t extpos = filepath.find_last_of(".");
	if (extpos != std::string::npos) fileparts["ext"] = filepath.substr(extpos);
	if (dirpos != std::string::npos) {
		if (extpos != std::string::npos) {
			fileparts["filename"] = filepath.substr(dirpos+1,(extpos) - (dirpos+1));
		}
		else {
			fileparts["filename"] = filepath.substr(dirpos+1);
		}
	}
	else {
		fileparts["filename"] = filepath;
	}
	
	return fileparts;
}
