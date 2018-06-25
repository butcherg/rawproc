#include "myFloatSlider.h"

myFloatSlider::myFloatSlider(wxWindow *parent, wxWindowID id, float value, float minValue, float maxValue, float increment, const wxPoint &pos, const wxSize &size, long style):
	wxSlider(parent, id, 0, 0, (maxValue-minValue)/increment, pos, size, style)
{
	val = value;
			
	min = minValue;
	max = maxValue;
	inc = increment;
	SetLineSize(1);
	SetValue(float2int());
			
	Bind(wxEVT_SLIDER, &myFloatSlider::OnChange, this);
}

void myFloatSlider::SetFloatValue(float value)
{
	val = value;
	SetValue(float2int());
	Refresh();
}

float myFloatSlider::GetFloatValue()
{
	return val;
}

void myFloatSlider::OnChange(wxCommandEvent& event)
{
	val = (inc * GetValue())+min;
	event.Skip();
}

int myFloatSlider::float2int()
{
	float range = max - min;
	float normval = val - min;
	float pct = normval / range;
	return (int) (range/inc) * pct;	
}


