
#include "PicProcessor.h"
#include "PicProcessorGray.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "gimage_parse.h"
#include "gimage_process.h"
#include "undo.xpm"
#include "copy.xpm"
#include "paste.xpm"

#include "util.h"

#define GRAYENABLE  6200
#define GRAYCOPY    6201
#define GRAYPASTE   6202
#define REDSLIDER   6203
#define GREENSLIDER 6204
#define BLUESLIDER  6205

class GrayPanel: public PicProcPanel
{

	public:
		GrayPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			Freeze();
			SetSize(parent->GetSize());
			
			wxArrayString p = split(params,",");

			rd = atof(p[0].ToStdString().c_str());
			gr = atof(p[1].ToStdString().c_str());
			bl = atof(p[2].ToStdString().c_str());

			enablebox = new wxCheckBox(this, GRAYENABLE, _("gray:"));
			enablebox->SetValue(true);

			red = new wxSlider(this, wxID_ANY, rd*100.0, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			val1 = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f",rd), wxDefaultPosition, wxSize(30, -1));

			green = new wxSlider(this, wxID_ANY, gr*100.0, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			val2 = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f",gr), wxDefaultPosition, wxSize(30, -1));

			blue = new wxSlider(this, wxID_ANY, bl*100.0, 0, 100, wxPoint(10, 30), wxSize(140, -1));
			val3 = new wxStaticText(this,wxID_ANY, wxString::Format("%2.2f",bl), wxDefaultPosition, wxSize(30, -1));

			val4 = new wxStaticText(this,wxID_ANY, wxString::Format(_("Total: %2.2f"), rd+gr+bl), wxDefaultPosition, wxSize(-1,-1));

			btn = new wxBitmapButton(this, wxID_ANY, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip(_("Reset RGB proportions to defaults"));


			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, GRAYCOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, GRAYPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));
			m->NextRow();

			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("red: "), wxDefaultPosition, wxSize(50,-1)), flags);
			m->AddRowItem(red, flags);
			m->AddRowItem(val1, flags);
			m->NextRow();

			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("green: "), wxDefaultPosition, wxSize(50,-1)), flags);
			m->AddRowItem(green, flags);
			m->AddRowItem(val2, flags);
			m->NextRow();

			m->AddRowItem(new wxStaticText(this,wxID_ANY, _("blue: "), wxDefaultPosition, wxSize(50,-1)), flags);
			m->AddRowItem(blue, flags);
			m->AddRowItem(val3, flags);
			m->AddRowItem(btn, flags);
			m->NextRow();

			m->AddRowItem(val4, flags);


			m->End();
			SetSizerAndFit(m);

			SetFocus();
			t.SetOwner(this);
			Bind(wxEVT_BUTTON, &GrayPanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &GrayPanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &GrayPanel::OnThumbTrack, this);
			Bind(wxEVT_CHECKBOX, &GrayPanel::onEnable, this, GRAYENABLE);
			Bind(wxEVT_TIMER, &GrayPanel::OnTimer,  this);
			Bind(wxEVT_BUTTON, &GrayPanel::OnCopy, this, GRAYCOPY);
			Bind(wxEVT_BUTTON, &GrayPanel::OnPaste, this, GRAYPASTE);
			Bind(wxEVT_CHAR_HOOK, &GrayPanel::OnKey,  this);
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

		void OnCopy(wxCommandEvent& event)
		{
			q->copyParamsToClipboard();
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Copied command to clipboard: %s"),q->getCommand()));
			
		}

		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {
				wxArrayString p = split(q->getParams(),",");
	
				rd = atof(p[0].ToStdString().c_str());
				gr = atof(p[1].ToStdString().c_str());
				bl = atof(p[2].ToStdString().c_str());
				red->SetValue(rd*100.0);
				green->SetValue(gr*100.0);
				blue->SetValue(bl*100.0);
				val1->SetLabel(wxString::Format("%2.2f", rd));
				val2->SetLabel(wxString::Format("%2.2f", gr));
				val3->SetLabel(wxString::Format("%2.2f", bl));
				val4->SetLabel(wxString::Format("Total: %2.2f", rd+gr+bl));

				q->processPic();
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format(_("Pasted command from clipboard: %s"),q->getCommand()));
				Refresh();
			}
			else wxMessageBox(wxString::Format("Invalid Paste"));
		}


		void OnChanged(wxCommandEvent& event)
		{
			rd = red->GetValue()/100.0;
			gr = green->GetValue()/100.0;
			bl = blue->GetValue()/100.0;
			val1->SetLabel(wxString::Format("%2.2f", rd));
			val2->SetLabel(wxString::Format("%2.2f", gr));
			val3->SetLabel(wxString::Format("%2.2f", bl));
			val4->SetLabel(wxString::Format(_("Total: %2.2f"), rd+gr+bl));
			t.Start(500,wxTIMER_ONE_SHOT);
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			rd = red->GetValue()/100.0;
			gr = green->GetValue()/100.0;
			bl = blue->GetValue()/100.0;
			val1->SetLabel(wxString::Format("%2.2f", rd));
			val2->SetLabel(wxString::Format("%2.2f", gr));
			val3->SetLabel(wxString::Format("%2.2f", bl));
			val4->SetLabel(wxString::Format(_("Total: %2.2f"), rd+gr+bl));
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%2.2f,%2.2f,%2.2f",red->GetValue()/100.0,green->GetValue()/100.0,blue->GetValue()/100.0));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
/*
			double resetredval = atof(myConfig::getConfig().getValueOrDefault("tool.gray.r","0.21").c_str());
			red->SetValue(resetredval*100.0);
			val1->SetLabel(wxString::Format("%2.2f", resetredval));

			double resetgreenval = atof(myConfig::getConfig().getValueOrDefault("tool.gray.g","0.72").c_str());
			green->SetValue(resetgreenval*100.0);
			val2->SetLabel(wxString::Format("%2.2f", resetgreenval));

			double resetblueval = atof(myConfig::getConfig().getValueOrDefault("tool.gray.b","0.07").c_str());
			blue->SetValue(resetblueval*100.0);
			val3->SetLabel(wxString::Format("%2.2f", resetblueval));

			val4->SetLabel(wxString::Format("Total: %2.2f", red->GetValue()/100.0+green->GetValue()/100.0+blue->GetValue()/100.0));

			q->setParams(wxString::Format("%2.2f,%2.2f,%2.2f",red->GetValue()/100.0,green->GetValue()/100.0,blue->GetValue()/100.0));
*/


			rd = atof(myConfig::getConfig().getValueOrDefault("tool.gray.r","0.21").c_str());
			red->SetValue(rd*100.0);
			val1->SetLabel(wxString::Format("%2.2f", rd));

			gr = atof(myConfig::getConfig().getValueOrDefault("tool.gray.g","0.72").c_str());
			green->SetValue(gr*100.0);
			val2->SetLabel(wxString::Format("%2.2f", gr));

			bl = atof(myConfig::getConfig().getValueOrDefault("tool.gray.b","0.07").c_str());
			blue->SetValue(bl*100.0);
			val3->SetLabel(wxString::Format("%2.2f", bl));

			val4->SetLabel(wxString::Format(_("Total: %2.2f"), rd+gr+bl));

			q->setParams(wxString::Format("%2.2f,%2.2f,%2.2f",rd,gr,bl));
			q->processPic();
			Refresh();
			event.Skip();

		}



	private:
		double rd, gr, bl;

		wxSlider *red, *green, *blue;
		wxStaticText *val1, *val2, *val3, *val4;
		wxBitmapButton *btn;
		wxCheckBox *enablebox;
		wxTimer t;

};


