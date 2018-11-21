
#include "PicProcPanel.h"


PicProcPanel::PicProcPanel(wxWindow *parent, PicProcessor *proc, wxString params): 
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, !wxTAB_TRAVERSAL) 
{
	q = proc;
	p = params;
	
	SetForegroundColour(*wxBLACK);
	SetSize(parent->GetSize());
	b = new wxBoxSizer(wxVERTICAL); 
	g = new wxGridBagSizer();
	SetBackgroundColour(parent->GetBackgroundColour());
	Bind(wxEVT_LEFT_DOWN, &PicProcPanel::OnLeftDown, this);
}

PicProcPanel::~PicProcPanel()
{

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
	Refresh();
}


wxString PicProcPanel::getParams()
{
	return p;
}



