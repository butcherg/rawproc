
#include "myTouchSlider.h"

BEGIN_EVENT_TABLE(myTouchSlider, wxWindow)
 
    EVT_MOTION(myTouchSlider::mouseMoved)
    EVT_LEFT_DOWN(myTouchSlider::mouseDown)
    EVT_LEFT_UP(myTouchSlider::mouseReleased)
    EVT_LEFT_DCLICK(myTouchSlider::mouseDoubleClicked)
    EVT_RIGHT_DOWN(myTouchSlider::rightClick)
    EVT_LEAVE_WINDOW(myTouchSlider::mouseLeftWindow)
    EVT_KEY_DOWN(myTouchSlider::keyPressed)
    EVT_KEY_UP(myTouchSlider::keyReleased)
    EVT_MOUSEWHEEL(myTouchSlider::mouseWheelMoved)
 
    // catch paint events
    EVT_PAINT(myTouchSlider::paintEvent)
 
END_EVENT_TABLE()

 
myTouchSlider::myTouchSlider(wxFrame* parent,  wxWindowID id, wxString label, double initialvalue, double increment, double min, double max, wxString format):
wxWindow(parent, id, wxPoint(0,0), wxSize(-1, int(((max-min)/increment)+44.0)))
{
	vsize = int((max-min)/increment);
	SetMinSize( wxSize(250, vsize+44.0) );
	SetSize(parent->GetSize());
	lbl = label;
	initval = initialvalue;
	val = initialvalue;
	inc = increment;
	mn = min;
	mx = max; 
	fmt = format;
	pressedDown = false;
	paintNow();
}

double myTouchSlider::GetDoubleValue()
{
	return val;
}

int myTouchSlider::GetIntValue()
{
	return (int) val;
}

void myTouchSlider::SetValue(double value)
{
	val = value;
	Refresh();
	Update();
}

void myTouchSlider::paintEvent(wxPaintEvent & evt)
{
	wxPaintDC dc(this);
	render(dc);
}
 
void myTouchSlider::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}
 
void myTouchSlider::render(wxDC&  dc)
{
	int w, h;
	GetSize(&w, &h);
	dc.Clear();
	wxString v = wxString::Format(fmt,val);
	int dcval = mn + (val/inc);
	wxSize t = dc.GetTextExtent(v);
	dc.DrawRoundedRectangle( 2, h-dcval-40, w-4, 40, 10 );
//dc.GradientFillLinear (wxRect(2, h-dcval-40, w-4, 40), *wxBLACK, *wxLIGHT_GREY, wxDOWN);
	dc.DrawText(v, w/2-t.GetWidth()/2, h-dcval+8-40 );
}
 
void myTouchSlider::mouseDown(wxMouseEvent& event)
{
	px = event.GetX();
	py = event.GetY();
	pressedDown = true;
	paintNow();
	//event.Skip();
}

void myTouchSlider::mouseMoved(wxMouseEvent& event) 
{
	if (pressedDown) {
		val += inc * (py - event.GetY());
		if (val<mn) val=mn;
		if (val>mx) val=mx;
		py = event.GetY();
		paintNow();
	}
	//event.Skip();
}

void myTouchSlider::mouseReleased(wxMouseEvent& event)
{
	pressedDown = false;
	paintNow();
	wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
	e->SetString(wxString::Format(fmt, val));
	wxQueueEvent(GetParent(),e);
	//event.Skip();
}

void myTouchSlider::mouseLeftWindow(wxMouseEvent& event)
{
	if (pressedDown)
	{
		pressedDown = false;
		paintNow();
	}
	//event.Skip();
}
 

void myTouchSlider::mouseDoubleClicked(wxMouseEvent& event)
{
	pressedDown = false;
	val = initval;
	paintNow();
	wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
	e->SetString(wxString::Format(fmt, val));
	wxQueueEvent(GetParent(),e);
	//event.Skip();
}

void myTouchSlider::mouseWheelMoved(wxMouseEvent& event) 
{
	if (event.GetWheelRotation() > 0)
		val+=inc;
	else
		val-=inc;
	if (val<mn) val=mn;
	if (val>mx) val=mx;
	py = event.GetY();
	paintNow();
	wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
	e->SetString(wxString::Format(fmt, val));
	wxQueueEvent(GetParent(),e);
	//event.Skip();
}

void myTouchSlider::rightClick(wxMouseEvent& event) {}
void myTouchSlider::keyPressed(wxKeyEvent& event) {}
void myTouchSlider::keyReleased(wxKeyEvent& event) {}
 
 

