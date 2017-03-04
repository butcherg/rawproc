
#ifndef __MYHISTOGRAMPANE_h__
#define __MYHISTOGRAMPANE_h__

#include <vector>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include "gimage.h"

class myHistogramPane : public wxWindow
{

public:

	myHistogramPane(wxDialog* parent, std::map<GIMAGE_CHANNEL, std::vector<unsigned> > histograms, const wxPoint &pos, const wxSize &size);

 
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
	std::map<GIMAGE_CHANNEL, std::vector<unsigned> > hdata;
	long hmax;
	unsigned hscale;
	
 
	DECLARE_EVENT_TABLE()
};
 

#endif
