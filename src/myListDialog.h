#ifndef MYLISTDIALOG_H_
#define MYLISTDIALOG_H_

#include <wx/wx.h>
#include "myListCtrl.h"

class myListDialog: public wxDialog
{
	public:
		myListDialog(wxWindow *parent, wxWindowID id, const wxString &title, wxArrayString items, const wxPoint &pos, const wxSize &size);
		void endDialog(wxCommandEvent& event);
		void doubleClicked(wxMouseEvent& event);
		void filterGrid(wxCommandEvent& event);
		wxString getSelection();
		
	private:
		myListCtrl *list;
		wxTextCtrl *fil;
		wxStaticText *count;
		wxString lens;

};


#endif
