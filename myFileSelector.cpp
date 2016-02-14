#include "myFileSelector.h"

/*
RAW_DEFAULT Load the file as linear RGB 48-bit
RAW_PREVIEW Try to load the embedded JPEG preview with included Exif data or default to RGB 24-bit
RAW_DISPLAY Load the file as RGB 24-bit
RAW_HALFSIZE Output a half-size color image
RAW_UNPROCESSED
*/

myFileSelector::myFileSelector(wxWindow* parent, wxWindowID id, wxString path, wxString title): wxDialog(parent, id, title, wxDefaultPosition, wxSize(600, 500))
{

	wxPanel* panel = new wxPanel(this, wxID_ANY);

	fileselector = new wxFileCtrl(panel, wxID_ANY, path, wxEmptyString, wxFileSelectorDefaultWildcardStr, wxFC_DEFAULT_STYLE, wxPoint(20,5), wxSize(550,300));
	wxArrayString flags;
	flags.Add("Default");
	flags.Add("Preview");
	flags.Add("Display");
	flags.Add("Half Size");
	flags.Add("Unprocessed");
	rawflags = new wxRadioBox (panel, wxID_ANY, "RAW Mode",  wxPoint(20,320), wxDefaultSize, flags, 5);
	wxButton* cancelButton = new wxButton(panel, wxID_ANY, "Cancel", wxPoint(20,400), wxDefaultSize);
	wxButton* okButton = new wxButton(panel, wxID_ANY, "Ok", wxPoint(150,400), wxDefaultSize);

	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &myFileSelector::onCancel, this, cancelButton->GetId());
	Bind(wxEVT_COMMAND_BUTTON_CLICKED, &myFileSelector::onOk, this, okButton->GetId());
	Bind(wxEVT_FILECTRL_SELECTIONCHANGED, &myFileSelector::onFileChange, this, fileselector->GetId());
	Bind(wxEVT_FILECTRL_FILEACTIVATED, &myFileSelector::onFileActivate, this, fileselector->GetId());
	rawflags->Disable();
	Center();
}

myFileSelector::~myFileSelector() {}

void myFileSelector::onCancel(wxCommandEvent& WXUNUSED(pEvent))    {
	EndModal(wxID_CANCEL);
	Destroy();
}

void myFileSelector::onOk(wxCommandEvent& WXUNUSED(pEvent))    {
	EndModal(wxID_OK);
	Destroy();
}

wxString myFileSelector::GetFileSelected()
{
	return fileselector->GetPath();
}

int myFileSelector::GetFlag()
{
	switch (rawflags->GetSelection()) {
	case wxNOT_FOUND:
	case 0:
		return RAW_DEFAULT;
	case 1:
		return RAW_PREVIEW;
	case 2:
		return RAW_DISPLAY;
	case 3:
		return RAW_HALFSIZE;
	case 4:
		return RAW_UNPROCESSED;
	default:
		return RAW_DEFAULT;
	}
}

void myFileSelector::onFileChange(wxFileCtrlEvent& WXUNUSED(pEvent))
{
	FREE_IMAGE_FORMAT fif;
	fif = FreeImage_GetFileType(fileselector->GetPath(), 0);
	if (fif == FIF_RAW)
		rawflags->Enable();
	else
		rawflags->Disable();
}

void myFileSelector::onFileActivate(wxFileCtrlEvent& WXUNUSED(pEvent))
{
	EndModal(wxID_OK);
	Destroy();
}



