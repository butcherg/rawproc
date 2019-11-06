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
#define GROUPUPDATE 8202

class GroupPanel: public PicProcPanel
{

	public:
		GroupPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);

			enablebox = new wxCheckBox(this, GROUPENABLE, "group:");
			enablebox->SetValue(true);

			edit = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,200), wxTE_MULTILINE);
			setEdit(params);
			//wxString editstring = params;
			//editstring.Replace(";","\n");
			//edit->SetValue(editstring);
			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();

			m->AddRowItem(new wxButton(this, GROUPFILESELECT, "Select File"), flags);
			m->AddRowItem(new wxButton(this, GROUPUPDATE, "Update Group"), flags);
			m->NextRow();
			m->AddRowItem(edit, flags);
			m->End();

			SetSizerAndFit(m);
			m->Layout();
			SetFocus();

			Bind(wxEVT_CHECKBOX, &GroupPanel::onEnable, this, GROUPENABLE);
			Bind(wxEVT_BUTTON, &GroupPanel::selectFile, this, GROUPFILESELECT);
			Bind(wxEVT_BUTTON, &GroupPanel::updateGroup, this, GROUPUPDATE);
			Refresh();
			Update();
		}
/*
		~GroupPanel()
		{

		}
*/
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

		void updateGroup(wxCommandEvent& event)
		{
			wxString commandstring = edit->GetValue();
			commandstring.Replace("\n",";");
			((PicProcessorGroup *) q)->loadCommands(commandstring);
			((PicProcessorGroup *) q)->setSource("");
			q->processPic();
		}

		void setEdit(wxString commandstring)
		{
			commandstring.Replace(";","\n");
			edit->SetValue(commandstring);
		}

		void selectFile(wxCommandEvent& event)
		{
			selectFile();
		}

		void selectFile()
		{
			wxString commandstring;
			wxFileName toollistpath;
			toollistpath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("app.toollistpath","")));

			wxString fname = wxFileSelector("Open Tool List...", toollistpath.GetPath());
			if (fname == "") return;
			wxFileName filepath(fname);

			wxTextFile toolfile(filepath.GetFullPath());
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
				setEdit(commandstring);
				((PicProcessorGroup *) q)->loadCommands(commandstring);
				((PicProcessorGroup *) q)->setSource(filepath.GetFullName());
				q->processPic();
			}
			else wxMessageBox("Error: tool file not found.");
		}


	private:
		wxCheckBox *enablebox;
		wxTextCtrl *edit;

};

class PicProcessorGroupItem : public PicProcessor
{
public:
	PicProcessorGroupItem(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxTreeItemId parent): PicProcessor(name, command, tree, display, parent)
	{
	}

};



PicProcessorGroup::PicProcessorGroup(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display)
{
	//showParams();
	loadCommands(command);
}

void PicProcessorGroup::createPanel(wxSimplebook* parent)
{
	toolpanel = new GroupPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

void PicProcessorGroup::selectFile()
{
	((GroupPanel *) toolpanel)->selectFile();
}

void PicProcessorGroup::loadCommands(wxString commandstring)
{
	PicProcessor *item;
	if (id.IsOk() & m_tree->GetSelection() == id) { 
		m_tree->DeleteChildren(id);
		wxArrayString p = split(commandstring,";");
		for (unsigned i=0; i< p.GetCount(); i++) {
			wxArrayString nc = split(p[i],":");
			if (nc.GetCount() < 2) nc.Add("");
			if (nc[1] != "") {
				wxArrayString firstp = split(nc[1], ",");
				nc[0] = nc[0]+":"+firstp[0];
			}
			//m_tree->SetItemBold(m_tree->AppendItem(id, nc[0], -1, -1, new GroupData(nc[0],nc[1])));	
			//m_tree->SetItemBold(m_tree->AppendItem(id, nc[0], -1, -1, new GroupData(nc[0],nc[1], m_tree, m_display, id)));	
			item = new PicProcessorGroupItem(nc[0],nc[1], m_tree, m_display, id);
		}
		m_tree->Expand(id);
		c = commandstring;
	}
}

void PicProcessorGroup::setSource(wxString src)
{
	source = src;
	m_tree->SetItemText(id, n+":"+source);
}

wxString PicProcessorGroup::getSource()
{
	return source;
}

bool PicProcessorGroup::processPicture(gImage *processdib) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("group...");

	bool result = true;
	wxArrayString p = split(wxString(c),";");
	
	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.group.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);
	
	dib = processdib;
	if (!global_processing_enabled) return true;

	if (processingenabled) {
		mark();
		for (int i=0; i<p.size(); i++) {
			((wxFrame*) m_display->GetParent())->SetStatusText(wxString::Format("group command: %s",p[i]));
			do_cmd(*dib, p[i].ToStdString(), "", false);
		}
		m_display->SetModified(true);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.group.log","0") == "1"))
			log(wxString::Format("tool=group,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}





