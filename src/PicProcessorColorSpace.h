#ifndef __PICPROCESSORCOLORSPACE_H__
#define __PICPROCESSORCOLORSPACE_H__

#include "PicProcessor.h"


class PicProcessorColorSpace: public PicProcessor
{
	public:
		PicProcessorColorSpace(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		void createPanel(wxSimplebook* parent);
		bool processPicture(gImage *processdib);
		void setOpenFilePath(wxString path);
		wxString getOpenFilePath();


	private:
		wxString openfilepath;
		wxString dcraw_primaries, primary_source, camdat_status;
};

#endif
