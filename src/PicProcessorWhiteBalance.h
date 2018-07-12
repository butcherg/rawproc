#ifndef __PICPROCESSORWHITEBALANCE_H__
#define __PICPROCESSORWHITEBALANCE_H__

#include "PicProcessor.h"


class PicProcessorWhiteBalance: public PicProcessor
{
	public:
		PicProcessorWhiteBalance(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		~PicProcessorWhiteBalance();
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
		std::vector<double> getPatchMeans(int x, int y, float radius);
		std::vector<double> getCameraMultipliers();
		void OnLeftDown(wxMouseEvent& event);

	private:
		void SetPatchCoord(int x, int y);
		coord patch;
};

#endif
