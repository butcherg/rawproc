#ifndef MYROWSIZER_H_
#define MYROWSIZER_H_

#include <wx/wx.h>
#include <wx/sizer.h>

class myRowSizer: public wxBoxSizer
{
	public:
		myRowSizer(wxSizerFlags rowflags=wxSizerFlags());
		wxSizerItem * AddRowItem(wxWindow *window, wxSizerFlags &flags);
		void NextRow(wxSizerFlags rowflags=wxSizerFlags());
		void End();

	private:
		wxBoxSizer *r;
		wxSizerFlags f;
};

#endif
