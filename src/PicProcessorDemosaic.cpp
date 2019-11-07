#include "PicProcessorDemosaic.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myIntegerCtrl.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"
#include "copy.xpm"
#include "paste.xpm"

#define DEMOSAICENABLE 6900
#define DEMOSAICOP 6901

#define DEMOSAICHALF 6902
#define DEMOSAICHALFRESIZE 6903
#define DEMOSAICCOLOR 6904
#define DEMOSAICCOPY 6905
#define DEMOSAICPASTE 6906

#ifdef USE_LIBRTPROCESS
#define DEMOSAICAHD 6907
#define DEMOSAICAMAZE 6908
#define DEMOSAICDCB 6909
#define DEMOSAICIGV 6910
#define DEMOSAICLMMSE 6911
#define DEMOSAICRCD 6912
#define DEMOSAICVNG 6913
#define DEMOSAICXTRANMARKESTEIJN 6914
#define DEMOSAICXTRANFAST 6915

//#define DCBITERATIONS 6916
//#define DCBENHANCE 6917
#endif


class DemosaicPanel: public PicProcPanel
{

	public:
		DemosaicPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL, 3).CenterVertical();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);

			enablebox = new wxCheckBox(this, DEMOSAICENABLE, "demosaic:");
			enablebox->SetValue(true);

			halfb = new wxRadioButton(this, DEMOSAICHALF, "half", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			halfb->SetToolTip("Turns each bayer quad into a single RGB pixel. Demosaiced mage is half the size of the original.  Fast.");
			halfresizeb = new wxRadioButton(this, DEMOSAICHALFRESIZE, "half_resize");
			halfresizeb->SetToolTip("Does half, then resizes to the original width and height. Somewhat fast, low quality.");
			colorb = new wxRadioButton(this, DEMOSAICCOLOR, "color");
			colorb->SetToolTip("Doesn't do demosaic, colors the image according to the bayer array");

#ifdef USE_LIBRTPROCESS
			//ahdb, amazeb, dcbb, igvb, lmmseb, rcdb, vngb, xtran_markesteijnb, xtran_fastb
			ahdb = new wxRadioButton(this, DEMOSAICAHD, "ahd");
			ahdb->SetToolTip("");
			amazeb = new wxRadioButton(this, DEMOSAICAMAZE, "amaze");
			amazeb->SetToolTip("Good for low ISO images, architectural images. Fast.");
			dcbb = new wxRadioButton(this, DEMOSAICDCB, "dcb:");
			dcbb->SetToolTip("");
				dcb_iterations = new myIntegerCtrl(this, wxID_ANY, "iterations:", 1, 1, 5, wxDefaultPosition, wxSize(50,-1));
				dcb_enhance = new wxCheckBox(this, wxID_ANY, "enhance:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			igvb = new wxRadioButton(this, DEMOSAICIGV, "igv");
			igvb->SetToolTip("");
			lmmseb = new wxRadioButton(this, DEMOSAICLMMSE, "lmmse:");
			lmmseb->SetToolTip("");
				lmmse_iterations = new myIntegerCtrl(this, wxID_ANY, "iterations:", 1, 1, 5, wxDefaultPosition, wxSize(50,-1));
			rcdb = new wxRadioButton(this, DEMOSAICRCD, "rcd");
			rcdb->SetToolTip("Good for low ISO images, nature images. Faster than amaze.");
			vngb = new wxRadioButton(this, DEMOSAICVNG, "vng");
			vngb->SetToolTip("Slow, good for medium ISO images.");
			xtran_markesteijnb = new wxRadioButton(this, DEMOSAICXTRANMARKESTEIJN, "xtran_markesteijn:");
				xtran_markesteijn_passes = new myIntegerCtrl(this, wxID_ANY, "passes:", 1, 1, 5, wxDefaultPosition, wxSize(50,-1));
				xtran_markesteijn_cielab = new wxCheckBox(this, wxID_ANY, "cielab:", wxDefaultPosition, wxDefaultSize, wxALIGN_RIGHT);
			xtran_markesteijnb->SetToolTip("");
			xtran_fastb = new wxRadioButton(this, DEMOSAICXTRANFAST, "xtran_fast");
			xtran_fastb->SetToolTip("");
#endif

			ImageType imgtype = ((PicProcessorDemosaic *)q)->getImageType();

			halfb->Enable(false);
			halfresizeb->Enable(false);
			colorb->Enable(false);
#ifdef USE_LIBRTPROCESS
			ahdb->Enable(false);
			amazeb->Enable(false);
			dcbb->Enable(false);
			igvb->Enable(false);
			lmmseb->Enable(false);
			rcdb->Enable(false);
			vngb->Enable(false);
			xtran_markesteijnb->Enable(false);
			xtran_fastb->Enable(false);
			dcb_enhance->Enable(false);
			dcb_iterations->Enable(false);
			lmmse_iterations->Enable(false);
			xtran_markesteijn_passes->Enable(false);
			xtran_markesteijn_cielab->Enable(false);
#endif

			switch (imgtype) {
				case IMAGETYPE_BAYER:
					halfb->Enable(true);
					halfresizeb->Enable(true);
					colorb->Enable(true);
#ifdef USE_LIBRTPROCESS
					ahdb->Enable(true);
					amazeb->Enable(true);
					dcbb->Enable(true);
					igvb->Enable(true);
					lmmseb->Enable(true);
					rcdb->Enable(true);
					vngb->Enable(true);
					dcb_enhance->Enable(true);
					dcb_iterations->Enable(true);
					lmmse_iterations->Enable(true);
#endif
					break;
#ifdef USE_LIBRTPROCESS
				case IMAGETYPE_XTRANS:
colorb->Enable(true);
					xtran_markesteijnb->Enable(true);
					xtran_fastb->Enable(true);
					xtran_markesteijn_passes->Enable(true);
					xtran_markesteijn_cielab->Enable(true);
					break;
#endif
			}

			if (params != "") {
				wxArrayString p = split(params, ",");
				if (p[0] == "half") 			{ halfb->SetValue(true); 		selected_algorithm = DEMOSAICHALF; }
				else if (p[0] == "half_resize") 	{ halfresizeb->SetValue(true); 		selected_algorithm = DEMOSAICHALFRESIZE; }
				else if (p[0] == "color") 		{ colorb->SetValue(true); 		selected_algorithm = DEMOSAICCOLOR; }
#ifdef USE_LIBRTPROCESS
				else if (p[0] == "ahd") 		{ ahdb->SetValue(true); 		selected_algorithm = DEMOSAICAHD; }
				else if (p[0] == "amaze") 		{ amazeb->SetValue(true); 		selected_algorithm = DEMOSAICAMAZE; }
				else if (p[0] == "dcb") 		{ dcbb->SetValue(true); 		selected_algorithm = DEMOSAICDCB; }
				else if (p[0] == "igv") 		{ igvb->SetValue(true); 		selected_algorithm = DEMOSAICIGV; }
				else if (p[0] == "lmmse") 		{ lmmseb->SetValue(true); 		selected_algorithm = DEMOSAICLMMSE; }
				else if (p[0] == "rcd") 		{ rcdb->SetValue(true); 		selected_algorithm = DEMOSAICRCD; }
				else if (p[0] == "vng") 		{ vngb->SetValue(true); 		selected_algorithm = DEMOSAICVNG; }
				else if (p[0] == "xtran_markesteijn") 	{ xtran_markesteijnb->SetValue(true); 	selected_algorithm = DEMOSAICXTRANMARKESTEIJN; }
				else if (p[0] == "xtran_fast") 		{ xtran_fastb->SetValue(true); 		selected_algorithm = DEMOSAICXTRANFAST; }
#endif
				else {
					wxMessageBox(wxString::Format("%s is not a valid demosaic algorithm.  Setting to default.",p[0].mb_str()));
					halfb->SetValue(true); //to-do: change to tool.demosaic.default value 
				}
			}
			else {
				halfb->SetValue(true); //to-do: change to tool.demosaic.default value 
			}

			//Lay out the controls in the panel:
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, DEMOSAICCOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, DEMOSAICPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(halfb, flags); m->NextRow();
			m->AddRowItem(halfresizeb, flags); m->NextRow();
			m->AddRowItem(colorb, flags); m->NextRow();

#ifdef USE_LIBRTPROCESS
			m->AddRowItem(ahdb, flags); m->NextRow();
			m->AddRowItem(amazeb, flags); m->NextRow();
			m->AddRowItem(dcbb, flags);
				m->NextRow();
				m->AddRowItem(new wxStaticText(this, wxID_ANY, " ", wxDefaultPosition, wxSize(30,-1)), flags);
				m->AddRowItem(dcb_iterations, flags);   
				m->AddRowItem(dcb_enhance, flags);
				m->NextRow();
			m->AddRowItem(igvb, flags); m->NextRow();
			m->AddRowItem(lmmseb, flags); 
				m->NextRow();
				m->AddRowItem(new wxStaticText(this, wxID_ANY, " ", wxDefaultPosition, wxSize(30,-1)), flags);
				m->AddRowItem(lmmse_iterations, flags);
				m->NextRow();
			m->AddRowItem(rcdb, flags); m->NextRow();
			m->AddRowItem(vngb, flags); m->NextRow();
			m->AddRowItem(xtran_markesteijnb, flags);
				m->NextRow();
				m->AddRowItem(new wxStaticText(this, wxID_ANY, " ", wxDefaultPosition, wxSize(30,-1)), flags);
				m->AddRowItem(xtran_markesteijn_passes, flags);
				m->AddRowItem(xtran_markesteijn_cielab, flags);
				m->NextRow();
			m->AddRowItem(xtran_fastb, flags); m->NextRow();
#endif

			m->End();
			
			SetSizerAndFit(m);
			SetFocus();
			Refresh();
			Update();

			t.SetOwner(this);

			Bind(wxEVT_CHECKBOX, &DemosaicPanel::onEnable, this, DEMOSAICENABLE);
			Bind(wxEVT_CHECKBOX, &DemosaicPanel::paramChanged, this);
			Bind(wxEVT_RADIOBUTTON, &DemosaicPanel::algorithmChanged, this);
			Bind(wxEVT_BUTTON, &DemosaicPanel::OnCopy, this, DEMOSAICCOPY);
			Bind(wxEVT_BUTTON, &DemosaicPanel::OnPaste, this, DEMOSAICPASTE);
			Bind(myINTEGERCTRL_UPDATE, &DemosaicPanel::paramChanged, this);
			Bind(myINTEGERCTRL_CHANGE, &DemosaicPanel::onWheel, this);
			Bind(wxEVT_TIMER, &DemosaicPanel::OnTimer, this);
		}

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				q->enableProcessing(true);
				q->processPic();
			}
			else {
				q->enableProcessing(false);
				q->processPic();
			}
		}

		void OnCopy(wxCommandEvent& event)
		{
			q->copyParamsToClipboard();
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format("Copied command to clipboard: %s",q->getCommand()));
			
		}

		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {

				if (q->getParams() != "") {
					wxArrayString p = split(q->getParams(), ",");
					if (p[0] == "half") 			{ halfb->SetValue(true); 		selected_algorithm = DEMOSAICHALF; }
					else if (p[0] == "half_resize") 	{ halfresizeb->SetValue(true); 		selected_algorithm = DEMOSAICHALFRESIZE; }
					else if (p[0] == "color") 		{ colorb->SetValue(true); 		selected_algorithm = DEMOSAICCOLOR; }
#ifdef USE_LIBRTPROCESS
					else if (p[0] == "ahd") 		{ ahdb->SetValue(true); 		selected_algorithm = DEMOSAICAHD; }
					else if (p[0] == "amaze") 		{ amazeb->SetValue(true); 		selected_algorithm = DEMOSAICAMAZE; }
					else if (p[0] == "dcb") 		{ 
											dcbb->SetValue(true); 		
											selected_algorithm = DEMOSAICDCB;
											dcb_iterations->SetIntegerValue(1);
											if (p.GetCount() >= 2) dcb_iterations->SetIntegerValue(atoi(p[1].c_str()));
											dcb_enhance->SetValue(false);
											if (p.GetCount() >= 3)
												if (p[2] == "1") 
													dcb_enhance->SetValue(true);
										}
					else if (p[0] == "igv") 		{ igvb->SetValue(true); 		selected_algorithm = DEMOSAICIGV; }
					else if (p[0] == "lmmse") 		{ 
											lmmseb->SetValue(true);
									 		selected_algorithm = DEMOSAICLMMSE; 
											lmmse_iterations->SetIntegerValue(1);
											if (p.GetCount() >= 2) lmmse_iterations->SetIntegerValue(atoi(p[1].c_str()));

										}
					else if (p[0] == "rcd") 		{ rcdb->SetValue(true); 		selected_algorithm = DEMOSAICRCD; }
					else if (p[0] == "vng") 		{ vngb->SetValue(true); 		selected_algorithm = DEMOSAICVNG; }
					else if (p[0] == "xtran_markesteijn") 	{ 
											xtran_markesteijnb->SetValue(true);
										 	selected_algorithm = DEMOSAICXTRANMARKESTEIJN; 
											xtran_markesteijn_passes->SetIntegerValue(1);
											if (p.GetCount() >= 2) xtran_markesteijn_passes->SetIntegerValue(atoi(p[1].c_str()));
											xtran_markesteijn_cielab->SetValue(false);
											if (p.GetCount() >= 3)
												if (p[2] == "1") 
													xtran_markesteijn_cielab->SetValue(true);
										}
					else if (p[0] == "xtran_fast") 		{ xtran_fastb->SetValue(true); 		selected_algorithm = DEMOSAICXTRANFAST; }
#endif
					else wxMessageBox(wxString::Format("%s is not a valid demosaic algorithm.",p[0].mb_str()));

				}


				q->processPic();
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format("Pasted command from clipboard: %s",q->getCommand()));
			}
			else wxMessageBox(wxString::Format("Invalid Paste"));
		}


		void onWheel(wxCommandEvent& event)
		{
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			processIt();
			event.Skip();
		}

		void paramChanged(wxCommandEvent& event)
		{
			processIt();
		}

		void algorithmChanged(wxCommandEvent& event)
		{
			selected_algorithm = event.GetId();
			processIt();
		}

		void processIt()
		{
			switch (selected_algorithm) {
				case DEMOSAICHALF:
					q->setParams("half");
					break;
				case DEMOSAICHALFRESIZE:
					q->setParams("half_resize");
					break;
				case DEMOSAICCOLOR:
					q->setParams("color");
					break;

#ifdef USE_LIBRTPROCESS
				case DEMOSAICAHD:
					q->setParams("ahd");
					break;
				case DEMOSAICAMAZE:
					q->setParams("amaze");
					break;
				case DEMOSAICDCB:
					if (dcb_enhance->GetValue())
						q->setParams(wxString::Format("dcb,%d,1",dcb_iterations->GetIntegerValue()));
					else
						q->setParams(wxString::Format("dcb,%d,0",dcb_iterations->GetIntegerValue()));
					break;
				case DEMOSAICIGV:
					q->setParams("igv");
					break;
				case DEMOSAICLMMSE:
					q->setParams(wxString::Format("lmmse,%d",lmmse_iterations->GetIntegerValue()));
					break;
				case DEMOSAICRCD:
					q->setParams("rcd");
					break;
				case DEMOSAICVNG:
					q->setParams("vng");
					break;
				case DEMOSAICXTRANMARKESTEIJN:
					if (xtran_markesteijn_cielab->GetValue())
						q->setParams(wxString::Format("xtran_markesteijn,%d,1",xtran_markesteijn_passes->GetIntegerValue()));
					else
						q->setParams(wxString::Format("xtran_markesteijn,%d,0",xtran_markesteijn_passes->GetIntegerValue()));
					break;
				case DEMOSAICXTRANFAST:
					q->setParams("xtran_fast");
					break;
#endif
			}

			q->processPic();
		}


	private:
		wxCheckBox *enablebox;
		wxRadioBox *operselect;
		wxRadioButton *halfb, *halfresizeb, *colorb;
		int selected_algorithm;
		wxTimer t;

