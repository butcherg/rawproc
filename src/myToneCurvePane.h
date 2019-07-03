
#ifndef __MYTONECURVEPANE_h__
#define __MYTONECURVEPANE_h__

#include <vector>

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include "gimage/gimage.h"

class myToneCurvePane : public wxWindow
{

public:
	myToneCurvePane(wxWindow* parent, const wxPoint &pos, const wxSize &size);
	~myToneCurvePane();
	void OnSize(wxSizeEvent& event);

	void paintEvent(wxPaintEvent & evt);
	void paintNow();

	void mouseWheelMoved(wxMouseEvent& event);
	void mouseDoubleClicked(wxMouseEvent& event);

	
	void SetCurve(std::vector<float> curve, bool rescale=false);
 
	void render(wxDC& dc);
 


private:
	std::vector<float> c;
	float scale, resetscale, scaleincrement;

};
 

#endif
