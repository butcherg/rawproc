#include "PicProcessorGMIC.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "util.h"
#include "gimage_cmd.h"
#include "strutil.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include <wx/textfile.h>
#include <wx/datetime.h>

#define GMICENABLE 8600
#define GMICFILE 8601
#define GMICSCRIPT 8602
#define GMICFILESELECT 8603
//#define GMICFILESAVE 8604
#define GMICFILEUPDATE 8605
#define GMICAUTOUPDATE 8606
#define GMICSCRIPTUPDATE 8607
#define GMICSCRIPTSAVE 8608

class GMICPanel: public PicProcPanel
{

	public:
		GMICPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);

			enablebox = new wxCheckBox(this, GMICENABLE, _("gmic:"));
			enablebox->SetValue(true);
			
			fileb = new wxRadioButton(this, GMICFILE, _("file:"));
			scriptb = new wxRadioButton(this, GMICSCRIPT, _("script"));

			edit = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,200), wxTE_MULTILINE);
			script = new wxStaticText(this, wxID_ANY,"");
			//setEdit(params);
			//wxString editstring = params;
			//editstring.Replace(";","\n");
			//edit->SetValue(editstring);
			
			file = new wxStaticText(this, wxID_ANY, "(no file)");
			autobox = new wxCheckBox(this, GMICAUTOUPDATE, _("auto update"));
			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(fileb, flags);
			m->NextRow();
			m->AddRowItem(new wxButton(this, GMICFILESELECT, _("Select File...")), flags);
			m->AddRowItem(file, flags);
			//m->AddRowItem(new wxButton(this, GMICFILESAVE, _("Save File...")), flags);
			m->NextRow();
			
			//m->NextRow();
			m->AddRowItem(new wxButton(this, GMICFILEUPDATE, _("Run File")), flags);
			m->AddRowItem(autobox, flags);
			
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(scriptb, flags);
			m->NextRow();
			m->AddRowItem(edit, flags);
			m->NextRow();
			m->AddRowItem(new wxButton(this, GMICSCRIPTUPDATE, _("Run Script")), flags);
			m->AddRowItem(new wxButton(this, GMICSCRIPTSAVE, _("Save Script...")), flags);
			m->AddRowItem(script, flags);
			
			m->End();

			SetSizerAndFit(m);
			
			if (!params.IsEmpty()) {
				std::vector<std::string> p = split(params.ToStdString(), ",");
				if (p[0] == "script") {
					edit->SetValue(wxString(gmic_decode(p[1])));
					scriptb->SetValue(true);
				}
				else {
					scriptfile = wxFileName(params);
					file->SetLabel(scriptfile.GetFullName());
					fileb->SetValue(true);
				}
			}

			t.SetOwner(this);

			Bind(wxEVT_CHECKBOX, &GMICPanel::onEnable, this, GMICENABLE);
			Bind(wxEVT_BUTTON, &GMICPanel::selectFile, this, GMICFILESELECT);
			//Bind(wxEVT_BUTTON, &GMICPanel::saveFile, this, GMICFILESAVE);
			Bind(wxEVT_BUTTON, &GMICPanel::update, this, GMICFILEUPDATE);
			Bind(wxEVT_CHAR_HOOK, &GMICPanel::OnKey,  this);
			Bind(wxEVT_TIMER, &GMICPanel::OnTimer,  this);
			Bind(wxEVT_CHECKBOX, &GMICPanel::onAuto, this, GMICAUTOUPDATE);
			Bind(wxEVT_BUTTON, &GMICPanel::update, this, GMICSCRIPTUPDATE);
			Bind(wxEVT_BUTTON, &GMICPanel::saveScript, this, GMICSCRIPTSAVE);
			Bind(wxEVT_RADIOBUTTON, &GMICPanel::update, this);
			Thaw();
		}
		

		void onEnable(wxCommandEvent& event)
		{
			if (enablebox->GetValue()) 
				q->enableProcessing(true);
			else 
				q->enableProcessing(false);
			processGMIC();
		}
		
		void processGMIC()
		{
			/*
			switch (src) {
				case GMICFILEUPDATE:
					q->setParams(filestr);
					break;
				case GMICSCRIPTUPDATE:
					q->setParams(wxString::Format("script,%s", wxString(gmic_encode(edit->GetValue().ToStdString()))));
					break;
			}
			*/
			if (fileb->GetValue()) {
				q->setParams(filestr);
				q->processPic();
			}
			else if (scriptb->GetValue()) {
				q->setParams(wxString::Format("script,%s", wxString(gmic_encode(edit->GetValue().ToStdString()))));
				q->processPic();
			}
			Refresh();
		}

		void update(wxCommandEvent& event)
		{
			processGMIC();
		}
		
		void setEdit(wxString commandstring)
		{
			//commandstring.Replace(";","\n");
			//edit->SetValue(commandstring);
		}

		void selectFile(wxCommandEvent& event)
		{
			wxString commandstring;
			//wxFileName toollistpath;
			//toollistpath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("app.toollistpath","")));

			wxString fname = wxFileSelector(_("Open GMIC Script..."), scriptfile.GetPath());  //, toollistpath.GetPath());
			if (fname == "") return;
			wxFileName filepath(fname);

			if (filepath.FileExists()) {
				filepath.MakeAbsolute();
				scriptfile = filepath;
				modtime = filepath.GetModificationTime();
				file->SetLabel(filepath.GetFullName());
				((PicProcessorGMIC *) q)->setSource(filepath.GetFullPath());
				if (filepath.GetPath() == wxFileName::GetCwd())
					//q->setParams(filepath.GetFullName());
					filestr = filepath.GetFullName();
				else
					//q->setParams(filepath.GetFullPath());
					filestr = filepath.GetFullPath();
				fileb->SetValue(true);
				processGMIC();
				
			}
			else wxMessageBox(_("Error: script file not found."));
		}
		
		void saveScript(wxCommandEvent& event)
		{
			wxString scriptstring = wxFileSelector(_("Save script..."),"","","",_("Script files (*.txt)|*.txt|"),wxFD_SAVE);
			if (scriptstring == "") return;
			wxFileName scriptname(scriptstring);
			
			wxFile f;
			if (!f.Open(scriptname.GetFullPath(), wxFile::write)) {
				wxMessageBox(wxString::Format("Script file %s save failed.",scriptname.GetFullName()));
			}
			else {
				f.Write(edit->GetValue());
				f.Close();
				wxMessageBox(wxString::Format("Script file %s saved.",scriptname.GetFullName()));
				script->SetLabel(scriptname.GetFullName());
			}
		}
		
		void onAuto(wxCommandEvent& event)
		{
			if (autobox->IsChecked()) {
				t.Stop();
				modtime = scriptfile.GetModificationTime();
				t.Start(500,wxTIMER_ONE_SHOT);
			}
			else t.Stop();
		}
		
		void OnTimer(wxTimerEvent& event)
		{
			wxDateTime m = scriptfile.GetModificationTime();
			if (!m.IsEqualTo(modtime)) {
				q->processPic();
				modtime = m;
			}
			if (autobox->IsChecked()) t.Start(500,wxTIMER_ONE_SHOT);
			event.Skip();
		}


	private:
		wxRadioButton *fileb, *scriptb;
		wxCheckBox *enablebox, *autobox;
		wxTextCtrl *edit;
		wxStaticText *file, *script;
		wxString filestr;
		wxFileName scriptfile;
		wxDateTime modtime;
		wxTimer t;

};


PicProcessorGMIC::PicProcessorGMIC(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display)
{
	//showParams();
	//loadCommands(command);
}

void PicProcessorGMIC::createPanel(wxSimplebook* parent)
{
	toolpanel = new GMICPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

//void PicProcessorGMIC::selectFile()
//{
//	((GroupPanel *) toolpanel)->selectFile();
//}

void PicProcessorGMIC::setSource(wxString src)
{
	source = src;
	//m_tree->SetItemText(id, n+":"+source);
}

wxString PicProcessorGMIC::getSource()
{
	return source;
}

bool PicProcessorGMIC::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("gmic..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_gmic(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	//else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
	//	wxMessageBox("Error - no mode");
	//	ret = false;
	//}
	else { 
		result = process_gmic(*dib, params);
		if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.gmic.log","0") == "1"))
					log(wxString::Format(_("tool=gmic,%s,imagesize=%dx%d,time=%s"),
						params["mode"].c_str(),
						dib->getWidth(), 
						dib->getHeight(),
						result["duration"].c_str())
					);
		}
	}

	dirty=false;
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return ret;
}





