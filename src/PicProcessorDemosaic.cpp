#include "PicProcessorDemosaic.h"
#include "PicProcPanel.h"
#include "myRowColumnSizer.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"

#define DEMOSAICENABLE 6900
#define DEMOSAICOP 6901

#define DEMOSAICHALF 6902
#define DEMOSAICHALFRESIZE 6903
#define DEMOSAICCOLOR 6904

#ifdef USE_LIBRTPROCESS
#define DEMOSAICAHD 6905
#define DEMOSAICAMAZE 6906
#define DEMOSAICDCB 6907
#define DEMOSAICIGV 6908
#define DEMOSAICLMMSE 6909
#define DEMOSAICRCD 6910
#define DEMOSAICVNG 6911
#define DEMOSAICXTRANMARKESTEIJN 6912
#define DEMOSAICXTRANFAST 6913
#endif


class DemosaicPanel: public PicProcPanel
{

	public:
		DemosaicPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
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
			dcbb = new wxRadioButton(this, DEMOSAICDCB, "dcb");
			dcbb->SetToolTip("");
			igvb = new wxRadioButton(this, DEMOSAICIGV, "igv");
			igvb->SetToolTip("");
			lmmseb = new wxRadioButton(this, DEMOSAICLMMSE, "lmmse");
			lmmseb->SetToolTip("");
			rcdb = new wxRadioButton(this, DEMOSAICRCD, "rcd");
			rcdb->SetToolTip("Good for low ISO images, nature images. Faster than amaze.");
			vngb = new wxRadioButton(this, DEMOSAICVNG, "vng");
			vngb->SetToolTip("Slow, good for medium ISO images.");
			xtran_markesteijnb = new wxRadioButton(this, DEMOSAICXTRANMARKESTEIJN, "xtran_markesteijn");
			xtran_markesteijnb->SetToolTip("");
			xtran_fastb = new wxRadioButton(this, DEMOSAICXTRANFAST, "xtran_fast");
			xtran_fastb->SetToolTip("");
#endif

			if (params != "") {
				wxArrayString p = split(params, ",");
				if (p[0] == "half") halfb->SetValue(true);
				else if (p[0] == "half_resize") halfresizeb->SetValue(true);
				else if (p[0] == "color") colorb->SetValue(true);
#ifdef USE_LIBRTPROCESS
				else if (p[0] == "ahd") ahdb->SetValue(true);
				else if (p[0] == "amaze") amazeb->SetValue(true);
				else if (p[0] == "dcb") dcbb->SetValue(true);
				else if (p[0] == "igv") igvb->SetValue(true);
				else if (p[0] == "lmmse") lmmseb->SetValue(true);
				else if (p[0] == "rcd") rcdb->SetValue(true);
				else if (p[0] == "vng") vngb->SetValue(true);
				else if (p[0] == "xtran_markesteijn") xtran_markesteijnb->SetValue(true);
				else if (p[0] == "xtran_fast") xtran_fastb->SetValue(true);
#endif
				else {
					wxMessageBox(wxString::Format("%s is not a valid demosaic algorithm.  Setting to default.",p[0].mb_str()));
					halfb->SetValue(true); //to-do: change to tool.demosaic.default value 
				}
			}
			else {
				halfb->SetValue(true); //to-do: change to tool.demosaic.default value 
			}




/*	//replace following with wxRadioButton, wxMyRowColumnSizer:
			b->Add(enablebox, flags);
			b->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			b->AddSpacer(10);

			wxArrayString opers;
#ifdef USE_LIBRTPROCESS
			opers.Add("ahd");
			opers.Add("amaze");
			opers.Add("dcb");
			opers.Add("igv");
			opers.Add("lmmse");
			opers.Add("rcd");
			opers.Add("vng");
			opers.Add("xtran_markesteijn");
			opers.Add("xtran_fast");
#endif
			opers.Add("half");
			opers.Add("half_resize");
			opers.Add("color");

			operselect = new wxRadioBox (this, DEMOSAICOP, "Operation", wxDefaultPosition, wxDefaultSize,  opers, 1, wxRA_SPECIFY_COLS);

#ifdef USE_LIBRTPROCESS
			operselect->SetItemToolTip(operselect->FindString("ahd"),"");
			operselect->SetItemToolTip(operselect->FindString("amaze"),"Good for low ISO images, architectural images. Fast.");
			operselect->SetItemToolTip(operselect->FindString("dcb"),"");
			operselect->SetItemToolTip(operselect->FindString("igv"),"");
			//operselect->SetItemToolTip(operselect->FindString("lmmse"),"");
			operselect->SetItemToolTip(operselect->FindString("rcd"),"Good for low ISO images, nature images. Faster than amaze.");
			operselect->SetItemToolTip(operselect->FindString("vng"),"Slow, good for medium ISO images.");
#endif
			operselect->SetItemToolTip(operselect->FindString("half"),"Turns each bayer quad into a single RGB pixel. Demosaiced mage is half the size of the original.  Fast.");
			operselect->SetItemToolTip(operselect->FindString("half_resize"),"Does half, then resizes to the original width and height. Somewhat fast, low quality.");
			operselect->SetItemToolTip(operselect->FindString("color"),"Doesn't demosaic, colors the image according to the bayer array");

			if (params != "") operselect->SetSelection(operselect->FindString(params));
			b->Add(operselect,flags);
*/  //end of replace following....




			//Lay out the controls in the panel:
			myRowColumnSizer *m = new myRowColumnSizer(10,3);
			m->AddItem(enablebox, wxALIGN_LEFT);
			m->NextRow();
			m->AddItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), wxALIGN_LEFT, 2);
			m->NextRow();
			m->AddItem(halfb, wxALIGN_LEFT); m->NextRow();
			m->AddItem(halfresizeb, wxALIGN_LEFT); m->NextRow();
			m->AddItem(colorb, wxALIGN_LEFT); m->NextRow();

