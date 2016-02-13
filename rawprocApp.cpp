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
	if (wxGetApp().argc == 2) {
		frame->OpenFile(wxGetApp().argv[1],0);
	}
	else if (wxGetApp().argc == 3) {
		if (wxGetApp().argv[1] == "-s") 
			frame->OpenFileSource(wxGetApp().argv[2]);
		else
			frame->OpenFile(wxGetApp().argv[2],0);
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

