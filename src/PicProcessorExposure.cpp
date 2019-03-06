#include "PicProcessor.h"
#include "PicProcessorExposure.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myFloatCtrl.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/strutil.h"
#include "undo.xpm"

#define EXPOSUREENABLE	 7000
#define EXPOSUREEV	 7001
#define EXPOSURETARGETEV 7002

class ExposurePanel: public PicProcPanel
{
	public:
		ExposurePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL, 3).CenterVertical();

			double initialvalue = atof(params.c_str());

			enablebox = new wxCheckBox(this, EXPOSUREENABLE, "exposure:");
			enablebox->SetValue(true);

			evb = new wxRadioButton(this, EXPOSUREEV, "compensation", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			evtgtb = new wxRadioButton(this, EXPOSURETARGETEV, "target patch");

			ev = new wxSlider(this, wxID_ANY, 50.0+(initialvalue*10.0), 0, 100, wxPoint(10, 30), wxSize(140, -1));
			val = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f", initialvalue), wxDefaultPosition, wxSize(30, -1));
			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset to default");
			
			patch  = new wxStaticText(this, wxID_ANY, " patch xy: -- ");
			radius = new myFloatCtrl(this, wxID_ANY, " radius: ", 1.5, 1, wxDefaultPosition, wxSize(40, -1));
			radius->SetIncrement(0.5);
			ev0    = new myFloatCtrl(this, wxID_ANY, " ev0: ", 0.18, 2, wxDefaultPosition, wxSize(40, -1));

			expmode = EXPOSUREEV;


			myRowSizer *m = new myRowSizer();
			m->AddRowItem(enablebox, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			m->NextRow();
			m->AddRowItem(evb, flags);
			m->NextRow();
			m->AddRowItem(ev, flags);
			m->AddRowItem(val, flags);
			m->AddRowItem(btn, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			m->NextRow();
			m->AddRowItem(evtgtb, flags);
			m->NextRow();
			m->AddRowItem(patch, flags);
			m->NextRow();
			m->AddRowItem(radius, flags);
			m->AddRowItem(ev0, flags);
			m->End();
			SetSizerAndFit(m);
			m->Layout();

			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &ExposurePanel::OnButton, this);
			Bind(wxEVT_RADIOBUTTON, &ExposurePanel::OnRadioButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &ExposurePanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &ExposurePanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &ExposurePanel::onEnable, this, EXPOSUREENABLE);
			Bind(wxEVT_TIMER, &ExposurePanel::OnTimer,  this);
		}

		~ExposurePanel()
		{
			t->~wxTimer();
		}

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				q->enableProcessing(true);
				//q->processPic();
				processEV();
			}
			else {
				q->enableProcessing(false);
				//q->processPic();
				processEV();
			}
		}

		void OnChanged(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.2f", (ev->GetValue()-50.0)/10.0));
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.2f", (ev->GetValue()-50.0)/10.0));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%2.2f",(ev->GetValue()-50.0)/10.0));
			//q->processPic();
			processEV();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval = atof(myConfig::getConfig().getValueOrDefault("tool.exposure.initialvalue","0.0").c_str());
			ev->SetValue(50.0+(resetval*10));
			q->setParams(wxString::Format("%2.2f",resetval));
			val->SetLabel(wxString::Format("%2.2f", resetval));
			//q->processPic();
			processEV();
			event.Skip();
		}

		void OnRadioButton(wxCommandEvent& event)
		{
			expmode = event.GetId();
			processEV();
		}

		void processEV()
		{
			switch (expmode) {
				case EXPOSUREEV:
					q->setParams(wxString::Format("%2.2f",(ev->GetValue()-50.0)/10.0));
					q->processPic();
					break;
				case EXPOSURETARGETEV:
					q->setParams(wxString::Format("patch=%d,%d;radius=1.5;ev0=0.18",patx,paty));
					q->processPic();
					break;
			}
		}

		void setPatch(coord p)
		{
			//parm tool.exposure.patchradius: (float), radius of patch.  Default=1.5
			patrad = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.patchradius","1.5").c_str());

			patx = p.x;
			paty = p.y;
			patch->SetLabel(wxString::Format(" patch xy: %d,%d",patx, paty));

			if (expmode == EXPOSURETARGETEV) processEV();

			//pb->Enable(true);
			//if (pb->GetValue() == true)
			//	processWB(WBPATCH);
			//else
			//	Refresh();
		}


		std::map<std::string,std::string> paramMap(std::string params)
		{
			std::map<std::string,std::string> p;

			if (params.find("=") == std::string::npos) {
				p["ev"] = atof(params.c_str());
			}
			else {
				p = parseparams(params);
			}
			return p;
		}


	private:
		wxSlider *ev;
		wxStaticText *val;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxRadioButton *evb, *evtgtb;
		int expmode;
		wxStaticText *patch;
		myFloatCtrl *radius, *ev0;
		unsigned patx, paty;
		double patrad;
		wxTimer *t;

};


PicProcessorExposure::PicProcessorExposure(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
	m_display->Bind(wxEVT_LEFT_DOWN, &PicProcessorExposure::OnLeftDown, this);
}

PicProcessorExposure::~PicProcessorExposure()
{
	m_display->Unbind(wxEVT_LEFT_DOWN, &PicProcessorExposure::OnLeftDown, this);
	m_display->SetDrawList("");
}

void PicProcessorExposure::SetPatchCoord(int x, int y)
{
	dcList = wxString::Format("cross,%d,%d;",x,y);
	m_display->SetDrawList(dcList);
	m_display->Refresh();
	m_display->Update();
}

void PicProcessorExposure::OnLeftDown(wxMouseEvent& event)
{
	if (m_tree->GetItemState(GetId()) != 1) {
		event.Skip();
		return;
	}
	if (event.ShiftDown()) {
		patch = m_display->GetImgCoords();
	}
	else {
		event.Skip();
		return;
	}
	SetPatchCoord(patch.x, patch.y);
	((ExposurePanel *) toolpanel)->setPatch(patch);
	event.Skip();
}


void PicProcessorExposure::createPanel(wxSimplebook* parent)
{
	toolpanel = new ExposurePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorExposure::processPic(bool processnext) 
{
	double ev;
	int x=0, y=0;
	float radius=0.0;
	bool expv = true;

	//params: 
	//	either: 
	//		ev
	//	or:
	//		patch=x,y;radius=r;ev0=e
	//	or:
	//		ev=e
	if (c.find("=") == std::string::npos) {
		ev = atof(c.c_str());
	}
	else {
		wxArrayString p = split(wxString(c),";");
		for (int i=0; i<p.GetCount(); i++) {
			wxArrayString nv = split(p[i],"=");
			if (nv.GetCount() == 2) {
				if (nv[0] == "patch") {
					wxArrayString xy = split(nv[1],",");
					if (xy.GetCount() == 2) {
						x = atoi(xy[0].mb_str());
						y = atoi(xy[1].mb_str());
					}
					else return false;
				}
				else if (nv[0] == "radius") {
					radius = atof(nv[1].mb_str());
				}
				else if (nv[0] == "ev0") {
					ev = atof(nv[1].mb_str());
				}
				else return false;
			}
			else return false;
		}
		expv = false;
	}


	
	((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("exposure compensation %2.2f...", ev));
	bool result = true;
	
	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.exposure.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());

	if (processingenabled) {

		mark();
		if (expv) {
			dib->ApplyExposureCompensation(ev, threadcount);
		}
		else {
			dib->ApplyExposureCompensation(x, y, radius, ev, threadcount);
		}
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.exposure.log","0") == "1"))
			log(wxString::Format("tool=exposure_compensation,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



