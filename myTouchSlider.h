
#ifndef __myTouchSlider_h__
#define __myTouchSlider_h__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

class myTouchSlider : public wxWindow
{
 
	bool pressedDown;
	wxString lbl;
	double initval, val, inc;
	double mn, mx;
	wxString fmt;
	int px, py;
	int vsize;
 
public:
	//default width:
	myTouchSlider(wxFrame* parent, wxWindowID id, wxString label, double initialvalue, double increment, double min, double max, wxString format="%3.0f");
	//explicit width:
	myTouchSlider(wxFrame* parent, wxWindowID id, wxString label, int width, double initialvalue, double increment, double min, double max, wxString format="%3.0f");

	void paintEvent(wxPaintEvent & evt);
	void paintNow();
 
	void render(wxDC& dc);

	double GetDoubleValue();
	int GetIntValue();
	void SetValue(double value);
 
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void mouseDoubleClicked(wxMouseEvent& event);
	void rightClick(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	void keyReleased(wxKeyEvent& event);
 
	DECLARE_EVENT_TABLE()
};
 

#endif
