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

#ifdef RAW_COLOR_RAW
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
#endif

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

#ifdef RAW_COLOR_RAW
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
#endif

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

int myFileSelector::GetFlag()
{
	int flags = 0;

	if (!israw) return 0;

	switch (rawflags->GetSelection()) {
	case wxNOT_FOUND:
	case 0:
		flags = flags | RAW_DEFAULT; break;
	case 1: 
		flags = flags | RAW_PREVIEW; break;
	case 2:
		flags = flags | RAW_DISPLAY; break;
	case 3:
		flags = flags | RAW_HALFSIZE; break;
	//case 4:
	//	flags = flags | RAW_UNPROCESSED; break;
	default:
		flags = flags | RAW_DEFAULT;
	}

#ifdef RAW_COLOR_RAW
	switch (colorflags->GetSelection()) {
	case 0:
		flags = flags | RAW_COLOR_RAW; break;
	case wxNOT_FOUND:
	case 1: 
		flags = flags | RAW_COLOR_SRGB; break;
	case 2:
		flags = flags | RAW_COLOR_ADOBE; break;
	case 3:
		flags = flags | RAW_COLOR_WIDE; break;
	case 4:
		flags = flags | RAW_COLOR_PROPHOTO; break;
	case 5:
		flags = flags | RAW_COLOR_XYZ; break;
	default:
		flags = flags | RAW_DEFAULT;
	}

	switch (qualityflags->GetSelection()) {
	case 0:
		flags = flags | RAW_QUAL_LINEAR; break;
	case 1: 
		flags = flags | RAW_QUAL_VNG; break;
	case 2:
		flags = flags | RAW_QUAL_PPG; break;
	case wxNOT_FOUND:
	case 3:
		flags = flags | RAW_QUAL_AHD; break;
	default:
		flags = flags | RAW_QUAL_AHD;
	}
#endif

	return flags;
}

void myFileSelector::onFileChange(wxFileCtrlEvent& WXUNUSED(pEvent))
{
	wxString FreeImageVersion(FreeImage_GetVersion());

	FREE_IMAGE_FORMAT fif;
	fif = FreeImage_GetFileType(fileselector->GetPath(), 0);
	if (fif == FIF_RAW) {
		israw=true;
		rawflags->Enable();

#ifdef RAW_COLOR_RAW
		if (FreeImageVersion.Contains("ggb")) {
			colorflags->Enable();
			qualityflags->Enable();
		}
#endif

	}
	else {
		israw=false;
		rawflags->Disable();

#ifdef RAW_COLOR_RAW
		colorflags->Disable();
		qualityflags->Disable();
#endif

	}
}

void myFileSelector::onFileActivate(wxFileCtrlEvent& WXUNUSED(pEvent))
{
	EndModal(wxID_OK);
	//Destroy();
}



