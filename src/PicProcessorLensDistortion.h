#ifndef __PICPROCESSORLENSDISTORTION_H__
#define __PICPROCESSORLENSDISTORTION_H__

#include "PicProcessor.h"

//#include <locale.h>
//#include <lensfun/lensfun.h>


class PicProcessorLensDistortion: public PicProcessor
{
	public:
		PicProcessorLensDistortion(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPicture(gImage *processdib);

};

#endif
