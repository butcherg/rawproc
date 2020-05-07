#ifndef MYDATADIALOG_H_
#define MYDATADIALOG_H_

#include <wx/wx.h>
#include <wx/textctrl.h>
#include "myListCtrl.h"

class myDataDialog: public wxDialog
{
	public:
		myDataDialog(wxWindow *parent, wxWindowID id, const wxString &title, wxArrayString items, const wxPoint &pos, const wxSize &size);
		void endDialog(wxCommandEvent& event);
		void resetDialog(wxCommandEvent& event);
		void doubleClicked(wxMouseEvent& event);
		void filterGrid(wxCommandEvent& event);
		
	private:
		wxTextCtrl *fil, *list;
		wxArrayString lines;
		wxStaticText *count;
		//wxString lens;

};


#endif
