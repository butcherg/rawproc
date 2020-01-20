#ifndef __PICPROCESSORLENSCORRECTION_H__
#define __PICPROCESSORLENSCORRECTION_H__

#include "PicProcessor.h"

#include <locale.h>
#include <lensfun/lensfun.h>


class PicProcessorLensCorrection: public PicProcessor
{
	public:
		PicProcessorLensCorrection(lfDatabase * lfdatabase, wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display);
		~PicProcessorLensCorrection();
		void createPanel(wxSimplebook* parent);
		void setAlternates(wxString acam, wxString alens);
		lfDatabase * getLensDatabase();
		bool isOk(); //returns false if lensfun database could not be opened or is incorrect format
		bool processPicture(gImage *processdib);
		
		static lfDatabase * findLensfunDatabase();

	private:
		wxString metadatacamera, metadatamount, metadatalens, altcamera, altmount, altlens;
		bool lfok;
		lfDatabase *ldb;
};

#endif
