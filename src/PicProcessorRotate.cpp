#include "PicProcessor.h"
#include "PicProcessorRotate.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
#include "undo.xpm"
#include "run.xpm"

#include <wx/treectrl.h>
#include "myFloatSlider.h"


class RotatePreview: public wxPanel
{
	public:
		RotatePreview(wxWindow *parent, wxImage image, double angle, bool autocrop, const wxSize &size=wxDefaultSize, const wxPoint &pos=wxDefaultPosition): wxPanel(parent, wxID_ANY, pos, size)
		{
			SetDoubleBuffered(true);
			haspect = (double) image.GetHeight() / (double) image.GetWidth();
			vaspect = (double) image.GetWidth() / (double) image.GetHeight();
			anglerad = angle * 0.01745329;
			angledeg = angle;
			orig = image;
			this->autocrop = autocrop;

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
			angledeg = angle;
		}

		void SetPic(wxImage image) 
		{
			wxSize size = GetSize();
			orig = image;

			haspect = (double) image.GetHeight() / (double) image.GetWidth();
			vaspect = (double) image.GetWidth() / (double) image.GetHeight();

			if (haspect < vaspect) {
				img = image.Scale(size.GetWidth(), size.GetWidth()* haspect);
				aspect = haspect;
			}
			else {
				img = image.Scale(size.GetHeight() * vaspect, size.GetHeight());
				aspect = vaspect;
			}

			//hTransform = q->getDisplay()->GetDisplayTransform();
			//if (hTransform)
			//	cmsDoTransform(hTransform, img.GetData(), img.GetData(), iw*ih);

			Refresh();
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

			if (w>0 & h>0) img = orig.Scale(w, h);
			event.Skip();
			Refresh();
		}
		
		void OnPaint(wxPaintEvent& event)
		{
			wxImage i;
			int w, h, iw, ih, sw, sh;
			float dx, dy;
			GetSize(&w,&h);
			if (haspect < vaspect) {
				i = img.Rotate(anglerad, wxPoint(img.GetWidth()/2,img.GetHeight()/2), true);
				i = i.Scale(w, w*haspect);
			}
			else {
				i = img.Rotate(anglerad, wxPoint(img.GetWidth()/2,img.GetHeight()/2), true);
				i = i.Scale(h*vaspect, h);
			}
			sw = i.GetWidth();
			sh = i.GetHeight();
			wxPaintDC dc(this);
			dc.DrawBitmap(wxBitmap(i),0,0);
 

			dc.SetPen(wxPen(*wxLIGHT_GREY, 1, wxPENSTYLE_SHORT_DASH));
			dc.DrawLine(0,h*0.2,w,h*0.2);
			dc.DrawLine(0,h*0.4,w,h*0.4);
			dc.DrawLine(0,h*0.6,w,h*0.6);
			dc.DrawLine(0,h*0.8,w,h*0.8);

			dc.DrawLine(w*0.2,0,w*0.2,h);
			dc.DrawLine(w*0.4,0,w*0.4,h);
			dc.DrawLine(w*0.6,0,w*0.6,h);
			dc.DrawLine(w*0.8,0,w*0.8,h);

			if (!autocrop) return;

			int x1, y1;
			if (angledeg > 0.0) {
				for (x1=sw-1; x1>0; x1--) if (i.GetRed(x1,2) > 0) break;
				for (y1=0; y1<sh; y1++) if (i.GetRed(2,y1) > 0) break;
				if (x1 > sw/2 & y1 < sh/2) {
					dc.SetPen(wxPen(*wxYELLOW, 1, wxPENSTYLE_SOLID));
					dc.DrawLine(0, y1, sw, y1);
					dc.DrawLine(0, sh-y1, sw, sh-y1);
					dc.DrawLine(x1, 0, x1, sh);
					dc.DrawLine(sw-x1, 0, sw-x1, sh);
				}
			}
			else {
				for (x1=0; x1<sw-1; x1++) if (i.GetRed(x1,2) > 0) break;
				for (y1=sh-1; y1 > 0; y1--) if (i.GetRed(2,y1) > 0) break;
				if (x1 < sw/2 & y1 > sh/2) {
					dc.SetPen(wxPen(*wxYELLOW, 1, wxPENSTYLE_SOLID));
					dc.DrawLine(0, y1, sw, y1);
					dc.DrawLine(0, sh-y1, sw, sh-y1);
					dc.DrawLine(x1, 0, x1, sh);
					dc.DrawLine(sw-x1, 0, sw-x1, sh);
				}
			}
		}
		
