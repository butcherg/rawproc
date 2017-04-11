#include "PicProcessor.h"
#include "PicProcessorRedEye.h"
#include "PicProcPanel.h"
#include "undo.xpm"
//#include "gimage/strutil.h"

#include "util.h"
#include <wx/fileconf.h>

class RedEyePanel: public PicProcPanel
{
	public:
		RedEyePanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			//std::vector<std::string> p = split(params,",");
			wxArrayString p = split(params,",");
			double thr = atof(p[0].c_str());
			int rad = atoi(p[1].c_str());
			double dpct = atof(p[2].c_str());

			g->Add(0,10, wxGBPosition(0,0));

			//Threshold slider
			g->Add(new wxStaticText(this,wxID_ANY, "threshold: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			threshold = new wxSlider(this, wxID_ANY, thr*10, 0, 50, wxPoint(10, 30), wxSize(140, -1));
			g->Add(threshold , wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val1 = new wxStaticText(this,wxID_ANY, p[0], wxDefaultPosition, wxSize(30, -1));
			g->Add(val1 , wxGBPosition(1,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn1 = new wxBitmapButton(this, 9000, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn1->SetToolTip("Reset to default");
			g->Add(btn1, wxGBPosition(1,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			
			//Limit, or radius, slider
			g->Add(new wxStaticText(this,wxID_ANY, "radius: "), wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			radius = new wxSlider(this, wxID_ANY, rad, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(radius , wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val2 = new wxStaticText(this,wxID_ANY, p[1], wxDefaultPosition, wxSize(30, -1));
			g->Add(val2, wxGBPosition(2,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn2 = new wxBitmapButton(this, 9001, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn2->SetToolTip("Reset to default");
			g->Add(btn2, wxGBPosition(2,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			
			g->Add(0,10, wxGBPosition(3,0));
			
			desat = new wxCheckBox(this, wxID_ANY, "desaturate");
			g->Add(desat, wxGBPosition(4,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			//desaturate adjustment
			g->Add(new wxStaticText(this,wxID_ANY, "percent: "), wxGBPosition(5,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			desatpct = new wxSlider(this, wxID_ANY, 100, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			g->Add(desatpct , wxGBPosition(5,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val3 = new wxStaticText(this,wxID_ANY, "1.0", wxDefaultPosition, wxSize(30, -1));
			g->Add(val3, wxGBPosition(5,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn3 = new wxBitmapButton(this, 9002, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn3->SetToolTip("Reset to default");
			g->Add(btn3, wxGBPosition(5,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);

			g->Add(0,10, wxGBPosition(6,0));
			wxString help = "Double-click in the display to toggle to scale 100%.\nShift-Click the center of the red eye to apply.\nCtrl-Click near the cross to remove application.\n\nOperation can only be done in display scale 100%.";
			g->Add(new wxStaticText(this, wxID_ANY, help), wxGBPosition(7,0), wxGBSpan(1,4), wxALIGN_LEFT | wxALL, 5);
			
			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &RedEyePanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &RedEyePanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &RedEyePanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &RedEyePanel::OnCheckBox, this);
			Bind(wxEVT_TIMER, &RedEyePanel::OnTimer,  this);
		}

		~RedEyePanel()
		{
			t->~wxTimer();
		}

		void OnChanged(wxCommandEvent& event)
		{
			val1->SetLabel(wxString::Format("%2.2f", (float) threshold->GetValue() / 10.0));
			val2->SetLabel(wxString::Format("%d", radius->GetValue()));
			val3->SetLabel(wxString::Format("%2.2f", (float) desatpct->GetValue() / 100.0));
			t->Start(500,wxTIMER_ONE_SHOT);
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
			((PicProcessorRedEye *) q)->setThresholdLimit(wxString::Format("%2.2f,%d", (float) threshold->GetValue() / 10.0, radius->GetValue()));
			((PicProcessorRedEye *) q)->setDesatPercent(desatpct->GetValue()/100.0);
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetthr; int resetrad;
			wxConfigBase::Get()->Read("tool.redeye.threshold.initialvalue",&resetthr,1.5);
			wxConfigBase::Get()->Read("tool.redeye.radius.initialvalue",&resetrad,50);
			threshold->SetValue(resetthr*10);
			radius->SetValue(resetrad);
			desat->SetValue(100);
			((PicProcessorRedEye *) q)->setThresholdLimit(wxString::Format("%2.2f,%d",resetthr,resetrad));
			val1->SetLabel(wxString::Format("%2.2f", resetthr));
			val2->SetLabel(wxString::Format("%d", resetrad));
			val3->SetLabel(wxString::Format("%2.2f", (float) desatpct->GetValue() / 100.0));
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *threshold, *radius, *desatpct;
		wxCheckBox *desat;
		wxStaticText *val1, *val2, *val3;
		wxBitmapButton *btn1, *btn2, *btn3;
		wxTimer *t;

};


PicProcessorRedEye::PicProcessorRedEye(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	threshold = 1.5; radius=50; desatpct = 1.0; desat=false;
	if (command != "") {
		wxArrayString pts = split(command,";");
		wxArrayString parms = split(pts[0],",");
		if (parms.size() >= 1) threshold = atof(parms[0].c_str());
		if (parms.size() >= 2) radius = atoi(parms[1].c_str());
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
	m_display->SetDrawList(dcList);
	showParams();
	m_display->Bind(wxEVT_LEFT_DOWN, &PicProcessorRedEye::OnLeftDown, this);
}

PicProcessorRedEye::~PicProcessorRedEye()
{
	m_display->Unbind(wxEVT_LEFT_DOWN, &PicProcessorRedEye::OnLeftDown, this);
	m_display->SetDrawList("");
}

void PicProcessorRedEye::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new RedEyePanel(m_parameters, this, wxString::Format("%2.2f,%d",threshold,radius));
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

void PicProcessorRedEye::setDesatPercent(double pct)
{
	desatpct = pct;
}

void PicProcessorRedEye::setDesat(bool d)
{
	desat = d;
}


bool PicProcessorRedEye::processPic() {
	((wxFrame*) m_display->GetParent())->SetStatusText("redeye...");
	bool result = true;

	int threadcount;
	wxConfigBase::Get()->Read("tool.redeye.cores",&threadcount,0);
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyRedeye(points, threshold, radius, desat, desatpct, threadcount);
	wxString d = duration();

	if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.redeye.log","0") == "1"))
		log(wxString::Format("tool=redeye,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty=false;


	((wxFrame*) m_display->GetParent())->SetStatusText("");
	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}

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
	if (m_tree->GetItemState(GetId()) != 1) {
		event.Skip();
		return;
	}
	if (event.ShiftDown() & m_display->GetScale() == 1.0) {
		coord c = m_display->GetImgCoords();
		points.push_back(c);
	}
	else if (event.ControlDown() & m_display->GetScale() == 1.0) {
		coord co = m_display->GetImgCoords();
		std::vector<coord> n;
		for (unsigned i=0; i<points.size(); i++) {
			int dx = abs(points[i].x - co.x);
			int dy = abs(points[i].y - co.y);
			if (dx > radius | dy > radius) n.push_back(points[i]);  
		}
		points = n;
	}
	else {
		event.Skip();
		return;
	}
	
	dcList = "";
	c = wxString::Format("%2.2f,%d;",threshold, radius);
	for (unsigned i=0; i<points.size(); i++) {
		dcList.Append(wxString::Format("cross,%d,%d;",points[i].x, points[i].y));
		c.Append(wxString::Format("%d,%d;",points[i].x, points[i].y));
	}
	m_display->SetDrawList(dcList);
	processPic();
	m_display->Refresh();
	event.Skip();
}


