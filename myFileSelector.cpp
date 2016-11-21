#include "myFileSelector.h"
#include <wx/fileconf.h>

/*
RAW_DEFAULT Load the file as linear RGB 48-bit
RAW_PREVIEW Try to load the embedded JPEG preview with included Exif data or default to RGB 24-bit
RAW_DISPLAY Load the file as RGB 24-bit
RAW_HALFSIZE Output a half-size color image
RAW_UNPROCESSED
*/

myFileSelector::myFileSelector(wxWindow* parent, wxWindowID id, wxString path, wxString title): wxDialog(parent, id, title, wxDefaultPosition, wxSize(730, 600))
{
	wxBoxSizer *sz = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer *bt = new wxBoxSizer(wxHORIZONTAL);

	fileselector = new wxFileCtrl(this, wxID_ANY, path, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFC_DEFAULT_STYLE, wxDefaultPosition, wxSize(600,300));
	sz->Add(fileselector, 0,wxALL,10);

	wxArrayString flags;
	flags.Add("Demosaic");
	flags.Add("Preview");
	flags.Add("Display");
	flags.Add("Half Size");
	//flags.Add("Unprocessed");
	rawflags = new wxRadioBox (this, wxID_ANY, "RAW Mode",  wxDefaultPosition, wxSize(580,70), flags, 5);
	sz->Add(rawflags, 0,wxALL,10);

	wxArrayString cflags;
	cflags.Add("Raw");
	cflags.Add("sRGB");
	cflags.Add("Adobe RGB");
	cflags.Add("Wide Gamut");
	cflags.Add("ProPhoto");
	cflags.Add("XYZ");
	colorflags = new wxRadioBox (this, wxID_ANY, "Output ColorSpace",  wxDefaultPosition, wxSize(670, 70), cflags, 6);
	sz->Add(colorflags, 0,wxALL,10);

	wxArrayString quflags;
	quflags.Add("Linear");
	quflags.Add("VNG");
	quflags.Add("PPG");
	quflags.Add("AHD");
	qualityflags = new wxRadioBox (this, wxID_ANY, "Demosaic Algorithm",  wxDefaultPosition, wxSize(300, 70), quflags, 4);
	sz->Add(qualityflags, 0,wxALL,10);

	wxButton* cancelButton = new wxButton(this, wxID_ANY, "Cancel", wxDefaultPosition, wxDefaultSize);
	bt->Add(cancelButton, 0,wxALL,10);
	wxButton* okButton = new wxButton(this, wxID_ANY, "Ok", wxDefaultPosition, wxDefaultSize);
	bt->Add(okButton, 0,wxALL,10);
	sz->Add(bt, 0,wxALL,10);

	SetSizerAndFit(sz);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &myFileSelector::onCancel, this, cancelButton->GetId());
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &myFileSelector::onOk, this, okButton->GetId());
	Bind(wxEVT_FILECTRL_SELECTIONCHANGED, &myFileSelector::onFileChange, this, fileselector->GetId());
	Bind(wxEVT_FILECTRL_FILEACTIVATED, &myFileSelector::onFileActivate, this, fileselector->GetId());

	wxString rdefault = wxConfigBase::Get()->Read("input.raw.mode","demosaic");
	if (rdefault.CmpNoCase("demosaic")==0)		rawflags->SetSelection(0);
	if (rdefault.CmpNoCase("preview")==0)		rawflags->SetSelection(1);
	if (rdefault.CmpNoCase("display")==0)		rawflags->SetSelection(2);
	if (rdefault.CmpNoCase("half")==0)		rawflags->SetSelection(3);
	//if (rdefault.CmpNoCase("unprocessed")==0) 	rawflags->SetSelection(4);
	rawflags->Disable();

	wxString cdefault = wxConfigBase::Get()->Read("input.raw.colorspace","srgb");
	if (cdefault.CmpNoCase("raw")==0)	colorflags->SetSelection(0);
	if (cdefault.CmpNoCase("srgb")==0)	colorflags->SetSelection(1);
	if (cdefault.CmpNoCase("adobe")==0)	colorflags->SetSelection(2);
	if (cdefault.CmpNoCase("wide")==0)	colorflags->SetSelection(3);
	if (cdefault.CmpNoCase("prophoto")==0) 	colorflags->SetSelection(4);
	if (cdefault.CmpNoCase("xyz")==0) 	colorflags->SetSelection(5);
	colorflags->Disable();

	wxString qdefault = wxConfigBase::Get()->Read("input.raw.algorithm","ahd");
	if (qdefault.CmpNoCase("linear")==0) 	qualityflags->SetSelection(0);
	if (qdefault.CmpNoCase("vng")==0) 	qualityflags->SetSelection(1);
	if (qdefault.CmpNoCase("ppg")==0) 	qualityflags->SetSelection(2);
	if (qdefault.CmpNoCase("ahd")==0) 	qualityflags->SetSelection(3);
	qualityflags->Disable();

	Center();
	israw=false;
}

