
#include "PicProcessor.h"
#include "PicProcessorCurve.h"
#include "PicProcPanel.h"
#include "CurvePane.h"
#include "myConfig.h"
#include "myRowColumnSizer.h"
#include "util.h"

#include <wx/choice.h>

#define CURVEENABLE 6800


class CurvePanel: public PicProcPanel
{
	public:

		CurvePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxArrayString str;
			str.Add("rgb");
			str.Add("red");
			str.Add("green");
			str.Add("blue");
			str.Add("tone"); 
			chan = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, str);

			enablebox = new wxCheckBox(this, CURVEENABLE, "curve:");
			enablebox->SetValue(true);
			curve = new CurvePane(this, params);

			myRowColumnSizer *m = new myRowColumnSizer(3,3);
			m->AddItem(enablebox, wxALIGN_LEFT);
			m->AddItem(chan, wxALIGN_RIGHT);
			m->NextRow();
			m->AddItem(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxSize(280,2)), wxALIGN_LEFT, 2);
			m->NextRow();
			m->AddItem(curve, wxALIGN_LEFT, 2);

			SetSizerAndFit(m);
			m->Layout();
			SetFocus();
			wxArrayString cpts = split(params,",");
			if ((cpts[0] == "rgb") | (cpts[0] == "red") | (cpts[0] == "green") | (cpts[0] == "blue") | (cpts[0] == "tone")) 
				chan->SetStringSelection(cpts[0]);
			else
				chan->SetSelection(chan->FindString("rgb"));
			Bind(wxEVT_CHOICE, &CurvePanel::channelChanged, this);
			Bind(myCURVE_UPDATE, &CurvePanel::paramUpdated, this);
			Bind(myCURVE_CHANGE, &CurvePanel::paramChanged, this);
			Bind(wxEVT_CHECKBOX, &CurvePanel::onEnable, this, CURVEENABLE);
			Refresh();
		}


		~CurvePanel()
		{
			curve->~CurvePane();
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

		void paramUpdated(wxCommandEvent& event)
		{
			wxString ch = chan->GetString(chan->GetSelection());
			((PicProcessorCurve *) q)->setControlPoints(curve->getPoints());
			((PicProcessorCurve *) q)->setChannel(ch);
			q->setParams(ch+","+curve->getControlPoints());
			q->processPic();
			event.Skip();
		}

		void paramChanged(wxCommandEvent& event)
		{
			if (rateAdapt) {
				wxString ch = chan->GetString(chan->GetSelection());
				((PicProcessorCurve *) q)->setControlPoints(curve->getPoints());
				((PicProcessorCurve *) q)->setChannel(ch);
				q->setParams(ch+","+curve->getControlPoints());
				q->processPic();
			}
		}

		
		void channelChanged(wxCommandEvent& event)
		{
			wxString ch = chan->GetString(chan->GetSelection());
			((PicProcessorCurve *) q)->setControlPoints(curve->getPoints());
			((PicProcessorCurve *) q)->setChannel(ch);
			q->setParams(ch+","+curve->getControlPoints());
			q->processPic();
			event.Skip();
		}


	private:
		CurvePane *curve;
		wxChoice *chan;
		wxCheckBox *enablebox;

};

PicProcessorCurve::PicProcessorCurve(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	Curve crv;
	int ctstart;
	wxArrayString cpts = split(command,",");
	if ((cpts[0] == "rgb") | (cpts[0] == "red") | (cpts[0] == "green") | (cpts[0] == "blue") | (cpts[0] == "tone")) {
		setChannel(cpts[0]);
		ctstart = 1;
	}
	else {
		setChannel("rgb");
		ctstart = 0;
	}
	for (int i=ctstart; i<cpts.GetCount()-1; i+=2) {
		crv.insertpoint(atof(cpts[i]), atof(cpts[i+1]));
	}
	ctrlpts = crv.getControlPoints();
	//showParams();
}

void PicProcessorCurve::createPanel(wxSimplebook* parent)
{
	toolpanel = new CurvePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

void PicProcessorCurve::setControlPoints(std::vector<cp> ctpts)
{
	ctrlpts.clear();
	ctrlpts = ctpts;
}

void PicProcessorCurve::setChannel(wxString chan)
{
	if (chan == "rgb")   channel = CHANNEL_RGB;
	if (chan == "red")   channel = CHANNEL_RED;
	if (chan == "green") channel = CHANNEL_GREEN;
	if (chan == "blue")  channel = CHANNEL_BLUE;
	if (chan == "tone")  channel = CHANNEL_TONE;
	m_tree->SetItemText(id, wxString::Format("curve:%s",chan));
}

void PicProcessorCurve::setParams(std::vector<cp> ctpts, wxString params)
{
	PicProcessor::setParams(params);
	ctrlpts.clear();
	ctrlpts = ctpts;
}

bool PicProcessorCurve::processPic(bool processnext) {
	((wxFrame*) m_display->GetParent())->SetStatusText("curve...");
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.curve.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (processingenabled) {
		mark();
		dib->ApplyToneCurve(ctrlpts, channel, threadcount);
		float d = durationf();

		toolpanel->setRateAdapt(false);
		//parm tool.all.rateadapt: 0/1 Enable/disable rate adaptation, where mousemoves will process the image.  Default=0
		if (myConfig::getConfig().getValueOrDefault("tool.all.rateadapt","0") == "1")
			//parm tool.curve.rateadapt.threshold: Specify threshold to turn on rate adaptation, in seconds. Default=0.01
			if (d < atof(myConfig::getConfig().getValueOrDefault("tool.curve.rateadapt.threshold","0.01").c_str()))
				toolpanel->setRateAdapt(true);

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.curve.log","0") == "1"))
			log(wxString::Format("tool=curve,imagesize=%dx%d,threads=%d,time=%0.3f",dib->getWidth(), dib->getHeight(), threadcount, d));
	}

	dirty = false;
	
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();

	return result;
}


void PicProcessorCurve::displayProcessedPic() 
{
	if (m_display) {
		m_display->SetPic(dib, channel);
		m_display->SetDrawList(dcList);
	}
}