#ifdef USE_LIBRTPROCESS
		wxRadioButton *ahdb, *amazeb, *dcbb, *igvb, *lmmseb, *rcdb, *vngb, *xtran_markesteijnb, *xtran_fastb;
		myIntegerCtrl *dcb_iterations, *lmmse_iterations, *xtran_markesteijn_passes;
		wxCheckBox *dcb_enhance, *xtran_markesteijn_cielab;
#endif

};

PicProcessorDemosaic::PicProcessorDemosaic(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	if (getImageType() == IMAGETYPE_XTRANS & c.find("xtran") == std::string::npos) 
		c = "xtran_fast";	
}

void PicProcessorDemosaic::createPanel(wxSimplebook* parent, PicProcessor* proc)
{
	toolpanel = new DemosaicPanel(parent, proc, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
}

ImageType PicProcessorDemosaic::getImageType()
{
	gImage * imgdib = getPreviousPicProcessor()->getProcessedPicPointer();

	if (imgdib->getInfoValue("LibrawMosaiced") == "1") {

		unsigned cfarray[2][2];	
		if (imgdib->cfArray(cfarray)) return IMAGETYPE_BAYER;

		unsigned xtarray[6][6];
		if (imgdib->xtranArray(xtarray)) return IMAGETYPE_XTRANS;
	}

	return IMAGETYPE_RGB;
}

bool PicProcessorDemosaic::processPicture(gImage *processdib) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("demosaic...");
	bool result = false;

	wxArrayString p = split(c,",");

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.demosaic.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	

	dib = processdib;
	if (!global_processing_enabled) return true;

	if (processingenabled) {
		mark();
		if (p[0] == "color") {
			result = dib->ApplyMosaicColor(threadcount);
			m_display->SetModified(true);
		}
		else if (p[0] == "half") {
			result = dib->ApplyDemosaicHalf(false, threadcount);
			m_display->SetModified(true);
		}
		else if (p[0] == "half_resize") {
			result = dib->ApplyDemosaicHalf(true, threadcount);
			m_display->SetModified(true);
		}
#ifdef USE_LIBRTPROCESS
		else if (p[0] == "vng") {
			result = dib->ApplyDemosaicVNG(threadcount);
			m_display->SetModified(true);
		}
		else if (p[0] == "rcd") {
			result = dib->ApplyDemosaicRCD(threadcount);
			m_display->SetModified(true);
		}
		else if (p[0] == "dcb") {
			int iterations = 1;
			if (p.GetCount() >= 2) iterations = atoi(p[1].c_str());
			bool dcb_enhance = false;
			if (p.GetCount() >= 3) if (p[2] == "1") dcb_enhance = true;
			result = dib->ApplyDemosaicDCB(iterations, dcb_enhance, threadcount);
			m_display->SetModified(true);
		}
		else if (p[0] == "amaze") {
			double initGain = 1.0;
			int border = 0;
			float inputScale = 1.0;
			float outputScale = 1.0;
			result = dib->ApplyDemosaicAMAZE(initGain, border, inputScale, outputScale, threadcount);
			m_display->SetModified(true);
		}
		else if (p[0] == "igv") {
			result = dib->ApplyDemosaicIGV(threadcount);
			m_display->SetModified(true);
		}
		else if (p[0] == "ahd") {
			result = dib->ApplyDemosaicAHD(threadcount);
			m_display->SetModified(true);
		}
		else if (p[0] == "lmmse") { 
			int iterations = 1;
			if (p.GetCount() >= 2) iterations = atoi(p[1].c_str());
			result = dib->ApplyDemosaicLMMSE(iterations, threadcount);
			m_display->SetModified(true);
		}
		else if (p[0] == "xtran_fast") {
			result = dib->ApplyDemosaicXTRANSFAST(threadcount);
			m_display->SetModified(true);
		}
		else if (p[0] == "xtran_markesteijn") { 
			int passes = 1;
			if (p.GetCount() >= 2) passes = atoi(p[1].c_str());
			bool useCieLab = false;
			if (p.GetCount() >= 3) if (p[2] == "1") useCieLab = true;
			result = dib->ApplyDemosaicXTRANSMARKESTEIJN(passes, useCieLab, threadcount);
			m_display->SetModified(true);
		}
#endif
		else {
			wxMessageBox(wxString::Format("Unknown demosaic algorithm: %s",p[0].c_str()));
			result = false;
		}

		if (result) {
			wxString d = duration();
			m_tree->SetItemText(id, wxString::Format("demosaic:%s",p[0].c_str()));
			dib->setInfo("LibrawMosaiced", "0");

			//parm tool.demosaic.orient: Rotate the image to represent the EXIF Orientation value originally inputted, then set the Orientation tag to 1.  If you're going to use demosaic in the tool chain, you actually need to set input.orient=0 an leave this setting at its default, so the normalization is deferred until after demosaic.  Demosaic requires the image to be in its original orientation to preserve the specified Bayer pattern.  Default=0
			if (myConfig::getConfig().getValueOrDefault("tool.demosaic.orient","0") == "1") {
				((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("Normalizing image orientation..."));
				dib->NormalizeRotation(threadcount);
			}

			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.demosaic.log","0") == "1"))
				log(wxString::Format("tool=demosaic,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

			dirty = false;

		}
		else wxMessageBox(wxString::Format("Demosaic algorithm %s didn't work on this image.",p[0].c_str()));
	}

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}





