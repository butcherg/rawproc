
#include "PicProcessor.h"
#include "PicProcessorBlackWhitePoint.h"
#include "PicProcPanel.h"
#include "myDoubleSlider.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "undo.xpm"
#include "copy.xpm"
#include "paste.xpm"
#include "util.h"

#define BLACKWHITEENABLE 6100
#define BLACKWHITEAUTORECALC 6101
#define BLACKWHITERECALC 6102
#define BLACKWHITESLIDER 6103
#define BLACKWHITEDATA   6104
#define BLACKWHITEMINWHITE   6105
#define BLACKWHITECAMERA 6106
#define BLACKWHITECOPY 6107
#define BLACKWHITEPASTE 6108

class BlackWhitePointPanel: public PicProcPanel
{
	public:
		BlackWhitePointPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			int blk=0, wht=255;
			wxArrayString p = split(params,",");

			//int whtlimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitelimit","128").c_str());
			//int blklimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blacklimit","128").c_str());

			//parm tool.blackwhitepoint.autorecalcdefault: 0/1, default setting for recalc when the tool is added.  Default=0
			int recalcdefault = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.autorecalcdefault","0").c_str());


			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL, 2); //.Expand();

			SetSize(parent->GetSize());
			
			enablebox = new wxCheckBox(this, BLACKWHITEENABLE, "black/white:");
			enablebox->SetValue(true);

			slideb = new wxRadioButton(this, BLACKWHITESLIDER, "auto/slider", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			datb   = new wxRadioButton(this, BLACKWHITEDATA,   "data", wxDefaultPosition, wxDefaultSize);
			camb   = new wxRadioButton(this, BLACKWHITECAMERA, "camera", wxDefaultPosition, wxDefaultSize);
			
			bwpoint = new myDoubleSlider(this, wxID_ANY, blk, wht, 0, 255, wxDefaultPosition, wxDefaultSize);
			recalc = new wxCheckBox(this, BLACKWHITEAUTORECALC, "auto recalc");
			if (recalcdefault) recalc->SetValue(true);

			wxArrayString str;
			str.Add("rgb");
			str.Add("red");
			str.Add("green");
			str.Add("blue");
			chan = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, str);

			bwmode = BLACKWHITESLIDER;
			slideb->SetValue(true);
			blk = 0.0;
			wht = 255.0;

			gImage &img = proc->getPreviousPicProcessor()->getProcessedPic();
			std::map<std::string,std::string> s = img.StatsMap();
			double datblk = fmin(fmin(atof(s["rmin"].c_str()),atof(s["gmin"].c_str())),atof(s["bmin"].c_str()));
			double datwht = fmax(fmax(atof(s["rmax"].c_str()),atof(s["gmax"].c_str())),atof(s["bmax"].c_str()));
			double minwht = fmin(fmin(atof(s["rmax"].c_str()),atof(s["gmax"].c_str())),atof(s["bmax"].c_str()));
			int librawblk = atoi(img.getInfoValue("LibrawBlack").c_str());
			double camblk = librawblk / 65536.0; 
			int librawwht = atoi(img.getInfoValue("LibrawMaximum").c_str());
			double camwht = librawwht / 65536.0; 

			minwhite = new wxCheckBox(this, BLACKWHITEMINWHITE, "min white:");

			
			if ((p[0] == "rgb") | (p[0] == "red") | (p[0] == "green") | (p[0] == "blue")) {
				chan->SetStringSelection(p[0]);
				if (p.size() >= 2 && p[1] == "data") {
					bwmode = BLACKWHITEDATA;
					if (p.size() >= 3 && p[2] == "minwhite") minwhite->SetValue(true);
					datb->SetValue(true);
				}
				else if (p.size() >= 2 && p[1] == "camera") {
					bwmode = BLACKWHITECAMERA;
					camb->SetValue(true);
				}
				else if (p.size() >= 3) {
					bwmode = BLACKWHITESLIDER;
					slideb->SetValue(true);
					blk = atoi(p[1]);
					wht = atoi(p[2]);
					bwpoint->SetLeftValue(blk);
					bwpoint->SetRightValue(wht);
				}
			}
			else {
				chan->SetSelection(chan->FindString("rgb"));
				bwmode = BLACKWHITESLIDER;
				slideb->SetValue(true);
				if (p.size() >= 3) {
					blk = atoi(p[0]);
					wht = atoi(p[1]);
					bwpoint->SetLeftValue(blk);
					bwpoint->SetRightValue(wht);
				}
			}

			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, BLACKWHITECOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, BLACKWHITEPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(chan, wxSizerFlags(0).Right().Border(wxRIGHT|wxTOP));
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(slideb, flags);
			m->NextRow();
			m->AddRowItem(bwpoint, flags);
			m->NextRow();
			m->AddRowItem(recalc, flags);
			m->AddRowItem(new wxButton(this, BLACKWHITERECALC, "recalc"), flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(datb, flags);
			m->AddRowItem(new wxStaticText(this, wxID_ANY, wxString::Format("black: %f\nwhite: %f",datblk, datwht)), flags);
			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "        "),flags);
			m->AddRowItem(minwhite, flags);
			m->AddRowItem(new wxStaticText(this, wxID_ANY, wxString::Format("%f",minwht)), flags.CenterVertical());

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(camb, flags);
			m->AddRowItem(new wxStaticText(this, wxID_ANY, wxString::Format("black: %f (%d)\nwhite: %f (%d)",camblk, librawblk, camwht,librawwht)), flags);
			m->End();

			SetSizerAndFit(m);
			m->Layout();
			bwpoint->Refresh();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_RADIOBUTTON, &BlackWhitePointPanel::OnRadioButton, this);
			Bind(wxEVT_BUTTON, &BlackWhitePointPanel::OnButton, this, BLACKWHITERECALC);
			Bind(wxEVT_BUTTON, &BlackWhitePointPanel::OnCopy, this, BLACKWHITECOPY);
			Bind(wxEVT_BUTTON, &BlackWhitePointPanel::OnPaste, this, BLACKWHITEPASTE);
			Bind(wxEVT_SCROLL_CHANGED, &BlackWhitePointPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &BlackWhitePointPanel::OnThumbTrack, this);
			Bind(wxEVT_CHOICE, &BlackWhitePointPanel::channelChanged, this);
			Bind(wxEVT_CHECKBOX, &BlackWhitePointPanel::OnCheckBox, this, BLACKWHITEAUTORECALC);
			Bind(wxEVT_CHECKBOX, &BlackWhitePointPanel::onEnable, this, BLACKWHITEENABLE);
			Bind(wxEVT_CHECKBOX, &BlackWhitePointPanel::OnMinWhite, this, BLACKWHITEMINWHITE);
			Bind(wxEVT_TIMER, &BlackWhitePointPanel::OnTimer,  this);
		}

		~BlackWhitePointPanel()
		{
			t->~wxTimer();
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
				double blk=0.0, wht=255.0;
				wxArrayString p = split(q->getParams(),",");

				if ((p[0] == "rgb") | (p[0] == "red") | (p[0] == "green") | (p[0] == "blue")) {
					chan->SetStringSelection(p[0]);
					if (p.size() >= 2 && p[1] == "data") {
						bwmode = BLACKWHITEDATA;
						datb->SetValue(true);
					}
					else if (p.size() >= 2 && p[1] == "camera") {
						bwmode = BLACKWHITECAMERA;
						if (p.size() >= 3 && p[2] == "minwhite") minwhite->SetValue(true);
						camb->SetValue(true);
					}
					else if (p.size() >= 3) {
						bwmode = BLACKWHITESLIDER;
						slideb->SetValue(true);
						blk = atoi(p[1]);
						wht = atoi(p[2]);
						bwpoint->SetLeftValue(blk);
						bwpoint->SetRightValue(wht);
					}
				}
				else {
					chan->SetSelection(chan->FindString("rgb"));
					bwmode = BLACKWHITESLIDER;
					slideb->SetValue(true);
					if (p.size() >= 3) {
						blk = atoi(p[0]);
						wht = atoi(p[1]);
						bwpoint->SetLeftValue(blk);
						bwpoint->SetRightValue(wht);
					}
				}

				q->processPic();
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format("Pasted command from clipboard: %s",q->getCommand()));
			}
			else wxMessageBox(wxString::Format("Invalid Paste"));
		}

		void OnRadioButton(wxCommandEvent& event)
		{
			bwmode = event.GetId();
			processBW();
		}

		void OnMinWhite(wxCommandEvent& event)
		{
			processBW();
		}

		void processBW()
		{
			switch (bwmode) {
				case BLACKWHITESLIDER:
					q->setParams(wxString::Format("%s,%d,%d",chan->GetString(chan->GetSelection()), bwpoint->GetLeftValue(),bwpoint->GetRightValue()));
					q->processPic();
					break;
				case BLACKWHITEDATA:
					if (minwhite->GetValue())
						q->setParams(wxString::Format("%s,data,minwhite",  chan->GetString(chan->GetSelection())));
					else
						q->setParams(wxString::Format("%s,data",  chan->GetString(chan->GetSelection())));
					q->processPic();
					break;
				case BLACKWHITECAMERA:
					q->setParams(wxString::Format("%s,camera",chan->GetString(chan->GetSelection())));
					q->processPic();
					break;
			}
		}



		void OnChanged(wxCommandEvent& event)
		{
			if (bwmode == BLACKWHITESLIDER) t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{

		}
		
		void OnCheckBox(wxCommandEvent& event)
		{
			((PicProcessorBlackWhitePoint *) q)->setReCalc(recalc->GetValue());
			processBW();
			event.Skip();
		}

		void OnTimer(wxTimerEvent& event)
		{
			processBW();
			event.Skip();
		}

		void updateSliders()
		{
			wxArrayString p = split(q->getParams(),",");
			bwpoint->SetLeftValue(atoi(p[1]));
			bwpoint->SetRightValue(atoi(p[2]));
		}
		
		void channelChanged(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%s,%d,%d",chan->GetString(chan->GetSelection()),0,255));
			((PicProcessorBlackWhitePoint *) q)->setChannel(chan->GetString(chan->GetSelection()));
			
			((PicProcessorBlackWhitePoint *) q)->reCalc();
			updateSliders();
			processBW();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			((PicProcessorBlackWhitePoint *) q)->reCalc();
			updateSliders();
			processBW();
			event.Skip();
		}


	private:
		wxChoice *chan;
		wxSlider *black, *white;
		wxCheckBox *recalc,*enablebox, *minwhite;
		wxRadioButton *slideb, *datb, *camb;
		myDoubleSlider *bwpoint;
		wxStaticText *val1, *val2;
		wxBitmapButton *btn1, * btn2;
		int bwmode;
		wxTimer *t;

};


