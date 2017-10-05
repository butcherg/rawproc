

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
		const wxString& name): wxControl(parent, id, pos, wxSize(maxValue-minValue+(SLIDER_MARGIN*2), 40), wxBORDER_NONE)
{
	SetBackgroundColour(parent->GetBackgroundColour());
	SetDoubleBuffered(true);
	leftval = leftValue; rightval = rightValue; minval = minValue; maxval = maxValue;
	selectedslider = 0;
	Bind(wxEVT_PAINT, &myDoubleSlider::OnPaint, this);
	Bind(wxEVT_LEFT_DOWN,&myDoubleSlider::OnLeftDown, this);
	Bind(wxEVT_MOTION,&myDoubleSlider::OnMotion, this);
	Bind(wxEVT_LEFT_UP,&myDoubleSlider::OnLeftUp, this);
	Bind(wxEVT_LEAVE_WINDOW, &myDoubleSlider::OnLeftUp, this);
	Bind(wxEVT_MOUSEWHEEL, &myDoubleSlider::OnWheel, this);
	Refresh();
	Update();
}

int myDoubleSlider::GetLeftValue()
{
	return leftval;
}

int myDoubleSlider::GetRightValue()
{
	return rightval;
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
	//dc.DrawLine(SLIDER_MARGIN,h/2, w-SLIDER_MARGIN, h/2);
	
	dc.SetPen(*wxLIGHT_GREY_PEN);
	dc.DrawLine(SLIDER_MARGIN,h/2-1, w-SLIDER_MARGIN+1, h/2-1);
	dc.DrawLine(SLIDER_MARGIN,h/2+1, w-SLIDER_MARGIN+1, h/2+1);
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawLine(SLIDER_MARGIN,h/2, w-SLIDER_MARGIN, h/2);
	
	//left slider:
	dc.DrawLine(SLIDER_MARGIN+leftval, h*0.8, SLIDER_MARGIN+leftval, h*0.5);
	dc.GetTextExtent(wxString::Format("%d",leftval), &tw, &th);
	//dc.DrawText(wxString::Format("%d",leftval),SLIDER_MARGIN+leftval-(tw+2), h*0.25);
	dc.DrawText(wxString::Format("%d",leftval),SLIDER_MARGIN+leftval+7, h*0.5+2);
	dc.SetPen(*wxGREY_PEN);
	dc.DrawCircle(SLIDER_MARGIN+leftval+1, h*0.8+1, 5);
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawCircle(SLIDER_MARGIN+leftval, h*0.8, 5);
	
	//right slider:
	dc.DrawLine(SLIDER_MARGIN+rightval, h*0.5, SLIDER_MARGIN+rightval, h*0.2);
	dc.GetTextExtent(wxString::Format("%d",rightval), &tw, &th);
	//dc.DrawText(wxString::Format("%d",rightval),SLIDER_MARGIN+rightval+2, h*0.75-(th-2));
	dc.DrawText(wxString::Format("%d",rightval),SLIDER_MARGIN+rightval-tw-6, h*0.5-th);
	dc.SetPen(*wxGREY_PEN);
	dc.DrawCircle(SLIDER_MARGIN+rightval+1, h*0.2+1, 5);
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawCircle(SLIDER_MARGIN+rightval, h*0.2, 5);
	
	//dc.DrawText("foo",10,10);
}


