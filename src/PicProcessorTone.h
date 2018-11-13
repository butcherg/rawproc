#ifndef __PICPROCESSORTONE_H__
#define __PICPROCESSORTone_H__

#include "PicProcessor.h"


class PicProcessorTone: public PicProcessor
{
	public:
		PicProcessorTone(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