PicProcessorBlackWhitePoint::PicProcessorBlackWhitePoint(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	int i;
	double blk, wht;
	wht = 255; blk = 0;
	recalc = false;

	//parm tool.blackwhitepoint.blackthreshold: The percent threshold used by the auto algorithm for the black adjustment. Only used when the blackwhitepoint tool is created. Default=0.05
	double blkthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blackthreshold","0.05").c_str());
	//parm tool.blackwhitepoint.whitethreshold: The percent threshold used by the auto algorithm for the white adjustment. Only used when the blackwhitepoint tool is created. Default=0.05
	double whtthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitethreshold","0.05").c_str());
	//parm tool.blackwhitepoint.whiteinitialvalue: The initial whitepoint setting, or the starting point in the histogram for walking down to the white threshold in auto.  Use to bypass bunched clipped highlights.  Default=255
	long whtinitial = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whiteinitialvalue","255").c_str());
	//parm tool.blackwhitepoint.automode: The calculation mode for auto.  tone=find the points based on the tone; min=find the points based on the min of the channel maxes. Default: tone
	wxString automode = myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.automode","tone").c_str();
	
	if (command == "") {
		std::vector<double> bwpts;
		if (automode=="min")
			bwpts = getPreviousPicProcessor()->getProcessedPic().CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial, "min");
		else
			bwpts = getPreviousPicProcessor()->getProcessedPic().CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial);
		blk = bwpts[0];
		wht = bwpts[1];
		setParams(wxString::Format("rgb,%d,%d",(unsigned) blk, (unsigned) wht));
	}
	else {
		wxArrayString p = split(command,",");
		if ((p[0] == "rgb") | (p[0] == "red") | (p[0] == "green") | (p[0] == "blue")) {
			if (p.GetCount() == 1) {
				std::vector<double> bwpts = getPreviousPicProcessor()->getProcessedPic().CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial, std::string(p[0].c_str()));
				blk = bwpts[0];
				wht = bwpts[1];
				setParams(wxString::Format("%s,%d,%d",p[0],(unsigned) blk, (unsigned) wht));
			}
			else {
				setParams(wxString::Format("%s,%s,%s",p[0],p[1],p[2]));
				
			}
		}
