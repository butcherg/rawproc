
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


myHistogramPane::myHistogramPane(wxDialog* parent, gImage &dib, const wxPoint &pos, const wxSize &size) :
 wxWindow(parent, wxID_ANY, pos, size)
{
	SetDoubleBuffered(true);
	//t = new wxTimer(this);
	unsigned hm = 0;
	wscale = 1.0;
	xorigin = 0;
	yorigin = 0;
	hmax = 0;
	hscale = 0;
	ord = 1;
	

	
	rdata = dib.Histogram(CHANNEL_RED, hm);
	if (hmax < hm) hmax = hm;
	if (hscale < rdata.size()) hscale = rdata.size();
	rlen=rdata.size();
	r = new wxPoint[rlen];
	for (unsigned i=0; i<rlen; i++) r[i] = wxPoint(i,rdata[i]);

	gdata = dib.Histogram(CHANNEL_GREEN, hm);
	if (hmax < hm) hmax = hm;
	if (hscale < gdata.size()) hscale = gdata.size();
	glen=gdata.size();
	g = new wxPoint[glen];
	for (unsigned i=0; i<rlen; i++) g[i] = wxPoint(i,gdata[i]);


	bdata = dib.Histogram(CHANNEL_BLUE, hm);
	if (hmax < hm) hmax = hm;
	if (hscale < bdata.size()) hscale = bdata.size();
	blen=bdata.size();
	b = new wxPoint[blen];
	for (unsigned i=0; i<blen; i++) b[i] = wxPoint(i,bdata[i]);

	MouseX = 0; MouseY=0;
	SetInitialSize(wxSize(500,400));
	pressedDown = false;

	Update();
	Refresh();
}

myHistogramPane::~myHistogramPane()
{
	if (r) delete[] r;
	if (g) delete[] g;
	if (b) delete[] b;
}

void myHistogramPane::OnSize(wxSizeEvent& event) 
{
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
	dc.SetBackground(*wxWHITE_BRUSH);
	dc.Clear();
	
	//go to histogram coordinates:
	dc.SetLogicalScale(((double) w / (double) hscale)* wscale, ((double) h/ (double) hmax) * wscale);
	dc.SetDeviceOrigin (xorigin, h-yorigin);
	dc.SetAxisOrientation(true,true);

	wxPoint * frontcolor;
	
	unsigned order = ord;
	for (unsigned i=0; i<3; i++) {
		if (order == 1) {
			dc.SetPen(wxPen(wxColour(255,0,0),1));
			dc.DrawLines(rlen,r,0,0);
			if (i==2) frontcolor = r;
		}
		if (order == 2) {
			dc.SetPen(wxPen(wxColour(0,255,0),1));
			dc.DrawLines(glen,g,0,0);
			if (i==2) frontcolor = g;
		}
		if (order == 3) {
			dc.SetPen(wxPen(wxColour(0,0,255),1));
			dc.DrawLines(blen,b,0,0);
			if (i==2) frontcolor = b;
		}
		order++;
		if (order>3) order=1;
	}

	//vertical marker line:
	dc.SetPen(wxPen(wxColour(192,192,192),1));
	unsigned mlx = dc.DeviceToLogicalX(wxCoord(MouseX));
	unsigned mly = wxCoord(frontcolor[mlx].y);
	
	dc.DrawLine(0,mly,hscale,mly);

	//return to window coords:
	dc.SetLogicalScale(1.0, 1.0);
	dc.SetDeviceOrigin (0, 0);
	dc.SetAxisOrientation(true,false);

	dc.SetTextBackground(wxColour(255,255,255));
	if (!pressedDown) { // & mlx > 0 & mlx < hscale) 
		dc.DrawLine(MouseX,0,MouseX,h);
		dc.DrawText(wxString::Format("x: %d y: %d      hscale=%d  wscale=%f",mlx,mly,hscale,wscale),10,h-20);
	}
}


void myHistogramPane::mouseWheelMoved(wxMouseEvent& event) 
{
	xcenter = event.m_x; ycenter = event.m_y;
	double inc = 0.1;
	if (event.ShiftDown()) inc = 1.0;
	if (event.GetWheelRotation() > 0) { 
		wscale += inc;
	}
	else {
		wscale -= inc;
	}
	if (wscale < 1.0) wscale = 1.0;

	Update();
	Refresh();
}

void myHistogramPane::keyPressed(wxKeyEvent& event) 
{
	//wxMessageBox(wxString::Format("keycode: %d", event.GetKeyCode()));
	switch (event.GetKeyCode()) {
		case 116: //T
		case 84: //t
			wscale = 1.0;
			xorigin = 0;
			yorigin = 0;
			break;
		case WXK_SPACE : // s?
			if (ord == 1) ord = 2;
			else if(ord == 2) ord = 3;
			else if(ord == 3) ord = 1;
			break;
		case 82: //r - pan right
			if (event.ShiftDown()) xorigin -= 10;
			else if (event.ControlDown()) xorigin -= 100;
			else xorigin -= 1;
			break;
		case 76: //l - pan left
			if (event.ShiftDown()) xorigin += 10;
			else if (event.ControlDown()) xorigin += 100;
			else xorigin += 1;
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
		if (event.ShiftDown())   if (dx > dy) dx *= 10;  else dy *= 10;
		if (event.ControlDown()) if (dx > dy) dx *= 100; else dy *= 100;
		xorigin -= dx; yorigin += dy;
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
 
 

