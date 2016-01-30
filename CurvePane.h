#ifndef __DRAWPANE_H__
#define __DRAWPANE_H__

#define NDEBUG

#include "wx/wx.h"
#include <vector>
#include "Curve.h"

using namespace std;

class CurvePane : public wxPanel
{
 
public:
    CurvePane(wxWindow* parent, wxString controlpoints);
 
	//bool loadData(vector v);

    void paintEvent(wxPaintEvent & evt);
    void paintNow();
	void OnSize(wxSizeEvent & evt);
    void render(wxDC& dc);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
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
     void mouseDown(wxMouseEvent& event);
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
};

#endif
