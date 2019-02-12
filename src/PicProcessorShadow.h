#ifndef __PICPROCESSORSHADOW_H__
#define __PICPROCESSORSHADOW_H__

#include <vector>

#include "PicProcessor.h"

class PicProcessorShadow: public PicProcessor
{
	public:
		PicProcessorShadow(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);

};

#endif
