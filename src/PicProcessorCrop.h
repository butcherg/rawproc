#ifndef __PICPROCESSORCROP_H__
#define __PICPROCESSORCROP_H__

#include "PicProcessor.h"


class PicProcessorCrop: public PicProcessor
{
	public:
		PicProcessorCrop(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		PicProcessorCrop(wxString name, wxTreeCtrl *tree, PicPanel *display); 
		void createPanel(wxSimplebook* parent);
		bool processPic(bool processnext=true);
};

#endif
