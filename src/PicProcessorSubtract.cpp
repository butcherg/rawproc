#include "PicProcessorSubtract.h"
#include "PicProcPanel.h"
#include "myRowSizer.h"
#include "myConfig.h"
#include "myFloatCtrl.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "util.h"
#include "gimage/strutil.h"
#include "gimage/curve.h"

#define SUBTRACTENABLE 8100
#define SUBTRACTVAL 8101
#define SUBTRACTFILE 8102
#define SUBTRACTCAMERA 8103

class SubtractPanel: public PicProcPanel
{

	public:
		SubtractPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 

			enablebox = new wxCheckBox(this, SUBTRACTENABLE, _("subtract:"));
			enablebox->SetValue(true);
			
			wxArrayString str;
			str.Add("rgb");
			str.Add("red");
			str.Add("green");
			str.Add("blue");
			chan = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, str);
			chan->SetStringSelection("rgb");

			subb = new wxRadioButton(this, SUBTRACTVAL, _("value:"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP); 
			fileb = new wxRadioButton(this, SUBTRACTFILE, _("file:"));
			camb = new wxRadioButton(this, SUBTRACTCAMERA, _("camera:"));
			subb->SetValue(true);

			subtract = new myFloatCtrl(this, wxID_ANY, 0.0, 7);
			darkfile = new wxTextCtrl(this, wxID_ANY, "(none)", wxDefaultPosition, wxSize(150,TEXTCTRLHEIGHT));
			cam = new wxStaticText(this, wxID_ANY, "--", wxDefaultPosition, wxDefaultSize, wxST_ELLIPSIZE_MIDDLE);

			std::vector<std::string> p = split(params.ToStdString(),",");
			std::string param = p[0];

			if (isFloat(param)) {
				submode = SUBTRACTVAL;
				subb->SetValue(true);
				subtract->SetFloatValue(atof(param.c_str()));
			}
			else if (param == "rgb" | param == "red" | param == "green" | param == "blue") {
				submode = SUBTRACTVAL;
				subb->SetValue(true);
				if (p.size() >= 2) 
					if (isFloat(p[1])) subtract->SetFloatValue(atof(p[1].c_str()));
			}
			else if (param == "camera") {
				submode = SUBTRACTCAMERA;
				camb->SetValue(true);
				chan->Enable(false);
			}
			else {
				submode = SUBTRACTFILE;
				fileb->SetValue(true);
				darkfile->SetValue(wxString(param));
				chan->Enable(false);
			}
			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(chan, wxSizerFlags(0).Right().Border(wxRIGHT|wxTOP));

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(subb, flags);
			m->AddRowItem(subtract, flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(fileb, flags);

			m->NextRow(wxSizerFlags().Expand());
			//m->AddRowItem(darkfile,  wxSizerFlags(1).Left().Border(wxLEFT|wxTOP).CenterVertical());  //wx3.1
			m->AddRowItem(darkfile,  wxSizerFlags(1).Left().Border(wxLEFT|wxTOP).Center());
			m->AddRowItem(new wxButton(this, wxID_ANY, _("Select")), flags);

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			//m->AddRowItem(camb, flags.CenterVertical());  //wx3.1
			m->AddRowItem(camb, flags.Center());
			m->AddRowItem(cam, flags);
			m->NextRow();

			m->End();
			SetSizerAndFit(m);
			SetFocus();
			t.SetOwner(this);

			Bind(wxEVT_TIMER, &SubtractPanel::OnTimer, this);
			Bind(wxEVT_TEXT_ENTER,&SubtractPanel::fileChanged, this);
			Bind(myFLOATCTRL_CHANGE, &SubtractPanel::paramChanged, this);
			Bind(myFLOATCTRL_UPDATE, &SubtractPanel::paramUpdated, this);
			Bind(wxEVT_CHECKBOX, &SubtractPanel::onEnable, this, SUBTRACTENABLE);
			Bind(wxEVT_RADIOBUTTON, &SubtractPanel::OnRadioButton, this);
			Bind(wxEVT_BUTTON, &SubtractPanel::selectDarkFile, this);
			Bind(wxEVT_CHAR_HOOK, &SubtractPanel::OnKey,  this);
			Bind(wxEVT_CHOICE, &SubtractPanel::onChoice,  this);
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
		
		void onChoice(wxCommandEvent& event)
		{
			processSUB();
		}

		void setCameraVal(float val)
		{
			camval = val;
			cam->SetLabel(wxString::Format("%f",val));
			Refresh();
		}

		void setCameraVal(wxString str)
		{
			cam->SetLabel(str);
			Refresh();
		}


		void selectDarkFile(wxCommandEvent& event)
		{
			wxFileName fname, pname;
			fname.Assign(wxFileSelector(_("Select dark file")));
			darkfile->SetValue(fname.GetFullName());
			if (submode == SUBTRACTFILE) processSUB();
		}

		void processSUB()
		{
			float evval;
			switch (submode) {
				case SUBTRACTVAL:
					q->setParams(wxString::Format("%s,%f",chan->GetString(chan->GetSelection()),subtract->GetFloatValue()));
					q->processPic();
					break;
				case SUBTRACTFILE:
					q->setParams(wxString::Format("file,%s",darkfile->GetValue()));
					q->processPic();
					break;
				case SUBTRACTCAMERA:
					q->setParams("camera");
					q->processPic();
					break;

			}
		}
		
		void OnRadioButton(wxCommandEvent& event)
		{
			submode = event.GetId();
			switch (submode) {
				case SUBTRACTVAL:
					chan->Enable(true);
					break;
				case SUBTRACTCAMERA:
				case SUBTRACTFILE:
					chan->Enable(false);
					break;
			}
			processSUB();
		}
		
		void fileChanged(wxCommandEvent& event)
		{
			if (submode == SUBTRACTFILE) processSUB();
		}

		void paramChanged(wxCommandEvent& event)
		{
			if (submode == SUBTRACTVAL) t.Start(500,wxTIMER_ONE_SHOT);
		}

		void paramUpdated(wxCommandEvent& event)
		{
			if (submode == SUBTRACTVAL) processSUB();
		}
		
		void OnTimer(wxTimerEvent& event)
		{
			processSUB();
		}

	private:
		wxChoice *chan;
		wxCheckBox *enablebox;
		wxRadioButton *subb, *fileb, *camb;
		wxTextCtrl *darkfile;
		myFloatCtrl *subtract;
		wxStaticText *cam;
		wxTimer t;
		float camval;
		int submode;

};

PicProcessorSubtract::PicProcessorSubtract(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorSubtract::createPanel(wxSimplebook* parent)
{
	toolpanel = new SubtractPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	gImage dib = getPreviousPicProcessor()->getProcessedPic();
	std::map<std::string, std::string> info = dib.getInfo();
	
	if (info.find("Libraw.CFABlack") != info.end()) {
		std::string is = info["Libraw.CFABlack"];
		std::vector<std::string> s = split(is,",");
		std::string fs;
		for (unsigned i = 0; i<s.size(); i++) {
			if (i==0) {
				fs.append( tostr(atof(s[i].c_str())/65536.0) );
			}
			else {
				fs.append(",");
				fs.append( tostr(atof(s[i].c_str())/65536.0) );
			}
		}
		((SubtractPanel *) toolpanel)->setCameraVal(wxString::Format("%s\n(%s)", wxString(is), wxString(fs)));
	}
	else if (info.find("Libraw.PerChannelBlack") != info.end()) {
		std::string is = info["Libraw.PerChannelBlack"];
		std::vector<std::string> s = split(is,",");
		std::string fs;
		for (unsigned i = 0; i<s.size(); i++) {
			if (i==0) {
				fs.append( tostr(atof(s[i].c_str())/65536.0) );
			}
			else {
				fs.append(",");
				fs.append( tostr(atof(s[i].c_str())/65536.0) );
			}
		}
		((SubtractPanel *) toolpanel)->setCameraVal(wxString::Format("%s\n(%s)", wxString(is), wxString(fs)));
	}
	else if (info.find("Libraw.Black") != info.end()) {
		int subtract = atoi(info["Libraw.Black"].c_str());
		((SubtractPanel *) toolpanel)->setCameraVal(wxString::Format("%f (%d)", (float) subtract / 65536.0, subtract));
	}
	
	
/*
	int subtract = atoi(dib.getInfoValue("Libraw.Black").c_str());
	if (subtract != 0) {
		((SubtractPanel *) toolpanel)->setCameraVal(wxString::Format("%f (%d)", (float) subtract / 65536.0, subtract));
	}
	else {
		float subr=0.0, subg1=0.0, subg2=0.0, subb=0.0;
		std::vector<std::string> s = split(dib.getInfoValue("Libraw.PerChannelBlack"),",");
		if (s.size() >= 4) {
			if (s[0] =="0" & s[1] =="0" & s[2] =="0" & s[3] =="0") {
				((SubtractPanel *) toolpanel)->setCameraVal("--");
			}
			else {
				subr = atof(s[0].c_str());
				subg1 = atof(s[0].c_str());
				subb = atof(s[0].c_str());
				subg2 = atof(s[0].c_str());
				((SubtractPanel *) toolpanel)->setCameraVal(wxString::Format("%s,%s,%s,%s (%d,%d,%d,%d)",wxString(s[0]),wxString(s[1]),wxString(s[2]),wxString(s[3]),subr,subg1,subg2,subb));
			}
		}
	}
*/
	toolpanel->Update();
}

bool PicProcessorSubtract::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("subtract..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_subtract(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_subtract(*dib, params);
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.subtract.log","0") == "1"))
					log(wxString::Format(_("tool=subtract,%s,imagesize=%dx%d,threads=%s,time=%s"),
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



