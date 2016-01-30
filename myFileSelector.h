
#ifndef MYFILESELECTOR_H_
#define MYFILESELECTOR_H_

#include <wx/wx.h>
#include <wx/filectrl.h>
#include "FreeImage.h"

class myFileSelector: public wxDialog {
public:
	myFileSelector(wxWindow* parent, wxWindowID id, wxString path, wxString title);
	virtual ~myFileSelector();
	void onCancel(wxCommandEvent& pEvent);
	void onOk(wxCommandEvent& pEvent);
	void onFileChange(wxFileCtrlEvent& WXUNUSED(pEvent));
	void onFileActivate(wxFileCtrlEvent& WXUNUSED(pEvent));

	wxString GetFileSelected();
	int GetFlag();



private:
	//wxTextCtrl* nameTextCtrl_;
	wxFileCtrl* fileselector;
	wxRadioBox* rawflags;
};

#endif 

