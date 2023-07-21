#include "PicProcessor.h"
#include "PicProcessorSpot.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "myIntegerCtrl.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "undo.xpm"
#include "util.h"

#include <vector>

#define SPOTENABLE 8800
#define SPOTRADIAL  8801
#define SPOTCLONE   8802
#define SPOTFILE	8803

class SpotPanel: public PicProcPanel
{
	public:
		SpotPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			SetSize(parent->GetSize());
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);

			p.x = p.y = s.x = s.y = 0;

			enablebox = new wxCheckBox(this, SPOTENABLE, _("spot:"));
			enablebox->SetValue(true);

			radialb = new wxRadioButton(this, SPOTRADIAL, _("radial"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			cloneb  = new wxRadioButton(this, SPOTCLONE, _("clone"));
			fileb  = new wxRadioButton(this, SPOTFILE, _("file"));
			spot = new wxStaticText(this, wxID_ANY, _("--,--"));
			patch = new wxStaticText(this, wxID_ANY, _("--,--"));

			//parm tool.spot.radius: Default value for spot/patch radius.  Default=20
			radius = new myIntegerCtrl(this, wxID_ANY, "Radius:", atoi(myConfig::getConfig().getValueOrDefault("tool.spot.radius","20").c_str()), 0, 100);

			cloneb->SetValue(true);
			spotmode = SPOTCLONE;

			wxArrayString pm = split(params,",");
			
			if (pm.size() >= 1) {
				if (pm[0] == "radial") {
					radialb->SetValue(true);
					spotmode = SPOTRADIAL;
					if (pm.size() >= 2) s.x = atoi(pm[1].c_str());
					if (pm.size() >= 3) s.y = atoi(pm[2].c_str());
					if (pm.size() >= 4) radius->SetIntegerValue(atoi(pm[3].c_str()));
				}
				else if (pm[0] == "clone") {
					cloneb->SetValue(true);
					spotmode = SPOTCLONE;
					if (pm.size() >= 2) s.x = atoi(pm[1].c_str());
					if (pm.size() >= 3) s.y = atoi(pm[2].c_str());
					if (pm.size() >= 4) p.x = atoi(pm[3].c_str());
					if (pm.size() >= 5) p.y = atoi(pm[4].c_str());
					if (pm.size() >= 6) radius->SetIntegerValue(atoi(pm[5].c_str()));
				}
				else if (pm[0] == "file") {
					fileb->SetValue(true);
					spotmode = SPOTFILE;
				}
				else {
					radialb->SetValue(true);
					spotmode = SPOTRADIAL;
				}
			}
			


			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("Spot:")), flags);
			m->AddRowItem(spot, flags);
			m->NextRow();
			m->AddRowItem(radius, flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(cloneb,flags);
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("Patch:")), flags);
			m->AddRowItem(patch, flags);

			m->NextRow();
			m->AddRowItem(radialb,flags);

			m->NextRow();
			m->AddRowItem(fileb,flags);

			m->End();
			SetSizerAndFit(m);
			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_BUTTON, &SpotPanel::OnButton, this);
			Bind(myINTEGERCTRL_CHANGE,&SpotPanel::OnChanged, this);
			Bind(myINTEGERCTRL_UPDATE,&SpotPanel::paramChanged, this);
			Bind(wxEVT_RADIOBUTTON, &SpotPanel::OnRadioButton, this);
			Bind(wxEVT_CHECKBOX, &SpotPanel::onEnable, this, SPOTENABLE);
			Bind(wxEVT_TIMER, &SpotPanel::OnTimer,  this);
			Bind(wxEVT_CHAR_HOOK, &SpotPanel::OnKey,  this);
			Thaw();
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

		void OnChanged(wxCommandEvent& event)
		{
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			processSP();
		}

		void paramChanged(wxCommandEvent& event)
		{
			processSP();
			event.Skip();
		}

		void SetSpot(coord spot)
		{
			s=spot;
			this->spot->SetLabel(wxString::Format("%d,%d", s.x, s.y));
			Refresh();
			processSP();
		}

		void SetPatch(coord patch)
		{
			p=patch;
			this->patch->SetLabel(wxString::Format("%d,%d", p.x, p.y));
			Refresh();
			processSP();
		}

		int GetRadius()
		{
			return radius->GetIntegerValue();
		}

		void OnThumbTrack(wxCommandEvent& event)
		{

		}

		void OnButton(wxCommandEvent& event)
		{
			processSP();
			event.Skip();
		}

		wxString DCList()
		{
			wxString dclist;
			if (p.x != 0) dclist.Append(wxString::Format("circle,%d,%d,%d;", p.x, p.y, radius->GetIntegerValue()/2));
			if (s.x != 0) dclist.Append(wxString::Format("cross,%d,%d", s.x, s.y));
			return dclist;
		}

		void processSP()
		{
			if (spotmode == SPOTRADIAL) {
				if (s.x != 0) {
					q->setParams(wxString::Format("radial,%d,%d,%d",s.x, s.y, radius->GetIntegerValue()));
					q->processPic();
				}
			}
			else if (spotmode == SPOTCLONE) {
				if (s.x != 0 & p.x != 0) {
					q->setParams(wxString::Format("clone,%d,%d,%d,%d,%d",s.x, s.y, p.x, p.y, radius->GetIntegerValue()));
					q->processPic();
				}
			}
			else if (spotmode == SPOTFILE) {
				//to-do...
					q->setParams(wxString::Format("file,%s","foo.txt"));
					q->processPic();
			}
		}

		void OnRadioButton(wxCommandEvent& event)
		{
			spotmode = event.GetId();
			processSP();
		}

	private:
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxRadioButton *radialb, *cloneb, *fileb;
		wxStaticText *spot, *patch;
		coord s, p;
		myIntegerCtrl * radius;
		int spotmode;
		wxTimer t;
};


PicProcessorSpot::PicProcessorSpot(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	m_display->Bind(wxEVT_LEFT_DOWN, &PicProcessorSpot::OnLeftDown, this);
}

PicProcessorSpot::~PicProcessorSpot()
{
	m_display->Unbind(wxEVT_LEFT_DOWN, &PicProcessorSpot::OnLeftDown, this);
//	m_display->SetDrawList("");
}

void PicProcessorSpot::createPanel(wxSimplebook* parent)
{
	toolpanel = new SpotPanel(parent, this, c);
	dcList = ((SpotPanel *) toolpanel)->DCList();
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorSpot::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;

	((wxFrame*) m_display->GetParent())->SetStatusText(_("spot..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_spot(std::string(pstr));

	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		dcList = ((SpotPanel *) toolpanel)->DCList();
		result = process_spot(*dib, params);

		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.spot.log","0") == "1"))
					log(wxString::Format(_("tool=spot,%s,imagesize=%dx%d,threads=%s,time=%s"),
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

void PicProcessorSpot::OnLeftDown(wxMouseEvent& event)
{
	coord spot, patch;
	if (m_tree->GetSelection() == GetId()) {
		if (event.ShiftDown()) {
			patch = m_display->GetImgCoords();
			((SpotPanel *) toolpanel)->SetPatch(patch);
			dcList = ((SpotPanel *) toolpanel)->DCList();
		}
		else if (event.ControlDown()) {
			spot = m_display->GetImgCoords();
			((SpotPanel *) toolpanel)->SetSpot(spot);
			dcList = ((SpotPanel *) toolpanel)->DCList();
		}
	}
	event.Skip();
}
