#ifndef __PICPROCESSORREDEYE_H__
#define __PICPROCESSORREDEYE_H__

#include "PicProcessor.h"


class PicProcessorRedEye: public PicProcessor
{
	public:
		PicProcessorRedEye(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		~PicProcessorRedEye();
		void showParams();
		bool processPic();
		
		void setThresholdLimit(wxString params);
		void OnLeftDown(wxMouseEvent& event);
		wxString getPointList();

		void setDesatPercent(double pct);
		void setDesat(bool d);
		
	private:
		std::vector<coord> points;
		double threshold, desatpct;
		bool desat;
		unsigned radius;
};

#endif
