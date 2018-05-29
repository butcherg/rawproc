
#include "PicProcessor.h"
#include "PicProcessorBlackWhitePoint.h"
#include "PicProcPanel.h"
#include "myDoubleSlider.h"
#include "myConfig.h"
#include "undo.xpm"
#include "util.h"


class BlackWhitePointPanel: public PicProcPanel
{
	public:
		BlackWhitePointPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			int blk, wht;
			wxArrayString p = split(params,",");

			//int whtlimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitelimit","128").c_str());
			//int blklimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blacklimit","128").c_str());

			//parm tool.blackwhitepoint.autorecalcdefault: 0/1, default setting for recalc when the tool is added.  Default=0
			int recalcdefault = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.autorecalcdefault","0").c_str());


			
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL, 2); //.Expand();
			wxArrayString str;
			str.Add("rgb");
			str.Add("red");
			str.Add("green");
			str.Add("blue");
			chan = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, str);
			
			if ((p[0] == "rgb") | (p[0] == "red") | (p[0] == "green") | (p[0] == "blue")) {
				chan->SetStringSelection(p[0]);
				blk = atoi(p[1]);
				wht = atoi(p[2]);
			}
			else {
				chan->SetSelection(chan->FindString("rgb"));
				blk = atoi(p[0]);
				wht = atoi(p[1]);
			}

			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);


			/*  Old sliders:
			g->Add(new wxStaticText(this,wxID_ANY, "black: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			black = new wxSlider(this, wxID_ANY, blk, 0, blklimit, wxPoint(10, 30), wxSize(140, -1));
			g->Add(black , wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val1 = new wxStaticText(this,wxID_ANY, wxString::Format("%4d",blk), wxDefaultPosition, wxSize(30, -1));
			g->Add(val1 , wxGBPosition(1,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn1 = new wxBitmapButton(this, 1000, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn1->SetToolTip("Reset black point to default");
			g->Add(btn1, wxGBPosition(1,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);


			g->Add(new wxStaticText(this,wxID_ANY, "white: "), wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			white = new wxSlider(this, wxID_ANY, wht, whtlimit, 255, wxPoint(10, 30), wxSize(140, -1));
			g->Add(white , wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val2 = new wxStaticText(this,wxID_ANY, wxString::Format("%4d",wht), wxDefaultPosition, wxSize(30, -1));
			g->Add(val2 , wxGBPosition(2,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn2 = new wxBitmapButton(this, 2000, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn2->SetToolTip("Reset white point to default");
			g->Add(btn2, wxGBPosition(2,3), wxDefaultSpan, wxALIGN_LEFT | wxLEFT | wxRIGHT | wxTOP, 3);
			*/
			
			

			g->Add(new wxStaticText(this,wxID_ANY, "black/white: "), wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT | wxLEFT | wxTOP, 3);
			//g->Add(1,10, wxGBPosition(0,0));
			
			g->Add(chan, wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT | wxLEFT | wxTOP, 3);
			bwpoint = new myDoubleSlider(this, wxID_ANY, blk, wht, 0, 255, wxDefaultPosition, wxDefaultSize);
			g->Add(bwpoint , wxGBPosition(3,0), wxGBSpan(1,4), wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM, 3);
			recalc = new wxCheckBox(this, wxID_ANY, "ReCalculate");
			g->Add(recalc, wxGBPosition(4,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			if (recalcdefault) recalc->SetValue(true);

			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &BlackWhitePointPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &BlackWhitePointPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &BlackWhitePointPanel::OnThumbTrack, this);
			Bind(wxEVT_CHOICE, &BlackWhitePointPanel::channelChanged, this);
			Bind(wxEVT_CHECKBOX, &BlackWhitePointPanel::OnCheckBox, this);
			Bind(wxEVT_TIMER, &BlackWhitePointPanel::OnTimer,  this);
		}

		~BlackWhitePointPanel()
		{
			t->~wxTimer();
		}

		void OnChanged(wxCommandEvent& event)
		{
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{

		}
		
		void OnCheckBox(wxCommandEvent& event)
		{
			((PicProcessorBlackWhitePoint *) q)->setReCalc(recalc->GetValue());
			q->processPic();
			event.Skip();
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%s,%d,%d",chan->GetString(chan->GetSelection()), bwpoint->GetLeftValue(),bwpoint->GetRightValue()));
			q->processPic();
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
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
/*
			int resetblackval, resetwhiteval;
			switch (event.GetId()) {
				case 1000:
					resetblackval = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blackinitialvalue","0").c_str());
					black->SetValue(resetblackval);
					val1->SetLabel(wxString::Format("%4d", resetblackval));
					break;
				case 2000:
					resetwhitekval = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whiteinitialvalue","255").c_str());
					white->SetValue(resetwhiteval);
					val2->SetLabel(wxString::Format("%4d", resetwhiteval));
					break;
			}
			q->setParams(wxString::Format("%d,%d",black->GetValue(),white->GetValue()));
			q->processPic();
*/
			event.Skip();
		}


	private:
		wxChoice *chan;
		wxSlider *black, *white;
		wxCheckBox *recalc;
		myDoubleSlider *bwpoint;
		wxStaticText *val1, *val2;
		wxBitmapButton *btn1, * btn2;
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

	
	if (command == "") {
		std::vector<double> bwpts = getPreviousPicProcessor()->getProcessedPic().CalculateBlackWhitePoint(blkthresh, whtthresh, true, whtinitial);
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

bool PicProcessorBlackWhitePoint::processPic(bool processnext) {
	double blk, wht; 
	((wxFrame*) m_display->GetParent())->SetStatusText("black/white point...");

	if (recalc) {
		reCalc();
		((BlackWhitePointPanel *) toolpanel)->updateSliders();
	}

	wxArrayString p = split(getParams(),",");
	
	if ((p[0] == "rgb") | (p[0] == "red") | (p[0] == "green") | (p[0] == "blue")) {
		setChannel(p[0]);
		blk = atof(p[1]);
		wht = atof(p[2]);
	}
	else {
		setChannel("rgb");
		blk = atof(p[0]);
		wht = atof(p[1]);
	}


	Curve ctrlpts;
	ctrlpts.insertpoint(blk,0);
	ctrlpts.insertpoint(wht,255);
	
	bool result = true;

	//parm tool.*.cores: Sets the number of processing cores used by the tool.  0=use all available, -N=use available minus n.  Default=0);
	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.cores","0").c_str());
	
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyToneCurve(ctrlpts.getControlPoints(), channel, threadcount);
	wxString d = duration();

	//parm tool.all.log: Turns on logging for all tools.  Default=0
	//parm tool.*.log: Turns on logging for the specified tool.  Default=0
	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.log","0") == "1"))
		log(wxString::Format("tool=blackwhitepoint,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty=false;
	
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();

	return result;
}

void PicProcessorBlackWhitePoint::displayProcessedPic() 
{
	if (m_display) {
		m_display->SetPic(dib, channel);
		m_display->SetDrawList(dcList);
	}
}


