#include "PicProcessorScript.h"
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

#include <fstream>
#include <sstream>
#include <string>

#define SCRIPTENABLE 8700
#define SCRIPTFILESELECT 8701
#define SCRIPTFILESAVE 8702
#define SCRIPTUPDATE 8703
#define SCRIPTAUTOUPDATE 8704
#define SCRIPTPGMUPDATE 8705
#define SCRIPTOUTPUT 8706

class ScriptPanel: public PicProcPanel
{

	public:
		ScriptPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);

			enablebox = new wxCheckBox(this, SCRIPTENABLE, _("script:"));
			enablebox->SetValue(true);
			
			wxArrayString str;
			
			std::map<std::string,unsigned> names;
			std::map<std::string, std::string> ss =  myConfig::getConfig().getSubset("tool.script.");
			for (std::map<std::string, std::string>::iterator it=ss.begin(); it!=ss.end(); ++it) {
				std::vector<std::string> tokens = split(it->first, ".");
				names[tokens[0]] = 1;
			}
			
			for (std::map<std::string, unsigned>::iterator it = names.begin(); it!=names.end(); ++it)
				str.Add(wxString(it->first.c_str()));

			pgm = new wxChoice(this, SCRIPTPGMUPDATE, wxDefaultPosition, wxDefaultSize, str);
			pgm->SetSelection(0);

			//edit = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(200,200), wxTE_MULTILINE);
			//setEdit(params);
			//wxString editstring = params;
			//editstring.Replace(";","\n");
			//edit->SetValue(editstring);
			
			file = new wxStaticText(this, wxID_ANY, "(no file)");
			autobox = new wxCheckBox(this, SCRIPTAUTOUPDATE, _("auto update"), wxDefaultPosition, wxDefaultSize);
			
			retainbox = new wxCheckBox(this, wxID_ANY, _("retain files"), wxDefaultPosition, wxDefaultSize);
			
			outputbtn = new wxButton(this, SCRIPTOUTPUT, _("Output"));
			outputbtn->Enable(false);
			
			std::map<std::string,std::string> p = parse_script(params.ToStdString());
			if (p.find("program") != p.end()) pgm->SetSelection(pgm->FindString(p["program"]));
			if (p.find("script") != p.end()) file->SetLabel(wxString(p["script"]));
			
			//supports file invalidation (no file) if the pgm is changed.
			pgmsel = pgm->GetSelection();
			
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));

			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow();
			m->AddRowItem(new wxStaticText(this, wxID_ANY, "program: "), flags);
			m->AddRowItem(pgm, flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();
			m->AddRowItem(new wxButton(this, SCRIPTFILESELECT, _("Select File...")), flags);
			m->AddRowItem(file, flags);
			//m->AddRowItem(new wxButton(this, SCRIPTFILESAVE, _("Save File...")), flags);
			m->NextRow();
			
			//m->AddRowItem(edit, flags);
			//m->NextRow();
			
			m->AddRowItem(new wxButton(this, SCRIPTUPDATE, _("Run Script")), flags);
			m->AddRowItem(autobox, flags);
			m->AddRowItem(retainbox, flags);
			m->NextRow();
			m->AddRowItem(outputbtn, flags);
			m->End();

			SetSizerAndFit(m);

			if (!params.IsEmpty()) {
				scriptfile = wxFileName(params);
				file->SetLabel(scriptfile.GetFullName());
			}
			
			((PicProcessorScript *) q)->SetMenuString(GetProgram());

			t.SetOwner(this);

			Bind(wxEVT_CHECKBOX, &ScriptPanel::onEnable, this, SCRIPTENABLE);
			Bind(wxEVT_BUTTON, &ScriptPanel::selectFile, this, SCRIPTFILESELECT);
			//Bind(wxEVT_BUTTON, &ScriptPanel::saveFile, this, SCRIPTFILESAVE);
			Bind(wxEVT_BUTTON, &ScriptPanel::updateScript, this, SCRIPTUPDATE);
			Bind(wxEVT_BUTTON, &ScriptPanel::onOutput, this, SCRIPTOUTPUT);
			Bind(wxEVT_CHAR_HOOK, &ScriptPanel::OnKey,  this);
			Bind(wxEVT_TIMER, &ScriptPanel::OnTimer,  this);
			Bind(wxEVT_CHECKBOX, &ScriptPanel::onAuto, this, SCRIPTAUTOUPDATE);
			Bind(wxEVT_CHOICE, &ScriptPanel::onPgm, this, SCRIPTPGMUPDATE);
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

			wxString fname = wxFileSelector(_("Open GMIC Script..."), scriptfile.GetPath());  //, toollistpath.GetPath());
			if (fname == "") return;
			wxFileName filepath(fname);

			if (filepath.FileExists()) {
				filepath.MakeAbsolute();
				scriptfile = filepath;
				modtime = filepath.GetModificationTime();
				file->SetLabel(filepath.GetFullName());
				((PicProcessorScript *) q)->setSource(filepath.GetFullPath());
				if (filepath.GetPath() == wxFileName::GetCwd())
					q->setParams(wxString::Format("%s,%s",pgm->GetString(pgm->GetSelection()),filepath.GetFullName()));
				else
					q->setParams(wxString::Format("%s,%s",pgm->GetString(pgm->GetSelection()),filepath.GetFullPath()));
				q->processPic();
				
			}
			else wxMessageBox(_("Error: script file not found."));
		}
		
		void onAuto(wxCommandEvent& event)
		{
			if (!scriptfile.FileExists()) { 
				wxMessageBox("Select a script file first.");
				autobox->SetValue(false); 
				event.Skip(); 
				return;
			}
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
				wxFileName filepath = file->GetLabel();
				filepath.MakeAbsolute();
				if (filepath.GetPath() == wxFileName::GetCwd())
					q->setParams(wxString::Format("%s,%s",pgm->GetString(pgm->GetSelection()),filepath.GetFullName()));
				else
					q->setParams(wxString::Format("%s,%s",pgm->GetString(pgm->GetSelection()),filepath.GetFullPath()));

				q->processPic();
				modtime = m;
			}
			if (autobox->IsChecked()) t.Start(500,wxTIMER_ONE_SHOT);
			event.Skip();
		}
		
		void onPgm(wxCommandEvent& event)
		{
			if(pgmsel != pgm->GetSelection())
				file->SetLabel("(no file)");
			((PicProcessorScript *) q)->SetMenuString(GetProgram());
			pgmsel = pgm->GetSelection();
			autobox->SetValue(false); 
			setCmdOutput("");
		}
		
		void onOutput(wxCommandEvent& event)
		{
			wxMessageBox(cmdoutput, "Program Output");
		}
		
		wxString GetProgram()
		{
			return pgm->GetString(pgm->GetSelection());
		}
		
		bool RetainFiles()
		{
			return retainbox->IsChecked();
		}
		
		void setCmdOutput(wxString o)
		{
			cmdoutput = o;
			if (cmdoutput.Length() > 0) 
				outputbtn->Enable(true);
			else
				outputbtn->Enable(false);
		}


	private:
		wxCheckBox *enablebox, *autobox, *retainbox;
		wxButton *outputbtn;
		wxChoice *pgm;
		wxTextCtrl *edit;
		wxStaticText *file;
		wxFileName scriptfile;
		wxDateTime modtime;
		wxString cmdoutput;
		int pgmsel;
		wxTimer t;

};


