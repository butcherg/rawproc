#ifndef __PICPROCESSORCURVE_H__
#define __PICPROCESSORCURVE_H__

#include <vector>

#include "PicProcessor.h"
#include "curve.h"

class PicProcessorCurve: public PicProcessor
{
	public:
		PicProcessorCurve(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		void showParams();
		void setControlPoints(std::vector<cp> ctpts);
		void setParams(std::vector<cp> ctpts, wxString params);
		bool processPic();

	private:
		std::vector<cp> ctrlpts;
};

#endif
