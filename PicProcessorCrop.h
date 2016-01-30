#ifndef __PICPROCESSORCROP_H__
#define __PICPROCESSORCROP_H__

#include <vector>

#include "PicProcessor.h"

//class PicPanel;

class PicProcessorCrop: public PicProcessor
{
	public:
		PicProcessorCrop(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		PicProcessorCrop(wxString name, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters);
		~PicProcessorCrop();
		void showParams();
		bool processPic();
		void CropMode(bool c);
		void paramChanged(wxCommandEvent& event);
		void displayProcessedPic(); 

	private:
		FIBITMAP *previous;

};

#endif
