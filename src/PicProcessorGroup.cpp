#include "PicProcessorGroup.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "util.h"
#include "gimage_cmd.h"
#include "gimage/strutil.h"
#include <wx/textfile.h>

#define GROUPENABLE 8200
#define GROUPFILESELECT 8201

class GroupPanel: public PicProcPanel
{

	public:
		GroupPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);

			enablebox = new wxCheckBox(this, GROUPENABLE, "group:");
			enablebox->SetValue(true);

			edit = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,TEXTCTRLHEIGHT),wxTE_PROCESS_ENTER);
			
			myRowSizer *m = new myRowSizer();
			m->AddRowItem(enablebox, flags);
			m->NextRow();
			m->AddRowItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), flags);
			m->NextRow();
			m->AddRowItem(edit, flags);
			m->AddRowItem(new wxButton(this, GROUPFILESELECT, "Select File"), flags);
			m->End();

			SetSizerAndFit(m);
			m->Layout();
			SetFocus();

			Bind(wxEVT_CHECKBOX, &GroupPanel::onEnable, this, GROUPENABLE);
			Bind(wxEVT_BUTTON, &GroupPanel::selectFile, this, GROUPFILESELECT);
			Refresh();
			Update();
		}

		~GroupPanel()
		{

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

		void selectFile(wxCommandEvent& event)
		{
			wxString commandstring;
			wxFileName toollistpath;
			toollistpath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("app.toollistpath","")));

			wxString fname = wxFileSelector("Open Tool List...", toollistpath.GetPath());
			if (fname == "") return;

			wxTextFile toolfile(fname);
			if (toolfile.Open()) {

				myConfig::getConfig().enableTempConfig(true);
				wxString token = toolfile.GetFirstLine();
				while (!toolfile.Eof())  {
					wxArrayString cmd = split(token, ":");	
					if (cmd.GetCount() > 0) {
						wxString params;
						if (cmd.GetCount() >=2) params = cmd[1];
						if (cmd[0] == "set") {
							wxArrayString prop = split(params,"=");
							if (prop.GetCount() >=2) myConfig::getConfig().setValue(std::string(prop[0].c_str()),std::string(prop[1].c_str()));
						}
						else {
							commandstring.Append(wxString::Format("%s;",token));
						}
					}

					token = toolfile.GetNextLine();
				}
				myConfig::getConfig().enableTempConfig(false);
				toolfile.Close();
				q->setParams(commandstring);
				q->processPic();
			}
			else wxMessageBox("Error: tool file not found.");
		}


	private:
		wxCheckBox *enablebox;
		wxTextCtrl *edit;

};

PicProcessorGroup::PicProcessorGroup(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	//showParams();
}

void PicProcessorGroup::createPanel(wxSimplebook* parent)
{
	toolpanel = new GroupPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorGroup::processPic(bool processnext) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("group...");

	bool result = true;
	wxArrayString p = split(wxString(c),";");
	
	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.group.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());

	if (processingenabled) {
		mark();
		for (int i=0; i<p.size(); i++) {
			((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("group command: %s",p[i]));
			do_cmd(*dib, p[i].ToStdString(), "", false);
		}
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.group.log","0") == "1"))
			log(wxString::Format("tool=group,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





