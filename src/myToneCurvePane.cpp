
#include "myToneCurvePane.h"
#include <math.h>
#include <algorithm>
#include <wx/clipbrd.h>
#include "myConfig.h"
#include "util.h"


myToneCurvePane::myToneCurvePane(wxWindow* parent, const wxPoint &pos, const wxSize &size) :
 wxWindow(parent, wxID_ANY, pos, size, wxBORDER_SUNKEN)
{
	scale = 1.0;
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

void myToneCurvePane::SetCurve(std::vector<float> curve, bool rescale)
{	
	int w,h;
	GetSize(&w,&h);
	if (rescale) {
		scale = (float) w / (float) curve.size();
		resetscale = scale;
		scaleincrement = scale / (float) w;
	}
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
	//wxString scaletext = wxString::Format("scale: %0.3f",scale);
	//wxSize ts = dc.GetTextExtent(scaletext);
	//dc.DrawText(scaletext, w-ts.GetWidth()-2, h-ts.GetHeight());
	dc.SetDeviceOrigin (0, h);
	dc.SetAxisOrientation(true,true);
	//dc.SetLogicalScale(scale, resetscale);
	dc.SetLogicalScale(scale, scale);
	wxPoint *cp = new wxPoint[range];
	for (unsigned i=0; i<range; i++) cp[i] = wxPoint(i, int((float) range*c[i]));
	unsigned gridcolor = 192;
	dc.SetPen(wxPen(wxColour(gridcolor,gridcolor,gridcolor),linewidth,wxPENSTYLE_DOT));
	dc.DrawLine(range*0.25,0,range*0.25,range);
	dc.DrawLine(range*0.5,0,range*0.5,range);
	dc.DrawLine(range*0.75,0,range*0.75,range);
	dc.SetPen(wxPen(wxColour(0,0,0),linewidth));
	dc.DrawLines(c.size(),cp,1,1);
	delete [] cp;
}

void myToneCurvePane::mouseWheelMoved(wxMouseEvent& event) 
{	
	double increment = scaleincrement;
	if (event.ShiftDown()) increment *= 10.0;
	if (event.ControlDown()) increment *= 100.0;
	if (event.GetWheelRotation() > 0)
		scale += increment;
	else
		scale -= increment;
	if (scale < resetscale) scale = resetscale;
	Refresh();
	
}


void myToneCurvePane::mouseDoubleClicked(wxMouseEvent& event)
{
	scale = resetscale;
	Refresh();
}

 
 

