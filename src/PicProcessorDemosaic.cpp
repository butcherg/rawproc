#include "PicProcessorDemosaic.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"

#define DEMOSAICENABLE 6900
#define DEMOSAICOP 6901

class DemosaicPanel: public PicProcPanel
{

	public:
		DemosaicPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);

			enablebox = new wxCheckBox(this, DEMOSAICENABLE, "demosaic:");
			enablebox->SetValue(true);
			b->Add(enablebox, flags);
			b->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			b->AddSpacer(10);

			wxArrayString opers;
#ifdef USE_LIBRTPROCESS
			opers.Add("ahd");
			opers.Add("amaze");
			opers.Add("dcb");
			opers.Add("igv");
			//opers.Add("lmmse");
			opers.Add("rcd");
			opers.Add("vng");
#endif
			opers.Add("half");
			opers.Add("half_resize");
			opers.Add("color");

			operselect = new wxRadioBox (this, DEMOSAICOP, "Operation", wxDefaultPosition, wxDefaultSize,  opers, 1, wxRA_SPECIFY_COLS);
			operselect->SetItemToolTip(operselect->FindString("ahd"),"");
			operselect->SetItemToolTip(operselect->FindString("amaze"),"Good for low ISO images, architectural images. Fast.");
			operselect->SetItemToolTip(operselect->FindString("dcb"),"");
			operselect->SetItemToolTip(operselect->FindString("igv"),"");
			//operselect->SetItemToolTip(operselect->FindString("lmmse"),"");
			operselect->SetItemToolTip(operselect->FindString("rcd"),"Good for low ISO images, nature images. Faster than amaze.");
			operselect->SetItemToolTip(operselect->FindString("vng"),"Slow, good for medium ISO images.");
			operselect->SetItemToolTip(operselect->FindString("half"),"Turns each bayer quad into a single RGB pixel. Demosaiced mage is half the size of the original.  Fast.");
			operselect->SetItemToolTip(operselect->FindString("half_resize"),"Does half, then resizes to the original width and height. Somewhat fast, low quality.");
			operselect->SetItemToolTip(operselect->FindString("color"),"Doesn't demosaic, colors the image according to the bayer array");

			if (params != "") operselect->SetSelection(operselect->FindString(params));
			b->Add(operselect,flags);

			SetSizerAndFit(b);
			b->Layout();
			SetFocus();
			Refresh();
			Update();

			Bind(wxEVT_CHECKBOX, &DemosaicPanel::onEnable, this, DEMOSAICENABLE);
			Bind(wxEVT_RADIOBOX, &DemosaicPanel::paramChanged, this, DEMOSAICOP);
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
			q->setParams(operselect->GetString(operselect->GetSelection()));
			q->processPic();
			event.Skip();
		}

	private:
		wxCheckBox *enablebox;
		wxRadioBox *operselect;

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
			dib->ApplyDemosaic(DEMOSAIC_COLOR, threadcount);
		else if (c == "half")
			dib->ApplyDemosaic(DEMOSAIC_HALF, threadcount);
		else if (c == "half_resize")
			dib->ApplyDemosaic(DEMOSAIC_HALF_RESIZE, threadcount);
#ifdef USE_LIBRTPROCESS
		else if (c == "vng") 
			dib->ApplyDemosaic(DEMOSAIC_VNG, threadcount);
		else if (c == "rcd") 
			dib->ApplyDemosaic(DEMOSAIC_RCD, threadcount);
		else if (c == "dcb") 
			dib->ApplyDemosaic(DEMOSAIC_DCB, threadcount);
		else if (c == "amaze") 
			dib->ApplyDemosaic(DEMOSAIC_AMAZE, threadcount);
		else if (c == "igv") 
			dib->ApplyDemosaic(DEMOSAIC_IGV, threadcount);
		else if (c == "ahd") 
			dib->ApplyDemosaic(DEMOSAIC_AHD, threadcount);
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





