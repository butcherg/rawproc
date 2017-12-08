
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
			wxArrayString p = split(params,",");

			int whtlimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitelimit","128").c_str());
			int blklimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blacklimit","128").c_str());

			int blk = atoi(p[0]);
			int wht = atoi(p[1]);

			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			g->Add(0,10, wxGBPosition(0,0));

			/*
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

			g->Add(new wxStaticText(this,wxID_ANY, "black/white: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxLEFT | wxTOP, 3);
			bwpoint = new myDoubleSlider(this, wxID_ANY, blk, wht, 0, 255, wxDefaultPosition, wxDefaultSize);
			g->Add(bwpoint , wxGBPosition(2,0), wxGBSpan(1,4), wxALIGN_LEFT | wxLEFT | wxRIGHT | wxBOTTOM, 3);


			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &BlackWhitePointPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &BlackWhitePointPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &BlackWhitePointPanel::OnThumbTrack, this);
			Bind(wxEVT_TIMER, &BlackWhitePointPanel::OnTimer,  this);
		}

		~BlackWhitePointPanel()
		{
			t->~wxTimer();
		}

		void OnChanged(wxCommandEvent& event)
		{
			//val1->SetLabel(wxString::Format("%4d", black->GetValue()));
			//val2->SetLabel(wxString::Format("%4d", white->GetValue()));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			//val1->SetLabel(wxString::Format("%4d", black->GetValue()));
			//val2->SetLabel(wxString::Format("%4d", white->GetValue()));
		}

		void OnTimer(wxTimerEvent& event)
		{
			//q->setParams(wxString::Format("%d,%d",black->GetValue(),white->GetValue()));
			q->setParams(wxString::Format("%d,%d",bwpoint->GetLeftValue(),bwpoint->GetRightValue()));
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
		wxSlider *black, *white;
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
	//parm tool.blackwhitepoint.whitelimit: The lower whitepoint limit.  Default=128
	int whtlimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitelimit","128").c_str());
	//parm tool.blackwhitepoint.blacklimit: The upper blackpoint limit.  Default=128
	int blklimit = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blacklimit","128").c_str());

	if (command == "") {
		std::vector<long> hdata = dib->Histogram();
		long hmax=0;
		long htotal = 0;
		
		//parm tool.blackwhitepoint.blackthreshold: The percent threshold used by the auto algorithm for the black adjustment. Only used when the blackwhitepoint tool is created. Default=0.05
		double blkthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.blackthreshold","0.05").c_str());
		//parm tool.blackwhitepoint.whitethreshold: The percent threshold used by the auto algorithm for the white adjustment. Only used when the blackwhitepoint tool is created. Default=0.05
		double whtthresh = atof(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whitethreshold","0.05").c_str());
		//parm tool.blackwhitepoint.whiteinitialvalue: The initial whitepoint setting, or the starting point in the histogram for walking down to the white threshold in auto.  Use to bypass bunched clipped highlights.  Default=255
		long whtinitial = atoi(myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.whiteinitialvalue","255").c_str());

		for (i=0; i<256; i++) htotal += hdata[i];

		//find black threshold:
		long hblack = 0;
		for (i=1; i<blklimit; i++) {
			hblack += hdata[i];
			if ((double) hblack / (double) htotal > blkthresh) break;
		}
		blk = (double) i;


		//find white threshold:
		long hwhite = 0;
		for (i=255; i>whtlimit; i--) {
			hwhite += hdata[i];
			if ((double) hwhite / (double) htotal > whtthresh) break;
		}
		wht = (double) i;
		
		setParams(wxString::Format("%d,%d",(unsigned) blk, (unsigned) wht));
	}
	else setParams(command);
	//showParams();
	//processPic();
}

void PicProcessorBlackWhitePoint::createPanel(wxSimplebook* parent)
{
	toolpanel = new BlackWhitePointPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorBlackWhitePoint::processPic(bool processnext) {
	((wxFrame*) m_display->GetParent())->SetStatusText("black/white point...");

	wxArrayString cp = split(getParams(),",");
	double blk = atof(cp[0]);
	double wht = atof(cp[1]);

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
	dib->ApplyToneCurve(ctrlpts.getControlPoints(), threadcount);
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



