
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
 

myHistogramPane::myHistogramPane(wxDialog* parent, std::map<GIMAGE_CHANNEL, std::vector<long> > histograms, const wxPoint &pos, const wxSize &size) :
 wxWindow(parent, wxID_ANY, pos, size)
{
	SetSize(parent->GetSize());
	hdata = histograms;
	hmax = 0;
	hscale = histograms[CHANNEL_RED].size();
	for (unsigned i = 0; i<hscale; i++) {
		if (histograms[CHANNEL_RED][i] < hmax) hmax = histograms[CHANNEL_RED][i];
		if (histograms[CHANNEL_GREEN][i] < hmax) hmax = histograms[CHANNEL_GREEN][i];
		if (histograms[CHANNEL_BLUE][i] < hmax) hmax = histograms[CHANNEL_BLUE][i];
	}
	
	pressedDown = false;
	paintNow();
}

 
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
	dc.SetUserScale((double) w / (double) hscale, (double) h/ (double) hmax);
	dc.SetPen(wxPen(wxColour(255,0,0),1));
	for(int x=0; x<hscale; x++) {
		dc.DrawLine(x,dc.DeviceToLogicalY(h),x,dc.DeviceToLogicalY(h)-hdata[CHANNEL_RED][x]);
	}

	//dc.SelectObject(wxNullBitmap);


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
 
 

