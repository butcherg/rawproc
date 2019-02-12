#ifndef __PICPROCESSORCONTRAST_H__
#define __PICPROCESSORCONTRAST_H__

#include "PicProcessor.h"


class PicProcessorContrast: public PicProcessor
{
	public:
		PicProcessorContrast(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
