#ifndef __PICPROCESSORCURVE_H__
#define __PICPROCESSORCURVE_H__

#include <vector>

#include "PicProcessor.h"
#include "gimage/curve.h"

class PicProcessorCurve: public PicProcessor
{
	public:
		PicProcessorCurve(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent, PicProcessor* proc);
		void setControlPoints(std::vector<cp> ctpts);
		void setChannel(wxString chan);
		void setParams(std::vector<cp> ctpts, wxString params);
		bool processPicture(gImage *processdib);
		void displayProcessedPic();

	private:
		std::vector<cp> ctrlpts;
		//wxString channel;
		//GIMAGE_CHANNEL channel;
};

#endif
