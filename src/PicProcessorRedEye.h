#ifndef __PICPROCESSORREDEYE_H__
#define __PICPROCESSORREDEYE_H__

#include "PicProcessor.h"


class PicProcessorRedEye: public PicProcessor
{
	public:
		PicProcessorRedEye(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		~PicProcessorRedEye();
		void createPanel(wxSimplebook* parent, PicProcessor* proc);
		bool processPicture(gImage *processdib);
		
		void setThresholdLimit(wxString params);
		void OnLeftDown(wxMouseEvent& event);
		wxString getPointList();

		void setThreshold(double t);
		void setRadius(int r);
		void setDesatPercent(double pct);
		void setDesat(bool d);
		
	private:
		wxString buildCommand();
		std::vector<coord> points;
		double threshold, desatpct;
		bool desat;
		unsigned radius;
};

#endif
