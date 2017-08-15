
#include "PicProcPanel.h"


PicProcPanel::PicProcPanel(wxWindow *parent, PicProcessor *proc, wxString params): wxPanel(parent) 
{
	q = proc;
	p = params;
	
	SetSize(parent->GetSize());
	b = new wxBoxSizer(wxVERTICAL); 
	g = new wxGridBagSizer();
}

PicProcPanel::~PicProcPanel()
{
	if (b) b->~wxBoxSizer();
	if (g) g->~wxGridBagSizer();
}

void PicProcPanel::OnSize(wxSizeEvent& event) 
{
	if (b) {
		Refresh();
	}
	event.Skip();
}

wxString PicProcPanel::getParams()
{
	return p;
}



