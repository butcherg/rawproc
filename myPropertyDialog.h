#ifndef MYPROPERTYDIALOG_H_
#define MYPROPERTYDIALOG_H_

#include <wx/dialog.h>
#include <wx/propgrid/propgrid.h>
#include <string>
#include <map>

class PropertyDialog: public wxDialog
{
	public:
		PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, std::map<std::string, std::string> props, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		~PropertyDialog();
		//void OnPropertyChange(wxPropertyGridEvent& event);

	private:
		wxPropertyGrid *pg;
};

#endif