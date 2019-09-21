#include "PicProcessor.h"
#include "PicProcessorRotate.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"
#include "undo.xpm"
#include "run.xpm"

#include <wx/treectrl.h>

#define ROTATEENABLE	7500
#define ROTATEAUTOCROP	7501
#define ROTATERESET 	7502

#define ROTATE45	7503
#define ROTATE90	7504
#define ROTATE180	7505
#define ROTATE270	7506

class rotateSlider: public wxControl
{
	public:
		rotateSlider(wxWindow *parent, wxWindowID id, int initialvalue, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize): wxControl(parent, id, pos, size, wxBORDER_NONE) 
		{
			SetBackgroundColour(parent->GetBackgroundColour());
			initval = initialvalue;

			wxBoxSizer *s = new wxBoxSizer(wxHORIZONTAL);
			rotate = new wxSlider(this, wxID_ANY, initialvalue*10.0, -450, 450, wxDefaultPosition, wxSize(200, -1));
			s->Add(rotate,  wxALIGN_LEFT | wxALL, 1);
			val = new wxStaticText(this,wxID_ANY, wxString::Format("%2.1f",initialvalue/10.0), wxDefaultPosition, wxSize(30, -1));
			s->Add(val, 0, wxALIGN_LEFT | wxALL, 1);
			btn = new wxBitmapButton(this, ROTATERESET, wxBitmap(undo_xpm), wxPoint(0,0), wxSize(-1,-1), wxBU_EXACTFIT);
			btn->SetToolTip("Reset to default");
			s->Add(btn, 0, wxALIGN_LEFT | wxALL, 1);
			
			SetSizerAndFit(s);
			
			Bind(wxEVT_SCROLL_CHANGED, &rotateSlider::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBTRACK, &rotateSlider::OnChanged, this);
			Bind(wxEVT_SCROLL_THUMBRELEASE, &rotateSlider::OnChanged, this);
		}
		
		void OnChanged(wxCommandEvent& event)
		{
			val->SetLabel(wxString::Format("%2.1f", rotate->GetValue()/10.0));
			event.Skip();
			Refresh();
			Update();
		}
		
		int GetValue()
		{
			return rotate->GetValue();
		}

		void SetValue(int v)
		{
			rotate->SetValue(v);
			val->SetLabel(wxString::Format("%2.1f", rotate->GetValue()/10.0));
			Refresh();
			Update();
		}
	
	
	private:
		wxSlider *rotate;
		wxStaticText *val;
		wxBitmapButton *btn;
		int initval;
	
};

class RotatePreview: public wxPanel
{
	public:
		RotatePreview(wxWindow *parent, wxImage image, double angle, bool autocrop, const wxSize &size=wxDefaultSize, const wxPoint &pos=wxDefaultPosition): wxPanel(parent, wxID_ANY, pos, size)
		{
			SetDoubleBuffered(true);
			haspect = (double) image.GetHeight() / (double) image.GetWidth();
			vaspect = (double) image.GetWidth() / (double) image.GetHeight();
			anglerad = -angle * 0.01745329;
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
			anglerad = -angle * 0.01745329;
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
			int w, h;
			GetSize(&w,&h);

			img.Destroy();

			if (haspect < vaspect) 
				img = orig.Scale(w, w* haspect);
			else 
				img = orig.Scale(h * vaspect, h);

			event.Skip();
			Refresh();
		}
		
		void OnPaint(wxPaintEvent& event)
		{
			wxImage i;
			int w, h, iw, ih, sw, sh;
			float dx, dy;
			GetSize(&w,&h);

			wxPaintDC dc(this);
			i = img.Rotate(anglerad, wxPoint(img.GetWidth()/2,img.GetHeight()/2), true);
			
			//if (((int) angledeg == 270) || ((int) angledeg == 90) || ((int) angledeg == 180)) return;
			
			if (haspect < vaspect) 
				i = i.Scale(w, w*haspect);
			else 
				i = i.Scale(h*vaspect, h);
			
			sw = i.GetWidth();
			sh = i.GetHeight();
			
			dc.SetDeviceOrigin((w-sw)/2, (h-sh)/2);
			
			wxBitmap * displayimg = new wxBitmap(i);
			wxMemoryDC mdc;
			mdc.SelectObject(*displayimg);
			dc.Blit(0,0,displayimg->GetWidth(), displayimg->GetHeight(), &mdc, 0, 0);
			mdc.SelectObject(wxNullBitmap);
			displayimg->~wxBitmap();

			//dc.DrawBitmap(wxBitmap(i),0,0);

			dc.SetPen(wxPen(*wxLIGHT_GREY, 1, wxPENSTYLE_SHORT_DASH));
			dc.DrawLine(0,sh*0.2,sw,sh*0.2);
			dc.DrawLine(0,sh*0.4,sw,sh*0.4);
			dc.DrawLine(0,sh*0.6,sw,sh*0.6);
			dc.DrawLine(0,sh*0.8,sw,sh*0.8);

			dc.DrawLine(sw*0.2,0,sw*0.2,sh);
			dc.DrawLine(sw*0.4,0,sw*0.4,sh);
			dc.DrawLine(sw*0.6,0,sw*0.6,sh);
			dc.DrawLine(sw*0.8,0,sw*0.8,sh);

			if (!autocrop) return;

			int x1, y1;
			if (angledeg < 0.0) {
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
			Refresh();
		}

	private:
		wxImage img, orig;
		double haspect, vaspect, aspect, anglerad, angledeg;
		bool autocrop, grid;
		bool orient; //true=horizontal, false=vertical
		cmsHTRANSFORM hTransform;

};



class RotatePanel: public PicProcPanel
{
	public:
		RotatePanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			bool acrop = false;
			SetDoubleBuffered(true);
			thumb = false;
			wxSize s = GetSize();

