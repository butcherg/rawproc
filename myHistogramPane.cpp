
#include "myHistogramPane.h"
#include <math.h>
#include <algorithm>
#include <wx/clipbrd.h>


myHistogramPane::myHistogramPane(wxWindow* parent, const wxPoint &pos, const wxSize &size) :
 wxWindow(parent, wxID_ANY, pos, size, wxBORDER_SUNKEN)
{
	blankpic = true;
	SetDoubleBuffered(true);
	//t = new wxTimer(this);
	unsigned hm = 0;
	wscale = 1.0;
	xorigin = 0;
	yorigin = 0;
	hmax = 0;
	hscale = 0;
	ord = 1;
	
	r = NULL; g = NULL; b = NULL;
	rlen=0; glen=0; blen=0;
		
	MouseX = 0; MouseY=0;
	pressedDown = false;
	inwindow = false;

	Bind(wxEVT_MOTION, &myHistogramPane::mouseMoved, this);
	Bind(wxEVT_LEFT_DOWN, &myHistogramPane::mouseDown, this);
	Bind(wxEVT_LEFT_UP, &myHistogramPane::mouseReleased, this);
	Bind(wxEVT_LEFT_DCLICK, &myHistogramPane::mouseDoubleClicked, this);
	Bind(wxEVT_RIGHT_DOWN, &myHistogramPane::rightClick, this);
	Bind(wxEVT_ENTER_WINDOW, &myHistogramPane::mouseEnterWindow, this);
	Bind(wxEVT_LEAVE_WINDOW, &myHistogramPane::mouseLeftWindow, this);
        //EVT_CHAR(myHistogramPane::keyPressed)
        Bind(wxEVT_KEY_DOWN, &myHistogramPane::keyPressed, this);
        //EVT_KEY_UP(myHistogramPane::keyReleased)
        Bind(wxEVT_MOUSEWHEEL, &myHistogramPane::mouseWheelMoved, this);
        Bind(wxEVT_SIZE, &myHistogramPane::OnSize, this);

    	Bind(wxEVT_PAINT, &myHistogramPane::paintEvent, this);

}