//		setParams(command);
	}
}

void PicProcessorBlackWhitePoint::createPanel(wxSimplebook* parent)
{
	toolpanel = new BlackWhitePointPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

void PicProcessorBlackWhitePoint::setReCalc(bool r)
{
	recalc = r;
	if (recalc) reCalc();
}

void PicProcessorBlackWhitePoint::setChannel(wxString chan)
{
	if (chan == "rgb")   channel = CHANNEL_RGB;
	if (chan == "red")   channel = CHANNEL_RED;
	if (chan == "green") channel = CHANNEL_GREEN;
	if (chan == "blue")  channel = CHANNEL_BLUE;
	m_tree->SetItemText(id, wxString::Format("blackwhitepoint:%s",chan));
}

void PicProcessorBlackWhitePoint::reCalc()
{
	double blkthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blackthreshold","0.05").c_str());
	double whtthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitethreshold","0.05").c_str());
	long whtinitial = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whiteinitialvalue","255").c_str());

	wxArrayString p = split(getParams(),",");
	std::vector<double> bwpts = getPreviousPicProcessor()->getProcessedPic().CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial, std::string(p[0].c_str()));
	double blk = bwpts[0];
	double wht = bwpts[1];
	setParams(wxString::Format("%s,%d,%d",p[0],(unsigned) blk, (unsigned) wht));
}