			wxBoxSizer *b = new wxBoxSizer(wxVERTICAL); 
			
			wxImage i = gImage2wxImage(proc->getPreviousPicProcessor()->getProcessedPic());
			
			wxArrayString tok = split(params, ",");
			double initialvalue = atof(tok[0].c_str());
			if (tok.GetCount() > 1)
				if (tok[1] == "autocrop")
					acrop = true;
				
			enablebox = new wxCheckBox(this, ROTATEENABLE, "rotate:");
			enablebox->SetValue(true);
			b->Add(enablebox, 0, wxALIGN_LEFT | wxALL, 3);
			b->Add(new wxStaticLine(this, wxID_ANY,  wxDefaultPosition, wxSize(280,2)), 0,  wxALIGN_LEFT | wxBOTTOM, 10);

			//preview = new RotatePreview(this,i,initialvalue, acrop);
			//b->Add(preview , 1, wxEXPAND | wxALIGN_CENTER_VERTICAL |wxALIGN_CENTER_HORIZONTAL | wxALL, 1);  // wxSHAPED |
			
			r90 = new wxRadioButton(this, ROTATE90, "Rotate 90", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			b->Add(r90 , 0, wxALIGN_LEFT | wxALL, 1);
			r180 = new wxRadioButton(this, ROTATE180, "Rotate 180");
			b->Add(r180 , 0, wxALIGN_LEFT | wxALL, 1);
			r270 = new wxRadioButton(this, ROTATE270, "Rotate 270");
			b->Add(r270 , 0, wxALIGN_LEFT | wxALL, 1);
			
			r45 = new wxRadioButton(this, ROTATE45, "Rotate < 45:");
			b->Add(r45 , 0, wxALIGN_LEFT | wxALL, 1);
			
			rotate = new rotateSlider(this, wxID_ANY, initialvalue);
			b->Add(rotate , 0, wxALIGN_LEFT | wxALL, 1);
			
			
			autocrop = new wxCheckBox(this, ROTATEAUTOCROP, "autocrop");
			b->Add(autocrop , 0, wxALIGN_LEFT | wxALL, 1);
			autocrop->SetValue(acrop);

			if (myConfig::getConfig().getValueOrDefault("display.cms","1") == "1") {
				hTransform = proc->getDisplay()->GetDisplayTransform();
				if (hTransform) {
					unsigned char* im = i.GetData();
					unsigned w = i.GetWidth();
					unsigned h = i.GetHeight();
					#pragma omp parallel for
					for (unsigned y=0; y<h; y++) {
						unsigned pos = y*w*3;
						cmsDoTransform(hTransform, &im[pos], &im[pos], w);
					}
				}
			}
				//cmsDoTransform(hTransform, i.GetData(), i.GetData(), i.GetWidth()*i.GetHeight());
			
			preview = new RotatePreview(this,i,initialvalue, acrop);
			b->Add(preview , 1, wxEXPAND | wxALIGN_CENTER_VERTICAL |wxALIGN_CENTER_HORIZONTAL | wxALL, 1);  // wxSHAPED |

			preview->setAutocrop(autocrop->GetValue());
			if ((int) initialvalue == 90) r90->SetValue(true);
			else if ((int) initialvalue == 180) r180->SetValue(true);
			else if ((int) initialvalue == 270) r270->SetValue(true);
			else r45->SetValue(true);
			
			rotateSelection();

			SetSizerAndFit(b);

			Refresh();
			Update();
			SetFocus();

			t.SetOwner(this);
			//Bind(wxEVT_SIZE,&RotatePanel::OnSize, this);
			rotate->Bind(wxEVT_BUTTON, &RotatePanel::OnButton, this);
			rotate->Bind(wxEVT_SCROLL_CHANGED, &RotatePanel::OnChanged, this);
			rotate->Bind(wxEVT_SCROLL_THUMBTRACK, &RotatePanel::OnThumbTrack, this);
			rotate->Bind(wxEVT_SCROLL_THUMBRELEASE, &RotatePanel::OnThumbRelease, this);
			Bind(wxEVT_CHECKBOX, &RotatePanel::OnThumbRelease, this, ROTATEAUTOCROP);
			Bind(wxEVT_CHECKBOX, &RotatePanel::onEnable, this, ROTATEENABLE);
			Bind(wxEVT_RADIOBUTTON, &RotatePanel::onRotateSelection, this);
			q->getCommandTree()->Bind(wxEVT_TREE_SEL_CHANGED, &RotatePanel::OnCommandtreeSelChanged, this);
			Bind(wxEVT_TIMER, &RotatePanel::OnTimer,  this);
		}

