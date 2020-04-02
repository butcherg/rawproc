#include "PicProcessor.h"
#include "PicProcessorExposure.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myFloatCtrl.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/strutil.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "copy.xpm"
#include "paste.xpm"
#include "undo.xpm"

#define EXPOSUREENABLE	 7000
#define EXPOSUREEV	 7001
#define EXPOSURETARGETEV 7002
#define EXPOSURECOPY	 7003
#define EXPOSUREPASTE	 7004
#define EXPOSUREUNDO	 7005

class ExposurePanel: public PicProcPanel
{
	public:
		ExposurePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL, 3).CenterVertical();

			patx = 0; paty=0;

			enablebox = new wxCheckBox(this, EXPOSUREENABLE, _("exposure:"));
			enablebox->SetValue(true);

			evb = new wxRadioButton(this, EXPOSUREEV, _("compensation"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			evtgtb = new wxRadioButton(this, EXPOSURETARGETEV, _("target patch"));

			ev = new wxSlider(this, wxID_ANY, 0, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			val = new wxStaticText(this,wxID_ANY, "0.0", wxDefaultPosition, wxSize(30, -1));
			btn = new wxBitmapButton(this, EXPOSUREUNDO, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip(_("Reset to default"));
			
			patch  = new wxStaticText(this, wxID_ANY, _(" patch xy: -- "));
			radius = new myFloatCtrl(this, wxID_ANY, _(" radius: "), 1.5, 1, wxDefaultPosition, wxSize(40, -1));
			radius->SetIncrement(1.0);
			ev0    = new myFloatCtrl(this, wxID_ANY, _(" ev0: "), 0.18, 2, wxDefaultPosition, wxSize(40, -1));
			stops  = new wxStaticText(this, wxID_ANY, _("stops: --"));

			std::map<std::string,std::string> p = proc->paramMap(params.ToStdString(), "ev");

			if (p.find("ev") != p.end()) { 
				evb->SetValue(true);
				expmode = EXPOSUREEV;
				ev->SetValue(50.0+(atof(p["ev"].c_str())*10.0));
				val->SetLabel(wxString::Format("%2.2f", (ev->GetValue()-50.0)/10.0));

			}
			if (p.find("patch") != p.end()) {
				evtgtb->SetValue(true);
				expmode = EXPOSURETARGETEV;
				std::vector<std::string> patstr = split(p["patch"],",");
				patx = atoi(patstr[0].c_str());
				paty = atoi(patstr[1].c_str());
				patch->SetLabel(wxString::Format(_(" patch xy: %d,%d"),patx, paty));
				radius->SetFloatValue(atof(p["radius"].c_str()));
				ev0->SetFloatValue(atof(p["ev0"].c_str()));
			}


			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, EXPOSURECOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, EXPOSUREPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(evb, flags);
			m->NextRow();
			m->AddRowItem(ev, flags);
			m->AddRowItem(val, flags);
			m->AddRowItem(btn, flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(evtgtb, flags);
			m->NextRow();
			m->AddRowItem(patch, flags);
			m->AddRowItem(stops, flags);
			m->NextRow();
			m->AddRowItem(radius, flags);
			m->AddRowItem(ev0, flags);
			m->End();
			SetSizerAndFit(m);
			m->Layout();

			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_BUTTON, &ExposurePanel::OnButton, this, EXPOSUREUNDO);
			Bind(wxEVT_BUTTON, &ExposurePanel::OnCopy, this, EXPOSURECOPY);
			Bind(wxEVT_BUTTON, &ExposurePanel::OnPaste, this, EXPOSUREPASTE);
			Bind(wxEVT_RADIOBUTTON, &ExposurePanel::OnRadioButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &ExposurePanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &ExposurePanel::OnThumbTrack, this);
			Bind(myFLOATCTRL_CHANGE, &ExposurePanel::OnFloatChange, this);
			Bind(myFLOATCTRL_UPDATE, &ExposurePanel::OnFloatUpdate, this);
			Bind(wxEVT_CHECKBOX, &ExposurePanel::onEnable, this, EXPOSUREENABLE);
			Bind(wxEVT_TIMER, &ExposurePanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &ExposurePanel::OnKey,  this);
			Thaw();
		}

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) {
				q->enableProcessing(true);
				processEV();
			}
			else {
				q->enableProcessing(false);
				processEV();
			}
		}

		void OnCopy(wxCommandEvent& event)
		{
			q->copyParamsToClipboard();
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Copied command to clipboard: %s"),q->getCommand()));
			
		}

		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {
				std::map<std::string,std::string> p = q->paramMap(q->getParams().ToStdString(), "ev");

				if (p.find("ev") != p.end()) { 
					evb->SetValue(true);
					expmode = EXPOSUREEV;
					ev->SetValue(50.0+(atof(p["ev"].c_str())*10.0));
					val->SetLabel(wxString::Format("%2.2f", (ev->GetValue()-50.0)/10.0));
				}
				if (p.find("patch") != p.end()) {
					evtgtb->SetValue(true);
					expmode = EXPOSURETARGETEV;
					std::vector<std::string> patstr = split(p["patch"],",");
					patx = atoi(patstr[0].c_str());
					paty = atoi(patstr[1].c_str());
					patch->SetLabel(wxString::Format(_(" patch xy: %d,%d"),patx, paty));
					radius->SetFloatValue(atof(p["radius"].c_str()));
					ev0->SetFloatValue(atof(p["ev0"].c_str()));
				}

				q->processPic();
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Pasted command from clipboard: %s"),q->getCommand()));
			}
			else wxMessageBox(_("Invalid Paste"));
		}


		void OnChanged(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.2f", (ev->GetValue()-50.0)/10.0));
			if (expmode == EXPOSUREEV) t.Start(500,wxTIMER_ONE_SHOT);
		}
		
		void OnFloatChange(wxCommandEvent& event)
		{
			if (expmode == EXPOSURETARGETEV) t.Start(500,wxTIMER_ONE_SHOT);
		}
		
		void OnFloatUpdate(wxCommandEvent& event)
		{
			if (expmode == EXPOSURETARGETEV) processEV();
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.2f", (ev->GetValue()-50.0)/10.0));
		}

		void OnTimer(wxTimerEvent& event)
		{
			processEV();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval = atof(myConfig::getConfig().getValueOrDefault("tool.exposure.initialvalue","0.0").c_str());
			ev->SetValue(50.0+(resetval*10));
			q->setParams(wxString::Format("%2.2f",resetval));
			val->SetLabel(wxString::Format("%2.2f", resetval));
			processEV();
			event.Skip();
		}

		void OnRadioButton(wxCommandEvent& event)
		{
			if (event.GetId() == EXPOSURETARGETEV) {
				if (patx == 0) {
					evb->SetValue(true);
					wxMessageBox(_("Select a patch first..."));
					return;
				}
			}
			expmode = event.GetId();
			processEV();
		}

		void processEV()
		{
			float evval;
			switch (expmode) {
				case EXPOSUREEV:
					evval = (ev->GetValue()-50.0)/10.0;
					q->setParams(wxString::Format("%2.2f",evval));
					q->processPic();
					break;
				case EXPOSURETARGETEV:
					if (patx > 0 & paty > 0) { 
						//q->setParams(wxString::Format("patch=%d,%d;radius=%0.1f;ev0=%0.2f",patx,paty, radius->GetFloatValue(), ev0->GetFloatValue()));
						q->setParams(wxString::Format("patch,%d,%d,%0.1f,%0.2f",patx,paty, radius->GetFloatValue(), ev0->GetFloatValue()));
						q->processPic();
					}
					break;
			}
		}

		void setPatch(coord p)
		{
			//parm tool.exposure.patchradius: (float), radius of patch.  Default=1.5
			patrad = atof(myConfig::getConfig().getValueOrDefault("tool.whitebalance.patchradius","1.5").c_str());

			patx = p.x;
			paty = p.y;
			patch->SetLabel(wxString::Format(_(" patch xy: %d,%d"),patx, paty));
			GetSizer()->Layout();

			if (expmode == EXPOSURETARGETEV) processEV();

		}

		void setStops(float s)
		{
			if (stops) {
				stops->SetLabel(wxString::Format(_("stops: %0.1f"),s));
				GetSizer()->Layout();
			}
		}

	private:
		wxSlider *ev;
		wxStaticText *val;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxRadioButton *evb, *evtgtb;
		int expmode;
		wxStaticText *patch, *stops;
		myFloatCtrl *radius, *ev0;
		unsigned patx, paty;
		double patrad;
		wxTimer t;

};


