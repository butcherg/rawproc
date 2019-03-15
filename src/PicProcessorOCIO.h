#ifndef __PICPROCESSOROCIO_H__
#define __PICPROCESSOROCIO_H__

#include "PicProcessor.h"


class PicProcessorOCIO: public PicProcessor
{
	public:
		PicProcessorOCIO(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
