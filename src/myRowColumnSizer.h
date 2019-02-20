#ifndef MYROWCOLUMNSIZER_H_
#define MYROWCOLUMNSIZER_H_

#include <wx/wx.h>
#include <wx/gbsizer.h>

class myRowColumnSizer: public wxGridBagSizer
{
	public:
		myRowColumnSizer(int vgap=0, int hgap=0);
		wxSizerItem * AddItem(wxWindow *window, int flags, int colspan=1);
		void AddEmptyItem();
		void NextRow();

	private:
		unsigned r, c;
};

#endif
