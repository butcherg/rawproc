#include "myFloatSlider.h"

myFloatSlider::myFloatSlider(wxWindow *parent, wxWindowID id, float value, float minValue, float maxValue, float increment, const wxPoint &pos, const wxSize &size, long style):
	wxSlider(parent, id, 0, 0, trunc((maxValue-minValue)/increment), pos, size, style)
{
	val = value;
			
	min = minValue;
	max = maxValue;
	inc = increment;
	//if (GetMax() % 2 == 0) SetMax(GetMax()+1);
	SetLineSize(1);
	SetValue(float2int());
			
	Bind(wxEVT_SLIDER, &myFloatSlider::OnChange, this);
	wxMessageBox(wxString::Format("%d %d %d",GetLineSize(), GetMin(), GetMax()));
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
	//float range = max - min;
	//float normval = val - min;
	//float pct = normval / range;
	//return (int) (range/inc) * pct;	
	return (int) (val - min) / inc;
	
}


