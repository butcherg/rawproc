//---------------------------------------------------------------------------
//
// Name:        rawprocApp.cpp
// Author:      Glenn
// Created:     11/18/2015 7:04:06 PM
// Description: 
//
//---------------------------------------------------------------------------

#include "rawprocApp.h"
#include "rawprocFrm.h"

#include <wx/filefn.h>
#include <wx/stdpaths.h>
#include <wx/cmdline.h>

#include "wx/filesys.h"
#include "wx/fs_zip.h"

#include "util.h"
#include "gimage/gimage.h"
#include "gimage/strutil.h"
#include "myConfig.h"

IMPLEMENT_APP(rawprocFrmApp)

static const wxCmdLineEntryDesc cmdLineDesc[] =
{
	{ wxCMD_LINE_SWITCH, "h", "help", "show this help message", wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },
	{ wxCMD_LINE_OPTION, "s", "opensource", "looks for and executes a rawproc command line in the metadata" },
	{ wxCMD_LINE_OPTION, "c", "conffile", "use the specified configuration file" },
	{ wxCMD_LINE_PARAM,  "",  "", "image file to open", wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},
	{ wxCMD_LINE_NONE }
};

bool rawprocFrmApp::OnInit()
{
	wxString fname;

	wxInitAllImageHandlers();
	wxFileSystem::AddHandler(new wxZipFSHandler);
	
	wxCmdLineParser cmdline(cmdLineDesc, wxGetApp().argc, wxGetApp().argv);
	if (cmdline.Parse() == -1) exit(0);

	rawprocFrm* frame = new rawprocFrm(NULL);
	SetTopWindow(frame);
	frame->Show();

	wxString configfile;
	wxString conf_exe = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath()+wxFileName::GetPathSeparator()+"rawproc.conf";
	//wxString conf_configd = wxStandardPaths::Get().GetUserDataDir()+wxFileName::GetPathSeparator()+"rawproc.conf";

	wxString conf_cwd = wxString(getCwdConfigFilePath().c_str());
	wxString conf_configd = wxString(getAppConfigFilePath().c_str());

	//config file search order: 
	//	1) path specified in the -c parameter; 
	//	2) current working directory; 
	//	3) executable directory; 
	//	4) OS-defined user data directory 
	if (cmdline.Found("c", &configfile)) {
		if (wxFileName::FileExists(configfile)) {
			wxFileName cfile(configfile);
			cfile.Normalize();
			frame->SetConfigFile(cfile.GetFullPath());
			myConfig::loadConfig(std::string(cfile.GetFullPath().c_str()));
		}
	}
	else if (wxFileName::FileExists(conf_exe)) {
		frame->SetConfigFile(conf_exe);
		myConfig::loadConfig(std::string(conf_exe.c_str()));
	}
	else if (wxFileName::FileExists(conf_cwd)) {
		frame->SetConfigFile(conf_cwd);
		myConfig::loadConfig(std::string(conf_cwd.c_str()));
	}
	else if (wxFileName::FileExists(conf_configd)) {
		frame->SetConfigFile(conf_configd);
		myConfig::loadConfig(std::string(conf_configd.c_str()));
	}

	//propvar EXEDIR: Use to substitute the path to the running executable in path properties
	myConfig::getConfig().setVariable("EXEDIR", std::string(wxFileName(conf_exe).GetPath().c_str()));
	
	wxFileName profpath;
	profpath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","")));
	gImage::setProfilePath(std::string(profpath.GetPathWithSep().c_str()));
	
	//parm app.start.logmessage: Message to print in the log when rawproc starts. 
	wxString startmessage = wxString(myConfig::getConfig().getValueOrDefault("app.start.logmessage",""));
	if (startmessage != "") log(startmessage);
	
	frame->SetBackground();

	int thumbmode = atoi(myConfig::getConfig().getValueOrDefault("display.thumb.initialmode","1").c_str());  //1=thumb, 2=histogram, 3=none
	frame->SetThumbMode(thumbmode);

	if (cmdline.Found("s", &fname)) {
		wxFileName f(fname);
		f.MakeAbsolute();
		wxSetWorkingDirectory (f.GetPath());
		frame->SetStartPath(f.GetPath());
		frame->OpenFileSource(f.GetFullPath());
	}
	else if (cmdline.GetParamCount() > 0) {
		wxFileName f(cmdline.GetParam());
		f.MakeAbsolute();
		wxSetWorkingDirectory (f.GetPath());
		frame->SetStartPath(f.GetPath());
		//if (ImageContainsRawprocCommand(wxGetApp().argv[1])) {
		if (ImageContainsRawprocCommand(f.GetFullPath())) {
			if (wxMessageBox("Image contains rawproc script.  Open the script?", "Contains Script", wxYES_NO | wxCANCEL | wxNO_DEFAULT) == wxYES)
				frame->OpenFileSource(f.GetFullPath());
			else	
				frame->OpenFile(f.GetFullPath());
		}
		else frame->OpenFile(f.GetFullPath());
	}
	else {
		//parm app.start.path: Specify the directory at which to start opening files.  Default="", rawproc uses the OS Pictures directory for the current user.
		wxString startpath = wxString(myConfig::getConfig().getValueOrDefault("app.start.path",""));
		if (startpath != "") {
			if (wxFileName::DirExists(startpath))
				wxSetWorkingDirectory(startpath);
				frame->SetStartPath(startpath);
		}
		else {
			wxFileName picdir = wxFileName::DirName(wxStandardPaths::Get().GetDocumentsDir());
			picdir.RemoveLastDir();
			picdir.AppendDir("Pictures");
			if (picdir.DirExists()) {
				wxSetWorkingDirectory(picdir.GetPath());
				frame->SetStartPath(picdir.GetPath());
			}
			else {
				wxSetWorkingDirectory(wxFileName::GetHomeDir());
				frame->SetStartPath(wxFileName::GetHomeDir());
			}
		}
	}
	return true;
}
 
int rawprocFrmApp::OnExit()
{
	return 0;
}

int rawprocFrmApp::OnRun()
{
	wxApp::OnRun();
	return 0;
}

void rawprocFrmApp::OnFatalException()
{
	wxMessageBox("rawprocFrmApp::OnFatalException...");
}

