#include "CurvePane.h"

#include <wx/dnd.h> 
#include <wx/dcbuffer.h> 
#include <string>
#include "util.h"

#define radius 10


BEGIN_EVENT_TABLE(CurvePane, wxPanel)
// some useful events
/*
 EVT_LEAVE_WINDOW(BasicCurvePane::mouseLeftWindow)
 
 EVT_KEY_UP(BasicCurvePane::keyReleased)
 
 */
 
// catch paint events
EVT_MOTION(CurvePane::mouseMoved)
EVT_LEFT_DOWN(CurvePane::mouseLeftDown)
//EVT_RIGHT_DOWN(CurvePane::mouseRightDown)
EVT_LEFT_DCLICK(CurvePane::mouseDclick)
EVT_LEFT_UP(CurvePane::mouseReleased)
EVT_PAINT(CurvePane::paintEvent)
EVT_SIZE(CurvePane::OnSize)
EVT_KEY_DOWN(CurvePane::keyPressed)
EVT_CHAR(CurvePane::keyPressed)
EVT_MOUSEWHEEL(CurvePane::mouseWheelMoved)
 
END_EVENT_TABLE()

CurvePane::CurvePane(wxWindow* parent, wxString controlpoints) :
wxPanel(parent, wxID_ANY, wxPoint(0,0), wxSize(300,300) )
{
	p = parent;
	z=1;
	mousemotion=false;
	wxArrayString ctrlpts = split(controlpoints,",");
	for (int i=0; i<ctrlpts.GetCount()-1; i+=2) {
		c.insertpoint(atof(ctrlpts[i]), atof(ctrlpts[i+1]));
	}
	selectedCP.x = -1.0;
	selectedCP.y = -1.0;
	c.clampto(0.0,255.0);
	paintNow();
	//Refresh();
	//Update();
}

void CurvePane::paintEvent(wxPaintEvent & evt)
{
	wxPaintDC dc(this);
	//wxBufferedPaintDC bdc(this);
	render(dc);
}

void CurvePane::paintNow()
{
	wxClientDC dc(this);
	//wxBufferedDC bdc(&dc);
	render(dc);
}

void CurvePane::OnSize(wxSizeEvent & evt)
{
	wxClientDC dc(this);
	render(dc);
}

void CurvePane::mouseMoved(wxMouseEvent& event)
{
	int m=10;
	int w, h;
	mousemoved = true;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	if( mousemotion) {
		pos = event.GetLogicalPosition(dc);
		pos.x = pos.x-m;
		pos.y = h-m-pos.y;
		if (selectedCP.x > -1.0) {
			c.deletepoint(selectedCP.x, selectedCP.y);
			selectedCP.x -= mouseCP.x - (double) pos.x;
			selectedCP.y -= mouseCP.y - (double) pos.y;
			if (selectedCP.x < 0.0) selectedCP.x = 0.0; if (selectedCP.x > 255.0) selectedCP.x = 255.0;
			if (selectedCP.y < 0.0) selectedCP.y = 0.0; if (selectedCP.y > 255.0) selectedCP.y = 255.0;
			c.insertpoint((double) selectedCP.x, (double) selectedCP.y);
		}
		mouseCP.x = (double) pos.x;
		mouseCP.y = (double) pos.y;
		paintNow();
		
	}
}



void CurvePane::mouseLeftDown(wxMouseEvent& event)
{
	int m=10;
	int w, h;
	double x, y;
	mousemoved = false;
	mousemotion=true;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	pos = event.GetLogicalPosition(dc);
	pos.x = pos.x-m;
	pos.y = h-m-pos.y;

	mouseCP.x = pos.x;
	mouseCP.y = pos.y;

	int pt = c.isctrlpoint(pos.x,pos.y,radius);
	if (pt != -1) {
		selectedCP = c.getctrlpoint(pt);
		paintNow();
		return;
	}

	for (x=1.0; x<255.0; x++) {
		y = c.getpoint(x);
		if ((pos.x > x-radius) & (pos.x < x+radius)) {
			if ((pos.y > y-radius) & (pos.y < y+radius)) {
				c.insertpoint(x,y);
				selectedCP.x = x;
				selectedCP.y = y;
				paintNow();
				return;
			}
		}
	}
}

void CurvePane::mouseReleased(wxMouseEvent& event)
{
	mousemotion=false;
	paintNow();
	if (mousemoved) {
		wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
		e->SetString("This is the data");
		wxQueueEvent(p,e);
	}
}

void CurvePane::mouseRightDown(wxMouseEvent& event)
{
	c.clearpoints();
	c.insertpoint(0,0);
	c.insertpoint(255.0,255.0);
	selectedCP.x = -1.0;
	selectedCP.y = -1.0;
	paintNow();
	wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
	e->SetString("This is the data");
	wxQueueEvent(p,e);
}

