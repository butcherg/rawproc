#include "PicProcessorTone.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myToneCurvePane.h"
#include "myFloatCtrl.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"
#include "copy.xpm"
#include "paste.xpm"
#include "undo.xpm"

#include <wx/clipbrd.h>

#define TONEENABLE 7900
#define TONEID 7901
#define TONECOPY 7902
#define TONEPASTE 7903
#define TONEGAMMA 7904
#define TONEREINHARD 7905
#define TONELOG2 7906
#define TONELOGGAM 7907
#define TONEFILMIC 7908
#define TONEDUALLOGISTIC 7909
#define TONECURVE 7910
#define TONENORM 7911
#define TONEFILMICRESET 7912
#define TONECURVECOPY 7913

class ToneCurveDialog: public wxDialog
{
	public:
		ToneCurveDialog(wxWindow *parent, wxWindowID id, const wxString &title, std::vector<float> xarray, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize):
		wxDialog(parent, id, title, pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
		{
			tcpane = new myToneCurvePane(this, wxDefaultPosition, wxSize(110,110));
			wxBoxSizer* s = new wxBoxSizer( wxVERTICAL );
			s->Add(tcpane, 1, wxEXPAND, 5);
			SetSizerAndFit(s);
			s->Layout();
			tcpane->SetCurve(xarray, true);
		}

		~ToneCurveDialog() {}

		void SetCurve(std::vector<float> curve)
		{
			tcpane->SetCurve(curve);
			Refresh();
		}

	private:
		myToneCurvePane *tcpane;
};

class TonePanel: public PicProcPanel
{

	public:
		TonePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			//wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);


			enablebox = new wxCheckBox(this, TONEENABLE, _("tone:"));
			enablebox->SetValue(true);

