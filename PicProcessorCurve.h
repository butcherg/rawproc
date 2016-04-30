#ifndef __PICPROCESSORCURVE_H__
#define __PICPROCESSORCURVE_H__

#include <vector>

#include "PicProcessor.h"
#include "FreeImage.h"
#include "Curve.h"

class PicProcessorCurve: public PicProcessor
{
	public:
		PicProcessorCurve(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		void showParams();
		void setControlPoints(std::vector<cp> ctpts);
		void setParams(std::vector<cp> ctpts, wxString params);
		bool processPic();
		void setLUT8(std::vector<BYTE> LUT);
		void setLUT16(std::vector<WORD> LUT);

	private:
		BYTE LUT8[256];
		WORD LUT16[65535];

		std::vector<cp> ctrlpts;


};

#endif
