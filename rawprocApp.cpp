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
#include <wx/fileconf.h>
#include <wx/stdpaths.h>

#include "wx/filesys.h"
#include "wx/fs_zip.h"

#include "util.h"
#include "FreeImage.h"

IMPLEMENT_APP(rawprocFrmApp)

bool rawprocFrmApp::OnInit()
{
#ifdef FREEIMAGE_LIB
	//FreeImage_Initialise();
#endif // FREEIMAGE_LIB

	wxInitAllImageHandlers();
	wxFileSystem::AddHandler(new wxZipFSHandler);

	rawprocFrm* frame = new rawprocFrm(NULL);
	SetTopWindow(frame);
	frame->Show();

	wxString conf_cwd = wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath()+wxFileName::GetPathSeparator()+"rawproc.conf";
	wxString conf_configd = wxStandardPaths::Get().GetUserDataDir()+wxFileName::GetPathSeparator()+"rawproc.conf";
	if (wxFileName::FileExists(conf_cwd)) {
		wxConfigBase::Set(new wxFileConfig("rawproc.conf", "", conf_cwd));
		frame->SetConfigFile(conf_cwd);
	}
	else if (wxFileName::FileExists(conf_configd)) {
		wxConfigBase::Set(new wxFileConfig("rawproc.conf", "", conf_configd));
		frame->SetConfigFile(conf_configd);
	}


	if (wxGetApp().argc == 2) {
		wxFileName f(wxGetApp().argv[1]);
		f.MakeAbsolute();
		wxSetWorkingDirectory (f.GetPath());
		if (ImageContainsRawprocCommand(wxGetApp().argv[1])) {
			if (wxMessageBox("Image contains rawproc script.  Open the script?", "Contains Script", wxYES_NO | wxCANCEL | wxNO_DEFAULT) == wxYES)
				frame->OpenFileSource(f.GetFullPath());
			else	
				frame->OpenFile(f.GetFullPath(),0);
		}
		else frame->OpenFile(f.GetFullPath(),0);
	}
	else if (wxGetApp().argc == 3) {
		wxFileName f(wxGetApp().argv[2]);
		f.MakeAbsolute();
		wxSetWorkingDirectory (f.GetPath());
		if (wxGetApp().argv[1] == "-s") 
			frame->OpenFileSource(f.GetFullPath());
		else
			frame->OpenFile(f.GetFullPath(),0);
	}
	else {
		wxFileName picdir = wxFileName::DirName(wxStandardPaths::Get().GetDocumentsDir());
		picdir.RemoveLastDir();
		picdir.AppendDir("Pictures");
		if (picdir.DirExists()) wxSetWorkingDirectory(picdir.GetPath());
	}
	return true;
}
 
int rawprocFrmApp::OnExit()
{
#ifdef FREEIMAGE_LIB
	//FreeImage_DeInitialise();
#endif // FREEIMAGE_LIB
	delete wxConfigBase::Get();
	return 0;
}

void rawprocFrmApp::OnFatalException()
{
	wxMessageBox("rawprocFrmApp::OnFatalException...");
}

