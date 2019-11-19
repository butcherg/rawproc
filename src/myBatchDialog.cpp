#include "myBatchDialog.h"
#include "rawprocFrm.h"
#include "myRowSizer.h"
#include "util.h"

//#include <wx/stattext.h>
#include "wx/process.h"
#include <wx/utils.h> 
#include <wx/clipbrd.h>

#define BATCHPROCESS 2010
#define BATCHSHOW 2011

myBatchDialog::myBatchDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE) // | wxRESIZE_BORDER)
{
	wxFileName inputspec = ((rawprocFrm *) parent)->getFileName().GetFullPath();
	wxFileName outputspec = ((rawprocFrm *) parent)->getSourceFileName().GetFullPath();
	inputspec.SetName("*");	
	outputspec.SetName("*");

	//directory = new wxTextCtrl(this, wxID_ANY, wxFileName::GetCwd(), wxDefaultPosition, wxSize(500,25));
	inputfilespec = new wxTextCtrl(this, wxID_ANY, inputspec.GetFullPath(), wxDefaultPosition, wxSize(500,TEXTHEIGHT+5));
	//inputfilespec = new wxTextCtrl(this, wxID_ANY, ((rawprocFrm *) parent)->getRootTool(), wxDefaultPosition, wxSize(500,25));
	outputfilespec = new wxTextCtrl(this, wxID_ANY, outputspec.GetFullPath(), wxDefaultPosition, wxSize(500,TEXTHEIGHT+5));
	toolchain = new wxTextCtrl(this, wxID_ANY, ((rawprocFrm *) parent)->getToolChain(), wxDefaultPosition, wxSize(500,TEXTHEIGHT*4), wxTE_MULTILINE);

	wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
	myRowSizer *s = new myRowSizer(wxSizerFlags().Expand());
	//s->AddRowItem(new wxStaticText(this, wxID_ANY, ((rawprocFrm *) parent)->getToolChain()),flags);
	//s->AddRowItem(directory,flags);
	s->NextRow();
	s->AddRowItem(inputfilespec,flags);
	s->NextRow();
	s->AddRowItem(outputfilespec,flags);
	s->NextRow();
	s->AddRowItem(toolchain,flags);
	s->NextRow();
	s->AddRowItem(new wxButton(this, wxID_OK, "Dismiss", wxDefaultPosition, wxDefaultSize),flags);
	s->AddRowItem(new wxButton(this, BATCHPROCESS, "Process", wxDefaultPosition, wxDefaultSize),flags);
	s->AddRowItem(new wxButton(this, BATCHSHOW, "Show", wxDefaultPosition, wxDefaultSize),flags);
	s->End();
	SetSizerAndFit(s);

	Bind(wxEVT_BUTTON, &myBatchDialog::OnProcess, this, BATCHPROCESS);
	Bind(wxEVT_BUTTON, &myBatchDialog::OnShow, this, BATCHSHOW);
}

wxString myBatchDialog::ConstructCommand()
{
	wxString term = "gnome-terminal --";
	wxString img = "/home/glenn/ImageStuff/rawproc9/rawproc/build-linux/src/img";
	wxString pause = "read line";
	wxString toolchainstr = "\"" + toolchain->GetValue() + "\"";
	toolchainstr.Replace(" ","\" \"");
	//return wxString::Format("%s %s \"%s\" %s \"%s\"",term,img,inputfilespec->GetValue(),toolchainstr,outputfilespec->GetValue());
	return wxString::Format("%s %s \"%s\" %s \"%s\"",term,img,inputfilespec->GetValue(),toolchainstr,outputfilespec->GetValue());
}

void myBatchDialog::OnProcess(wxCommandEvent& event)
{
	wxExecute(ConstructCommand());
}

void myBatchDialog::OnShow(wxCommandEvent& event)
{
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(ConstructCommand()) );
		wxTheClipboard->Close();
	}
	wxMessageBox(ConstructCommand());
}



