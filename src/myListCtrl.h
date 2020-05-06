#ifndef MYLISTCTRL_H_
#define MYLISTCTRL_H_

#include <wx/wx.h>
#include <wx/listctrl.h>
#include <wx/editlbox.h>

class myListCtrl: public wxListCtrl
{
	public:
		//Constructs the list control, populated with the items passed in the listitems wxArrayString:
		myListCtrl(wxWindow *parent, wxWindowID id, wxString listname, wxArrayString listitems, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		void setFilter(wxString f);
		void Selected(wxListEvent& event);
		wxString GetSelected();
		
	private:
		int width;
		wxArrayString itemlist;
		wxString filter, selected, name;
};

#endif
