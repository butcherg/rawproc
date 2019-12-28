
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

	//myHistogramPane(wxWindow* parent,  gImage &dib,  const wxPoint &pos, const wxSize &size);
	myHistogramPane(wxWindow* parent, const wxPoint &pos, const wxSize &size);
	~myHistogramPane();
	void OnSize(wxSizeEvent& event);
	bool ToggleToolTip();
	void paintEvent(wxPaintEvent & evt);
	void paintNow();
	
	void SetPic(gImage *dib, unsigned scale=256);
	void RecalcHistogram();
	void SetChannel(GIMAGE_CHANNEL channel);
	void BlankPic();
 
	void render(wxDC& dc);
 
	void mouseMoved(wxMouseEvent& event);
	void mouseDown(wxMouseEvent& event);
	void mouseWheelMoved(wxMouseEvent& event);
	void mouseReleased(wxMouseEvent& event);
	void mouseDoubleClicked(wxMouseEvent& event);
	//void rightClick(wxMouseEvent& event);
	void mouseEnterWindow(wxMouseEvent& event);
	void mouseLeftWindow(wxMouseEvent& event);
	void keyPressed(wxKeyEvent& event);
	//void keyReleased(wxKeyEvent& event);
	void OnGetFocus(wxFocusEvent& event);
	void OnLoseFocus(wxFocusEvent& event);

private:
	bool blankpic, pressedDown;
	std::vector<long> rdata, gdata, bdata, smalldata;

	gImage *db;
	std::vector<histogramdata> histogram;
	
	wxPoint *r, *g, *b;
	int rlen, glen, blen;

	int zerobucket, onebucket;
	bool EVaxis, Unbounded, TextVisible, FloatCursor;
	float EV0, dmin, dmax;

	GIMAGE_CHANNEL display_channels;
	
	bool inwindow;
	
	long hmax;
	unsigned hscale;
	int xorigin, yorigin, MouseX, MouseY, xcenter, ycenter;
	unsigned ord;

	double wscale;
	
 
//	DECLARE_EVENT_TABLE()
};
 

#endif