PicProcessorGray::PicProcessorGray(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorGray::createPanel(wxSimplebook* parent)
{
	toolpanel = new GrayPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorGray::processPicture(gImage *processdib) 
{
	if (!processingenabled) return true;
	
	((wxFrame*) m_display->GetParent())->SetStatusText(_("gray..."));
	bool ret = true;
	std::map<std::string,std::string> result;

	std::map<std::string,std::string> params;
	std::string pstr = getParams().ToStdString();

	if (!pstr.empty())
		params = parse_gray(std::string(pstr));
	
	if (params.find("error") != params.end()) {
		wxMessageBox(params["error"]);
		ret = false; 
	}
	else if (params.find("mode") == params.end()) {  //all variants need a mode, now...
		wxMessageBox("Error - no mode");
		ret = false;
	}
	else { 
		result = process_gray(*dib, params);
		//if (paramexists(result,"treelabel")) 
		//	m_tree->SetItemText(id, wxString(result["treelabel"]));
		//else
			m_tree->SetItemText(id, "gray");
		
		if (result.find("error") != result.end()) {
			wxMessageBox(wxString(result["error"]));
			ret = false;
		}
		else {
			m_display->SetModified(true);
			if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || 
				(myConfig::getConfig().getValueOrDefault("tool.gray.log","0") == "1"))
					log(wxString::Format(_("tool=gray,imagesize=%dx%d,threads=%s,time=%s"),
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

