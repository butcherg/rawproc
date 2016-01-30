
#include "wxTouchSlider.h"

BEGIN_EVENT_TABLE(wxTouchSlider, wxWindow)
 
    EVT_MOTION(wxTouchSlider::mouseMoved)
    EVT_LEFT_DOWN(wxTouchSlider::mouseDown)
    EVT_LEFT_UP(wxTouchSlider::mouseReleased)
    EVT_LEFT_DCLICK(wxTouchSlider::mouseDoubleClicked)
    EVT_RIGHT_DOWN(wxTouchSlider::rightClick)
    EVT_LEAVE_WINDOW(wxTouchSlider::mouseLeftWindow)
    EVT_KEY_DOWN(wxTouchSlider::keyPressed)
    EVT_KEY_UP(wxTouchSlider::keyReleased)
    EVT_MOUSEWHEEL(wxTouchSlider::mouseWheelMoved)
 
    // catch paint events
    EVT_PAINT(wxTouchSlider::paintEvent)
 
END_EVENT_TABLE()
 

wxTouchSlider::wxTouchSlider(wxFrame* parent, wxString text, int val, int min, int max) :
 wxWindow(parent, wxID_ANY, wxPoint(0,0), wxSize(-1, max-min+44))
{
	SetMinSize( wxSize(280, max - min + 44) );
	SetSize(parent->GetSize());
	this->text = text;
	pressedDown = false;
	value=val;
	mn = min;
	mx = max;
	paintNow();
}
 
void wxTouchSlider::paintEvent(wxPaintEvent & evt)
{
	wxPaintDC dc(this);
	render(dc);
}
 
void wxTouchSlider::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}
 
void wxTouchSlider::render(wxDC&  dc)
{
	int w, h;
	GetSize(&w, &h);
	dc.Clear();
	wxString v = wxString::Format("%d",value);
	wxSize t = dc.GetTextExtent(v);
	//dc.DrawRoundedRectangle(0,(h/2)-((mx-mn)/2)-22, w, (mx-mn)+44, 10);
	dc.DrawRoundedRectangle( 2, (h/2)-value-20, w-4, 40, 10 );
	dc.DrawText(v, w/2-t.GetWidth()/2, (h/2)-value-8 );
}
 
void wxTouchSlider::mouseDown(wxMouseEvent& event)
{
	px = event.GetX();
	py = event.GetY();
	pressedDown = true;
	paintNow();
	event.Skip();
}

void wxTouchSlider::mouseMoved(wxMouseEvent& event) 
{
	if (pressedDown) {
		value += py - event.GetY();
		if (value<mn) value=mn;
		if (value>mx) value=mx;
		py = event.GetY();
		paintNow();
	}
	event.Skip();
}

void wxTouchSlider::mouseReleased(wxMouseEvent& event)
{
	pressedDown = false;
	paintNow();
	wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
	e->SetInt(value);
	wxQueueEvent(GetParent(),e);
	event.Skip();
}

void wxTouchSlider::mouseLeftWindow(wxMouseEvent& event)
{
	if (pressedDown)
	{
		pressedDown = false;
		paintNow();
	}
	event.Skip();
}
 

void wxTouchSlider::mouseDoubleClicked(wxMouseEvent& event)
{
	pressedDown = false;
	value = 0;
	paintNow();
	wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
	e->SetInt(value);
	wxQueueEvent(GetParent(),e);
	event.Skip();
}

void wxTouchSlider::mouseWheelMoved(wxMouseEvent& event) 
{
	if (event.GetWheelRotation() > 0)
		value++;
	else
		value--;
	if (value<mn) value=mn;
	if (value>mx) value=mx;
	py = event.GetY();
	paintNow();
	wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
	e->SetInt(value);
	wxQueueEvent(GetParent(),e);
	event.Skip();
}

void wxTouchSlider::rightClick(wxMouseEvent& event) {}
void wxTouchSlider::keyPressed(wxKeyEvent& event) {}
void wxTouchSlider::keyReleased(wxKeyEvent& event) {}
 
 

