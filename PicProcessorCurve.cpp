
#include "PicProcessor.h"
#include "PicProcessorCurve.h"
#include "PicProcPanel.h"
#include "CurvePane.h"
#include "util.h"

#include <wx/fileconf.h>
#include <wx/choice.h>

class CurvePanel: public PicProcPanel
{
	public:
		CurvePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxALL, 2); //.Expand();
			wxArrayString str;
			str.Add("rgb");
			str.Add("red");
			str.Add("green");
			str.Add("blue");
			chan = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, str);

			b->Add(chan, flags);
			curve = new CurvePane(this, params);
			b->Add(curve, flags);
			SetSizerAndFit(b);
			b->Layout();
			Refresh();
			Update();
			SetFocus();
			wxArrayString cpts = split(params,",");
			if ((cpts[0] == "rgb") | (cpts[0] == "red") | (cpts[0] == "green") | (cpts[0] == "blue")) 
				chan->SetStringSelection(cpts[0]);
			else
				chan->SetSelection(chan->FindString("rgb"));
			Bind(wxEVT_CHOICE, &CurvePanel::channelChanged, this);
			Bind(wxEVT_SCROLL_THUMBRELEASE, &CurvePanel::paramChanged, this);
		}

		~CurvePanel()
		{
			curve->~CurvePane();
		}


		void paramChanged(wxScrollEvent& event)
		{
			wxString ch = chan->GetString(chan->GetSelection());
			((PicProcessorCurve *) q)->setControlPoints(curve->getPoints());
			((PicProcessorCurve *) q)->setChannel(ch);
			q->setParams(ch+","+curve->getControlPoints());
			q->processPic();
			//event.Skip();
		}
		
		void channelChanged(wxCommandEvent& event)
		{
			wxString ch = chan->GetString(chan->GetSelection());
			((PicProcessorCurve *) q)->setControlPoints(curve->getPoints());
			((PicProcessorCurve *) q)->setChannel(ch);
			q->setParams(ch+","+curve->getControlPoints());
			q->processPic();
			//event.Skip();
		}


	private:
		CurvePane *curve;
		wxChoice *chan;

};

PicProcessorCurve::PicProcessorCurve(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	Curve crv;
	int ctstart;
	wxArrayString cpts = split(command,",");
	if ((cpts[0] == "rgb") | (cpts[0] == "red") | (cpts[0] == "green") | (cpts[0] == "blue")) {
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

void PicProcessorCurve::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new CurvePanel(m_parameters, this, c);
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
}

void PicProcessorCurve::setParams(std::vector<cp> ctpts, wxString params)
{
	PicProcessor::setParams(params);
	ctrlpts.clear();
	ctrlpts = ctpts;
}

bool PicProcessorCurve::processPic() {
	((wxFrame*) m_display->GetParent())->SetStatusText("curve...");
	bool result = true;

	int threadcount;
	wxConfigBase::Get()->Read("tool.curve.cores",&threadcount,0);
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyToneCurve(ctrlpts, channel, threadcount);
	wxString d = duration();

	if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.curve.log","0") == "1"))
		log(wxString::Format("tool=curve,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(), threadcount, d));

	dirty = false;
	
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	processNext();

	return result;
}



