#ifndef MYBATCHDIALOG_H_
#define MYBATCHDIALOG_H_

#include <wx/wx.h>
#include <wx/dialog.h>

class myBatchDialog: public wxDialog
{
	public:
		myBatchDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		void OnDirSelect(wxCommandEvent& event);
		wxString ConstructCommand();
		void OnProcess(wxCommandEvent& event);
		void OnShow(wxCommandEvent& event);

	private:
		wxTextCtrl *termcmd, *imgcmd, *directory, *inputfilespec, *outputfilespec, *toolchain;
		wxButton *process, *dismiss;
		wxStaticText *toolchaintxt;
		

};

#endif