void CurvePane::mouseDclick(wxMouseEvent& event)
{
	if (c.isendpoint(selectedCP.x, selectedCP.y, radius)) return;
	c.deletepoint(selectedCP.x, selectedCP.y);
	selectedCP.x = -1.0;
	selectedCP.y = -1.0;
	paintNow();
	wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
	e->SetString("This is the data");
	wxQueueEvent(p,e);
}

void CurvePane::mouseWheelMoved(wxMouseEvent& event)
{
	if (event.GetWheelRotation() > 0)
		z++;
	else
		z--;
	if (z<1) z=1;
	//Refresh();
}

void CurvePane::keyPressed(wxKeyEvent &event)
{
	//wxMessageBox(wxString::Format("%d",event.GetKeyCode()));
	switch (event.GetKeyCode()) {
		case 127:  //delete
		case 8: //Backspace
			c.deletepoint(selectedCP.x, selectedCP.y);
			Refresh();
			wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
			e->SetString("This is the data");
			wxQueueEvent(p,e);
			break;
	}
	event.Skip();
}

wxString CurvePane::getControlPoints()
{
	wxString s = "";
	bool first = true;
	vector<cp> pts = c.getControlPoints();
	for (unsigned int i=0; i<pts.size(); i++) {
		if (!first) s.Append(",");
		first=false;
		s.Append(wxString::Format("%.1f,%.1f",pts[i].x,pts[i].y));
	}
	return s;
}

wxString CurvePane::getXYPoints()
{
	wxString s = "";
	bool first = true;
	double x=0, y=0;
	for (double x=0.0; x<256.0; x++) {
		if (!first) s.Append(",");
		first=false;
		s.Append(wxString::Format("%.1f,%.1f",x,c.getpoint(x)));
	}
	return s;
}

wxString CurvePane::getYPoints()
{
	wxString s = "";
	bool first = true;
	double x=0, y=0;
	for (double x=0.0; x<256.0; x++) {
		if (!first) s.Append("\n");
		first=false;
		s.Append(wxString::Format("%.1f",c.getpoint(x)));
	}
	return s;
}

std::vector<cp> CurvePane::getPoints()
{
	return c.getControlPoints();
}

void CurvePane::setPoints(std::vector<cp> pts)
{
	c.setControlPoints(pts);
	Refresh();
}

void CurvePane::render(wxDC&  dc)
{
	double px = 0.0;
	double py = 0.0;
	double x=0, y=0;
	dc.Clear();
	int w, h;
	dc.GetSize(&w, &h);
	int m=10;

	
	//x axis:
	dc.DrawLine(m,h-m,m,h-m-255);
	//y axis:
	dc.DrawLine(m,h-m,m+255,h-m);

	//center lines:
	dc.SetPen(*wxLIGHT_GREY_PEN);
	dc.DrawLine(m+128,h-m,m+128,h-m-255);
	dc.DrawLine(m,h-m-128,m+255,h-m-128);
	//null curve:
	dc.DrawLine(m,h-m,m+255,h-m-255);
	//quarter lines:
	dc.SetPen(wxPen(wxColour(192,192,192), 1, wxPENSTYLE_DOT_DASH ));
	dc.DrawLine(m+64,h-m,m+64,h-m-255);
	dc.DrawLine(m,h-m-64,m+255,h-m-64);
	dc.DrawLine(m+192,h-m,m+192,h-m-255);
	dc.DrawLine(m,h-m-192,m+255,h-m-192);
	dc.SetPen(*wxBLACK_PEN);

	//double ax = ((double) w/x);
	//double ay = ((double) h/y)*z;


	for (double x=0.0; x<256.0; x++) {
		y=c.getpoint(x);
		if (y>255.0) y = 255.0; if (y<0.0) y=0.0;
		dc.DrawLine(m+px,h-m-py,m+x,h-m-y);
		px = x;
		py = y;
	}

	//if (mousemotion)dc.DrawCircle(pos,5);
	std::vector<cp> controlpts = c.getControlPoints();
	for (unsigned int i=0; i<controlpts.size(); i++) {
		if ((controlpts[i].x == selectedCP.x) & (controlpts[i].y == selectedCP.y)) {
			//dc.DrawText(wxString::Format("%d,%d",selectedCP.x,selectedCP.y),selectedCP.x-20,selectedCP.y-20);
			dc.SetPen(*wxRED_PEN);
		}
		dc.DrawCircle(m+controlpts[i].x,h-m-controlpts[i].y,radius);
		dc.SetPen(*wxBLACK_PEN);
	}
}





void CurvePane::bump(int i)
{
	cp ctpt = c.getctrlpoint(1);
	ctpt.x -= 10;
	c.setctrlpoint(1, ctpt);
	Refresh();
}
