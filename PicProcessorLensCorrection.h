#ifndef __PICPROCESSORLENSCORRECTION_H__
#define __PICPROCESSORLENSCORRECTION_H__

#include "PicProcessor.h"


class PicProcessorLensCorrection: public PicProcessor
{
	public:
		PicProcessorLensCorrection(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
