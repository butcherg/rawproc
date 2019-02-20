#ifndef MYROWSIZER_H_
#define MYROWSIZER_H_

#include <wx/wx.h>
#include <wx/sizer.h>

class myRowSizer: public wxBoxSizer
{
	public:
		myRowSizer();
		wxSizerItem * AddRowItem(wxWindow *window, wxSizerFlags &flags);
		void NextRow();
		void End();

	private:
		wxBoxSizer *r;
};

#endif