myHistogramPane::myHistogramPane(wxWindow* parent, gImage &dib, const wxPoint &pos, const wxSize &size) :
 wxWindow(parent, wxID_ANY, pos, size, wxBORDER_SUNKEN)
{
	blankpic = false;
	SetDoubleBuffered(true);
	//t = new wxTimer(this);
	unsigned hm = 0;
	wscale = 1.0;
	xorigin = 0;
	yorigin = 0;
	hmax = 0;
	hscale = 0;
	ord = 1;

	//not needed, for now; renders 'c' key command inop
	//smalldata = dib.Histogram();
	
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
	inwindow = false;

	Bind(wxEVT_MOTION, &myHistogramPane::mouseMoved, this);
	Bind(wxEVT_LEFT_DOWN, &myHistogramPane::mouseDown, this);
	Bind(wxEVT_LEFT_UP, &myHistogramPane::mouseReleased, this);
	Bind(wxEVT_LEFT_DCLICK, &myHistogramPane::mouseDoubleClicked, this);
	Bind(wxEVT_RIGHT_DOWN, &myHistogramPane::rightClick, this);
	Bind(wxEVT_ENTER_WINDOW, &myHistogramPane::mouseEnterWindow, this);
	Bind(wxEVT_LEAVE_WINDOW, &myHistogramPane::mouseLeftWindow, this);
        //EVT_CHAR(myHistogramPane::keyPressed)
        Bind(wxEVT_KEY_DOWN, &myHistogramPane::keyPressed, this);
        //EVT_KEY_UP(myHistogramPane::keyReleased)
        Bind(wxEVT_MOUSEWHEEL, &myHistogramPane::mouseWheelMoved, this);
        Bind(wxEVT_SIZE, &myHistogramPane::OnSize, this);

    	Bind(wxEVT_PAINT, &myHistogramPane::paintEvent, this);


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

void myHistogramPane::BlankPic()
{
	blankpic = true;
	Refresh();
	Update();
}

void myHistogramPane::SetPic(gImage &dib, unsigned scale)
{	
	blankpic = false;
	hmax = 0;
	hscale = scale;
	rlen=scale; glen=scale; blen=scale;
	
	std::vector<histogramdata> histogram = dib.Histogram(scale);
	
	if (r) delete r;
	if (g) delete g;
	if (b) delete b;
	r = new wxPoint[scale];
	g = new wxPoint[scale];
	b = new wxPoint[scale];
	
	unsigned lower = scale * 0.05;
	unsigned upper = scale * 0.95;
	
	for (unsigned i=0; i<scale; i++) {
		r[i] = wxPoint(i, histogram[i].r);
		g[i] = wxPoint(i, histogram[i].g);
		b[i] = wxPoint(i, histogram[i].b);
		if (i > lower & i < upper) {
			if (hmax < histogram[i].r) hmax = histogram[i].r;
			if (hmax < histogram[i].g) hmax = histogram[i].g;
			if (hmax < histogram[i].b) hmax = histogram[i].b;
		}
	}
	
	paintNow();

}

 
void myHistogramPane::render(wxDC&  dc)
{
	int w, h;
	int hx;
	GetSize(&w, &h);
	w-=6;
	h-=6;
	//dc.SetBackground(*wxWHITE_BRUSH);
	dc.Clear();
	if (blankpic) return;
	
	//go to histogram coordinates:
	dc.SetLogicalScale(((double) w / (double) hscale)* wscale, ((double) h/ (double) hmax) * wscale);
	dc.SetDeviceOrigin (xorigin, h-yorigin);
	dc.SetAxisOrientation(true,true);

	wxPoint * frontcolor;
	
	unsigned order = ord;
	for (unsigned i=0; i<3; i++) {
		if (order == 1) {
			if (r) {
				dc.SetPen(wxPen(wxColour(255,0,0),1));
				dc.DrawLines(rlen,r,0,0);
				if (i==2) frontcolor = r;
			}
		}
		if (order == 2) {
			if (g) {
				dc.SetPen(wxPen(wxColour(0,255,0),1));
				dc.DrawLines(glen,g,0,0);
				if (i==2) frontcolor = g;
			}
		}
		if (order == 3) {
			if (b) {
				dc.SetPen(wxPen(wxColour(0,0,255),1));
				dc.DrawLines(blen,b,0,0);
				if (i==2) frontcolor = b;
			}
		}
		order++;
		if (order>3) order=1;
	}

	//marker lines:
	dc.SetPen(wxPen(wxColour(192,192,192),1));
	int mlx = dc.DeviceToLogicalX(wxCoord(MouseX));
	
	unsigned mly=0;
	//if (mlx > 0 & mlx < hscale) {
	//	mly = wxCoord(frontcolor[mlx].y);
	//	dc.DrawLine(0,mly,hscale,mly);
	//}

	//return to window coords:
	dc.SetLogicalScale(1.0, 1.0);
	dc.SetDeviceOrigin (0, 0);
	dc.SetAxisOrientation(true,false);

	dc.SetTextBackground(wxColour(255,255,255));
	if (inwindow & !pressedDown & mlx > 0 & mlx < hscale) 
		dc.DrawLine(MouseX,0,MouseX,h);
	if (mlx < 0) mlx = 0;
	if (mlx > hscale) mlx = hscale;
	//wxString str = wxString::Format("x: %d y: %d    hscale=%d",mlx,mly,hscale);
	wxString str = "";
	if (inwindow)
		str = wxString::Format("x: %d    hscale=%d",mlx,hscale);
	else
		str = wxString::Format("hscale=%d",hscale);
	wxSize sz = dc.GetTextExtent(str);
	dc.DrawText(str,w-sz.GetWidth()-3,2);   //h-20);
}


void myHistogramPane::mouseWheelMoved(wxMouseEvent& event) 
{
	xcenter = event.m_x; ycenter = event.m_y;
	double inc = 0.1;
	if (event.ShiftDown()) inc = 1.0;
	if (event.ControlDown()) inc = 10.0;
	if (event.GetWheelRotation() > 0) { 
		wscale += inc;
	}
	else {
		wscale -= inc;
	}
	if (wscale < 1.0) wscale = 1.0;

	Update();
	Refresh();
	event.Skip();
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
		case 67: //c - with Ctrl, copy 256-scale histogram to clipboard
			//wxString histogram = wxString::Format("%d", (int) smalldata[0]);
			//for (int i = 1; i< smalldata.size(); i++) 
			//	histogram.Append(wxString::Format(",%d", (int) smalldata[i]));
			//if (wxTheClipboard->Open())
			//{
			//	wxTheClipboard->SetData( new wxTextDataObject(histogram) );
			//	wxTheClipboard->Close();
			//}
			break;
	}

	Update();
	Refresh();
	event.Skip();
}
 
void myHistogramPane::mouseDown(wxMouseEvent& event) 
{
	pressedDown = true;
	MouseX = event.m_x;
	MouseY = event.m_y;
	Update();
	Refresh();
	event.Skip();
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
	event.Skip();
}

void myHistogramPane::mouseReleased(wxMouseEvent& event) 
{
	pressedDown = false;
	Update();
	Refresh();
	event.Skip();
}

void myHistogramPane::mouseDoubleClicked(wxMouseEvent& event)
{
	wscale = 1.0;
	xorigin = 0;
	yorigin = 0;
	Update();
	Refresh();
	event.Skip();
}


void myHistogramPane::mouseEnterWindow(wxMouseEvent& event) 
{
	inwindow = true;
	Update();
	Refresh();
}

void myHistogramPane::mouseLeftWindow(wxMouseEvent& event) 
{
	inwindow =  false;
	Update();
	Refresh();
}

void myHistogramPane::rightClick(wxMouseEvent& event) {}
void myHistogramPane::keyReleased(wxKeyEvent& event) {}
 
 

