
#include "PicProcPanel.h"


//PicProcPanel::PicProcPanel(wxPanel *parent, PicProcessor *proc, wxString params): wxScrolledWindow(parent) 
PicProcPanel::PicProcPanel(wxPanel *parent, PicProcessor *proc, wxString params): wxPanel(parent) 
{
	q = proc;
	p = params;
	SetSize(parent->GetSize());
//	SetScrollRate( 5, 5 );
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



