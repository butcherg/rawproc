#ifndef __PICPROCESSORBLACKWHITEPOINT_H__
#define __PICPROCESSORBLACKWHITEPOINT_H__

#include <vector>

#include "PicProcessor.h"

class PicProcessorBlackWhitePoint: public PicProcessor
{
	public:
		PicProcessorBlackWhitePoint(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		void setChannel(wxString chan);
		void reCalc();
		bool processPic(bool processnext=true);
		void displayProcessedPic();

};

#endif
