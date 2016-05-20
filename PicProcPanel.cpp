
#include "PicProcPanel.h"

//BEGIN_EVENT_TABLE(PicProcPanel, wxPanel)
//	EVT_SIZE(PicProcPanel::OnSize)
//END_EVENT_TABLE()

//PicProcPanel::PicProcPanel(wxPanel *parent, PicProcessor *proc, wxString params): wxScrolledWindow(parent) 
PicProcPanel::PicProcPanel(wxPanel *parent, PicProcessor *proc, wxString params): wxPanel(parent) 
{
	q = proc;
	p = params;
	
	SetSize(parent->GetSize());
//	SetScrollRate( 5, 5 ); 
	b = new wxBoxSizer(wxVERTICAL); 
	g = new wxGridBagSizer();
}

PicProcPanel::~PicProcPanel()
{
	b->~wxBoxSizer();
}

void PicProcPanel::OnSize(wxSizeEvent& event) 
{
	if (b) {
		//b->Layout();
		Refresh();
	}
}

wxString PicProcPanel::getParams()
{
	return p;
}



