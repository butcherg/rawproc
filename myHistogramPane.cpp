
#include "myHistogramPane.h"

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
	SetInitialSize(wxSize(500,400));
	long hm = 0;
	//rdata = dib.Histogram(CHANNEL_RED, 256, hm);
	rdata = dib.Histogram();
	for (unsigned i = 0; i<256; i++) if (hm < rdata[i]) hm = rdata[i];
	hmax = hm;
	hscale = 256;
	wscale = 1.0;
	xorigin = 0; yorigin = 0;
	MouseX = 0; MouseY=0;

	//wxMessageBox(wxString::Format("scale: %d; max: %ld",hscale, hmax));
	
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
	//dc.SetBackground(*wxMEDIUM_GREY_BRUSH);
	//dc.SetLogicalScale(w/256.0, h/hmax);

	dc.SetTextForeground(wxColour(0,0,0));
	//if (!pressedDown) 
	//	dc.DrawText(wxString::Format("x: %d y: %d",MouseX,MouseY),10,h-20);
	
	dc.SetUserScale(((double) w / (double) 256.0)* wscale, ((double) h/ (double) hmax) * wscale);
	dc.SetDeviceOrigin (xorigin, yorigin);


	dc.DrawLine(0,0,0,hmax);
	dc.DrawLine(0,0,hscale,0);


	dc.SetPen(wxPen(wxColour(255,0,0),1));
	for (int x = 0; x <256-1; x++) {
		//dc.DrawLine(x,dc.DeviceToLogicalY(h)-rdata[x],x+1,dc.DeviceToLogicalY(h)-rdata[x+1]);
		dc.DrawLine(x,hmax-rdata[x],x+1,hmax-rdata[x+1]);
	}

	dc.SetPen(wxPen(wxColour(192,192,192),1));
	if (!pressedDown) {
		dc.DrawLine(dc.DeviceToLogicalX(MouseX),0,dc.DeviceToLogicalX(MouseX),hmax);
	}
	hx = dc.DeviceToLogicalX(MouseX);

	dc.SetUserScale(1.0, 1.0);
	dc.SetDeviceOrigin (0, 0);
	dc.DrawText(wxString::Format("x: %d",hx),10,h-20);
}

void myHistogramPane::mouseWheelMoved(wxMouseEvent& event) 
{
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
	switch (event.GetKeyCode()) {
		case 116: //t
		case 84: //T - toggle display thumbnail
			wscale = 1.0;
			xorigin = 0;
			yorigin = 0;
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
 
 

