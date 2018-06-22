#ifndef __MYFLOATSLIDER_H__
#define __MYFLOATSLIDER_H__

#include "wx/wx.h"
#include <wx/slider.h>

class myFloatSlider: public wxSlider
{
	public:
		myFloatSlider(wxWindow *parent, wxWindowID id, float value, float minValue, float maxValue, float increment, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0);

		void SetFloatValue(float value);
		float GetFloatValue();
		void OnChange(wxCommandEvent& event);

	private:
		int float2int();
		
		float val;
		float min, max, inc;
		int sliderlength;
};

#endif