PicProcessorScript::PicProcessorScript(wxString name, wxString command, wxTreeCtrl *tree, wxString imagefile, PicPanel *display): PicProcessor(name, command, tree, display)
{
	//showParams();
	//loadCommands(command);
	img = imagefile;
}

void PicProcessorScript::createPanel(wxSimplebook* parent)
{
	toolpanel = new ScriptPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

//void PicProcessorScript::selectFile()
//{
//	((GroupPanel *) toolpanel)->selectFile();
//}

void PicProcessorScript::setSource(wxString src)
{
	source = src;
}

wxString PicProcessorScript::getSource()
{
	return source;
}

void PicProcessorScript::SetMenuString(wxString pgm)
{
	m_tree->SetItemText(id, n+":"+pgm);
}

bool PicProcessorScript::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("script..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_script(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else {
		m_display->StopStatusBar(true);  //keeps image replacement and image query for status bar from stopming on each other...
		//create temp file from dib:
		wxFileName inimage, outimage;
		inimage = outimage = img;
		inimage.SetName(inimage.GetName()+"-in");
		inimage.SetExt("tif");
		outimage.SetName(outimage.GetName()+"-out");
		outimage.SetExt("tif");
		wxString pgmstr = params["program"];
		std::string chanfmt = wxString::Format("tool.script.%s.channelformat",pgmstr).ToStdString();
		//parm tool.script.[scriptprogram].channelformat: 8bit|16bit|float|unboundedfloat, this is the image bit format to be used in the TIFF files used for input to and output from the script program Default=16bit.
		std::string channelformat = myConfig::getConfig().getValueOrDefault(chanfmt,"16bit");
		BPP fmt = BPP_16;
		if (channelformat == "8bit") fmt = BPP_8;
		if (channelformat == "16bit") fmt = BPP_16;
		if (channelformat == "float") fmt = BPP_FP;
		if (channelformat == "unboundedfloat") fmt = BPP_UFP;
		dib->saveTIFF(inimage.GetFullName().ToStdString().c_str(), fmt, "");
		
		//create command string:
		std::string scriptprop = wxString::Format("tool.script.%s.command",pgmstr).ToStdString();
		//parm tool.script.[scriptprogram].command: Full path/filename to the [scriptprogram].exe program.  Default=(none), won't work without a valid program.
		wxString scriptcommand = wxString(myConfig::getConfig().getValueOrDefault(scriptprop,""));
		if (scriptcommand == "") {
			m_display->StopStatusBar(false);
			return false;
		}
		
		std::string fn = params["script"];
		std::ifstream ifs(fn);
		std::string scr( (std::istreambuf_iterator<char>(ifs) ),(std::istreambuf_iterator<char>()    ) );
		wxString script = wxString(scr);
		script.Replace("\n", " ");
		
		std::string cmdstr = wxString::Format("tool.script.%s.commandstring",pgmstr).ToStdString();
		//parm tool.script.[scriptprogram].commandstring: Command string for the script tool to run, e.g., "[program] [infile] [script] output [outfile],float" for a G'MIC invocation.  The [] parameters will be replaced with the script program, input file, script file, and output file names at invocation.
		wxString cmd = wxString(myConfig::getConfig().getValueOrDefault(cmdstr,""));
		cmd.Replace("[program]", scriptcommand);
		cmd.Replace("[infile]", inimage.GetFullName());
		cmd.Replace("[script]", script);
		cmd.Replace("[outfile]", outimage.GetFullName());

		//Execute command string, wait to finish:
		std::string shellstr = wxString::Format("tool.script.%s.shell",pgmstr).ToStdString();
		//parm tool.script.[scriptprogram].shell: Shell to use to run the script application and script.  Default=(empty), script application will be run as its own process, no shell window.  Note: rawproc's wxcmd is available for use here as a single-command shell that will display its execution.
		std::string shell = myConfig::getConfig().getValueOrDefault(shellstr,"");
		if (shell.size() > 0) {
			cmd = shell + " " + cmd;
			printf("%s\n", cmd.ToStdString().c_str()); fflush(stdout);
			wxExecuteEnv env;
			wxExecute(cmd,wxEXEC_SYNC,NULL,&env);
		}
		else {
			wxArrayString output, errors;
			printf("%s\n", cmd.ToStdString().c_str()); fflush(stdout);
			wxExecute (cmd, output, errors, wxEXEC_NODISABLE);
			if (output.GetCount() > 0 | errors.GetCount() > 0)
				((ScriptPanel *) toolpanel)->setCmdOutput(wxString::Format("output:\n%s\n\nerrors:\n%s\n", toWxString(output), toWxString(errors)));
			else
				((ScriptPanel *) toolpanel)->setCmdOutput("");
		}

		//get output image and put it in the dib:
		if (outimage.FileExists()) {
			gImage newdib(gImage::loadTIFF(outimage.GetFullName().ToStdString().c_str(), ""));
			dib->setImage(newdib.getImageData(), newdib.getWidth(), newdib.getHeight()); //retains the metadata and profile
			// delete temp image files if the retain box is not checked:
			if (!((ScriptPanel *) toolpanel)->RetainFiles()) {
				wxRemoveFile(inimage.GetFullName());
				wxRemoveFile(outimage.GetFullName());
			}
		}
		else wxMessageBox("Script failed to produce result file.");

		m_display->StopStatusBar(false);
		
		//result = process_gmic(*dib, params)  //ToDo: replace all above with this line...
		
		//if (paramexists(result,"treelabel")) m_tree->SetItemText(id, wxString(result["treelabel"]));
		
		//if (result.find("error") != result.end()) {
		//	wxMessageBox(wxString(result["error"]));
		//	ret = false;
		//}
		//else {
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.script.log","0") == "1"))
					log(wxString::Format(_("tool=script,%s,imagesize=%dx%d,time=%s"),
						params["mode"].c_str(),
						dib->getWidth(), 
						dib->getHeight(),
						result["duration"].c_str())
					);
		//}
	}

	dirty=false;
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return ret;
}