		void setAutocrop(bool a)
		{
			autocrop = a;
		}

	private:
		wxImage img, orig;
		double haspect, vaspect, aspect, anglerad, angledeg;
		bool autocrop, grid;
		cmsHTRANSFORM hTransform;

};

class RotatePanel: public PicProcPanel
{
	public:
		RotatePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			bool acrop = false;
			SetDoubleBuffered(true);
			wxSize s = GetSize();
			//SetSize(s);
			wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			thumb = false;
			
			wxArrayString tok = split(params, ",");
			double initialvalue = atof(tok[0].c_str());
			if (tok.GetCount() > 1)
				if (tok[1] == "autocrop")
					acrop = true;

//with gridbagsizer:
			//g->Add(0,10, wxGBPosition(0,0));
			g->Add(new wxStaticText(this,wxID_ANY, "rotate: "), wxGBPosition(0,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			rotate = new wxSlider(this, wxID_ANY, initialvalue*10.0, -450, 450, wxPoint(10, 30), wxSize(140, -1));
			//rotate = new myFloatSlider(this, wxID_ANY, 0.0, -45.0, 45.0, 0.1, wxDefaultPosition, wxSize(100, 20));
			g->Add(rotate , wxGBPosition(0,1), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			val = new wxStaticText(this,wxID_ANY, tok[0], wxDefaultPosition, wxSize(30, -1));
			g->Add(val , wxGBPosition(0,2), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			btn1 = new wxBitmapButton(this, 8000, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn1->SetToolTip("Reset to default");
			g->Add(btn1, wxGBPosition(0,3), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			//btn2 = new wxBitmapButton(this, 9000, wxBitmap(run_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			//btn2->SetToolTip("Apply rotation");
			//g->Add(btn2, wxGBPosition(0,4), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			//g->Add(0,10, wxGBPosition(0,5), wxDefaultSpan, wxEXPAND | wxALIGN_LEFT | wxALL, 1);
			
			autocrop = new wxCheckBox(this, wxID_ANY, "autocrop");
			g->Add(autocrop , wxGBPosition(1,0), wxDefaultSpan, wxALIGN_LEFT | wxALL, 1);
			autocrop->SetValue(acrop);

			wxImage i = gImage2wxImage(proc->getPreviousPicProcessor()->getProcessedPic());
			int pw = s.GetWidth();
			int ph = pw * ((double)s.GetHeight()/(double)pw);

			hTransform = proc->getDisplay()->GetDisplayTransform();
			if (hTransform)
				cmsDoTransform(hTransform, i.GetData(), i.GetData(), i.GetWidth()*i.GetHeight());

			preview = new RotatePreview(this,i,initialvalue, acrop, wxSize(pw, ph));
			g->Add(preview , wxGBPosition(2,0), wxGBSpan(1,5), wxEXPAND | wxSHAPED | wxALIGN_LEFT |wxALIGN_TOP | wxALL, 1);

			preview->setAutocrop(autocrop->GetValue());
			
			SetSizerAndFit(g);
			g->Layout();

			Refresh();
			Update();
			SetFocus();
			t = new wxTimer(this);
			Bind(wxEVT_SIZE,&RotatePanel::OnSize, this);
			Bind(wxEVT_BUTTON, &RotatePanel::OnButton, this);
			//Bind(wxEVT_SCROLL_CHANGED, &RotatePanel::OnChanged, this);
			Bind(wxEVT_MOUSEWHEEL, &RotatePanel::OnWheel, this);
			Bind(wxEVT_SCROLL_CHANGED, &RotatePanel::OnThumbTrack, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &RotatePanel::OnThumbTrack, this);
			Bind(wxEVT_SCROLL_THUMBRELEASE, &RotatePanel::OnThumbRelease, this);
			Bind(wxEVT_CHECKBOX, &RotatePanel::OnChanged, this);
			q->getCommandTree()->Bind(wxEVT_TREE_SEL_CHANGED, &RotatePanel::OnCommandtreeSelChanged, this);
			Bind(wxEVT_TIMER, &RotatePanel::OnTimer,  this);
		}

		~RotatePanel()
		{
			t->~wxTimer();
			//q->getCommandTree()-Unbind(wxEVT_TREE_SEL_CHANGED, &RotatePanel::OnCommandtreeSelChanged, this);
		}

		void OnCommandtreeSelChanged(wxTreeEvent& event)
		{
			event.Skip();
			//preview->SetPic(gImage2wxImage(q->getPreviousPicProcessor()->getProcessedPic()));
			wxImage i = gImage2wxImage(q->getPreviousPicProcessor()->getProcessedPic());
			//if (hTransform)
			//	cmsDoTransform(hTransform, i.GetData(), i.GetData(), i.GetWidth()*i.GetHeight());
			preview->SetPic(i);
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

		void OnWheel(wxMouseEvent &event)
		{
			t->Start(500,wxTIMER_ONE_SHOT);
			event.Skip();
		}

		void OnChanged(wxCommandEvent& event)
		{
			//if (!thumb) {
				val->SetLabel(wxString::Format("%2.1f", rotate->GetValue()/10.0));
				preview->setAutocrop(autocrop->GetValue());
				preview->Rotate(rotate->GetValue()/10.0);
				t->Start(500,wxTIMER_ONE_SHOT);
				Refresh();
				//Update();
				//q->processPic();
			//}
		}

		void OnThumbTrack(wxCommandEvent& event)
		{
			thumb = true;
			val->SetLabel(wxString::Format("%2.1f", rotate->GetValue()/10.0));
			preview->Rotate(rotate->GetValue()/10.0);
			preview->Refresh();
			t->Start(500,wxTIMER_ONE_SHOT);
			//Update();
		}

		void OnThumbRelease(wxCommandEvent& event)
		{
			if (autocrop->GetValue())
				q->setParams(wxString::Format("%2.1f,autocrop",rotate->GetValue()/10.0));
			else
				q->setParams(wxString::Format("%2.1f",rotate->GetValue()/10.0));
			t->Start(500,wxTIMER_ONE_SHOT);
			//q->processPic();
			thumb = false;
		}

		void OnTimer(wxTimerEvent& event)
		{
			if (autocrop->GetValue())
				q->setParams(wxString::Format("%2.1f,autocrop",rotate->GetValue()/10.0));
			else
				q->setParams(wxString::Format("%2.1f",rotate->GetValue()/10.0));
			q->processPic();
			DeletePendingEvents();
			t->Stop();
			event.Skip();
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval;
			switch(event.GetId()) {
				case 8000:
					resetval = atof(myConfig::getConfig().getValueOrDefault("tool.rotate.initialvalue","0.0").c_str());
					rotate->SetValue(resetval);
					if (autocrop->GetValue())
						q->setParams(wxString::Format("%2.1f,autocrop",rotate->GetValue()/10.0));
					else
						q->setParams(wxString::Format("%2.1f",rotate->GetValue()/10.0));
					val->SetLabel(wxString::Format("%2.1f", resetval));
					preview->Rotate(0.0);
					break;
				case 9000:
					if (autocrop->GetValue())
						q->setParams(wxString::Format("%2.1f,autocrop",rotate->GetValue()/10.0));
					else
						q->setParams(wxString::Format("%2.1f",rotate->GetValue()/10.0));
					break;
			}
			Refresh();
			q->processPic();
			event.Skip();
		}


	private:
		wxCheckBox *autocrop;
		wxSlider *rotate;
		//myFloatSlider *rotate;
		wxStaticText *val;
		wxBitmapButton *btn1;
		wxButton *btn2;
		wxTimer *t;
		RotatePreview *preview;
		bool thumb;
		cmsHTRANSFORM hTransform;

};


PicProcessorRotate::PicProcessorRotate(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

void PicProcessorRotate::createPanel(wxSimplebook* parent)
{
	toolpanel = new RotatePanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorRotate::processPic(bool processnext) {
	((wxFrame*) m_display->GetParent())->SetStatusText("rotate...");
	bool autocrop = false;
	
	wxArrayString t = split(c, ",");
	
	double angle = atof(t[0].c_str());
	if (t.GetCount() > 1)
		if (t[1] == "autocrop")
			autocrop = true;
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.rotate.cores","0").c_str());
	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	if (angle != 0.0) {
		mark();
		dib->ApplyRotate(-angle, autocrop, threadcount);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.rotate.log","0") == "1"))
			log(wxString::Format("tool=rotate,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;
		
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



