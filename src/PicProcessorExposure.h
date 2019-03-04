#ifndef __PICPROCESSOREXPOSURE_H__
#define __PICPROCESSOREXPOSURE_H__

#include "PicProcessor.h"


class PicProcessorExposure: public PicProcessor
{
	public:
		PicProcessorExposure(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		~PicProcessorExposure();
		void createPanel(wxSimplebook* parent);
		void SetPatchCoord(int x, int y);
		void OnLeftDown(wxMouseEvent& event);
		bool processPic(bool processnext=true);

	private:
		coord patch;
};

#endif
