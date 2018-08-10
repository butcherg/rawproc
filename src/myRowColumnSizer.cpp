#include "myRowColumnSizer.h"


myRowColumnSizer::myRowColumnSizer(int vgap, int hgap): wxGridBagSizer(vgap, hgap) 
{
	r=0;
	c=0;
}

wxSizerItem * myRowColumnSizer::AddItem(wxWindow *window, int flags, int colspan)
{
	wxSizerItem * i = Add(window, wxGBPosition(r,c), wxGBSpan(1,colspan), flags);
	c += colspan;
	return i;
}

void myRowColumnSizer::NextRow()
{
	r++;
	c=0;
}
		


