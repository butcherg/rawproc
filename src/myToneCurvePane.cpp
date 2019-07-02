
#include "myToneCurvePane.h"
#include <math.h>
#include <algorithm>
#include <wx/clipbrd.h>
#include "myConfig.h"
#include "util.h"


myToneCurvePane::myToneCurvePane(wxWindow* parent, const wxPoint &pos, const wxSize &size) :
 wxWindow(parent, wxID_ANY, pos, size, wxBORDER_SUNKEN)
{
	Bind(wxEVT_PAINT, &myToneCurvePane::paintEvent, this);
}


myToneCurvePane::~myToneCurvePane()
{

}

void myToneCurvePane::OnSize(wxSizeEvent& event) 
{
	event.Skip();
	Refresh();
}

 
void myToneCurvePane::paintEvent(wxPaintEvent & evt)
{
	wxPaintDC dc(this);
	render(dc);
}
 
void myToneCurvePane::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}



void myToneCurvePane::SetCurve(std::vector<float> curve)
{	
	c = curve;
	paintNow();
}




void myToneCurvePane::render(wxDC&  dc)
{
	int w, h;
	GetSize(&w, &h);
	dc.Clear();
	wxPoint *cp = new wxPoint[c.size()];
	for (unsigned i=0; i<c.size(); i++) {
		cp[i] = wxPoint(i+5, (h-5)-(int(100.0*c[i])));
		//printf("%d:%d\n",i, int(100.0*c[i]));
	}
	dc.SetPen(wxPen(wxColour(255,0,0),1));
	dc.DrawLines(c.size(),cp,1,1);
	delete [] cp;
}




 
 

