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

#include "wx/filesys.h"
#include "wx/fs_zip.h"

#include "util.h"
#include <gimage/gimage.h>
#include "myConfig.h"

IMPLEMENT_APP(rawprocFrmApp)

bool rawprocFrmApp::OnInit()
{


	wxInitAllImageHandlers();
	wxFileSystem::AddHandler(new wxZipFSHandler);

	rawprocFrm* frame = new rawprocFrm(NULL);
	SetTopWindow(frame);
	frame->Show();

	wxString conf_cwd = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath()+wxFileName::GetPathSeparator()+"rawproc.conf";
	wxString conf_configd = wxStandardPaths::Get().GetUserDataDir()+wxFileName::GetPathSeparator()+"rawproc.conf";
	if (wxFileName::FileExists(conf_cwd)) {
		frame->SetConfigFile(conf_cwd);
		myConfig::loadConfig(std::string(conf_cwd.c_str()));
	}
	else if (wxFileName::FileExists(conf_configd)) {
		frame->SetConfigFile(conf_configd);
		myConfig::loadConfig(std::string(conf_configd.c_str()));
	}
	
	wxFileName profpath;
	profpath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","")));
	gImage::setProfilePath(std::string(profpath.GetPathWithSep().c_str()));
	
	//parm app.start.logmessage: Message to print in the log when rawproc starts. 
	wxString startmessage = wxString(myConfig::getConfig().getValueOrDefault("app.start.logmessage",""));
	if (startmessage != "") log(startmessage);
	
	frame->SetBackground();

	int thumbmode = atoi(myConfig::getConfig().getValueOrDefault("display.thumb.initialmode","1").c_str());  //1=thumb, 2=histogram, 3=none
	frame->SetThumbMode(thumbmode);

	if (wxGetApp().argc == 2) {
		wxFileName f(wxGetApp().argv[1]);
		f.MakeAbsolute();
		wxSetWorkingDirectory (f.GetPath());
		frame->SetStartPath(f.GetPath());
		if (ImageContainsRawprocCommand(wxGetApp().argv[1])) {
			if (wxMessageBox("Image contains rawproc script.  Open the script?", "Contains Script", wxYES_NO | wxCANCEL | wxNO_DEFAULT) == wxYES)
				frame->OpenFileSource(f.GetFullPath());
			else	
				frame->OpenFile(f.GetFullPath());
		}
		else frame->OpenFile(f.GetFullPath());
	}
	else if (wxGetApp().argc == 3) {
		wxFileName f(wxGetApp().argv[2]);
		f.MakeAbsolute();
		wxSetWorkingDirectory (f.GetPath());
		frame->SetStartPath(f.GetPath());
		if (wxGetApp().argv[1] == "-s") 
			frame->OpenFileSource(f.GetFullPath());
		else
			frame->OpenFile(f.GetFullPath());
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

void rawprocFrmApp::OnFatalException()
{
	wxMessageBox("rawprocFrmApp::OnFatalException...");
}

