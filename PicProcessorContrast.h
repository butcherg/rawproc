#ifndef __PICPROCESSORCONTRAST_H__
#define __PICPROCESSORCONTRAST_H__

#include "PicProcessor.h"


class PicProcessorContrast: public PicProcessor
{
	public:
		PicProcessorContrast(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		void createPanel(wxSimplebook* parent);
		void showParams();
		bool processPic();
};

#endif
