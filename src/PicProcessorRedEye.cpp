#include "PicProcessor.h"
#include "PicProcessorRedEye.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "undo.xpm"

#include "util.h"

#define REDEYEENABLE 7300
#define REDEYEDESAT 7301

class RedEyePanel: public PicProcPanel
{
	public:
		RedEyePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			wxGridBagSizer *g = new wxGridBagSizer();

			//std::vector<std::string> p = split(params,",");
			wxArrayString p = split(params,",");
			double thr = atof(p[0].c_str());
			int rad = atoi(p[1].c_str());
			int desatbool = atoi(p[2].c_str());
			double dpct = atof(p[3].c_str());
			

			enablebox = new wxCheckBox(this, REDEYEENABLE, _("redeye:"));
			enablebox->SetValue(true);
			g->Add(enablebox, wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)),  wxGBPosition(1,0), wxGBSpan(1,4), wxALIGN_LEFT | wxBOTTOM | wxEXPAND, 10);

			//Threshold slider
			g->Add(new wxStaticText(this,wxID_ANY, _("threshold: ")), wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			threshold = new wxSlider(this, wxID_ANY, thr*10, 0, 50, wxPoint(10, 30), wxSize(140, -1));
			g->Add(threshold , wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val1 = new wxStaticText(this,wxID_ANY, p[0], wxDefaultPosition, wxSize(30, -1));
			g->Add(val1 , wxGBPosition(2,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn1 = new wxBitmapButton(this, 9000, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn1->SetToolTip(_("Reset to default"));
			g->Add(btn1, wxGBPosition(2,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			
			//Limit, or radius, slider
			g->Add(new wxStaticText(this,wxID_ANY, _("radius: ")), wxGBPosition(3,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			radius = new wxSlider(this, wxID_ANY, rad, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(radius , wxGBPosition(3,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val2 = new wxStaticText(this,wxID_ANY, p[1], wxDefaultPosition, wxSize(30, -1));
			g->Add(val2, wxGBPosition(3,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn2 = new wxBitmapButton(this, 9001, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn2->SetToolTip(_("Reset to default"));
			g->Add(btn2, wxGBPosition(3,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			
			g->Add(0,10, wxGBPosition(4,0));
			
			desat = new wxCheckBox(this, REDEYEDESAT, _("desaturate"));
			g->Add(desat, wxGBPosition(5,0), wxGBSpan(1,2), wxALIGN_LEFT | wxALL, 3);
			if (desatbool == 1) desat->SetValue(true);

			//desaturate adjustment
			g->Add(new wxStaticText(this,wxID_ANY, _("percent: ")), wxGBPosition(6,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			desatpct = new wxSlider(this, wxID_ANY, dpct*100, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(desatpct , wxGBPosition(6,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val3 = new wxStaticText(this,wxID_ANY, p[3], wxDefaultPosition, wxSize(30, -1));
			g->Add(val3, wxGBPosition(6,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn3 = new wxBitmapButton(this, 9002, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn3->SetToolTip(_("Reset to default"));
			g->Add(btn3, wxGBPosition(6,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			g->Add(0,10, wxGBPosition(7,0));
			wxString help = _("Shift-Click the center of the red eye to apply.\nCtrl-Click near the cross to remove application.");
			g->Add(new wxStaticText(this, wxID_ANY, help), wxGBPosition(8,0), wxGBSpan(1,4), wxALIGN_LEFT | wxALL, 5);
			
			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_BUTTON, &RedEyePanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &RedEyePanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &RedEyePanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &RedEyePanel::OnCheckBox, this, REDEYEDESAT);
			Bind(wxEVT_CHECKBOX, &RedEyePanel::OnEnable, this, REDEYEENABLE);
			Bind(wxEVT_TIMER, &RedEyePanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &RedEyePanel::OnKey,  this);
		}

		void OnEnable(wxCommandEvent& event)
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

		void OnChanged(wxCommandEvent& event)
		{
			val1->SetLabel(wxString::Format("%2.2f", (float) threshold->GetValue() / 10.0));
			val2->SetLabel(wxString::Format("%d", radius->GetValue()));
			val3->SetLabel(wxString::Format("%2.2f", (float) desatpct->GetValue() / 100.0));
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val1->SetLabel(wxString::Format("%2.2f", (float) threshold->GetValue() / 10.0));
			val2->SetLabel(wxString::Format("%d", radius->GetValue()));	
			val3->SetLabel(wxString::Format("%2.2f", (float) desatpct->GetValue() / 100.0));
		}
		
		void OnCheckBox(wxCommandEvent& event)
		{
			((PicProcessorRedEye *) q)->setDesat(desat->GetValue());
			q->processPic();
		}

		void OnTimer(wxTimerEvent& event)
		{
			((PicProcessorRedEye *) q)->setThreshold((float) threshold->GetValue() / 10.0);
			((PicProcessorRedEye *) q)->setRadius(radius->GetValue());
			((PicProcessorRedEye *) q)->setDesatPercent(desatpct->GetValue()/100.0);
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetthr = atof(myConfig::getConfig().getValueOrDefault("tool.redeye.threshold.initialvalue","1.5").c_str());
			int resetrad = atoi(myConfig::getConfig().getValueOrDefault("tool.redeye.radius.initialvalue","50").c_str());
			double desatpct = atof(myConfig::getConfig().getValueOrDefault("tool.redeye.desaturatepercent.initialvalue","1.0").c_str());
			threshold->SetValue(resetthr*10);
			radius->SetValue(resetrad);
			desat->SetValue(100);
			//((PicProcessorRedEye *) q)->setThresholdLimit(wxString::Format("%2.2f,%d",resetthr,resetrad));
			((PicProcessorRedEye *) q)->setThreshold(resetthr);
			((PicProcessorRedEye *) q)->setRadius(resetrad);
			((PicProcessorRedEye *) q)->setDesatPercent(desatpct);
			val1->SetLabel(wxString::Format("%2.2f", resetthr));
			val2->SetLabel(wxString::Format("%d", resetrad));
			val3->SetLabel(wxString::Format("%2.2f", desatpct));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *threshold, *radius, *desatpct;
		wxCheckBox *desat, *enablebox;
		wxStaticText *val1, *val2, *val3;
		wxBitmapButton *btn1, *btn2, *btn3;
		wxTimer t;

};


PicProcessorRedEye::PicProcessorRedEye(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	threshold = 1.5; radius=50; desatpct = 1.0; desat=false;
	if (command != "") {
		wxArrayString pts = split(command,";");
		wxArrayString parms = split(pts[0],",");
		if (parms.size() >= 1) threshold = atof(parms[0].c_str());
		if (parms.size() >= 2) radius = atoi(parms[1].c_str());
		if (parms.size() >= 3) if (parms[2] == "1") desat = true;
		if (parms.size() >= 4) desatpct = atof(parms[3].c_str());
		for (unsigned i=1; i<pts.size(); i++) {
			struct coord p;
			wxArrayString c = split(pts[i],",");
			p.x = atoi(c[0].c_str());
			p.y = atoi(c[1].c_str());
			points.push_back(p);
		}
	}
	for (unsigned i=0; i<points.size(); i++) {
		dcList.Append(wxString::Format("cross,%d,%d;",points[i].x, points[i].y));
		c.Append(wxString::Format("%d,%d;",points[i].x, points[i].y));
	}
//	m_display->SetDrawList(dcList);
	//showParams();
	m_display->Bind(wxEVT_LEFT_DOWN, &PicProcessorRedEye::OnLeftDown, this);
}

PicProcessorRedEye::~PicProcessorRedEye()
{
	m_display->Unbind(wxEVT_LEFT_DOWN, &PicProcessorRedEye::OnLeftDown, this);
//	m_display->SetDrawList("");
}

void PicProcessorRedEye::createPanel(wxSimplebook* parent)
{
	toolpanel = new RedEyePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

wxString PicProcessorRedEye::buildCommand()
{
	wxString cmd = wxString::Format("%2.2f,%d,%d,%2.2f;",threshold,radius,desat,desatpct);
	cmd.Append(getPointList());
	return cmd;
}

void PicProcessorRedEye::setThresholdLimit(wxString params)
{
	wxArrayString nbrs = split(params,",");
	threshold = atof(nbrs[0]);
	radius = atoi(nbrs[1]);
	wxString p = params;
	p.Append(";");
	p.Append(getPointList());
	c = p;
	dirty = true;
}

void PicProcessorRedEye::setThreshold(double t)
{
	threshold = t;
	c = buildCommand();
}

void PicProcessorRedEye::setRadius(int r)
{
	radius = r;
	c = buildCommand();
}

void PicProcessorRedEye::setDesatPercent(double pct)
{
	desatpct = pct;
	c = buildCommand();
}

void PicProcessorRedEye::setDesat(bool d)
{
	desat = d;
	c = buildCommand();
}


bool PicProcessorRedEye::processPicture(gImage *processdib) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText(_("redeye..."));
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.redeye.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	dib = processdib;
	if (!global_processing_enabled) return true;

	if (processingenabled) {
		mark();
		dib->ApplyRedeye(points, threshold, radius, desat, desatpct, threadcount);
		m_display->SetModified(true);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.redeye.log","0") == "1"))
		log(wxString::Format(_("tool=redeye,imagesize=%dx%d,threads=%d,time=%s"),dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty=false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");

	return result;
}


wxString PicProcessorRedEye::getPointList()
{
	wxString list;
	for (unsigned i=0; i<points.size(); i++) {
		list.Append(wxString::Format("%d,%d;",points[i].x, points[i].y));
	}
	return list;
}

void PicProcessorRedEye::OnLeftDown(wxMouseEvent& event)
{
	if (m_tree->GetSelection() == GetId()) {
		if (event.ShiftDown()) {

			coord c = m_display->GetImgCoords();
			points.push_back(c);
		}
		else if (event.ControlDown()) {
			coord co = m_display->GetImgCoords();
			std::vector<coord> n;
			for (unsigned i=0; i<points.size(); i++) {
				int dx = abs(points[i].x - co.x);
				int dy = abs(points[i].y - co.y);
				if (dx > radius | dy > radius) n.push_back(points[i]);  
			}
			points = n;
		}
		dcList.clear();
		c = wxString::Format("%2.2f,%d,%d,%2.2f;",threshold,radius,desat,desatpct);
		for (unsigned i=0; i<points.size(); i++) {
			dcList.Append(wxString::Format("cross,%d,%d;",points[i].x, points[i].y));
			c.Append(wxString::Format("%d,%d;",points[i].x, points[i].y));
		}
		processPic();
	}	

	event.Skip();
}


