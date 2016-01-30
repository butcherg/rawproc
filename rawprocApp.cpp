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

IMPLEMENT_APP(rawprocFrmApp)

bool rawprocFrmApp::OnInit()
{
	rawprocFrm* frame = new rawprocFrm(NULL);
	SetTopWindow(frame);
	frame->Show();
	if (wxGetApp().argc > 1) {
		wxString  fname(wxGetApp().argv[1]);
		if (!fname.IsEmpty()) frame->OpenFile(fname,0);
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

