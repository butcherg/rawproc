#include "myBatchDialog.h"
#include "rawprocFrm.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "util.h"
#include "folder.xpm"

//#include <wx/stattext.h>
#include "wx/process.h"
#include <wx/utils.h> 
#include <wx/clipbrd.h>

#define BATCHPROCESS 2010
#define BATCHSHOW 2011
#define BATCHCWD 2012

myBatchDialog::myBatchDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos, const wxSize &size):
wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE) // | wxRESIZE_BORDER)
{
	wxFileName dirspec, inputspec, outputspec;
	wxString ispec, ospec, tchain, tchainsource;
	dirspec = wxFileName(wxFileName::GetCwd(), "");

	//parm batch.termcommand: path/executable to use as the batch command shell. Default: wxcmd (somewhere in $PATH)
	wxString term = wxString(myConfig::getConfig().getValueOrDefault("batch.termcommand","wxcmd"));
	//parm batch.imgcommand: path/executable for the img command line raw processor. Default: img (somewhere in $PATH)
	wxString img = wxString(myConfig::getConfig().getValueOrDefault("batch.imgcommand","img"));
	
	if (((rawprocFrm *) parent)->isOpenSource()) {
		
		inputspec = ((rawprocFrm *) parent)->getFileName().GetFullPath();
		inputspec.MakeRelativeTo();
		inputspec.SetName("*");	
		wxString roottool = inputfilecommand(((rawprocFrm *) parent)->getRootTool())[1];
		if (roottool != "") roottool = ":"+roottool;
		ispec = inputspec.GetFullPath()+roottool;
		
		outputspec = ((rawprocFrm *) parent)->getSourceFileName().GetFullPath();
		outputspec.MakeRelativeTo();
		outputspec.SetName("*");
		ospec = outputspec.GetFullPath();
		

		tchain = ((rawprocFrm *) parent)->getToolChain();
		tchainsource = "(Source: processed image)";
		
	}
	else if (((rawprocFrm *) parent)->isOpen()) {
		
		inputspec = ((rawprocFrm *) parent)->getFileName().GetFullPath();
		inputspec.MakeRelativeTo();
		inputspec.SetName("*");	
		wxString roottool = inputfilecommand(((rawprocFrm *) parent)->getRootTool())[1];
		if (roottool != "") roottool = ":"+roottool;
		ispec = inputspec.GetFullPath()+roottool;
		
		if (((rawprocFrm *) parent)->getSourceFileName().GetFullPath() != wxEmptyString) {
			outputspec = ((rawprocFrm *) parent)->getSourceFileName().GetFullPath();
			outputspec.MakeRelativeTo();
			outputspec.SetName("*");
			ospec = outputspec.GetFullPath();
		}
		else {
			ospec = wxString(myConfig::getConfig().getValueOrDefault("batch.outputspec",""));
		}
		

		tchain = ((rawprocFrm *) parent)->getToolChain();
		tchainsource = "(Source: processed image)";
		
	}
	else {
		//parm batch.inputspec: Path/file specification for input.  Append input processing with a ':', e.g., *.NEF:rawdata=crop.  Default: None, you need to specify your own.
		ispec = wxString(myConfig::getConfig().getValueOrDefault("batch.inputspec","*.NEF:rawdata=crop"));
		//parm batch.outputspec: Path/file specification for output.  Append output processing with a ':', e.g., *.jpg:quality=75.  Default: None, you need to specify your own.
		ospec = wxString(myConfig::getConfig().getValueOrDefault("batch.outputspec","../*.jpg"));
		//parm batch.toolchain: The tool chain to be applied to each input image to produce the output image.  See the img command line documentation for syntax.
		tchain = wxString(myConfig::getConfig().getValueOrDefault("batch.toolchain","colorspace:camera,assign subtract:camera whitebalance:camera demosaic:proof blackwhitepoint:rgb,data tone:filmic resize:800 sharpen:0.5"));
		tchainsource = "(Source: batch.toolchain property)";
	}

	termcmd = new wxTextCtrl(this, wxID_ANY, term, wxDefaultPosition, wxSize(250,TEXTHEIGHT+5));
	imgcmd =  new wxTextCtrl(this, wxID_ANY, img, wxDefaultPosition, wxSize(250,TEXTHEIGHT+5));
	directory = new wxTextCtrl(this, wxID_ANY, wxFileName::GetCwd(), wxDefaultPosition, wxSize(500,TEXTHEIGHT+5));
	inputfilespec = new wxTextCtrl(this, wxID_ANY, ispec, wxDefaultPosition, wxSize(250,TEXTHEIGHT+5));
	outputfilespec = new wxTextCtrl(this, wxID_ANY, ospec, wxDefaultPosition, wxSize(250,TEXTHEIGHT+5));
	toolchain = new wxTextCtrl(this, wxID_ANY, tchain, wxDefaultPosition, wxSize(500,TEXTHEIGHT*4), wxTE_MULTILINE);
	toolchaintxt = new wxStaticText(this, wxID_ANY, "Tool Chain: " + tchainsource);

	wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
	wxSizerFlags labelflags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
	myRowSizer *s = new myRowSizer(wxSizerFlags().Expand());
	s->AddRowItem(new wxStaticText(this, wxID_ANY, "Directory:"),labelflags);
	s->NextRow();

	s->AddRowItem(directory,flags);
	s->AddRowItem(new wxBitmapButton(this, BATCHCWD, wxBitmap(folder_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
	s->NextRow();

	s->AddRowItem(new wxStaticText(this, wxID_ANY, "Term Commmand:",wxDefaultPosition, wxSize(250,-1)),labelflags);
	s->AddRowItem(new wxStaticText(this, wxID_ANY, "Img Command:",wxDefaultPosition, wxSize(250,-1)),labelflags);
	s->NextRow();
	s->AddRowItem(termcmd,flags);
	s->AddRowItem(imgcmd,flags);
	s->NextRow();

	s->AddRowItem(new wxStaticText(this, wxID_ANY, "Input File Specification:",wxDefaultPosition, wxSize(250,-1)),labelflags);
	s->AddRowItem(new wxStaticText(this, wxID_ANY, "Output File Specification:",wxDefaultPosition, wxSize(250,-1)),labelflags);
	s->NextRow();
	s->AddRowItem(inputfilespec,flags);
	s->AddRowItem(outputfilespec,flags);
	s->NextRow();

	//s->AddRowItem(new wxStaticText(this, wxID_ANY, "Tool Chain:"),labelflags);
	s->AddRowItem(toolchaintxt, labelflags);
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
	Bind(wxEVT_BUTTON, &myBatchDialog::OnDirSelect, this, BATCHCWD);
}

void myBatchDialog::OnDirSelect(wxCommandEvent& event)
{
	const wxString& dir = wxDirSelector("Select Batch Working Directory", directory->GetValue());
	if ( !dir.empty() ) {
		directory->SetValue(dir);
		directory->Refresh();
	}
}

wxString myBatchDialog::ConstructCommand()
{
	wxString term = termcmd->GetValue().Trim();
	wxString img = imgcmd->GetValue().Trim();
	wxString toolchainstr = toolchain->GetValue().Trim();
	toolchainstr = "\"" + toolchainstr + "\"";
	toolchainstr.Replace(" ","\" \"");
	return wxString::Format("%s %s \"%s\" %s \"%s\"",term,img,inputfilespec->GetValue(),toolchainstr,outputfilespec->GetValue());
}

void myBatchDialog::OnProcess(wxCommandEvent& event)
{
	wxExecuteEnv env;
	env.cwd = directory->GetValue();
	wxExecute(ConstructCommand(),wxEXEC_ASYNC,NULL,&env);
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



