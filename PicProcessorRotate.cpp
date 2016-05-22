#include "PicProcessor.h"
#include "PicProcessorRotate.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include "util.h"
#include "undo.xpm"
#include "run1.xpm"

#include <wx/fileconf.h>


class RotatePreview: public wxPanel
{
	public:
		RotatePreview(wxPanel *parent, wxImage image, const wxSize &size=wxDefaultSize): wxPanel(parent, wxID_ANY, wxDefaultPosition, size)
		{
			aspect = (double) image.GetHeight() / (double) image.GetWidth();
			anglerad = 0;
			img = image.Scale(size.GetWidth(), size.GetWidth()* aspect);
			Bind(wxEVT_PAINT,&RotatePreview::OnPaint, this);
			Bind(wxEVT_SIZE,&RotatePreview::OnSize, this);
			Refresh();
			Update();
		}

		void Rotate(double angle)
		{
			anglerad = angle * 0.01745329;
		}

		void OnSize(wxSizeEvent& event) 
		{
			 SetSize(event.GetSize());
		}

		void OnPaint(wxPaintEvent& event)
		{
			int w, h, iw, ih;
			GetSize(&w,&h);
			wxImage i = img.Rotate(anglerad, wxPoint(img.GetWidth()/2,img.GetHeight()/2), true).Scale(w,w*aspect);
			iw = i.GetWidth();
			ih = i.GetHeight();
			wxPaintDC dc(this);
			dc.DrawBitmap(wxBitmap(i),0,0);
			dc.SetPen(wxPen(*wxYELLOW, 1, wxPENSTYLE_SHORT_DASH));
			dc.DrawLine(0,ih*0.2,iw,ih*0.2);
			dc.DrawLine(0,ih*0.4,iw,ih*0.4);
			dc.DrawLine(0,ih*0.6,iw,ih*0.6);
			dc.DrawLine(0,ih*0.8,iw,ih*0.8);
		}

	private:
		wxImage img;
		double aspect, anglerad;

};

class RotatePanel: public PicProcPanel
{
	public:
		RotatePanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxSize s = parent->GetSize();
			SetSize(s);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			thumb = false;

			double initialvalue = atof(params.c_str());

			g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "rotate: "), wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			rotate = new wxSlider(this, wxID_ANY, initialvalue*10.0, -450, 450, wxPoint(10, 30), wxSize(140, -1));
			g->Add(rotate , wxGBPosition(1,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			val = new wxStaticText(this,wxID_ANY, params, wxDefaultPosition, wxSize(30, -1));
			g->Add(val , wxGBPosition(1,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn1 = new wxBitmapButton(this, 8000, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn1->SetToolTip("Reset to default");
			g->Add(btn1, wxGBPosition(1,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			btn2 = new wxBitmapButton(this, 9000, wxBitmap(run_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn2->SetToolTip("Apply rotation");
			g->Add(btn2, wxGBPosition(1,4), wxDefaultSpan, wxALIGN_LEFT | wxALL, 3);
			wxImage i = ThreadedFreeImage2wxImage(proc->getPreviousPicProcessor()->getProcessedPic());
			int pw = s.GetWidth();
			int ph = pw * (s.GetHeight()/pw);
			preview = new RotatePreview(this,i,wxSize(pw, ph));
			g->Add(preview , wxGBPosition(2,0), wxGBSpan(1,5), wxALIGN_LEFT | wxALL, 3);

			SetSizerAndFit(g);
			g->Layout();
			Refresh();
			Update();
			SetFocus();
			//t = new wxTimer(this);
			Bind(wxEVT_BUTTON, &RotatePanel::OnButton, this);
			Bind(wxEVT_SCROLL_CHANGED, &RotatePanel::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &RotatePanel::OnThumbTrack, this);
			Bind(wxEVT_SCROLL_THUMBRELEASE, &RotatePanel::OnThumbRelease, this);
			//Bind(wxEVT_TIMER, &RotatePanel::OnTimer,  this);
		}

		~RotatePanel()
		{
			//t->~wxTimer();
		}

		void OnChanged(wxCommandEvent& event)
		{
			//if (!thumb) {
				val->SetLabel(wxString::Format("%2.1f", rotate->GetValue()/10.0));
				preview->Rotate(rotate->GetValue()/10.0);
				//t->Start(500,wxTIMER_ONE_SHOT);
				Refresh();
				Update();
			//}
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			thumb = true;
			val->SetLabel(wxString::Format("%2.1f", rotate->GetValue()/10.0));
			preview->Rotate(rotate->GetValue()/10.0);
			Refresh();
			Update();
		}

		void OnThumbRelease(wxCommandEvent& event)
		{
			q->setParams(wxString::Format("%2.1f",rotate->GetValue()/10.0));
			//q->processPic();
			thumb = false;
		}

//		void OnTimer(wxTimerEvent& event)
//		{
//			q->setParams(wxString::Format("%2.1f",rotate->GetValue()/10.0));
//			q->processPic();
//			DeletePendingEvents();
//			t->Stop();
//			event.Skip();
//		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval;
			switch(event.GetId()) {
				case 8000:
					wxConfigBase::Get()->Read("tool.rotate.initialvalue",&resetval,0);
					rotate->SetValue(resetval);
					q->setParams(wxString::Format("%2.1f",resetval));
					val->SetLabel(wxString::Format("%2.1f", resetval));
					preview->Rotate(0.0);
					break;
				case 9000:
					q->setParams(wxString::Format("%2.1f",rotate->GetValue()/10.0));
					break;
			}
			Refresh();
			q->processPic();
			event.Skip();
		}


	private:
		wxSlider *rotate;
		wxStaticText *val;
		wxBitmapButton *btn1;
		wxButton *btn2;
		//wxTimer *t;
		RotatePreview *preview;
		bool thumb;
};


PicProcessorRotate::PicProcessorRotate(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	//p->DestroyChildren();
	//r = new RotatePanel(p,this,c);
	showParams();
}

void PicProcessorRotate::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new RotatePanel(m_parameters, this, c);
}


bool PicProcessorRotate::processPic() {
	((wxFrame*) m_parameters->GetParent())->SetStatusText("rotate...");
	double angle = atof(c.c_str());
	bool result = true;

	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Rotate(getPreviousPicProcessor()->getProcessedPic(), angle);
	dirty = false;

	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}

	((wxFrame*) m_parameters->GetParent())->SetStatusText("");
	return result;
}



