#include "PicProcessorGMIC.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "util.h"
#include "gimage_cmd.h"
#include "gimage/strutil.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include <wx/textfile.h>
#include <wx/datetime.h>

#define GMICENABLE 8600
#define GMICFILESELECT 8601
#define GMICFILESAVE 8602
#define GMICUPDATE 8603
#define GMICAUTOUPDATE 8604

class GMICPanel: public PicProcPanel
{

	public:
		GMICPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);

			enablebox = new wxCheckBox(this, GMICENABLE, _("gmic:"));
			enablebox->SetValue(true);

			//edit = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,200), wxTE_MULTILINE);
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
			m->AddRowItem(new wxButton(this, GMICFILESELECT, _("Select File...")), flags);
			m->AddRowItem(file, flags);
			//m->AddRowItem(new wxButton(this, GMICFILESAVE, _("Save File...")), flags);
			m->NextRow();
			//m->AddRowItem(edit, flags);
			
			//m->NextRow();
			m->AddRowItem(new wxButton(this, GMICUPDATE, _("Run Script")), flags);
			m->AddRowItem(autobox, flags);
			m->End();

			SetSizerAndFit(m);
			
			t.SetOwner(this);

			Bind(wxEVT_CHECKBOX, &GMICPanel::onEnable, this, GMICENABLE);
			Bind(wxEVT_BUTTON, &GMICPanel::selectFile, this, GMICFILESELECT);
			//Bind(wxEVT_BUTTON, &GMICPanel::saveFile, this, GMICFILESAVE);
			Bind(wxEVT_BUTTON, &GMICPanel::updateScript, this, GMICUPDATE);
			Bind(wxEVT_CHAR_HOOK, &GMICPanel::OnKey,  this);
			Bind(wxEVT_TIMER, &GMICPanel::OnTimer,  this);
			Bind(wxEVT_CHECKBOX, &GMICPanel::onAuto, this, GMICAUTOUPDATE);
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

		void updateScript(wxCommandEvent& event)
		{
			q->processPic();
		}

		void setEdit(wxString commandstring)
		{
			//commandstring.Replace(";","\n");
			//edit->SetValue(commandstring);
		}

		void selectFile(wxCommandEvent& event)
		{
			selectFile();
		}

		void selectFile()
		{
			wxString commandstring;
			//wxFileName toollistpath;
			//toollistpath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("app.toollistpath","")));

			wxString fname = wxFileSelector(_("Open GMIC Script..."));  //, toollistpath.GetPath());
			if (fname == "") return;
			wxFileName filepath(fname);

			if (filepath.FileExists()) {
				scriptfile = filepath;
				modtime = filepath.GetModificationTime();
				file->SetLabel(filepath.GetFullName());
				((PicProcessorGMIC *) q)->setSource(filepath.GetFullName());
				q->setParams(filepath.GetFullName());
				q->processPic();
				
			}
			else wxMessageBox(_("Error: script file not found."));
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
		wxCheckBox *enablebox, *autobox;
		wxTextCtrl *edit;
		wxStaticText *file;
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





