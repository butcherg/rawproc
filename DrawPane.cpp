#include "DrawPane.h"

#include <wx/dnd.h> 
#include <wx/dcbuffer.h> 
#include <string>
#include <math.h>


//BEGIN_EVENT_TABLE(DrawPane, wxPanel)
BEGIN_EVENT_TABLE(DrawPane, wxControl)
// some useful events
/*
 
 EVT_RIGHT_DOWN(BasicDrawPane::rightClick)
 EVT_LEAVE_WINDOW(BasicDrawPane::mouseLeftWindow)
 
 EVT_KEY_UP(BasicDrawPane::keyReleased)
 
 */
 
// catch paint events
EVT_MOTION(DrawPane::mouseMoved)
EVT_LEFT_DOWN(DrawPane::mouseDown)
EVT_LEFT_UP(DrawPane::mouseReleased)
EVT_LEFT_DCLICK(DrawPane::mouseDclick)
EVT_PAINT(DrawPane::paintEvent)
EVT_SIZE(DrawPane::OnSize)
EVT_KEY_DOWN(DrawPane::keyPressed)
EVT_MOUSEWHEEL(DrawPane::mouseWheelMoved)
 
END_EVENT_TABLE()

DrawPane::DrawPane(wxPanel* parent,  wxString controlpoints,  wxWindowID id, const wxPoint &pos, const wxSize &size) :
//wxPanel(parent, id, pos, size)
wxControl(parent, id, pos, size)
{
	p = parent;
	ctrlptlanding = 10;
	//SetDropTarget(new MyDropTarget(this));
	z=1;
	mousemotion=false;
	initctrlpts =  controlpoints;
	//initCurve(controlpoints);

	char *x, *y;
	char ctrlpts[1024];
	//wxMessageBox(controlpoints);
	//c.clearpoints();
	strncpy(ctrlpts, controlpoints, 1023);
	x = strtok(ctrlpts,",");
	y = strtok(NULL, ",");
	while ((x!=NULL) & (y!=NULL)) {
		c.insertpoint(atof(x), atof(y));
		x = strtok(NULL, ",");
		y = strtok(NULL, ",");
	}

	selectedCP.x = -1.0;
	selectedCP.y = -1.0;
	c.clampto(0.0,255.0);
}

void DrawPane::initCurve(wxString controlpoints)
{

	char *x, *y;
	char ctrlpts[1024];
	//wxMessageBox(controlpoints);
	c.clearpoints();
	strncpy(ctrlpts, controlpoints, 1023);
	x = strtok(ctrlpts,",");
	y = strtok(NULL, ",");
	while ((x!=NULL) & (y!=NULL)) {
		c.insertpoint(atof(x), atof(y));
		x = strtok(NULL, ",");
		y = strtok(NULL, ",");
	}

	selectedCP.x = -1.0;
	selectedCP.y = -1.0;
	c.clampto(0.0,255.0);

}

void DrawPane::paintEvent(wxPaintEvent & evt)
{
	wxPaintDC dc(this);
	render(dc);
}

void DrawPane::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}

void DrawPane::OnSize(wxSizeEvent & evt)
{
	wxClientDC dc(this);
	render(dc);
}

void DrawPane::mouseMoved(wxMouseEvent& event)
{
	int m=10;
	int w, h;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	if( mousemotion) {
		pos = event.GetLogicalPosition(dc);
		pos.x = pos.x-m;
		pos.y = h-m-pos.y;
		c.deletepoint(mouseCP.x, mouseCP.y);
		if (c.isctrlpoint(pos.x, pos.y, ctrlptlanding)) return;
		c.insertpoint((double) pos.x, (double) pos.y);
		mouseCP.x = (double) pos.x;
		mouseCP.y = (double) pos.y;
		selectedCP = mouseCP;
		Refresh();
	}
	Refresh();
}

void DrawPane::mouseDown(wxMouseEvent& event)
{
	//wxMessageBox("mouse down.");
	int radius = 5;
	int m=10;
	int w, h;
	double x, y;
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	pos = event.GetLogicalPosition(dc);
	pos.x = pos.x-m;
	pos.y = h-pos.y;

	if (c.isctrlpoint(pos.x,pos.y,ctrlptlanding)) {
		cp ctpt = c.getctrlpoint();
		mouseCP.x = ctpt.x;
		mouseCP.y = ctpt.y;
		selectedCP = mouseCP;
		mousemotion=true;
		Refresh();
		return;
	}

	for (x=1.0; x<255.0; x++) {
		y = c.getpoint(x);
		if ((pos.x > x-ctrlptlanding) & (pos.x < x+ctrlptlanding)) {
			if ((pos.y > y-ctrlptlanding) & (pos.y < y+ctrlptlanding)) {
				c.insertpoint(x,y);
				mouseCP.x = x;
				mouseCP.y = y;
				selectedCP = mouseCP;
				mousemotion=true;
				break;
			}
		}
	}
	Refresh();
}