#ifdef USE_LIBRTPROCESS
			m->AddItem(ahdb, wxALIGN_LEFT); m->NextRow();
			m->AddItem(amazeb, wxALIGN_LEFT); m->NextRow();
			m->AddItem(dcbb, wxALIGN_LEFT); m->NextRow();
			m->AddItem(igvb, wxALIGN_LEFT); m->NextRow();
			m->AddItem(lmmseb, wxALIGN_LEFT); m->NextRow();
			m->AddItem(rcdb, wxALIGN_LEFT); m->NextRow();
			m->AddItem(vngb, wxALIGN_LEFT); m->NextRow();
			m->AddItem(xtran_markesteijnb, wxALIGN_LEFT); m->NextRow();
			m->AddItem(xtran_fastb, wxALIGN_LEFT); m->NextRow();
#endif
			SetSizerAndFit(m);

//			SetSizerAndFit(b);
			b->Layout();
			SetFocus();
			Refresh();
			Update();

			Bind(wxEVT_CHECKBOX, &DemosaicPanel::onEnable, this, DEMOSAICENABLE);
//			Bind(wxEVT_RADIOBOX, &DemosaicPanel::paramChanged, this, DEMOSAICOP);
			Bind(wxEVT_RADIOBUTTON, &DemosaicPanel::paramChanged, this);
		}

		~DemosaicPanel()
		{
			
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

/*
		void paramChanged(wxCommandEvent& event)
		{
			q->setParams(operselect->GetString(operselect->GetSelection()));
			q->processPic();
			event.Skip();
		}
*/
		void paramChanged(wxCommandEvent& event)
		{
			switch (event.GetId()) {
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
					q->setParams("dcb");
					break;
				case DEMOSAICIGV:
					q->setParams("igv");
					break;
				case DEMOSAICLMMSE:
					q->setParams("lmmse");
					break;
				case DEMOSAICRCD:
					q->setParams("rcd");
					break;
				case DEMOSAICVNG:
					q->setParams("vng");
					break;
				case DEMOSAICXTRANMARKESTEIJN:
					q->setParams("xtran_markesteijn");
					break;
				case DEMOSAICXTRANFAST:
					q->setParams("xtran_fast");
					break;
#endif
			}
			q->processPic();
			event.Skip();
		}


	private:
		wxCheckBox *enablebox;
		wxRadioBox *operselect;
		wxRadioButton *halfb, *halfresizeb, *colorb;
#ifdef USE_LIBRTPROCESS
		wxRadioButton *ahdb, *amazeb, *dcbb, *igvb, *lmmseb, *rcdb, *vngb, *xtran_markesteijnb, *xtran_fastb;
#endif


};

PicProcessorDemosaic::PicProcessorDemosaic(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorDemosaic::createPanel(wxSimplebook* parent)
{
	toolpanel = new DemosaicPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorDemosaic::processPic(bool processnext) 
{

	((wxFrame*) m_display->GetParent())->SetStatusText("demosaic...");
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.demosaic.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (processingenabled) {
		mark();
		if (c == "color")
			dib->ApplyMosaicColor(threadcount);
		else if (c == "half")
			dib->ApplyDemosaicHalf(false, threadcount);
		else if (c == "half_resize")
			dib->ApplyDemosaicHalf(true, threadcount);
#ifdef USE_LIBRTPROCESS
		else if (c == "vng") {
			dib->ApplyDemosaicVNG(threadcount);
		}
		else if (c == "rcd") {
			dib->ApplyDemosaicRCD(threadcount);
		}
		else if (c == "dcb") {
			int iterations = 1;
			bool dcb_enhance = false;
			dib->ApplyDemosaicDCB(iterations, dcb_enhance, threadcount);
		}
		else if (c == "amaze") {
			double initGain = 1.0;
			int border = 0;
			float inputScale = 1.0;
			float outputScale = 1.0;
			dib->ApplyDemosaicAMAZE(initGain, border, inputScale, outputScale, threadcount);
		}
		else if (c == "igv") {
			dib->ApplyDemosaicIGV(threadcount);
		}
		else if (c == "ahd") {
			dib->ApplyDemosaicAHD(threadcount);
		}
		else if (c == "lmmse") { 
			int iterations = 1;
			dib->ApplyDemosaicLMMSE(iterations, threadcount);
		}
		else if (c == "xtran_fast") {
			dib->ApplyDemosaicXTRANSFAST(threadcount);
		}
		else if (c == "xtran_markesteijn") { 
			int passes = 1;
			bool useCieLab = false;
			dib->ApplyDemosaicXTRANSMARKESTEIJN(passes, useCieLab, threadcount);
		}
#endif
		else 
			wxMessageBox(wxString::Format("Unknown demosaic algorithm: %s",c.c_str()));
		wxString d = duration();
		m_tree->SetItemText(id, wxString::Format("demosaic:%s",c));
		dib->setInfo("LibrawMosaiced", "0");

		//parm tool.demosaic.orient: Rotate the image to represent the EXIF Orientation value originally inputted, then set the Orientation tag to 1.  If you're going to use demosaic in the tool chain, you actually need to set input.orient=0 an leave this setting at its default, so the normalization is deferred until after demosaic.  Demosaic requires the image to be in its original orientation to preserve the specified Bayer pattern.  Default=1
		if (myConfig::getConfig().getValueOrDefault("tool.demosaic.orient","1") == "1") {
			((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("Normalizing image orientation..."));
			dib->NormalizeRotation(threadcount);
		}

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.demosaic.log","0") == "1"))
			log(wxString::Format("tool=demosaic,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





