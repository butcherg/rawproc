#ifndef __PICPROCESSORWHITEBALANCE_H__
#define __PICPROCESSORWHITEBALANCE_H__

#include "PicProcessor.h"


class PicProcessorWhiteBalance: public PicProcessor
{
	public:
		PicProcessorWhiteBalance(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		~PicProcessorWhiteBalance();
		void createPanel(wxSimplebook* parent);
		void SetPatchCoord(int x, int y);
		bool processPicture(gImage *processdib);
		std::vector<double> getPatchMeans(int x, int y, float radius);
		std::vector<double> getCameraMultipliers();
		void OnLeftDown(wxMouseEvent& event);

	private:
		coord patch;
};

#endif
