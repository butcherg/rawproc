#ifndef MYDOUBLESLIDER
#define MYDOUBLESLIDER


#include "wx/wx.h"
#include <wx/slider.h>


class myDoubleSlider : public wxControl
{
public:

	myDoubleSlider(wxWindow *parent,
		wxWindowID id,
		//const wxString& label,
		int leftValue, int rightValue, int minValue, int maxValue,
		const wxPoint& pos = wxDefaultPosition,
		const wxSize& size = wxDefaultSize,
		long style = wxSL_HORIZONTAL,
		const wxValidator& val = wxDefaultValidator,
		const wxString& name = "myDoubleSlider");

	int GetLeftValue();
	int GetRightValue();
	wxSize DoGetBestSize();	
	void OnPaint(wxPaintEvent&);
	void OnLeftDown(wxMouseEvent& event);
	void OnMotion(wxMouseEvent& event);
	void OnLeftUp(wxMouseEvent& event);
	void OnWheel(wxMouseEvent& event);

protected:
	void paintNow();
	void render(wxDC& dc);
	

private:

	int leftval, rightval, minval, maxval;
	int selectedslider;
	int prevx, prevy;

};


#endif
