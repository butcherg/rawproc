#include "myRowSizer.h"


myRowSizer::myRowSizer(wxSizerFlags rowflags): wxBoxSizer(wxVERTICAL) 
{
	r = new wxBoxSizer(wxHORIZONTAL);
	f = rowflags;
}

wxSizerItem * myRowSizer::AddRowItem(wxWindow *window, wxSizerFlags &flags)
{
	wxSizerItem * i = r->Add(window, flags);
	return i;
}

void myRowSizer::NextRow(wxSizerFlags rowflags)
{
	Add(r, f);
	r = new wxBoxSizer(wxHORIZONTAL);
	f = rowflags;
}

void myRowSizer::End()
{
	Add(r, f);
}

	


