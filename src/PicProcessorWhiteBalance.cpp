#include "PicProcessorWhiteBalance.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
#include "gimage/curve.h"
#include <wx/spinctrl.h>
#include <wx/clipbrd.h>

#define WBAUTO 8600
#define WBPATCH 8700

/*
class myDoubleCtrl: public wxTextCtrl
{
public:
	myDoubleCtrl(wxWindow *parent, wxWindowID id, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0): wxTextCtrl(parent, id, "", pos, size, wxTE_PROCESS_ENTER)

	{
		dig = 2;
		wxTextCtrl::SetValue(wxString::Format("%0.2f", 0.0).c_str());
		Bind(wxEVT_TEXT_ENTER, &myDoubleCtrl::paramChanged, this);
		Bind(wxEVT_MOUSEWHEEL, &myDoubleCtrl::onWheel, this);
		Refresh();
	}

	void paramChanged(wxCommandEvent& event)
	{
		val = atof(wxTextCtrl::GetValue().c_str());
		wxCommandEvent e(wxEVT_SPINCTRLDOUBLE);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
		Refresh();
		event.Skip();
	}

	void onWheel(wxMouseEvent& event)
	{
printf("Mousewheel...\n");
		if (event.GetWheelRotation() > 0)
			val += inc;
		else
			val -= inc;
		if (val > up) val = up;
		if (val < low) val = low;
		wxTextCtrl::SetValue(wxString::Format("%0.2f", val).c_str());
		wxCommandEvent e(wxEVT_SPINCTRLDOUBLE);
		e.SetEventObject(this);
		e.SetString("update");
		ProcessWindowEvent(e);
		event.Skip();
	}

	double GetValue()
	{
		val = atof(GetLineText(0).c_str());
		return val;
	}

	void SetValue(double value)
	{
		val = value;
		if (val > up) val = up;
		if (val < low) val = low;
		wxTextCtrl::SetValue(wxString::Format("%0.2f", val).c_str());
		Refresh();
	}

	void SetDigits(int digits)
	{
		dig = digits;
	}

	void SetRange(double lower, double upper)
	{
		up = upper;
		low = lower;
	}

	void SetIncrement(double increment)
	{
		inc = increment;
	}

private:
	double val;
	unsigned dig;
	double up, low, inc;


};
*/

class WhiteBalancePanel: public PicProcPanel
{

