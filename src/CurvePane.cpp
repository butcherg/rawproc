#include "CurvePane.h"

#include <wx/dnd.h> 
#include <wx/dcbuffer.h> 
#include <wx/clipbrd.h>
#include <string>

#include "util.h"
#include "myConfig.h"

wxDEFINE_EVENT(myCURVE_UPDATE, wxCommandEvent);
wxDEFINE_EVENT(myCURVE_CHANGE, wxCommandEvent);

/*
BEGIN_EVENT_TABLE(CurvePane, wxPanel)
 
// catch paint events
EVT_MOTION(CurvePane::mouseMoved)
EVT_LEFT_DOWN(CurvePane::mouseLeftDown)
//EVT_RIGHT_DOWN(CurvePane::mouseRightDown)
EVT_LEFT_DCLICK(CurvePane::mouseDclick)
EVT_LEFT_UP(CurvePane::mouseReleased)
EVT_LEAVE_WINDOW(CurvePane::mouseReleased)
EVT_PAINT(CurvePane::paintEvent)
EVT_SIZE(CurvePane::OnSize)
EVT_KEY_DOWN(CurvePane::keyPressed)
EVT_CHAR(CurvePane::keyPressed)
EVT_MOUSEWHEEL(CurvePane::mouseWheelMoved)
 
END_EVENT_TABLE()
*/

CurvePane::CurvePane(wxWindow* parent, wxString controlpoints) :
wxPanel(parent, wxID_ANY, wxPoint(0,0), wxSize(275,275) )
{
	int r,g,b;
	int ctstart;
	p = parent;
	z=1;
	mousemotion=false;
	SetDoubleBuffered(true);

	t = new wxTimer(this);
	Bind(wxEVT_TIMER, &CurvePane::OnTimer,  this);

	SetBackgroundColour(parent->GetBackgroundColour());

	wxArrayString ctrlpts = split(controlpoints,",");

	if ((ctrlpts[0] == "rgb") | (ctrlpts[0] == "red") | (ctrlpts[0] == "green") | (ctrlpts[0] == "blue") | (ctrlpts[0] == "bright")) {
		ctstart = 1;
	}
	else {
		ctstart = 0;
	}

	for (int i=ctstart; i<ctrlpts.GetCount()-1; i+=2) {
		c.insertpoint(atof(ctrlpts[i]), atof(ctrlpts[i+1]));
	}
	selectedCP.x = -1.0;
	selectedCP.y = -1.0;
	//c.clampto(0.0,255.0);

	if (myConfig::getConfig().getValueOrDefault("app.tooltip","0") == "1")
		SetToolTip("Curve Keyboard Commands:\n   t: toggle tooltip\n   del/backspace: delete selected control point\n   ctrl-c: copy curve y data to clipboard");


	Bind(wxEVT_MOTION, &CurvePane::mouseMoved, this);
	Bind(wxEVT_LEFT_DOWN, &CurvePane::mouseLeftDown, this);
	//EVT_RIGHT_DOWN(CurvePane::mouseRightDown, this);
	Bind(wxEVT_LEFT_DCLICK, &CurvePane::mouseDclick, this);
	Bind(wxEVT_LEFT_UP, &CurvePane::mouseReleased, this);
	Bind(wxEVT_LEAVE_WINDOW, &CurvePane::mouseReleased, this);
	Bind(wxEVT_PAINT, &CurvePane::paintEvent, this);
	Bind(wxEVT_SIZE, &CurvePane::OnSize, this);
	Bind(wxEVT_KEY_DOWN, &CurvePane::keyPressed, this);
	Bind(wxEVT_MOUSEWHEEL, &CurvePane::mouseWheelMoved, this);

	paintNow();

}

CurvePane::~CurvePane()
{
	t->~wxTimer();
}

bool CurvePane::ToggleToolTip()
{
	if (GetToolTipText() == "") {
		SetToolTip("Curve Keyboard Commands:\n   t: toggle tooltip\n   del/backspace: delete selected control point\n   ctrl-c: copy curve y data to clipboard");
		return true;
	}
	else {
		UnsetToolTip();
		return false;
	}
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

void CurvePane::OnTimer(wxTimerEvent& event)
{
	wxCommandEvent e(myCURVE_UPDATE);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
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
		wxCommandEvent e(myCURVE_CHANGE);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
	}
}



