#ifndef __PICPROCESSORHIGHLIGHT_H__
#define __PICPROCESSORHIGHLIGHT_H__

#include <vector>

#include "PicProcessor.h"
#include "FreeImage.h"


class PicProcessorHighlight: public PicProcessor
{
	public:
		PicProcessorHighlight(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		void showParams();
		bool processPic();
		void setLUT8(std::vector<BYTE> LUT);
		void setLUT16(std::vector<WORD> LUT);
	private:
		BYTE LUT8[256];
		WORD LUT16[65535];

};

#endif
