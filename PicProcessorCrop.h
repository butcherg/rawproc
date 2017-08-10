#ifndef __PICPROCESSORCROP_H__
#define __PICPROCESSORCROP_H__

#include "PicProcessor.h"


class PicProcessorCrop: public PicProcessor
{
	public:
		PicProcessorCrop(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		PicProcessorCrop(wxString name, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters); 
		void createPanel(wxSimplebook* parent);
		void showParams();
		bool processPic();
};

#endif
