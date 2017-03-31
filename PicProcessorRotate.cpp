#include "PicProcessor.h"
#include "PicProcessorRotate.h"
#include "PicProcPanel.h"
#include "util.h"
#include "undo.xpm"
#include "run.xpm"

#include <wx/fileconf.h>


class RotatePreview: public wxPanel
{
	public:
		RotatePreview(wxPanel *parent, wxImage image, double angle, const wxSize &size=wxDefaultSize, const wxPoint &pos=wxDefaultPosition): wxPanel(parent, wxID_ANY, pos, size)
		{
			SetDoubleBuffered(true);
			haspect = (double) image.GetHeight() / (double) image.GetWidth();
			vaspect = (double) image.GetWidth() / (double) image.GetHeight();
			anglerad = angle * 0.01745329;
			orig = image;

			if (haspect < vaspect) {
				img = image.Scale(size.GetWidth(), size.GetWidth()* haspect);
				aspect = haspect;
			}
			else {
				img = image.Scale(size.GetHeight() * vaspect, size.GetHeight());
				aspect = vaspect;
			}

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
			wxSize size = GetParent()->GetParent()->GetSize();
			SetSize(size.GetWidth(), size.GetWidth() * aspect);
			int w, h;
			GetSize(&w,&h);

			img.Destroy();

			//if (haspect < vaspect) 
			//	img = orig.Scale(w, w* haspect);
			//else 
			//	img = orig.Scale(h * vaspect, h);

			img = orig.Scale(w, h);
			event.Skip();
			Refresh();
		}

		void OnPaint(wxPaintEvent& event)
		{
			wxImage i;
			int w, h, iw, ih;
			GetSize(&w,&h);
			if (haspect < vaspect)
				i = img.Rotate(anglerad, wxPoint(img.GetWidth()/2,img.GetHeight()/2), true).Scale(w, w*haspect);
			else
				i = img.Rotate(anglerad, wxPoint(img.GetWidth()/2,img.GetHeight()/2), true).Scale(h*vaspect, h);
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
		wxImage img, orig;
		double haspect, vaspect, aspect, anglerad;

};

class RotatePanel: public PicProcPanel
{
	public:
		RotatePanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetDoubleBuffered(true);
			wxSize s = GetSize();
			//SetSize(s);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			thumb = false;
			double initialvalue = atof(params.c_str());

//with gridbagsizer:
			//g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "rotate: "), wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			rotate = new wxSlider(this, wxID_ANY, initialvalue*10.0, -450, 450, wxPoint(10, 30), wxSize(140, -1));
			g->Add(rotate , wxGBPosition(0,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			val = new wxStaticText(this,wxID_ANY, params, wxDefaultPosition, wxSize(30, -1));
			g->Add(val , wxGBPosition(0,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			btn1 = new wxBitmapButton(this, 8000, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn1->SetToolTip("Reset to default");
			g->Add(btn1, wxGBPosition(0,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			btn2 = new wxBitmapButton(this, 9000, wxBitmap(run_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn2->SetToolTip("Apply rotation");
			g->Add(btn2, wxGBPosition(0,4), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			//g->Add(0,10, wxGBPosition(0,5), wxDefaultSpan, wxEXPAND | wxALIGN_LEFT | wxALL, 1);

			wxImage i = gImage2wxImage(proc->getPreviousPicProcessor()->getProcessedPic());
			int pw = s.GetWidth();
			int ph = pw * ((double)s.GetHeight()/(double)pw);

			hTransform = proc->getDisplay()->GetDisplayTransform();
			if (hTransform)
				cmsDoTransform(hTransform, i.GetData(), i.GetData(), i.GetWidth()*i.GetHeight());

			preview = new RotatePreview(this,i,initialvalue, wxSize(pw, ph));
			g->Add(preview , wxGBPosition(2,0), wxGBSpan(1,5), wxEXPAND | wxSHAPED | wxALIGN_LEFT |wxALIGN_TOP | wxALL, 1);

			SetSizerAndFit(g);
			g->Layout();

			Refresh();
			Update();
			SetFocus();
			//t = new wxTimer(this);
			Bind(wxEVT_SIZE,&RotatePanel::OnSize, this);
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

		void OnSize(wxSizeEvent& event) 
		{
			wxSize s = GetParent()->GetSize();
			SetSize(s);

			preview->SetSize(g->GetCellSize(2,0));

			//g->RecalcSizes();
			//g->Layout();
			event.Skip();
			Refresh();

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
					wxConfigBase::Get()->Read("tool.rotate.initialvalue",&resetval,0.0);
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
		cmsHTRANSFORM hTransform;

};


PicProcessorRotate::PicProcessorRotate(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

void PicProcessorRotate::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new RotatePanel(m_parameters, this, c);
}


bool PicProcessorRotate::processPic() {
	((wxFrame*) m_display->GetParent())->SetStatusText("rotate...");
	double angle = atof(c.c_str());
	bool result = true;

	int threadcount;
	wxConfigBase::Get()->Read("tool.rotate.cores",&threadcount,0);
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);


	mark();
	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	dib->ApplyRotate(-angle, false, threadcount);
	dirty = false;
	wxString d = duration();

	if ((wxConfigBase::Get()->Read("tool.all.log","0") == "1") || (wxConfigBase::Get()->Read("tool.rotate.log","0") == "1"))
		log(wxString::Format("tool=rotate,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));


	//put in every processPic()...
	if (m_tree->GetItemState(GetId()) == 1) m_display->SetPic(dib);
	wxTreeItemId next = m_tree->GetNextSibling(GetId());
	if (next.IsOk()) {
		PicProcessor * nextitem = (PicProcessor *) m_tree->GetItemData(next);
		nextitem->processPic();
	}

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	return result;
}



