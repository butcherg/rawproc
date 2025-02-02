#ifndef MYHISTOGRAMDIALOG_H_
#define MYHISTOGRAMDIALOG_H_

#include <wx/wx.h>

#include <wx/dialog.h>
#include "myHistogramPane.h"
#include "gimage.h"


class myHistogramDialog: public wxDialog
{
	public:
		myHistogramDialog(wxWindow *parent, wxWindowID id, const wxString &title, gImage *dib, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		void OnSize(wxSizeEvent& event);
		~myHistogramDialog();

	private:
		myHistogramPane *hpane;
		wxBoxSizer *s;

};

#endif
