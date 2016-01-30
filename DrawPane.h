#ifndef __DRAWPANE_H__
#define __DRAWPANE_H__

#define NDEBUG

#include "wx/wx.h"
#include <vector>
#include "Curve.h"
#include "FreeImage.h"

using namespace std;

//class DrawPane : public wxPanel
class DrawPane : public wxControl
{
 
public:
    DrawPane(wxPanel* parent, wxString controlpoints,  wxWindowID id=wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
 
	//bool loadData(vector v);

	void initCurve(wxString controlpoints);
	void paintEvent(wxPaintEvent & evt);
	void paintNow();
	void OnSize(wxSizeEvent & evt);
	void render(wxDC& dc);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void mouseDclick(wxMouseEvent& event);
	void keyPressed(wxKeyEvent &event);
	wxString getControlPoints();
	wxString getXYPoints();
	wxString getYPoints();
	vector<BYTE> LookupTable8();
	vector<WORD> LookupTable16();
 
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
	wxPanel * p;
	Curve c;
	bool mousemotion;
	wxPoint pos;
	cp mouseCP, selectedCP;
	int ctrlptlanding;
	wxString initctrlpts;
};

#endif
