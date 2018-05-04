#ifndef __DRAWPANE_H__
#define __DRAWPANE_H__


#include "wx/wx.h"
#include <vector>
#include "gimage/curve.h"

using namespace std;

wxDECLARE_EVENT(myCURVE_UPDATE, wxCommandEvent);

class CurvePane : public wxPanel
{
 
public:	
	CurvePane(wxWindow* parent, wxString controlpoints);
	~CurvePane();
	void paintEvent(wxPaintEvent & evt);	
	void paintNow();
	void OnTimer(wxTimerEvent& event);
	void OnSize(wxSizeEvent & evt);
	void render(wxDC& dc);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseLeftDown(wxMouseEvent& event);
	void mouseRightDown(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void mouseDclick(wxMouseEvent& event);
	void keyPressed(wxKeyEvent &event);
	wxString getControlPoints();
	wxString getXYPoints();
	wxString getYPoints();
	std::vector<cp> getPoints();
	void setPoints(std::vector<cp> pts);
	void bump(int i);
 
    // some useful events
    /*
     void mouseMoved(wxMouseEvent& event);
     void mouseWheelMoved(wxMouseEvent& event);
     void mouseReleased(wxMouseEvent& event);
     void rightClick(wxMouseEvent& event);
     void mouseLeftWindow(wxMouseEvent& event);
     void keyPressed(wxKeyEvent& event);
     void keyReleased(wxKeyEvent& event);
     */
 
    DECLARE_EVENT_TABLE()

private:
	vector< vector<int> > data;
	int x, y, z;
	wxWindow * p;
	Curve c;
	bool mousemotion;
	bool mousemoved;
	wxPoint pos;
	cp mouseCP, selectedCP;
	wxTimer *t;
};

#endif
