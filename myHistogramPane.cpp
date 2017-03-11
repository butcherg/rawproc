
#include "myHistogramPane.h"
#include <math.h>
#include <algorithm>

BEGIN_EVENT_TABLE(myHistogramPane, wxWindow)
 
    EVT_MOTION(myHistogramPane::mouseMoved)
    EVT_LEFT_DOWN(myHistogramPane::mouseDown)
    EVT_LEFT_UP(myHistogramPane::mouseReleased)
    EVT_LEFT_DCLICK(myHistogramPane::mouseDoubleClicked)
    EVT_RIGHT_DOWN(myHistogramPane::rightClick)
    EVT_LEAVE_WINDOW(myHistogramPane::mouseLeftWindow)
    EVT_KEY_DOWN(myHistogramPane::keyPressed)
    EVT_KEY_UP(myHistogramPane::keyReleased)
    EVT_MOUSEWHEEL(myHistogramPane::mouseWheelMoved)
    EVT_SIZE(myHistogramPane::OnSize)
 
    // catch paint events
    EVT_PAINT(myHistogramPane::paintEvent)
 
END_EVENT_TABLE()
 

myHistogramPane::myHistogramPane(wxDialog* parent, gImage dib, const wxPoint &pos, const wxSize &size) :
 wxPanel(parent, wxID_ANY, pos, size)
{
	SetDoubleBuffered(true);

	unsigned hm = 0;
	hmax = 0;
	hscale = 0;
	rdata = dib.Histogram(CHANNEL_RED, hm);
	if (hmax < hm) hmax = hm;
	if (hscale < rdata.size()) hscale = rdata.size();

	gdata = dib.Histogram(CHANNEL_GREEN, hm);
	if (hmax < hm) hmax = hm;
	if (hscale < gdata.size()) hscale = gdata.size();

	bdata = dib.Histogram(CHANNEL_BLUE, hm);
	if (hmax < hm) hmax = hm;
	if (hscale < bdata.size()) hscale = bdata.size();

	ord = 1;

	wscale = 1.0;
	xorigin = 0; yorigin = 0;
	MouseX = 0; MouseY=0;

	//wxMessageBox(wxString::Format("scale: %d; max: %ld",hscale, hmax));
	SetInitialSize(wxSize(500,400));
	pressedDown = false;
	paintNow();
}

void myHistogramPane::OnSize(wxSizeEvent& event) 
{
	//SetSize(GetParent()->GetSize());
	Update();
	Refresh();
}

 
void myHistogramPane::paintEvent(wxPaintEvent & evt)
{
	wxPaintDC dc(this);
	render(dc);
}
 
void myHistogramPane::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}
 
void myHistogramPane::render(wxDC&  dc)
{
	int w, h;
	int hx;
	GetSize(&w, &h);
	dc.Clear();

	dc.DrawRectangle(0,0,w,h);

	//dc.SetTextForeground(wxColour(0,0,0));
	
	//go to histogram coordinates:
	dc.SetUserScale(((double) w / (double) hscale)* wscale, ((double) h/ (double) hmax) * wscale);
	dc.SetDeviceOrigin (xorigin, yorigin);

	unsigned order = ord;
	for (unsigned i=0; i<3; i++) {
		//red histogram line:
		if (order == 1) {
			dc.SetPen(wxPen(wxColour(255,0,0),1));
			for (int x = 0; x <rdata.size()-1; x++) {
				dc.DrawLine(x,hmax-rdata[x],x+1,hmax-rdata[x+1]);
			}
		}

		if (order == 2) {
			dc.SetPen(wxPen(wxColour(0,255,0),1));
			for (int x = 0; x <gdata.size()-1; x++) {
				dc.DrawLine(x,hmax-gdata[x],x+1,hmax-gdata[x+1]);
			}
		}

		if (order == 3) {
			dc.SetPen(wxPen(wxColour(0,0,255),1));
			for (int x = 0; x <bdata.size()-1; x++) {
				dc.DrawLine(x,hmax-bdata[x],x+1,hmax-bdata[x+1]);
			}
		}

		order++;
		if (order > 3) order = 1;
	}

	//vertical marker line:
	dc.SetPen(wxPen(wxColour(192,192,192),1));
	unsigned mlx = fmin(fmax(dc.DeviceToLogicalX(MouseX),0.0),hscale);
	
	if (!pressedDown) {
		dc.DrawLine(mlx,0,mlx,hmax);
	}

	//vertical marker value:
	hx = dc.DeviceToLogicalX(MouseX);

	//return to window coords:
	dc.SetUserScale(1.0, 1.0);
	dc.SetDeviceOrigin (0, 0);

	if (!pressedDown & mlx > 0 & mlx < hscale) 
		dc.DrawText(wxString::Format("x: %d",hx),10,h-20);

}

void myHistogramPane::mouseWheelMoved(wxMouseEvent& event) 
{
	xcenter = event.m_x; ycenter = event.m_y;
	if (event.GetWheelRotation() > 0) { 
		wscale += 0.1;
	}
	else {
		wscale -= 0.1;
	}
	Update();
	Refresh();
}

void myHistogramPane::keyPressed(wxKeyEvent& event) 
{
	//printf("keycode: %d\n", event.GetKeyCode());
	switch (event.GetKeyCode()) {
		case 116: //t
		case 84: //T - toggle display thumbnail
			wscale = 1.0;
			xorigin = 0;
			yorigin = 0;
			break;
		case 9: //tab
		case 115: // s?
			if (ord == 1) ord = 2;
			else if(ord == 2) ord = 3;
			else if(ord == 3) ord = 1;
			break;
	}
	Update();
	Refresh();
}
 
void myHistogramPane::mouseDown(wxMouseEvent& event) 
{
	pressedDown = true;
	MouseX = event.m_x;
	MouseY = event.m_y;
	Update();
	Refresh();
}

void myHistogramPane::mouseMoved(wxMouseEvent& event)  
{
	unsigned x, y, dx, dy;
	x=event.m_x; y=event.m_y;
	if (pressedDown) {
		dx = MouseX-x;
		dy = MouseY-y;
		xorigin -= dx; yorigin -= dy;
	}
	MouseX = x;
	MouseY = y;
	Update();
	Refresh();
}

void myHistogramPane::mouseReleased(wxMouseEvent& event) 
{
	pressedDown = false;
	Update();
	Refresh();
}

void myHistogramPane::mouseDoubleClicked(wxMouseEvent& event)
{
	wscale = 1.0;
	xorigin = 0;
	yorigin = 0;
	Update();
	Refresh();
}



void myHistogramPane::mouseLeftWindow(wxMouseEvent& event) {}
void myHistogramPane::rightClick(wxMouseEvent& event) {}

void myHistogramPane::keyReleased(wxKeyEvent& event) {}
 
 

