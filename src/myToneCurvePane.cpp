
#include "myToneCurvePane.h"
#include <math.h>
#include <algorithm>
#include <wx/clipbrd.h>
#include "myConfig.h"
#include "util.h"


myToneCurvePane::myToneCurvePane(wxWindow* parent, const wxPoint &pos, const wxSize &size) :
 wxWindow(parent, wxID_ANY, pos, size, wxBORDER_SUNKEN)
{
	scale = 0.1;
	Bind(wxEVT_PAINT, &myToneCurvePane::paintEvent, this);
	Bind(wxEVT_MOUSEWHEEL, &myToneCurvePane::mouseWheelMoved, this);
	Bind(wxEVT_LEFT_DCLICK, &myToneCurvePane::mouseDoubleClicked, this);

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
	int range = c.size();
	int linewidth = int(std::max(1.0, 1.0/scale));
	int w, h;
	GetSize(&w, &h);
	dc.Clear();
	dc.DrawText(wxString::Format("%f",scale), w/2, h-10);
	dc.SetDeviceOrigin (5, h-5);
	dc.SetAxisOrientation(true,true);
	dc.SetLogicalScale(scale, scale);
	wxPoint *cp = new wxPoint[range];
	for (unsigned i=0; i<range; i++) cp[i] = wxPoint(i, int((float) range *c[i]));
	dc.SetPen(wxPen(wxColour(255,0,0),linewidth));
	dc.DrawLines(c.size(),cp,1,1);
	delete [] cp;
}

void myToneCurvePane::mouseWheelMoved(wxMouseEvent& event) 
{	
	event.Skip();
	double increment = 0.05;
	if (event.ShiftDown()) increment = 0.2;
	if (event.ControlDown()) increment = 1.0;
	if (event.GetWheelRotation() > 0)
		scale += increment;
	else
		scale -= increment;
	Refresh();
	
}


void myToneCurvePane::mouseDoubleClicked(wxMouseEvent& event)
{
	event.Skip();
	scale = 0.1;
	Refresh();
}

 
 

