

#include "myDoubleSlider.h"

static const int BORDER_THICKNESS = 2;

static const wxCoord SLIDER_MARGIN = 6; // margin around slider
static const wxCoord SLIDER_THUMB_LENGTH = 18;
static const wxCoord SLIDER_TICK_LENGTH = 6;


myDoubleSlider::myDoubleSlider(wxWindow *parent,
		wxWindowID id,
		//const wxString& label,
		int leftValue, int rightValue, int minValue, int maxValue,
		const wxPoint& pos,
		const wxSize& size,
		long style,
		const wxValidator& val,
		const wxString& name): wxControl(parent, id, pos, wxSize(maxValue-minValue+(SLIDER_MARGIN*2), 50), wxBORDER_SIMPLE) //wxBORDER_NONE)
{
	SetBackgroundColour(parent->GetBackgroundColour());
	leftval = leftValue; rightval = rightValue; minval = minValue; maxval = maxValue;
	selectedslider = 0;
	Bind(wxEVT_PAINT, &myDoubleSlider::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN,&myDoubleSlider::OnLeftDown, this);
	Bind(wxEVT_MOTION,&myDoubleSlider::OnMotion, this);
	Bind(wxEVT_LEFT_UP,&myDoubleSlider::OnLeftUp, this);
	Refresh();
	Update();
}

wxSize myDoubleSlider::DoGetBestSize()
{
	return (wxSize(1,1));
}

void myDoubleSlider::OnPaint(wxPaintEvent&)
{
	wxPaintDC dc(this);
	render(dc);
}

void myDoubleSlider::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}

void myDoubleSlider::render(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);
	wxCoord tw, th;
	dc.DrawLine(SLIDER_MARGIN,h/2, w-SLIDER_MARGIN, h/2);
	
	//left slider:
	dc.DrawLine(SLIDER_MARGIN+leftval, h*0.75, SLIDER_MARGIN+leftval, h*0.25);
	dc.GetTextExtent(wxString::Format("%d",leftval), &tw, &th);
	dc.DrawText(wxString::Format("%d",leftval),SLIDER_MARGIN+leftval-(tw+2), h*0.25);
	dc.DrawCircle(SLIDER_MARGIN+leftval, h*0.75, 5);
	
	//right slider:
	dc.DrawLine(SLIDER_MARGIN+rightval, h*0.75, SLIDER_MARGIN+rightval, h*0.25);
	dc.GetTextExtent(wxString::Format("%d",rightval), &tw, &th);
	dc.DrawText(wxString::Format("%d",rightval),SLIDER_MARGIN+rightval+2, h*0.75-(th-2));
	dc.DrawCircle(SLIDER_MARGIN+rightval, h*0.25, 5);
	
	//dc.DrawText("foo",10,10);
}

void myDoubleSlider::OnLeftDown(wxMouseEvent& event)
{
	int w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);
	pos.x = pos.x-SLIDER_MARGIN;
	pos.y = h-SLIDER_MARGIN-pos.y;
	if (abs(pos.x-leftval) < 5)  selectedslider = 1;
	if (abs(pos.x-rightval) < 5) selectedslider = 2;
	prevx = pos.x;
	prevy = pos.y;
	event.Skip();
}

void myDoubleSlider::OnMotion(wxMouseEvent& event)
{
	int w, h;
	if (selectedslider != 0) {
		wxClientDC dc(this);
		dc.GetSize(&w, &h);
		wxPoint pos = event.GetLogicalPosition(dc);
		pos.x = pos.x-SLIDER_MARGIN;
		pos.y = h-SLIDER_MARGIN-pos.y;
		int m = prevx - pos.x;
		if (selectedslider == 1) 
			leftval -= m;
		if (selectedslider == 2)
			rightval -= m;
		if (leftval < minval) leftval = minval; if (leftval >= rightval) leftval = rightval-1;
		if (rightval < minval) rightval = minval; if (rightval <= leftval) rightval = leftval+1;
		prevx = pos.x;
		prevy = pos.y;
		Refresh();
		Update();
	}
	event.Skip();
}

void myDoubleSlider::OnLeftUp(wxMouseEvent& event)
{
	selectedslider = 0;
	event.Skip();
}










