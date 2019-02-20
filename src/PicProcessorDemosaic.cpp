#include "PicProcessorDemosaic.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myIntegerCtrl.h"
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

#define DCBITERATIONS 6914
#define DCBENHANCE 6915
#endif


class DemosaicPanel: public PicProcPanel
{

	public:
		DemosaicPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL, 3);

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
				dcb_iterations = new myIntegerCtrl(this, wxID_ANY, 1, 1, 5, wxDefaultPosition, wxSize(20,-1));
				dcb_enhance = new wxCheckBox(this, DCBENHANCE, "enhance");
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


			//Lay out the controls in the panel:
			myRowSizer *m = new myRowSizer();
			m->AddRowItem(enablebox, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			m->NextRow();
			m->AddRowItem(halfb, flags); m->NextRow();
			m->AddRowItem(halfresizeb, flags); m->NextRow();
			m->AddRowItem(colorb, flags); m->NextRow();

#ifdef USE_LIBRTPROCESS
			m->AddRowItem(ahdb, flags); m->NextRow();
			m->AddRowItem(amazeb, flags); m->NextRow();
			m->AddRowItem(dcbb, flags);
				m->AddRowItem(dcb_iterations, flags); m->AddRowItem(new wxStaticText(this, wxID_ANY, "iterations"),flags);  
				m->AddRowItem(dcb_enhance, flags);
				m->NextRow();
			m->AddRowItem(igvb, flags); m->NextRow();
			m->AddRowItem(lmmseb, flags); m->NextRow();
			m->AddRowItem(rcdb, flags); m->NextRow();
			m->AddRowItem(vngb, flags); m->NextRow();
			m->AddRowItem(xtran_markesteijnb, flags); m->NextRow();
			m->AddRowItem(xtran_fastb, flags); m->NextRow();
			m->End();
#endif

			SetSizerAndFit(m);
			b->Layout();
			SetFocus();
			Refresh();
			Update();

			Bind(wxEVT_CHECKBOX, &DemosaicPanel::onEnable, this, DEMOSAICENABLE);
			Bind(wxEVT_CHECKBOX, &DemosaicPanel::paramChanged, this);
			Bind(wxEVT_RADIOBUTTON, &DemosaicPanel::algorithmChanged, this);
			Bind(wxEVT_TEXT_ENTER, &DemosaicPanel::paramChanged, this);
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
		}


	private:
		wxCheckBox *enablebox;
		wxRadioBox *operselect;
		wxRadioButton *halfb, *halfresizeb, *colorb;
		int selected_algorithm;

#ifdef USE_LIBRTPROCESS
		wxRadioButton *ahdb, *amazeb, *dcbb, *igvb, *lmmseb, *rcdb, *vngb, *xtran_markesteijnb, *xtran_fastb;
		myIntegerCtrl *dcb_iterations;
		wxCheckBox *dcb_enhance;
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

	wxArrayString p = split(c,",");

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.demosaic.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (processingenabled) {
		mark();
		if (p[0] == "color")
			dib->ApplyMosaicColor(threadcount);
		else if (p[0] == "half")
			dib->ApplyDemosaicHalf(false, threadcount);
		else if (p[0] == "half_resize")
			dib->ApplyDemosaicHalf(true, threadcount);
#ifdef USE_LIBRTPROCESS
		else if (p[0] == "vng") {
			dib->ApplyDemosaicVNG(threadcount);
		}
		else if (p[0] == "rcd") {
			dib->ApplyDemosaicRCD(threadcount);
		}
		else if (p[0] == "dcb") {
			int iterations = 1;
			if (p.GetCount() >= 2) iterations = atoi(p[1].c_str());
			bool dcb_enhance = false;
			if (p.GetCount() >= 3) if (p[2] == "1") dcb_enhance = true;
			dib->ApplyDemosaicDCB(iterations, dcb_enhance, threadcount);
		}
		else if (p[0] == "amaze") {
			double initGain = 1.0;
			int border = 0;
			float inputScale = 1.0;
			float outputScale = 1.0;
			dib->ApplyDemosaicAMAZE(initGain, border, inputScale, outputScale, threadcount);
		}
		else if (p[0] == "igv") {
			dib->ApplyDemosaicIGV(threadcount);
		}
		else if (p[0] == "ahd") {
			dib->ApplyDemosaicAHD(threadcount);
		}
		else if (p[0] == "lmmse") { 
			int iterations = 1;
			dib->ApplyDemosaicLMMSE(iterations, threadcount);
		}
		else if (p[0] == "xtran_fast") {
			dib->ApplyDemosaicXTRANSFAST(threadcount);
		}
		else if (p[0] == "xtran_markesteijn") { 
			int passes = 1;
			bool useCieLab = false;
			dib->ApplyDemosaicXTRANSMARKESTEIJN(passes, useCieLab, threadcount);
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

			//parm tool.demosaic.orient: Rotate the image to represent the EXIF Orientation value originally inputted, then set the Orientation tag to 1.  If you're going to use demosaic in the tool chain, you actually need to set input.orient=0 an leave this setting at its default, so the normalization is deferred until after demosaic.  Demosaic requires the image to be in its original orientation to preserve the specified Bayer pattern.  Default=1
			if (myConfig::getConfig().getValueOrDefault("tool.demosaic.orient","1") == "1") {
				((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("Normalizing image orientation..."));
				dib->NormalizeRotation(threadcount);
			}

			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.demosaic.log","0") == "1"))
				log(wxString::Format("tool=demosaic,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

			dirty = false;
		}
	}

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





