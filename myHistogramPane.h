
#ifndef __MYHISTOGRAMPANE_h__
#define __MYHISTOGRAMPANE_h__

#include <vector>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include "FreeImage.h"

class myHistogramPane : public wxWindow
{

public:
	myHistogramPane(wxFrame* parent, std::vector<unsigned int> data, int maxval=0, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	myHistogramPane(wxFrame* parent, wxImage img, int maxval=0, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
	//myHistogramPane(wxFrame* parent, FIBITMAP *dib, int maxval=0, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize);
 
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

private:
	bool pressedDown;
	std::vector<unsigned int> hdata;
	unsigned int hmax;
	
 
	DECLARE_EVENT_TABLE()
};
 

#endif
