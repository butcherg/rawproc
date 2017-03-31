
#ifndef __MYHISTOGRAMPANE_h__
#define __MYHISTOGRAMPANE_h__

#include <vector>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include "gimage/gimage.h"

class myHistogramPane : public wxWindow
{

public:

	myHistogramPane(wxDialog* parent,  gImage &dib,  const wxPoint &pos, const wxSize &size);
	~myHistogramPane();
	void OnSize(wxSizeEvent& event);
 
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
	std::vector<long> rdata, gdata, bdata;
	
	wxPoint *r, *g, *b;
	int rlen, glen, blen;
	
	long hmax;
	unsigned hscale;
	unsigned xorigin, yorigin, MouseX, MouseY, xcenter, ycenter;
	unsigned ord;

	double wscale;
	
 
	DECLARE_EVENT_TABLE()
};
 

#endif