			//All the radio buttons in the same group:
			gamb = new wxRadioButton(this, TONEGAMMA, _("gamma"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			reinb = new wxRadioButton(this, TONEREINHARD, _("reinhard"));
			//log2b = new wxRadioButton(this, TONELOG2, "log2");
			hybloggamb = new wxRadioButton(this, TONELOGGAM, _("loggamma"));
			filmicb = new wxRadioButton(this, TONEFILMIC, _("filmic"));
			doublelogisticb = new wxRadioButton(this, TONEDUALLOGISTIC, _("doublelogistic"));

			tonenorm = new wxCheckBox(this, TONENORM, _("norm"));
			tonenorm->SetValue(false);

			tcpane = new myToneCurvePane(this, wxDefaultPosition, wxSize(130,130));
			//tc = NULL;

			//parm tool.tone.gamma: Default value for gamma tone operator.  Default=2.2
			gamma = new myFloatCtrl(this, wxID_ANY, atof(myConfig::getConfig().getValueOrDefault("tool.tone.gamma","2.2").c_str()), 2);

			//parm tool.tone.filmic.A: Default value for filmic tone operator A coefficient.  Default=6.2
			filmicA = new myFloatCtrl(this, wxID_ANY, "A:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.A","6.2").c_str()), 1, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			//parm tool.tone.filmic.B: Default value for filmic tone operator B coefficient.  Default=0.5
			filmicB = new myFloatCtrl(this, wxID_ANY, "B:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.B","0.5").c_str()), 2, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			//parm tool.tone.filmic.C: Default value for filmic tone operator C coefficient.  Default=1.7
			filmicC = new myFloatCtrl(this, wxID_ANY, "C:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.C","1.7").c_str()), 1, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			//parm tool.tone.filmic.D: Default value for filmic tone operator D coefficient.  Default=0.06
			filmicD = new myFloatCtrl(this, wxID_ANY, "D:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.D","0.06").c_str()), 2, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			//parm tool.tone.filmic.power: Default value for filmic tone operator power coefficient.  Set this to 1.0 to remove the effect of this coefficient.  Default=1.0
			power   = new myFloatCtrl(this, wxID_ANY, "power:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.power","1.0").c_str()), 1, wxDefaultPosition, wxSize(50,TEXTCTRLHEIGHT));
			
			//parm tool.tone.doublelogistic.L: Default value for doublelogistic L operator power, sets the position of the transition from the left equation to the right equation. Default=0.002
			dlL = new myFloatCtrl(this, wxID_ANY, "L:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.doublelogistic.L","0.002").c_str()), 3, wxDefaultPosition, wxSize(55,TEXTCTRLHEIGHT));
			//parm tool.tone.doublelogistic.c: Default value for doublelogistic c operator power, sets the height of the right equation.  Default=3.0
			dlc = new myFloatCtrl(this, wxID_ANY, "c:", atof(myConfig::getConfig().getValueOrDefault("tool.tone.doublelogistic.c","3.0").c_str()), 2, wxDefaultPosition, wxSize(47,TEXTCTRLHEIGHT));

			wxArrayString str;
			str.Add("channel");
			str.Add("luminance");
			reinop = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(100,TEXTCTRLHEIGHT), str);

			wxArrayString p = split(params,",");
			setPanel(p);

			tcpane->SetCurve(makeXArray(10000), true);

			//log2b->Enable(false);  //log2 doesn't do anything, yet.

			//Lay out the controls in the panel:
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, TONECOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, TONEPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);

			//gamma:
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(gamb, flags);
			m->AddRowItem(gamma, flags);

			//reinhard:
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(reinb, flags);
			m->AddRowItem(reinop, flags);

			//hybrid log-gamma:
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(hybloggamb, flags);
			
			//doublelogistic:
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(doublelogisticb, flags);
			m->AddRowItem(dlL, flags);
			m->AddRowItem(dlc, flags);

			//filmic:
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(filmicb, flags);
			m->AddRowItem(power, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, TONEFILMICRESET, wxBitmap(undo_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(); 
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "     "), flags);
			m->AddRowItem(filmicA, flags);
			//m->NextRow();
			m->AddRowItem(filmicB, flags);
			m->NextRow(); 
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "     "), flags);
			m->AddRowItem(filmicC, flags);
			//m->NextRow();
			m->AddRowItem(filmicD, flags);
			
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("tone curve:")), wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxStaticText(this, wxID_ANY, _("copy curve:")), flags);
			m->AddRowItem(new wxBitmapButton(this, TONECURVECOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "  "), wxSizerFlags(1).Left());
			m->AddRowItem(tcpane, wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP));
			m->AddRowItem(tonenorm, flags);
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "  "), wxSizerFlags(1).Left());

			m->End();
			SetSizerAndFit(m);
			SetFocus();
			t.SetOwner(this);
			
			processTone(tonemode);

			Bind(wxEVT_TIMER, &TonePanel::OnTimer, this);
			Bind(wxEVT_BUTTON, &TonePanel::OnCopy, this, TONECOPY);
			Bind(wxEVT_BUTTON, &TonePanel::OnPaste, this, TONEPASTE);
			Bind(wxEVT_BUTTON, &TonePanel::OnReset, this, TONEFILMICRESET);
			Bind(wxEVT_BUTTON, &TonePanel::OnCurveCopy, this, TONECURVECOPY);
			Bind(myFLOATCTRL_CHANGE, &TonePanel::floatParamChanged, this);
			Bind(myFLOATCTRL_UPDATE, &TonePanel::floatParamUpdated, this);
			Bind(wxEVT_CHECKBOX, &TonePanel::onEnable, this, TONEENABLE);
			Bind(wxEVT_CHECKBOX, &TonePanel::OnNorm, this, TONENORM);
			Bind(wxEVT_RADIOBUTTON, &TonePanel::OnButton, this);
			Bind(wxEVT_CHOICE, &TonePanel::reinopChanged, this);
			Bind(wxEVT_CHAR_HOOK, &TonePanel::OnKey,  this);
			Thaw();
		}

		void setPanel(wxArrayString p)
		{
			if (p[0] == "gamma") {
				gamb->SetValue(true);
				tonemode = TONEGAMMA;
				if (p.GetCount() >=2) 
					gamma->SetFloatValue(atof(p[1].c_str()));
				else
					gamma->SetFloatValue(1.0);
			}
			else {
				gamma->SetFloatValue(1.0);
			}
			if (p[0] == "reinhard") {
				reinb->SetValue(true);
				tonemode = TONEREINHARD;
				if (p.GetCount() >=2) 
					reinop->SetStringSelection(p[1]);
				else
					reinop->SetSelection(reinop->FindString("channel"));
				if (p.GetCount() >=3) 
					if (p[2] == "norm")
						tonenorm->SetValue(true);
			}
			else {
				reinop->SetSelection(reinop->FindString("channel"));
			}
			//if (p[0] == "log2") {
			//	log2b->SetValue(true);
			//}
			if (p[0] == "loggamma") {
				hybloggamb->SetValue(true);
				tonemode = TONELOGGAM;
			}
			if (p[0] == "doublelogistic") {
				doublelogisticb->SetValue(true);
				tonemode = TONEDUALLOGISTIC;
				if (p.GetCount() >=2) 
					dlL->SetFloatValue(atof(p[1].c_str()));
				if (p.GetCount() >=3) 
					dlc->SetFloatValue(atof(p[2].c_str()));
			}
			if (p[0] == "filmic") {
				filmicb->SetValue(true);
				tonemode = TONEFILMIC;
				if (p.GetCount() >=6) {
					filmicA->SetFloatValue(atof(p[1].c_str()));
					filmicB->SetFloatValue(atof(p[2].c_str()));
					filmicC->SetFloatValue(atof(p[3].c_str()));
					filmicD->SetFloatValue(atof(p[4].c_str()));
					power->SetFloatValue(atof(p[5].c_str()));
				}
				if (p.GetCount() >=7) 
					if (p[6] == "norm")
						tonenorm->SetValue(true);
			}

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
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Copied command to clipboard: %s"),q->getCommand()));
			
		}

		void OnCurveCopy(wxCommandEvent& event)
		{
			//parm tool.tone.curve.arraysize: number of values between 0 and 1 delivered by the copy curve to clipboard button.  Default: 256
			unsigned arraysize = atoi(myConfig::getConfig().getValueOrDefault("tool.tone.curve.arraysize","256").c_str());

			wxString cdatstr;
			//parm tool.tone.curve.type: horizontal|vertical|curvetool.  If horizontal, a comma-separated list.  If vertical, one number per line. If curvetool, a curve tool command with a number of control points corresponding to the arraysize will be place in the clipboard; for this type, arraysize can't be larger than 255. Default: vertical.
			std::string datatype = myConfig::getConfig().getValueOrDefault("tool.tone.curve.type","vertical");
			if (datatype == "horizontal") {
				std::vector<float> cdat =  makeXArray(arraysize);
				cdatstr.Append(wxString::Format("%f",cdat[0]));
				for (unsigned i=1; i<cdat.size(); i++) cdatstr.Append(wxString::Format(",%f",cdat[i]));
			}
			else if (datatype == "curvetool") {
				std::vector<float> cdat =  makeXArray(255);
				cdatstr.Append(wxString::Format("curve:rgb,0,0"));
				for (unsigned i=1; i<255; i+=255/arraysize) cdatstr.Append(wxString::Format(",%d,%d",i, (int) (cdat[i]*255.0)));
				
			}
			else { //default to vertical
				std::vector<float> cdat =  makeXArray(arraysize);
				for (unsigned i=0; i<cdat.size(); i++) cdatstr.Append(wxString::Format("%f\n",cdat[i]));
			}
			if (wxTheClipboard->Open())
			{
				wxTheClipboard->SetData( new wxTextDataObject(cdatstr));
				wxTheClipboard->Close();
			}
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Copied curve to clipboard")));
			
		}

		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {
				wxArrayString p = split(q->getParams(),",");
				setPanel(p);
				processTone(event.GetId());
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Pasted command from clipboard: %s"),q->getCommand()));
			}
			else wxMessageBox(wxString::Format(_("Invalid Paste")));
		}

		void reinopChanged(wxCommandEvent& event)
		{
			if (reinb->GetValue()) {
				if (tonenorm->GetValue())
					q->setParams(wxString::Format("reinhard,%s,norm",reinop->GetString(reinop->GetSelection())));
				else
					q->setParams(wxString::Format("reinhard,%s",reinop->GetString(reinop->GetSelection())));
				q->processPic();
			}
			event.Skip();
		}

		void OnReset(wxCommandEvent& event)
		{
			if (event.GetId() == TONEFILMICRESET) {
				wxArrayString p;
				p.Add("filmic");
				p.Add(wxString(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.A","6.2")));
				p.Add(wxString(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.B","0.5")));
				p.Add(wxString(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.C","1.7")));
				p.Add(wxString(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.D","0.06")));
				p.Add(wxString(myConfig::getConfig().getValueOrDefault("tool.tone.filmic.power","1.0")));
				setPanel(p);
				if (tonemode == TONEFILMIC) processTone(TONEFILMIC);
			}
		}

		void OnButton(wxCommandEvent& event)
		{
			processTone(event.GetId());
			tonemode = event.GetId();
			event.Skip();
		}

		void OnNorm(wxCommandEvent& event)
		{
			processTone(tonemode);
			event.Skip();
		}

		void processTone(int src)
		{
			switch (src) {
				case TONEGAMMA:
					q->setParams(wxString::Format("gamma,%0.2f",gamma->GetFloatValue()));
					break;
				case TONEREINHARD:
					if (tonenorm->GetValue())
						q->setParams(wxString::Format("reinhard,%s,norm",reinop->GetString(reinop->GetSelection())));
					else
						q->setParams(wxString::Format("reinhard,%s",reinop->GetString(reinop->GetSelection())));
					break;
				case TONELOG2:
					q->setParams(wxString::Format("log2"));
					break;
				case TONELOGGAM:
					q->setParams(wxString::Format("loggamma"));
					break;
				case TONEDUALLOGISTIC:
					q->setParams(wxString::Format("doublelogistic,%f,%f",dlL->GetFloatValue(), dlc->GetFloatValue()));
					break;
				case TONEFILMIC:
					if (tonenorm->GetValue())
						q->setParams(wxString::Format("filmic,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f,norm",filmicA->GetFloatValue(),filmicB->GetFloatValue(),filmicC->GetFloatValue(),filmicD->GetFloatValue(),power->GetFloatValue()));
					else
						q->setParams(wxString::Format("filmic,%0.2f,%0.2f,%0.2f,%0.2f,%0.2f",filmicA->GetFloatValue(),filmicB->GetFloatValue(),filmicC->GetFloatValue(),filmicD->GetFloatValue(),power->GetFloatValue()));
					break;
			}
			//if (tc) tc->SetCurve(makeXArray(10000));
			tcpane->SetCurve(makeXArray(10000));
			q->processPic();
			Refresh();
		}


		void floatParamChanged(wxCommandEvent& event)
		{
			if (gamb->GetValue() | filmicb->GetValue() | doublelogisticb->GetValue()) t.Start(500,wxTIMER_ONE_SHOT);
		}
		
		void floatParamUpdated(wxCommandEvent& event)
		{
			if (gamb->GetValue()) processTone(TONEGAMMA);
			if (filmicb->GetValue()) processTone(TONEFILMIC);
			if (doublelogisticb->GetValue()) processTone(TONEDUALLOGISTIC);
		}
		
		void OnTimer(wxTimerEvent& event)
		{
			if (gamb->GetValue()) processTone(TONEGAMMA);
			else if (filmicb->GetValue()) processTone(TONEFILMIC);
			else if (doublelogisticb->GetValue()) processTone(TONEDUALLOGISTIC);
		}

		std::vector<float> makeXArray(unsigned arraysize)
		{
			std::vector<float> xarray;
			std::map<std::string,std::string> imgdata;
			gImage X(arraysize,1,3,imgdata);
			std::vector<pix>& x = X.getImageData();
			for (unsigned i=0; i<x.size(); i++) x[i].r = (float) i / x.size();

			//tone map operation on X, builds transform curve for plotting
			if (gamb->GetValue()) X.ApplyToneMapGamma(gamma->GetFloatValue());
			else if (reinb->GetValue() ) {
				bool channel = true;
				if (reinop->GetString(reinop->GetSelection()) == "luminance") channel = false;
				X.ApplyToneMapReinhard(channel, tonenorm->GetValue());
			}
			else if (hybloggamb->GetValue()) X.ApplyToneMapLogGamma();
			else if (doublelogisticb->GetValue()) {
				std::map<std::string,std::string> p;
				p["L"] = wxString::Format("%f",dlL->GetFloatValue()).ToStdString();
				p["c"] = wxString::Format("%f",dlc->GetFloatValue()).ToStdString();
				X.ApplyToneMapDualLogistic(p);
			}
			else if (filmicb->GetValue())  X.ApplyToneMapFilmic(filmicA->GetFloatValue(),filmicB->GetFloatValue(),filmicC->GetFloatValue(),filmicD->GetFloatValue(),power->GetFloatValue(), tonenorm->GetValue());

			for (unsigned i=0; i<x.size(); i++) xarray.push_back(x[i].r);
			return xarray;
		}

	private:
		wxTimer t;
		myFloatCtrl *gamma, *filmicA, *filmicB, *filmicC, *filmicD, *power, *dlL, *dlc;
		wxCheckBox *enablebox, *tonenorm;
		wxRadioButton *gamb, *reinb, *log2b, *hybloggamb, *filmicb, *doublelogisticb;
		wxChoice *reinop;
		myToneCurvePane *tcpane;
		int tonemode;
		//ToneCurveDialog *tc;
};

PicProcessorTone::PicProcessorTone(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorTone::createPanel(wxSimplebook* parent)
{
	toolpanel = new TonePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorTone::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("tone..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_tone(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_tone(*dib, params);
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.tone.log","0") == "1"))
					log(wxString::Format(_("tool=tone,%s,imagesize=%dx%d,threads=%s,time=%s"),
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





