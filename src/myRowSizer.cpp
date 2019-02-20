#include "myRowSizer.h"


myRowSizer::myRowSizer(): wxBoxSizer(wxVERTICAL) 
{
	r = new wxBoxSizer(wxHORIZONTAL);
}

wxSizerItem * myRowSizer::AddRowItem(wxWindow *window, wxSizerFlags &flags)
{
	wxSizerItem * i = r->Add(window, flags);
	return i;
}

void myRowSizer::NextRow()
{
	Add(r);
	r = new wxBoxSizer(wxHORIZONTAL);
}

void myRowSizer::End()
{
	Add(r);
}

	


