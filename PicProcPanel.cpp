
#include "PicProcPanel.h"


PicProcPanel::PicProcPanel(wxWindow *parent, PicProcessor *proc, wxString params): wxPanel(parent) 
{
	q = proc;
	p = params;
	
	SetSize(parent->GetSize());
	b = new wxBoxSizer(wxVERTICAL); 
	g = new wxGridBagSizer();
	Bind(wxEVT_LEFT_DOWN, &PicProcPanel::OnLeftDown, this);
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

void PicProcPanel::OnLeftDown(wxMouseEvent& event)
{
	SetFocus();
	//Refresh();
}


wxString PicProcPanel::getParams()
{
	return p;
}



