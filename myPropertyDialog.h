#ifndef MYPROPERTYDIALOG_H_
#define MYPROPERTYDIALOG_H_

#include <wx/dialog.h>
#include <wx/propgrid/propgrid.h>
#include <wx/fileconf.h>
#include <wx/stattext.h>
#include <string>
#include <map>

class PropertyDialog: public wxDialog
{
	public:
		PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, std::map<std::string, std::string> props, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		PropertyDialog(wxWindow *parent, wxWindowID id, const wxString &title, wxFileConfig *config, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		~PropertyDialog();
		void UpdateProperty(wxPropertyGridEvent& event);


	private:
		wxPropertyGrid *pg;

};

#endif