void CurvePane::mouseLeftDown(wxMouseEvent& event)
{
	SetFocus();
	int m=10;
	int w, h;
	double x, y;
	mousemoved = false;
	mousemotion=true;
	//parm tool.curve.landingradius: radius of control point area sensitive to mouseclicks.  Doesn't have to be the radius of the control point circle.  Default=5
	int landingradius = atoi(myConfig::getConfig().getValueOrDefault("tool.curve.landingradius","5").c_str());
	wxClientDC dc(this);
	dc.GetSize(&w, &h);
	pos = event.GetLogicalPosition(dc);
	pos.x = pos.x-m;
	pos.y = h-m-pos.y;

	mouseCP.x = pos.x;
	mouseCP.y = pos.y;

	int pt = c.isctrlpoint(pos.x,pos.y,landingradius);
	if (pt != -1) {
		selectedCP = c.getctrlpoint(pt);
		paintNow();
		return;
	}

	for (x=1.0; x<255.0; x++) {
		y = c.getpoint(x);
		if ((pos.x > x-landingradius) & (pos.x < x+landingradius)) {
			if ((pos.y > y-landingradius) & (pos.y < y+landingradius)) {
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
	if (mousemotion) {
		mousemotion=false;
		paintNow();
		if (mousemoved) {
			wxCommandEvent e(myCURVE_UPDATE);
			e.SetEventObject(this);
			e.SetString("update");
			ProcessWindowEvent(e);
		}
	}
	event.Skip();
}

void CurvePane::mouseRightDown(wxMouseEvent& event)
{
	c.clearpoints();
	c.insertpoint(0,0);
	c.insertpoint(255.0,255.0);
	selectedCP.x = -1.0;
	selectedCP.y = -1.0;
	paintNow();
	wxCommandEvent e(myCURVE_UPDATE);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
}

void CurvePane::mouseDclick(wxMouseEvent& event)
{

	//parm tool.curve.controlpointradius: Radius of the circle displayed to indicate a control point.  Default=5
	int radius = atoi(myConfig::getConfig().getValueOrDefault("tool.curve.controlpointradius","5").c_str());
	if (c.isendpoint(selectedCP.x, selectedCP.y, radius)) return;
	c.deletepoint(selectedCP.x, selectedCP.y);
	selectedCP.x = -1.0;
	selectedCP.y = -1.0;
	paintNow();
	wxCommandEvent e(myCURVE_UPDATE);
	e.SetEventObject(this);
	e.SetString("update");
	ProcessWindowEvent(e);
}

void CurvePane::mouseWheelMoved(wxMouseEvent& event)
{
	int z;
	if (event.ControlDown() | event.AltDown()) {
		if (event.GetWheelRotation() > 0)
			z=1;
		else
			z=-1;
		if (event.ShiftDown()) z *= 10;
		if (selectedCP.x > -1.0) {
			c.deletepoint(selectedCP.x, selectedCP.y);
			if (event.ControlDown())
				selectedCP.x += z;
			if (event.AltDown())
				selectedCP.y += z;
			if (selectedCP.x < 0.0) selectedCP.x = 0.0; if (selectedCP.x > 255.0) selectedCP.x = 255.0;
			if (selectedCP.y < 0.0) selectedCP.y = 0.0; if (selectedCP.y > 255.0) selectedCP.y = 255.0;
			c.insertpoint((double) selectedCP.x, (double) selectedCP.y);
			t->Start(500,wxTIMER_ONE_SHOT);
			paintNow();
		}
	}
}

void CurvePane::keyPressed(wxKeyEvent &event)
{
	wxString curvedata;
	wxCommandEvent e(myCURVE_UPDATE);
	//wxMessageBox(wxString::Format("%d",event.GetKeyCode()));
	event.Skip();
	switch (event.GetKeyCode()) {
		case 84: //t - toggle tooltip
			if (ToggleToolTip())
				((wxFrame *) GetParent()->GetGrandParent())->SetStatusText("Curve tooltip display: on");
			else
				((wxFrame *) GetParent()->GetGrandParent())->SetStatusText("Curve tooltip display: off");
			break;
		case 127:  //delete
		case 8: //Backspace
			c.deletepoint(selectedCP.x, selectedCP.y);
			paintNow();
			
			e.SetEventObject(this);
			e.SetString("update");
			ProcessWindowEvent(e);
			break;
		case 67: // c - with Ctrl, copy curve Y to clipboard
			if (!event.ControlDown()) break;
			curvedata = getYPoints();
			curvedata.Append("\n");
			if (wxTheClipboard->Open())
			{
				wxTheClipboard->SetData( new wxTextDataObject(curvedata) );
				wxTheClipboard->Close();
			}
			((wxFrame *) GetGrandParent()->GetParent())->SetStatusText("curve Y data copied to clipboard");
			break;
	}
}

wxString CurvePane::getControlPoints()
{
	wxString s;
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
	wxString s;
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
	wxString s;
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
	wxColour b = GetBackgroundColour();
	unsigned g = (b.Red() + b.Green() + b.Blue()) / 3;
		
	double px = 0.0;
	double py = 0.0;
	double x=0, y=0;
	dc.Clear();
	int w, h;
	dc.GetSize(&w, &h);
	int m=10;
	int radius = atoi(myConfig::getConfig().getValueOrDefault("tool.curve.controlpointradius","5").c_str());

	//histogram:
	float hmax = 0.0;
	int inc = 30;
	wxColour hc = b; hc.Set(b.Red()+inc, b.Green()+inc, b.Blue()+inc);
	dc.SetPen(hc);
	for (unsigned i=0; i<histogram.size(); i++) {
		if (hmax < histogram[i]) hmax = histogram[i];
	}
	for (unsigned i=0; i<histogram.size(); i++) {
		//dc.DrawLine(m+px,h-m-py,m+x,h-m-y);
		dc.DrawLine(m+i,h-m-0,m+i,h-m-((float) histogram[i]/hmax)*255.0);
	}
	
	//center lines:
	if (g < 192)
		dc.SetPen(*wxWHITE_PEN);
	else 
		dc.SetPen(*wxLIGHT_GREY_PEN);
	dc.DrawLine(m+128,h-m,m+128,h-m-255);
	dc.DrawLine(m,h-m-128,m+255,h-m-128);
	//null curve:
	dc.DrawLine(m,h-m,m+255,h-m-255);
	//quarter lines:
	if (g < 192)
		//dc.SetPen(*wxWHITE_PEN);
		dc.SetPen(wxPen(wxColour(255,255,255), 1, wxPENSTYLE_DOT_DASH ));
	else 
		//dc.SetPen(*wxLIGHT_GREY_PEN);
		dc.SetPen(wxPen(wxColour(192,192,192), 1, wxPENSTYLE_DOT_DASH ));
	dc.DrawLine(m+64,h-m,m+64,h-m-255);
	dc.DrawLine(m,h-m-64,m+255,h-m-64);
	dc.DrawLine(m+192,h-m,m+192,h-m-255);
	dc.DrawLine(m,h-m-192,m+255,h-m-192);

	dc.SetPen(*wxBLACK_PEN);
	//x axis:
	dc.DrawLine(m,h-m,m,h-m-255);
	//y axis:
	dc.DrawLine(m,h-m,m+255,h-m);

	for (double x=0.0; x<256.0; x++) {
		y=c.getpoint(x);
		if (y>255.0) y = 255.0; if (y<0.0) y=0.0;
		dc.DrawLine(m+px,h-m-py,m+x,h-m-y);
		px = x;
		py = y;
	}

	std::vector<cp> controlpts = c.getControlPoints();
	for (unsigned int i=0; i<controlpts.size(); i++) {
		if ((controlpts[i].x == selectedCP.x) & (controlpts[i].y == selectedCP.y)) {
			dc.DrawText(wxString::Format("%0.0f,%0.0f",controlpts[i].x,controlpts[i].y),m+5,m+5);
			dc.SetPen(*wxRED_PEN);
		}
		dc.DrawCircle(m+controlpts[i].x,h-m-controlpts[i].y,radius);
		dc.SetPen(*wxBLACK_PEN);
	}
}

void CurvePane::setHistogram(std::vector<long> hgram)
{
	histogram = hgram;
}


void CurvePane::bump(int i)
{
	cp ctpt = c.getctrlpoint(1);
	ctpt.x -= 10;
	c.setctrlpoint(1, ctpt);
	Refresh();
}