PicProcessorExposure::PicProcessorExposure(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
	m_display->Bind(wxEVT_LEFT_DOWN, &PicProcessorExposure::OnLeftDown, this);
}

PicProcessorExposure::~PicProcessorExposure()
{
	m_display->Unbind(wxEVT_LEFT_DOWN, &PicProcessorExposure::OnLeftDown, this);
//	m_display->SetDrawList("");
}

void PicProcessorExposure::SetPatchCoord(int x, int y)
{
	dcList = wxString::Format("cross,%d,%d;",x,y);
}

void PicProcessorExposure::OnLeftDown(wxMouseEvent& event)
{
	if (m_tree->GetSelection() == GetId()) {
		if (event.ShiftDown()) {
			patch = m_display->GetImgCoords();
			//SetPatchCoord(patch.x, patch.y);
			dcList = wxString::Format("cross,%d,%d;",patch.x, patch.y);
			((ExposurePanel *) toolpanel)->setPatch(patch);
		}
	}
	event.Skip();

}


void PicProcessorExposure::createPanel(wxSimplebook* parent)
{
	toolpanel = new ExposurePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorExposure::processPicture(gImage *processdib) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText(_("exposure..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_exposure(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_exposure(*dib, params);
		if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			((ExposurePanel *) toolpanel)->setStops(atof(result["stops"].c_str()));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.exposure.log","0") == "1"))
					log(wxString::Format(_("tool=exposure,%s,imagesize=%dx%d,threads=%s,time=%s"),
						params["mode"].c_str(),
						dib->getWidth(), 
						dib->getHeight(),
						result["threadcount"].c_str(),
						result["duration"].c_str())
					);
		}
	}

	dirty=false;
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return ret;
}


/*
bool PicProcessorExposure::processPicture(gImage *processdib) 
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


	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("exposure..."));
	bool result = true;
	
	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.exposure.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	dib = processdib;
	if (!global_processing_enabled) return true;

	if (processingenabled) {

		mark();
		if (expv) {
			m_tree->SetItemText(id, _("exposure:ev"));
			if (ev != 0.0) {
				dib->ApplyExposureCompensation(ev, threadcount);
				m_display->SetModified(true);
			}
		}
		else {
			m_tree->SetItemText(id, _("exposure:patch"));
			float stops = dib->ApplyExposureCompensation(x, y, radius, ev, threadcount);
			m_display->SetModified(true);
			((ExposurePanel *) toolpanel)->setStops(stops);
		}
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.exposure.log","0") == "1"))
			log(wxString::Format(_("tool=exposure_compensation,imagesize=%dx%d,threads=%d,time=%s"),dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}
*/


