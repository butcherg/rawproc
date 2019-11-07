#ifndef __PICPROCESSOREXPOSURE_H__
#define __PICPROCESSOREXPOSURE_H__

#include "PicProcessor.h"


class PicProcessorExposure: public PicProcessor
{
	public:
		PicProcessorExposure(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		~PicProcessorExposure();
		void createPanel(wxSimplebook* parent, PicProcessor* proc);
		void SetPatchCoord(int x, int y);
		void OnLeftDown(wxMouseEvent& event);
		bool processPicture(gImage *processdib);

	private:
		coord patch;
};

#endif
