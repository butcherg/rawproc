
#include "PicProcPanel.h"


PicProcPanel::PicProcPanel(wxPanel *parent, PicProcessor *proc, wxString params): wxPanel(parent) 
{
	q = proc;
	p = params;
	SetSize(parent->GetSize());
	b = new wxBoxSizer(wxVERTICAL); 
}

PicProcPanel::~PicProcPanel()
{
	b->~wxBoxSizer();
}

wxString PicProcPanel::getParams()
{
	return p;
}



