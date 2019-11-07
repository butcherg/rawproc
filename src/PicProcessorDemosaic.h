#ifndef __PICPROCESSORDEMOSAIC_H__
#define __PICPROCESSORDEMOSAIC_H__

#include "PicProcessor.h"

enum ImageType {
	IMAGETYPE_RGB,
	IMAGETYPE_BAYER,
	IMAGETYPE_XTRANS
};

class PicProcessorDemosaic: public PicProcessor
{
	public:
		PicProcessorDemosaic(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent, PicProcessor* proc);
		bool processPicture(gImage *processdib);

		ImageType getImageType();
};

#endif