	public:
		WhiteBalancePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			double rm, gm, bm;
			double min=0.001, max=3.0, digits=3, increment=0.001;
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).Expand();
			wxArrayString p = split(params,",");
			rm = atof(p[0]);
			gm = atof(p[1]);
			bm = atof(p[2]);


			g->Add(new wxStaticText(this,wxID_ANY, "red mult:"), wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			rmult = new wxSpinCtrlDouble(this, wxID_ANY,"1.0",wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, min, max, 0, 0.001);
			rmult->SetValue(rm);
			rmult->SetDigits(3);
			//rmult->SetRange(0.001,3.0);
			//rmult->SetIncrement(0.001);
			g->Add(rmult, wxGBPosition(0,1), wxGBSpan(1,2), wxEXPAND | wxALIGN_LEFT |wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "green mult:"), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			gmult = new wxSpinCtrlDouble(this, wxID_ANY,"1.0");
			gmult->SetValue(gm);
			gmult->SetDigits(3);
			gmult->SetRange(0.001,3.0);
			gmult->SetIncrement(0.001);
			g->Add(gmult, wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			g->Add(new wxStaticText(this,wxID_ANY, "blue mult:"), wxGBPosition(2,0), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);
			bmult = new wxSpinCtrlDouble(this, wxID_ANY,"1.0");
			//bmult = new myDoubleCtrl(this, wxID_ANY);
			bmult->SetValue(bm);
			bmult->SetDigits(3);
			bmult->SetRange(0.001,3.0);
			bmult->SetIncrement(0.001);
			g->Add(bmult, wxGBPosition(2,1), wxDefaultSpan, wxALIGN_LEFT |wxALL, 3);

			g->Add(0,10, wxGBPosition(3,0));

			g->Add(new wxButton(this, WBAUTO, "Auto"), wxGBPosition(4,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			g->Add(new wxButton(this, WBPATCH, "Patch"), wxGBPosition(5,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			
			SetSizerAndFit(g);
			g->Layout();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_SPINCTRLDOUBLE,&WhiteBalancePanel::paramChanged, this);
			Bind(wxEVT_TIMER, &WhiteBalancePanel::OnTimer, this);
			Bind(wxEVT_BUTTON, &WhiteBalancePanel::OnButton, this);
			//Bind(wxEVT_TEXT_ENTER, &WhiteBalancePanel::paramChanged, this);
			Refresh();
			Update();
		}

		~WhiteBalancePanel()
		{
			t->~wxTimer();
		}

		void setMultipliers(double rm, double gm, double bm)
		{
			rmult->SetValue(rm);
			gmult->SetValue(gm);
			bmult->SetValue(bm);
			Refresh();
		}

		void paramChanged(wxCommandEvent& event)
		{
			t->Start(500,wxTIMER_ONE_SHOT);
		}

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%f,%f,%f",rmult->GetValue(), gmult->GetValue(), bmult->GetValue()));
			q->processPic();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			switch (event.GetId()) {
				case WBAUTO:
					q->setParams("");
					q->processPic();
					break;
				case WBPATCH:
					//wxMessageBox("Not yet!!");
					wxTextDataObject cb;
					unsigned x, y;
					double rm=1.0, gm=1.0, bm=1.0;
					double ra, ga, ba;
					if (wxTheClipboard->Open()) {
						wxTheClipboard->GetData(cb);
						wxArrayString p = split(cb.GetText(),",");

						if (p.GetCount() == 2) {
							x = atoi(p[0]);
							y = atoi(p[1]);
							std::vector<double> a = ((PicProcessorWhiteBalance *)q)->getPatchMeans(x, y, 1.5);
							ra = a[0];
							ga = a[1];
							ba = a[2];
							rm = ga / ra;
							bm = ga / ba;
							rmult->SetValue(rm);
							gmult->SetValue(gm);
							bmult->SetValue(bm);
							q->setParams(wxString::Format("%f,%f,%f",rm, gm, bm));
							q->processPic();
							Refresh();
						}
						wxTheClipboard->Close();
					}
					break;
			}
			event.Skip();
		}

	private:
		wxTextCtrl *edit;
		wxSpinCtrlDouble *rmult, *gmult ,*bmult;
		//myDoubleCtrl *bmult;
		wxTimer *t;

};

PicProcessorWhiteBalance::PicProcessorWhiteBalance(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command, tree, display) 
{
	
	double redmult=1.0, greenmult=1.0, bluemult=1.0;
	if (command == "") {
		std::vector<double> rgbmeans = 	getPreviousPicProcessor()->getProcessedPic().CalculateChannelMeans();
		//redmult = rgbmeans[0] / rgbmeans[1];
		//bluemult = rgbmeans[2] / rgbmeans[1];
		redmult = rgbmeans[1] / rgbmeans[0];
		bluemult = rgbmeans[1] / rgbmeans[2];
		c = wxString::Format("%f,%f,%f",redmult, greenmult, bluemult);
	}
}

void PicProcessorWhiteBalance::createPanel(wxSimplebook* parent)
{
	toolpanel = new WhiteBalancePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

std::vector<double> PicProcessorWhiteBalance::getPatchMeans(int x, int y, float radius)
{
	std::vector<double> means;
	double rsum=0.0, gsum=0.0, bsum=0.0;
	gImage prev = getPreviousPicProcessor()->getProcessedPic();
	int count = 0;
	for (int i=x-radius; i<x+radius; i++) {
		for (int j=y-radius; j<y+radius; j++) {
			pix p = prev.getRGB(i,j);
			rsum += p.r;
			gsum += p.g;
			bsum += p.b;
			count++;
		}
	}
	means.push_back(rsum/(double)count);
	means.push_back(gsum/(double)count);
	means.push_back(bsum/(double)count);
	return means;
}

bool PicProcessorWhiteBalance::processPic(bool processnext) 
{
	double redmult=1.0, greenmult=1.0, bluemult=1.0;
	((wxFrame*) m_display->GetParent())->SetStatusText("white balance...");


	if (c == "") {
		std::vector<double> rgbmeans = 	getPreviousPicProcessor()->getProcessedPic().CalculateChannelMeans();
		//redmult = rgbmeans[0] / rgbmeans[1];
		//bluemult = rgbmeans[2] / rgbmeans[1];
		redmult = rgbmeans[1] / rgbmeans[0];
		bluemult = rgbmeans[1] / rgbmeans[2];
		setParams(wxString::Format("%f,%f,%f",redmult, greenmult, bluemult));
		((WhiteBalancePanel *) toolpanel)->setMultipliers(redmult, greenmult, bluemult);
	}
	else {
		wxArrayString p = split(c, ",");
		redmult = atof(p[0]);
		greenmult = atof(p[1]);
		bluemult = atof(p[2]);
	}

	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.whitebalance.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyWhiteBalance(redmult, greenmult, bluemult, threadcount);
	wxString d = duration();

	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.gamma.log","0") == "1"))
		log(wxString::Format("tool=gamma,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}





