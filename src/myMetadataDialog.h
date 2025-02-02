#ifndef MYMETADATADIALOG_H_
#define MYMETADATADIALOG_H_

#include <wx/dialog.h>
#include <wx/propgrid/propgrid.h>
#include <wx/fileconf.h>
#include <wx/stattext.h>
#include <string>
#include <map>
#include "gimage.h"

class MetadataDialog: public wxDialog
{
	public:
		MetadataDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);

		~MetadataDialog();
		void LoadMetadata(gImage *infodib);
		void UpdateTag(wxPropertyGridEvent& event);
		void FilterGrid(wxCommandEvent& event);
		void resetFilter(wxCommandEvent& event);
		std::map<std::string,std::string> FilterList(wxString filter);
		std::string FilterString(wxString filter);
		
		bool TagExists(wxString name);
		void AddTag(wxCommandEvent& event);
		void DelTag(wxCommandEvent& event);
		void HideDialog(wxCommandEvent& event);
		void ClearModifiedStatus();


	private:
		wxPropertyGrid *pg;
		wxTextCtrl *fil;
		wxBoxSizer *sz, *ct;
		gImage *dib;

};

#endif
