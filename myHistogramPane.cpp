
#include "myHistogramPane.h"

BEGIN_EVENT_TABLE(myHistogramPane, wxWindow)
 
    EVT_MOTION(myHistogramPane::mouseMoved)
    EVT_LEFT_DOWN(myHistogramPane::mouseDown)
    EVT_LEFT_UP(myHistogramPane::mouseReleased)
    EVT_LEFT_DCLICK(myHistogramPane::mouseDoubleClicked)
    EVT_RIGHT_DOWN(myHistogramPane::rightClick)
    EVT_LEAVE_WINDOW(myHistogramPane::mouseLeftWindow)
    EVT_KEY_DOWN(myHistogramPane::keyPressed)
    EVT_KEY_UP(myHistogramPane::keyReleased)
    EVT_MOUSEWHEEL(myHistogramPane::mouseWheelMoved)
 
    // catch paint events
    EVT_PAINT(myHistogramPane::paintEvent)
 
END_EVENT_TABLE()
 

myHistogramPane::myHistogramPane(wxFrame* parent, std::vector<unsigned int> data, int maxval, const wxPoint &pos, const wxSize &size) :
 wxWindow(parent, wxID_ANY, pos, size)
{
	SetSize(parent->GetSize());
	hdata = data;
	hmax = maxval;
	if (hmax == 0) {
		for (int i = 0; i<hdata.size(); i++) {
			if (hdata[i] > hmax) hmax = hdata[i];
		}
	} 
	pressedDown = false;
	paintNow();
}

myHistogramPane::myHistogramPane(wxFrame* parent, wxImage img, int maxval, const wxPoint &pos, const wxSize &size) :
 wxWindow(parent, wxID_ANY, pos, size)
{
	SetSize(parent->GetSize());
	int iw = img.GetWidth();
	int ih = img.GetHeight();
	hmax = 0;
	hdata.resize(256);
	for (int i=0; i<256; i++) hdata[i] = 0;
	for (int x=0; x<iw; x++) {
		for (int y=0; y<ih; y++) {
			int gray = 0.21 * img.GetRed(x,y) + 0.72 * img.GetGreen(x,y) + 0.07 * img.GetBlue(x,y);
			hdata[gray]++;
			if (hdata[gray] > hmax) hmax = hdata[gray];
		}
	}
			
	pressedDown = false;
	paintNow();
}

/*
myHistogramPane::myHistogramPane(wxFrame* parent, FIBITMAP *dib, int maxval, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize):
wxWindow(parent, wxID_ANY, pos, size)
{


}
*/
 
void myHistogramPane::paintEvent(wxPaintEvent & evt)
{
	wxPaintDC dc(this);
	render(dc);
}
 
void myHistogramPane::paintNow()
{
	wxClientDC dc(this);
	render(dc);
}
 
void myHistogramPane::render(wxDC&  dc)
{
	int w, h;
	GetSize(&w, &h);
	dc.Clear();
	dc.SetLogicalScale(w/hdata.size(), h/hmax);
	for(int x=0; x<hdata.size(); x++) {
		dc.DrawLine(x,0,x,hdata[x]);
	}

}
 
void myHistogramPane::mouseDown(wxMouseEvent& event) {}
void myHistogramPane::mouseMoved(wxMouseEvent& event)  {}
void myHistogramPane::mouseReleased(wxMouseEvent& event) {}
void myHistogramPane::mouseLeftWindow(wxMouseEvent& event) {}
void myHistogramPane::mouseDoubleClicked(wxMouseEvent& event) {}
void myHistogramPane::mouseWheelMoved(wxMouseEvent& event) {}
void myHistogramPane::rightClick(wxMouseEvent& event) {}
void myHistogramPane::keyPressed(wxKeyEvent& event) {}
void myHistogramPane::keyReleased(wxKeyEvent& event) {}
 
 

