//---------------------------------------------------------------------------
//
// Name:        rawprocApp.cpp
// Author:      Glenn
// Created:     11/18/2015 7:04:06 PM
// Description: 
//
//---------------------------------------------------------------------------

#include "rawprocApp.h"

#include <wx/filefn.h>
#include <wx/stdpaths.h>
#include <wx/cmdline.h>

#include "wx/filesys.h"
#include "wx/fs_zip.h"

#include "util.h"
#include "gimage.h"
#include "strutil.h"
#include "fileutil.h"
#include "myConfig.h"
#include "icon.xpm"

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
	wxString psep = wxFileName::GetPathSeparator();

	wxDisableAsserts();
	wxInitAllImageHandlers();
	wxFileSystem::AddHandler(new wxZipFSHandler);
	
	wxCmdLineParser cmdline(cmdLineDesc, wxGetApp().argc, wxGetApp().argv);
	if (cmdline.Parse() == -1) exit(0);

	wxString configfile;
	std::string configfilepath;
	cmdline.Found("c", &configfile);
	configfilepath = wxString(getRawprocConfPath(std::string(configfile.c_str())));
	if (configfilepath != "(none)") myConfig::loadConfig(configfilepath);
	
	frame = new rawprocFrm(NULL);
	frame->SetIcon(icon_xpm);
	SetTopWindow(frame);
	frame->Show();
	frame->SetConfigFile(configfilepath);

	//propvar EXEDIR: Use to substitute the path to the running executable in path properties
	myConfig::getConfig().setVariable("EXEDIR", getExeDir());
	
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
	frame->Shutdown();
	frame->Destroy();
}