/*
void myDoubleSlider::render(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);
	wxCoord tw, th;

	dc.SetPen(*wxLIGHT_GREY_PEN);
	dc.DrawLine(SLIDER_MARGIN,h/2-1, w-SLIDER_MARGIN+1, h/2-1);
	dc.DrawLine(SLIDER_MARGIN,h/2+1, w-SLIDER_MARGIN+1, h/2+1);
	dc.SetPen(*wxBLACK_PEN);
	dc.DrawLine(SLIDER_MARGIN,h/2, w-SLIDER_MARGIN, h/2);

	//left slider:
	dc.GetTextExtent(wxString::Format("%d",leftval), &tw, &th);
	dc.SetPen(*wxMEDIUM_GREY_PEN);
	dc.DrawLine(SLIDER_MARGIN+leftval,   h*0.5,   SLIDER_MARGIN+leftval,   h*0.5+th);
	dc.DrawLine(SLIDER_MARGIN+leftval,   h*0.5,   SLIDER_MARGIN+leftval+6, h*0.5+6);

	dc.SetPen(*wxGREY_PEN);	
	dc.DrawLine(SLIDER_MARGIN+leftval+1, h*0.5+2, SLIDER_MARGIN+leftval+1, h*0.5+th);
	dc.DrawLine(SLIDER_MARGIN+leftval+1, h*0.5+2, SLIDER_MARGIN+leftval+6, h*0.5+7);
	dc.SetPen(*wxLIGHT_GREY_PEN);
	dc.DrawLine(SLIDER_MARGIN+leftval+2, h*0.5+4, SLIDER_MARGIN+leftval+6, h*0.5+8);

	//dc.SetPen(*wxLIGHT_GREY_PEN);
	dc.SetPen(wxColour(192,192,192));
	dc.DrawLine(SLIDER_MARGIN+leftval+2, h*0.5+4, SLIDER_MARGIN+leftval+2, h*0.5+th);
	dc.DrawLine(SLIDER_MARGIN+leftval+3, h*0.5+5, SLIDER_MARGIN+leftval+3, h*0.5+th);
	dc.DrawLine(SLIDER_MARGIN+leftval+4, h*0.5+6, SLIDER_MARGIN+leftval+4, h*0.5+th);
	dc.DrawLine(SLIDER_MARGIN+leftval+5, h*0.5+7, SLIDER_MARGIN+leftval+5, h*0.5+th);

	dc.SetPen(*wxBLACK_PEN);
	dc.DrawText(wxString::Format("%d",leftval),SLIDER_MARGIN+leftval+7, h*0.5+2);
	//dc.DrawCircle(SLIDER_MARGIN+leftval, h*0.75, 5);

	//right slider:
	dc.GetTextExtent(wxString::Format("%d",rightval), &tw, &th);
	dc.DrawLine(SLIDER_MARGIN+rightval, h*0.5-th, SLIDER_MARGIN+rightval, h*0.5);
	dc.DrawText(wxString::Format("%d",rightval),SLIDER_MARGIN+rightval-tw-2, h*0.5-th);
	//dc.DrawCircle(SLIDER_MARGIN+rightval, h*0.25, 5);
	
}
*/

void myDoubleSlider::OnLeftDown(wxMouseEvent& event)
{
	int w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);
	pos.x = pos.x-SLIDER_MARGIN;
	pos.y = h-SLIDER_MARGIN-pos.y;
	if (pos.y < h/2) selectedslider = 1;
	if (pos.y > h/2) selectedslider = 2;
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
		if (selectedslider == 1) {
			leftval -= m;
			if (leftval < minval)  leftval = minval;
			if (leftval >= rightval) leftval = rightval-1;
		}
		else if (selectedslider == 2) {
			rightval -= m;
			if (rightval > maxval) rightval = maxval;
			if (rightval <= leftval) rightval = leftval+1;
		}
		//if (leftval < minval)  leftval = minval;
		//if (leftval >= rightval) leftval = rightval-1;
		//if (rightval > maxval) rightval = maxval;
		//if (rightval <= leftval) rightval = leftval+1;
		prevx = pos.x;
		prevy = pos.y;
		Refresh();
		Update();
	}
	event.Skip();
}

void myDoubleSlider::OnLeftUp(wxMouseEvent& event)
{
	event.Skip();
	if (selectedslider != 0) {
		selectedslider = 0;
		wxCommandEvent e(wxEVT_SCROLL_CHANGED);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
	}
}

void myDoubleSlider::OnWheel(wxMouseEvent& event)
{
	int m, w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	wxPoint pos = event.GetLogicalPosition(dc);
	pos.x = pos.x-SLIDER_MARGIN;
	pos.y = h-SLIDER_MARGIN-pos.y;
	if (event.GetWheelRotation() > 0)
		m=1;
	else
		m=-1;
	if (pos.y < h/2) {
		leftval -= m;
		if (leftval < minval)  leftval = minval;
		if (leftval >= rightval) leftval = rightval-1;
	}
	else if (pos.y > h/2) {
		rightval -= m;
		if (rightval > maxval) rightval = maxval;
		if (rightval <= leftval) rightval = leftval+1;
	}
	Refresh();
	Update();
	wxCommandEvent e(wxEVT_SCROLL_CHANGED);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
	event.Skip();
}