myFileSelector::~myFileSelector() {}

void myFileSelector::onCancel(wxCommandEvent& WXUNUSED(pEvent))    {
	EndModal(wxID_CANCEL);
	//Destroy();
}

void myFileSelector::onOk(wxCommandEvent& WXUNUSED(pEvent))    {
	EndModal(wxID_OK);
	//Destroy();
}

wxString myFileSelector::GetFileSelected()
{
	return fileselector->GetPath();
}

//ToDo: Some kind of options dialog
wxString myFileSelector::GetFlags()
{
	wxString flags = "";

	if (!israw) return "";

	switch (rawflags->GetSelection()) {
	case wxNOT_FOUND:
	case 0:
		flags.Append(wxString::Format("rawinput:default")); break;
	case 1: 
		flags.Append(wxString::Format("rawinput:preview")); break;
	case 2:
		flags.Append(wxString::Format("rawinput:display")); break;
	case 3:
		flags.Append(wxString::Format("rawinput:half")); break;
	//case 4:
	//	flags.Append(wxString::Format("rawinput:unprocessed")); break;
	default:
		flags.Append(wxString::Format("rawinput:default")); break;
	}

	switch (colorflags->GetSelection()) {
	case 0:
		flags.Append(wxString::Format(",colorspace:raw")); break;
	case wxNOT_FOUND:
	case 1: 
		flags.Append(wxString::Format(",colorspace:srgb")); break;
	case 2:
		flags.Append(wxString::Format(",colorspace:adobe")); break;
	case 3:
		flags.Append(wxString::Format(",colorspace:wide")); break;
	case 4:
		flags.Append(wxString::Format(",colorspace:prophoto")); break;
	case 5:
		flags.Append(wxString::Format(",colorspace:xyz")); break;
	default:
		flags.Append(wxString::Format(",colorspace:srgb")); break;
	}

	switch (qualityflags->GetSelection()) {
	case 0:
		flags.Append(wxString::Format(",demosaic:linear")); break;
	case 1: 
		flags.Append(wxString::Format(",demosaic:vng")); break;
	case 2:
		flags.Append(wxString::Format(",demosaic:ppg")); break;
	case wxNOT_FOUND:
	case 3:
	default:
		flags.Append(wxString::Format(",demosaic:ahd")); break;
	}

	return flags;
}

void myFileSelector::onFileChange(wxFileCtrlEvent& WXUNUSED(pEvent))
{
	if (gImage::getFileType(fileselector->GetPath().c_str()) == FILETYPE_RAW) {
		israw=true;
		rawflags->Enable();
		colorflags->Enable();
		qualityflags->Enable();

	}
	else {
		israw=false;
		rawflags->Disable();
		colorflags->Disable();
		qualityflags->Disable();

	}
}

void myFileSelector::onFileActivate(wxFileCtrlEvent& WXUNUSED(pEvent))
{
	EndModal(wxID_OK);
	//Destroy();
}



