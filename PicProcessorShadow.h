#ifndef __PICPROCESSORSHADOW_H__
#define __PICPROCESSORSHADOW_H__

#include <vector>

#include "PicProcessor.h"
#include "FreeImage.h"


class PicProcessorShadow: public PicProcessor
{
	public:
		PicProcessorShadow(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		void showParams();
		bool processPic();
		void setLUT8(std::vector<BYTE> LUT);
		void setLUT16(std::vector<WORD> LUT);
	private:
		BYTE LUT8[256];
		WORD LUT16[65535];

};

#endif
