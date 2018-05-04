

#include "myDoubleSlider.h"
#include "thumb_up.xpm"
#include "thumb_down.xpm"

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
		const wxString& name): wxControl(parent, id, pos, wxSize(maxValue-minValue+(SLIDER_MARGIN*2), 50), wxBORDER_NONE)
{
	SetBackgroundColour(parent->GetBackgroundColour());
	SetDoubleBuffered(true);
	leftval = leftValue; rightval = rightValue; minval = minValue; maxval = maxValue;
	upthumb = wxBitmap(thumb_up_xpm);
	downthumb = wxBitmap(thumb_down_xpm);
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

void myDoubleSlider::SetLeftValue(int lval)
{
	leftval = lval;
	Refresh();
	Update();
}

void myDoubleSlider::SetRightValue(int rval)
{
	rightval = rval;
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

void  myDoubleSlider::DrawUpThumb(wxDC& dc, wxCoord x, wxCoord y)
{
	dc.SetPen(wxColour("#696969"));
	dc.DrawLine(x+1, y+4, x+6, y-1);
	dc.DrawLine(x+0, y+5, x+0, y+22);
	dc.DrawLine(x+3, y+4, x+8, y+4);

	dc.SetPen(wxColour("#A0A0A0"));
	dc.DrawLine(x+2, y+4, x+6, y+0);
	dc.DrawLine(x+1, y+5, x+1, y+21);

	dc.SetPen(wxColour("#E3E3E3"));
	dc.DrawLine(x+6, y+0, x+10, y+4);
	dc.DrawLine(x+6, y+2, x+9, y+5);
	dc.DrawLine(x+5, y+2, x+7, y+4);
	dc.DrawLine(x+4, y+3, x+6, y+3);
	for (int i=2; i<=9; i++) dc.DrawLine(x+i, y+5, x+i, y+21);

	dc.SetPen(wxColour("#F0F0F0"));
	dc.DrawLine(x+1, y+21, x+11, y+21);
	dc.DrawLine(x+10, y+21, x+10, y+4);
	dc.DrawLine(x+10, y+5, x+5, y+0);
}


void  myDoubleSlider::DrawDownThumb(wxDC& dc, wxCoord x, wxCoord y)
{
	dc.SetPen(wxColour("#696969"));
	dc.DrawLine(x+10, y+16, x+4, y+22);
	dc.DrawLine(x+10, y+0, x+10, y+17);
	dc.DrawLine(x+3, y+17, x+8, y+17);

	dc.SetPen(wxColour("#A0A0A0"));
	dc.DrawLine(x+5, y+20, x+10, y+15);
	dc.DrawLine(x+9, y+1, x+9, y+17);

	dc.SetPen(wxColour("#E3E3E3"));
	dc.DrawLine(x+4, y+21, x+0, y+17);
	dc.DrawLine(x+5, y+20, x+0, y+15);
	dc.DrawLine(x+5, y+19, x+3, y+17);
	dc.DrawLine(x+5, y+18, x+7, y+18);
	for (int i=1; i<=8; i++) dc.DrawLine(x+i, y+1, x+i, y+17);

	dc.SetPen(wxColour("#F0F0F0"));
	dc.DrawLine(x+0, y+0, x+10, y+0);
	dc.DrawLine(x+0, y+1, x+0, y+17);
	dc.DrawLine(x+1, y+17, x+5, y+21);

}



void myDoubleSlider::render(wxDC& dc)
{
	int w, h;
	GetSize(&w, &h);
	wxCoord tw, th;
	
	dc.SetPen(*wxMEDIUM_GREY_PEN);
	dc.DrawLine(SLIDER_MARGIN,h/2-1, w-SLIDER_MARGIN+1, h/2-1);
	dc.SetPen(*wxGREY_PEN);
	dc.DrawLine(SLIDER_MARGIN,h/2,   w-SLIDER_MARGIN+1, h/2);
	dc.SetPen(*wxLIGHT_GREY_PEN);
	dc.DrawLine(SLIDER_MARGIN,h/2+1, w-SLIDER_MARGIN+1, h/2+1);
	

	//left slider:
	dc.GetTextExtent(wxString::Format("%d",leftval), &tw, &th);
	DrawUpThumb(dc,SLIDER_MARGIN+leftval-4, h*0.5+2);
	dc.DrawText(wxString::Format("%d",leftval),SLIDER_MARGIN+leftval+8, h*0.5+5);
	
	//right slider:
	dc.GetTextExtent(wxString::Format("%d",rightval), &tw, &th);
	DrawDownThumb(dc,SLIDER_MARGIN+rightval-4, h*0.5-th-8);
	dc.DrawText(wxString::Format("%d",rightval),SLIDER_MARGIN+rightval-tw-7, h*0.5-th-4);
}


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