bool PicProcessorBlackWhitePoint::processPic(bool processnext) 
{
	double blk, wht; 
	((wxFrame*) m_display->GetParent())->SetStatusText("black/white point...");

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (!global_processing_enabled) return true;

	if (recalc) {
		reCalc();
		((BlackWhitePointPanel *) toolpanel)->updateSliders();
	}

	wxArrayString p = split(getParams(),",");
	
	if ((p[0] == "rgb") | (p[0] == "red") | (p[0] == "green") | (p[0] == "blue")) {
		setChannel(p[0]);
		if (p[1] == "data") {
			std::map<std::string,std::string> s = dib->StatsMap();
			if (channel == CHANNEL_RED) {
				blk = atof(s["rmin"].c_str());
				wht = atof(s["rmax"].c_str());
			} 
			else if (channel == CHANNEL_GREEN) {
				blk = atof(s["gmin"].c_str());
				wht = atof(s["gmax"].c_str());
			}
			else if (channel == CHANNEL_BLUE) {
				blk = atof(s["bmin"].c_str());
				wht = atof(s["bmax"].c_str());
			}
			else if (channel == CHANNEL_RGB) {
				blk = fmin(fmin(atof(s["rmin"].c_str()),atof(s["gmin"].c_str())),atof(s["bmin"].c_str()));
				if (p.size() >= 3 && p[2] == "minwhite")
					wht = fmin(fmin(atof(s["rmax"].c_str()),atof(s["gmax"].c_str())),atof(s["bmax"].c_str()));
				else
					wht = fmax(fmax(atof(s["rmax"].c_str()),atof(s["gmax"].c_str())),atof(s["bmax"].c_str()));
			}
			if (blk < 0.0) blk = 0.0;
		}
		else if (p[1] == "camera") {
			blk = atof(dib->getInfoValue("LibrawBlack").c_str())/65536.0;
			wht = atof(dib->getInfoValue("LibrawMaximum").c_str())/65536.0;
		}
		else {
			blk = atof(p[1]);
			wht = atof(p[2]);
		}
	}
	else {
		setChannel("rgb");
		blk = atof(p[0]);
		wht = atof(p[1]);
	}

	bool result = true;

	//parm tool.*.cores: Sets the number of processing cores used by the tool.  0=use all available, -N=use available minus n.  Default=0);
	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.cores","0").c_str());
	
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);


	if (processingenabled) {
		mark();
		dib->ApplyToneLine(blk, wht, channel, threadcount);
		wxString d = duration();

		//parm tool.all.log: Turns on logging for all tools.  Default=0
		//parm tool.*.log: Turns on logging for the specified tool.  Default=0
		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.log","0") == "1"))
			log(wxString::Format("tool=blackwhitepoint,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty=false;
	
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();

	return result;
}

void PicProcessorBlackWhitePoint::displayProcessedPic() 
{
	if (m_display) {
		m_display->SetPic(dib, channel);
//		m_display->SetDrawList(dcList);
	}
}


