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

#include "util.h"

IMPLEMENT_APP(rawprocFrmApp)

bool rawprocFrmApp::OnInit()
{
	wxConfigBase::Set(new wxFileConfig("rawproc", "", wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath()+wxFileName::GetPathSeparator()+"rawproc.conf"));
	rawprocFrm* frame = new rawprocFrm(NULL);
	SetTopWindow(frame);
	frame->Show();

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
	return true;
}
 
int rawprocFrmApp::OnExit()
{
	delete wxConfigBase::Get();
	return 0;
}

void rawprocFrmApp::OnFatalException()
{
	wxMessageBox("rawprocFrmApp::OnFatalException...");
}

