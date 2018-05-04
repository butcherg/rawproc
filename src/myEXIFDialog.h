#ifndef MYEXIFDIALOG_H_
#define MYEXIFDIALOG_H_

#include <wx/wx.h>
#include <wx/dialog.h>
#include <wx/html/htmlwin.h>

class myEXIFDialog: public wxDialog
{
	public:
		myEXIFDialog(wxWindow *parent, wxWindowID id, const wxString &title, const wxString &exif, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
		~myEXIFDialog();

	private:
		wxHtmlWindow *html;
		wxBoxSizer *s;

};

#endif