		~RotatePanel()
		{
			q->getCommandTree()-Unbind(wxEVT_TREE_SEL_CHANGED, &RotatePanel::OnCommandtreeSelChanged, this);
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
			wxSize s = GetSize();
			//SetSize(s);

			preview->SetSize(s);

			//g->RecalcSizes();
			//g->Layout();
			event.Skip();
			Refresh();

		}

		void OnTimer(wxTimerEvent& event)
		{
			if (autocrop->GetValue())
				q->setParams(wxString::Format("%2.1f,autocrop",rotate->GetValue()/10.0));
			else
				q->setParams(wxString::Format("%2.1f",rotate->GetValue()/10.0));
			q->processPic();
			DeletePendingEvents();
			t.Stop();
			event.Skip();
		}
		
		void rotateSelection()
		{
			double rotation;
			if (r45->GetValue()) {
				///rotate->Enable(true);
				///autocrop->Enable(true);
				///preview->Enable(true);
				
				rotate->Show(true);
				autocrop->Show(true);
				preview->Show(true);
				
				preview->setAutocrop(autocrop->GetValue());
				rotation = rotate->GetValue()/10.0;
				if (autocrop->GetValue())
					q->setParams(wxString::Format("%2.1f,autocrop",rotation));
				else
					q->setParams(wxString::Format("%2.1f",rotation));
			}
			else {
				//rotate->Enable(false);
				//autocrop->Enable(false);
				//preview->Enable(false);
				
				rotate->Show(false);
				autocrop->Show(false);
				preview->Show(false);
				
				preview->setAutocrop(false);
				if (r90->GetValue())  {q->setParams(wxString::Format("%2.1f",90.0));  rotation = 90.0;}
				if (r180->GetValue()) {q->setParams(wxString::Format("%2.1f",180.0)); rotation = 180.0;}
				if (r270->GetValue()) {q->setParams(wxString::Format("%2.1f",270.0)); rotation = 270.0;}
			}
			preview->Rotate(rotation);
			Refresh();
			q->processPic();
		}

		void onRotateSelection(wxCommandEvent& event)
		{
			rotateSelection();
			event.Skip();
		}

		void OnChanged(wxCommandEvent& event)
		{
			preview->setAutocrop(autocrop->GetValue());
			preview->Rotate(rotate->GetValue()/10.0);
			if (thumb) {
				thumb = false;
			}
			else {
				t.Start(500,wxTIMER_ONE_SHOT);
				Refresh();
				Update();
			}
		}


		void OnThumbTrack(wxCommandEvent& event)
		{
			thumb = true;
			preview->Rotate(rotate->GetValue()/10.0);
			event.Skip();
			Refresh();
			Update();
		}

		void OnThumbRelease(wxCommandEvent& event)
		{
			preview->setAutocrop(autocrop->GetValue());
			if (autocrop->GetValue())
				q->setParams(wxString::Format("%2.1f,autocrop",rotate->GetValue()/10.0));
			else
				q->setParams(wxString::Format("%2.1f",rotate->GetValue()/10.0));
			event.Skip();
			q->processPic();
			thumb = true;
		}

		void OnButton(wxCommandEvent& event)
		{
			double resetval = atof(myConfig::getConfig().getValueOrDefault("tool.rotate.initialvalue","0.0").c_str());
			rotate->SetValue(resetval);
			if (autocrop->GetValue())
				q->setParams(wxString::Format("%2.1f,autocrop",rotate->GetValue()/10.0));
			else
				q->setParams(wxString::Format("%2.1f",rotate->GetValue()/10.0));
			preview->Rotate(0.0);
			Refresh();
			q->processPic();
			event.Skip();
		}

		
	private:
		wxCheckBox *autocrop, *enablebox;
		wxRadioButton *r45, *r90, *r180, *r270;
		rotateSlider *rotate;
		wxTimer t;
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

bool PicProcessorRotate::processPic(bool processnext) 
{
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
	if (!global_processing_enabled) return true;

	if (processingenabled & angle != 0.0) {
		mark();
		if ((int) angle == 270) dib->ApplyRotate270(threadcount);
		else if ((int) angle == 180) dib->ApplyRotate180(threadcount);
		else if ((int) angle == 90) dib->ApplyRotate90(threadcount);
		else dib->ApplyRotate(angle, autocrop, threadcount);
		m_display->SetModified(true);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.rotate.log","0") == "1"))
			log(wxString::Format("tool=rotate,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;
		
	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



