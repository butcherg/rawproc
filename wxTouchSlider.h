
#ifndef __WXTOUCHSLIDER_h__
#define __WXTOUCHSLIDER_h__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

class wxTouchSlider : public wxWindow
{
 
	bool pressedDown;
	wxString text;
	int value, px, py;
	int mx, mn;
 
public:
	wxTouchSlider(wxFrame* parent, wxString text, int val, int min, int max);
 
	void paintEvent(wxPaintEvent & evt);
	void paintNow();
 
	void render(wxDC& dc);
 
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