void DrawPane::mouseReleased(wxMouseEvent& event)
{
	mousemotion=false;
	event.Skip();
	Refresh();

	wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
	e->SetString("This is the data");
	wxQueueEvent(p,e);

}

void DrawPane::mouseDclick(wxMouseEvent& event)
{

	initCurve(initctrlpts);
	mousemotion=false;
	event.Skip();
	Refresh();
	wxCommandEvent *e = new wxCommandEvent(wxEVT_SCROLL_THUMBRELEASE);
	e->SetString("This is the data");
	wxQueueEvent(p,e);

}

void DrawPane::mouseWheelMoved(wxMouseEvent& event)
{
	if (event.GetWheelRotation() > 0)
		z++;
	else
		z--;
	if (z<1) z=1;
	Refresh();
}

void DrawPane::keyPressed(wxKeyEvent &event)
{
	//wxMessageBox(wxString::Format("%d",event.GetKeyCode()));
	switch (event.GetKeyCode()) {
		case 127:  //delete?
			c.deletepoint(selectedCP.x, selectedCP.y);
			Refresh();
			break;
	}
}

wxString DrawPane::getControlPoints()
{
	wxString s = "";
	bool first = true;
	vector<cp> pts = c.getControlPoints();
	for (unsigned int i=0; i<pts.size(); i++) {
		if (!first) s.Append(",");
		first=false;
		s.Append(wxString::Format("%f,%f",pts[i].x,pts[i].y));
	}
	return s;
}

wxString DrawPane::getXYPoints()
{
	wxString s = "";
	bool first = true;
	double x=0, y=0;
	for (double x=0.0; x<256.0; x++) {
		if (!first) s.Append(",");
		first=false;
		s.Append(wxString::Format("%f,%f",x,c.getpoint(x)));
	}
	return s;
}

wxString DrawPane::getYPoints()
{
	wxString s = "";
	bool first = true;
	double x=0, y=0;
	for (double x=0.0; x<256.0; x++) {
		if (!first) s.Append("\n");
		first=false;
		s.Append(wxString::Format("%f",c.getpoint(x)));
	}
	return s;
}

vector<BYTE> DrawPane::LookupTable8()
{
	vector<BYTE> v;
	for (double x=0.0; x<256.0; x++) {
		v.push_back((BYTE) round(c.getpoint(x)));
	}
	return v;
}

vector<WORD> DrawPane::LookupTable16()
{
	vector<WORD> v;
	double increment = 256.0 / 65535.0;
	for (double x=0.0; x<256.0; x+increment) {
		v.push_back((WORD) round(c.getpoint(x)));  //needs work to convert to integer 0>x>65535
	}
	return v;
}


void DrawPane::render(wxDC&  dc)
{
	double px = 0.0;
	double py = 0.0;
	double x=0, y=0;
	dc.Clear();
	int w, h;
	dc.GetSize(&w, &h);
	int m=10;

	dc.SetPen(*wxGREY_PEN);
	//x axis:
	dc.DrawLine(m,h-m,m,h-m-255);
	//y axis:
	dc.DrawLine(m,h-m,m+255,h-m);
	//null curve:
	dc.DrawLine(m,h-m,m+255,h-m-255);
	dc.SetPen(wxPen (*wxLIGHT_GREY, 1, wxPENSTYLE_DOT_DASH));
	dc.DrawLine(m+128,h-m,m+128,h-m-255);
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
		if ((controlpts[i].x == selectedCP.x) & (controlpts[i].y == selectedCP.y)) dc.SetPen(*wxRED_PEN);
		dc.DrawCircle(m+controlpts[i].x,h-m-controlpts[i].y,ctrlptlanding);
		dc.SetPen(*wxBLACK_PEN);
	}
}

class MyDropTarget : public wxFileDropTarget
{
public:
	MyDropTarget(DrawPane *pane)
	{
		owner = pane;
	}

	bool OnDropFiles (wxCoord x, wxCoord y, const wxArrayString &filenames)
	{
		//owner->loadData((string)filenames[0]);
		return true;
	}
private:
	DrawPane *owner;
};


