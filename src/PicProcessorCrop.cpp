#include "PicProcessor.h"
#include "PicProcessorCrop.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "myRowSizer.h"
#include "util.h"
#include "copy.xpm"
#include "paste.xpm"

#define CROPENABLE 9500
#define CROPCOPY 9501
#define CROPPASTE 9502

wxDECLARE_EVENT(myCROP_UPDATE, wxCommandEvent);
wxDEFINE_EVENT(myCROP_UPDATE, wxCommandEvent);

class CropPane: public wxPanel
{
	public:
		CropPane(wxWindow *parent,  wxWindowID id, wxImage image, wxString params, const wxPoint &pos=wxDefaultPosition,  const wxSize &size=wxDefaultSize): wxPanel(parent, id, pos, size)
		{
			SetDoubleBuffered(true);
			img = new wxBitmap(image);
			SetSize(image.GetWidth(), image.GetHeight());

			cropmode = 1; //0=freeform; 1=maintain aspect
			//parm tool.crop.controlpointradius: Radius of the rectangle displayed to indicate a control point.  Default=7
			cpradius = atoi(myConfig::getConfig().getValueOrDefault("tool.crop.controlpointradius","7").c_str());
			//parm tool.crop.landingradius: radius of control point area sensitive to mouseclicks.  Doesn't have to be the radius of the control point rectangle.  Default=7
			landingradius = atoi(myConfig::getConfig().getValueOrDefault("tool.crop.landingradius","7").c_str());
			isaspect = true;

			int indent = atoi(myConfig::getConfig().getValueOrDefault("tool.crop.initialindent","0").c_str());

			wxArrayString p = split(params,",");
			left = atof(p[0]);
			top = atof(p[1]);
			right = atof(p[2]);
			bottom = atof(p[3]);

			GetSize(&ww, &wh);
			iw = img->GetWidth();
			ih = img->GetHeight();
			wa = (double) ww/ (double) iw;
			ha = (double) wh/ (double) ih;
			aspect = wa > ha? ha : wa;

			
			Bind(wxEVT_PAINT,&CropPane::OnPaint, this);
			//Bind(wxEVT_SIZE,&CropPane::OnSize, this);
			Bind(wxEVT_LEFT_DOWN, &CropPane::OnMouseDown, this);
			Bind(wxEVT_MOTION , &CropPane::OnMouseMove, this);
			Bind(wxEVT_LEFT_UP, &CropPane::OnMouseUp, this);
			Bind(wxEVT_LEAVE_WINDOW, &CropPane::OnMouseUp, this);
		}
		
		void OnPaint(wxPaintEvent& event)
		{
			wxPaintDC dc(this);
			render(dc);
		}

		void PaintNow()
		{
			wxClientDC dc(this);
			render(dc);
		}
		
		void render (wxDC &dc)
		{
			int w, h;
			GetSize(&w, &h);
			
			wxMemoryDC mdc;
			mdc.SelectObject(*img);
			dc.StretchBlit(0,0, w, h, &mdc, 0, 0, img->GetWidth(), img->GetHeight());
			mdc.SelectObject(wxNullBitmap);

			if (isaspect)
				dc.SetPen(*wxYELLOW_PEN);
			else
				dc.SetPen(*wxRED_PEN);
			dc.SetBrush(*wxYELLOW_BRUSH);

			dc.DrawLine(left*w, top*h, right*w-1, top*h);
			dc.DrawLine(right*w-1, top*h, right*w-1, bottom*h-1);
			dc.DrawLine(right*w-1, bottom*h-1, left*w, bottom*h-1);
			dc.DrawLine(left*w, bottom*h-1, left*w, top*h);
			dc.SetBrush(*wxYELLOW_BRUSH);
			dc.SetPen(*wxYELLOW_PEN);
			dc.DrawRectangle(left*w+1, top*h+1,cpradius*2,cpradius*2);
			dc.SetBrush(*wxRED_BRUSH);
			dc.SetPen(*wxRED_PEN);
			dc.DrawRectangle((right*w-cpradius*2)-1, (bottom*h-cpradius*2),cpradius*2,cpradius*2);
		}

		wxString getParams()
		{
			return wxString::Format("%f,%f,%f,%f",left,top,right,bottom);
		}

		void setParams(wxString params)
		{
			wxArrayString p = split(params,",");
			left = atof(p[0]);
			top = atof(p[1]);
			right = atof(p[2]);
			bottom = atof(p[3]);
			Refresh();
		}


		void OnMouseDown(wxMouseEvent& event)
		{
			mousex = event.m_x;
			mousey = event.m_y;

			int w, h;
			GetSize(&w, &h);

			int radius = atoi(myConfig::getConfig().getValueOrDefault("tool.crop.landingradius","7").c_str());

			if ((mousex > left*w) & (mousex < left*w+radius*2)) {
				if ((mousey > top*h) & (mousey < top*h+landingradius*2)) {
					node = 1;  //top left
					cropmode = 1;  //preserve aspect
					isaspect = true;
				}
			}
			else if ((mousex > right*w-landingradius*2) & (mousex < right*w)) {
				if ((mousey > bottom*h-landingradius*2) & (mousey < bottom*h)) {
					node = 2; //bottom right
					cropmode = 0;  //free-form
					isaspect = false;
				}
			}
			else if ((mousex > left*w) * (mousex < right*w)) {
				if ((mousey > top*h) & (mousey < bottom*h)) {
					node = 3; //move
				}
			}
			mousemoved = false;
			SetFocus();
			Refresh();
			Update();
			event.Skip();
		}

		void OnMouseMove(wxMouseEvent& event)
		{

			int w, h;
			GetSize(&w, &h);

			bool anchorx;
			if (node == 0) {
				event.Skip();
				return;
			}

			int dx = mousex-event.m_x;
			int dy = mousey-event.m_y;
			float fdx = (float) dx / (float) w;
			float fdy = (float) dy / (float) h;

			if (cropmode == 0) {  //free form
				isaspect = false;
				switch (node) {
					case 1:  //top left
						if (left - fdx < right) left -= fdx;
						if (top - fdy < bottom) top  -= fdy;
						if (left < 0.0) {
							//bottom -= left;
							left = 0.0;
						}
						if (top < 0.0) {
							//right -= top;
							top = 0.0;
						}
						break;
					case 2:  //bottom right
						if (right - fdx > left) right -= fdx;
						if (bottom - fdy > top) bottom  -= fdy;
						if (right > 1.0) right = 1.0;
						if (bottom > 1.0) bottom = 1.0;
						break;
					case 3:  //move
						float dx = right - left;
						float dy = bottom - top;
						if (left - fdx < 0.0) {
							right = dx; 
							left = 0.0;
						}
						else if (right - fdx > 1.0) {
							left = 1.0 - dx;
							right = 1.0;
						}
						else {
							left -= fdx;
							right -= fdx;
						}
						if (top - fdy < 0.0) {
							bottom = dy;
							top = 0.0;
						}
						else if (bottom - fdy > 1.0) {
							top = 1.0 - dy;
							bottom = 1.0;
						}
						else {
							top -= fdy;
							bottom -= fdy;
						}
						break;
				}
			}
			else if (cropmode == 1) {  //preserve aspect
				if (abs(dx) > abs(dy))
					anchorx = true;  //x
				else 
					anchorx = false;  //y

				switch (node) {
					case 1:
						isaspect = true;
						if (anchorx) {
							if (left - fdx < right) left -= fdx;
							if (left < 0.0) {
								//bottom -= left;
								left = 0.0;
							}
							float width = right-left;
							bottom = top + width;
						}
						else {
							if (top - fdy < bottom) top -= fdy;
							if (top < 0.0) {
								//right -= top;
								top = 0.0;
							}
							float height = bottom-top;
							if (left + fdx < 1.0)
								right = left + height;
							//else, do something to preserve aspect...
						}
						break;
					case 2:
						isaspect = false;
						if (anchorx) {
							if (right - fdx > left) right -= fdx;
							if (right > 1.0) {
								//top -= right-iw;
								right = 1.0;
							}
							int width = right-left;
						}
						else {
							if (bottom - fdy >top) bottom -= fdy;
							if (bottom > 1.0) {
								//left -= bottom-ih;
								bottom = 1.0;
							}
							int height = bottom-top;
						}
						break;
					case 3:
						float dx = right - left;
						float dy = bottom - top;
						if (left - fdx < 0.0) {
							right = dx; 
							left = 0.0;
						}
						else if (right - fdx > 1.0) {
							left = 1.0 - dx;
							right = 1.0;
						}
						else {
							left -= fdx;
							right -= fdx;
						}
						if (top - fdy < 0.0) {
							bottom = dy;
							top = 0.0;
						}
						else if (bottom - fdy > 1.0) {
							top = 1.0 - dy;
							bottom = 1.0;
						}
						else {
							top -= fdy;
							bottom -= fdy;
						}
						break;
				}

			}
			if (left < 0.0) left = 0.0;
			if (right > 1.0) right = 1.0;
			if (top < 0.0) top = 0.0;
			if (bottom > 1.0) bottom = 1.0;
			mousemoved = true;
			mousex = event.m_x;
			mousey = event.m_y;
			//Refresh();
			//Update();
			PaintNow();
			event.Skip();
		}

		void OnMouseUp(wxMouseEvent& event)
		{
			if (node != 0) node = 0;
			wxCommandEvent e(myCROP_UPDATE);
			e.SetEventObject(this);
			e.SetString("update");
			ProcessWindowEvent(e);
			event.Skip();
		}

	private:

		wxBitmap *img;
		
		int ww, iw, wh, ih;
		int node, cropmode, mousex, mousey;
		double aspect, wa, ha, iwa, iha;
		double left, top , bottom, right;
		int cpradius, landingradius;
		bool isaspect, mousemoved;

};

class CropPanel: public PicProcPanel
{
	public:
		CropPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			wxImage img;
			SetDoubleBuffered(true);

			img = gImage2wxImage(proc->getPreviousPicProcessor()->getProcessedPic());

			if (myConfig::getConfig().getValueOrDefault("display.cms","1") == "1") {
				hTransform = q->getDisplay()->GetDisplayTransform();
				if (hTransform) {
					unsigned char* im = img.GetData();
					unsigned w = img.GetWidth();
					unsigned h = img.GetHeight();
					#pragma omp parallel for
					for (unsigned y=0; y<h; y++) {
						unsigned pos = y*w*3;
						cmsDoTransform(hTransform, &im[pos], &im[pos], w);
					}
				}
			}


			enablebox = new wxCheckBox(this, CROPENABLE, "crop:");
			enablebox->SetValue(true);
			cpane = new CropPane(this, wxID_ANY, img, params);
			
			wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP);
			myRowSizer *m = new myRowSizer(wxSizerFlags().Expand());
			m->AddRowItem(enablebox, wxSizerFlags(1).Left().Border(wxLEFT|wxTOP));
			m->AddRowItem(new wxBitmapButton(this, CROPCOPY, wxBitmap(copy_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->AddRowItem(new wxBitmapButton(this, CROPPASTE, wxBitmap(paste_xpm), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT), flags);
			m->NextRow(wxSizerFlags().Expand());
			m->AddRowItem(new wxStaticLine(this, wxID_ANY), wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->NextRow(wxSizerFlags(1).Expand());
			m->AddRowItem(cpane, 
				wxSizerFlags(1).Left().Shaped().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM));

			m->End();
			SetSizerAndFit(m);
			m->Layout();
		
			SetFocus();

			Bind(wxEVT_CHECKBOX, &CropPanel::onEnable, this, CROPENABLE);
			Bind(myCROP_UPDATE, &CropPanel::OnCropUpdate, this);
			Bind(wxEVT_BUTTON, &CropPanel::OnCopy, this, CROPCOPY);
			Bind(wxEVT_BUTTON, &CropPanel::OnPaste, this, CROPPASTE);
			//Bind(wxEVT_CHAR, &CropPanel::OnKey, this);
			q->getCommandTree()->Bind(wxEVT_TREE_SEL_CHANGED, &CropPanel::OnCommandtreeSelChanged, this);
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
			((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format("Copied command to clipboard: %s",q->getCommand()));
			
		}

		void OnPaste(wxCommandEvent& event)
		{
			if (q->pasteParamsFromClipboard()) {
				cpane->setParams(q->getParams());
				q->processPic();
				((wxFrame *) GetGrandParent())->SetStatusText(wxString::Format("Pasted command from clipboard: %s",q->getCommand()));
			}
			else wxMessageBox(wxString::Format("Invalid Paste"));
		}

		
		
		void OnCommandtreeSelChanged(wxTreeEvent& event)
		{
			wxImage img;
			event.Skip();
			img = gImage2wxImage(q->getPreviousPicProcessor()->getProcessedPic());

			if (myConfig::getConfig().getValueOrDefault("display.cms","1") == "1") {
				hTransform = q->getDisplay()->GetDisplayTransform();
				if (hTransform) {
					unsigned char* im = img.GetData();
					unsigned w = img.GetWidth();
					unsigned h = img.GetHeight();
					#pragma omp parallel for
					for (unsigned y=0; y<h; y++) {
						unsigned pos = y*w*3;
						cmsDoTransform(hTransform, &im[pos], &im[pos], w);
					}
				}
			}

			Refresh();
		}




		void OnCropUpdate(wxCommandEvent& event)
		{
			q->setParams(cpane->getParams());
			q->processPic();
		}

/*
		void OnKey(wxKeyEvent& event)
		{
			int newleft, newright, newtop, newbottom;
			int k = event.GetKeyCode();

			int inc = 1;
			if (event.ShiftDown()) inc = 10;
			if (event.ControlDown()) inc = 100;

			if (k == WXK_SPACE) {  //toggle aspect
				//node = 1;  //top left
				//cropmode = 1;
				if (isaspect)
					isaspect = false;
				else
					isaspect = true;
				Refresh(); Update();
			}
			else if (event.AltDown()) {   //move
				switch ( event.GetKeyCode() )
				{
					case WXK_LEFT:
						if (left-inc > 0) {
							left-=inc;
							right-=inc;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
					case WXK_RIGHT:
						if (right+inc < iw) {
							left+=inc;
							right+=inc;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
					case WXK_UP:
						if (top-inc > 0) {
							top-=inc;
							bottom-=inc;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
					case WXK_DOWN:
						if (bottom+inc < ih) {
							top+=inc;
							bottom+=inc;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
				}
				Refresh();
				Update();
				t->Start(500,wxTIMER_ONE_SHOT);
			}
			else if (isaspect) {  //change left, top
				switch (k)
				{
					case WXK_LEFT:
						newleft = left-inc;
						newbottom = top + ((right - newleft)*iha); 
						if (!(newleft<0) & !(newbottom>ih) ) {
							left=newleft;
							bottom=newbottom;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
					case WXK_RIGHT:
						newleft = left+inc;
						newbottom = top + ((right - newleft)*iha); 
						if (!(newleft<0) & !(newbottom>ih) ) {
							left=newleft;
							bottom=newbottom;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
					case WXK_UP:
						newtop = top-inc;
						newright = left + ((bottom-newtop)*iwa);
						if (!(newtop<0) & !(newright>iw) ) {
							top = newtop;
							right = newright;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
					case WXK_DOWN:
						newtop = top+inc;
						newright = left + ((bottom-newtop)*iwa);
						if (!(newtop<0) & !(newright>iw) ) {
							top = newtop;
							right = newright;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
				}

			}
			else {  //change right, bottom
				switch ( event.GetKeyCode() )
				{
					case WXK_LEFT:
						if (right-inc > left) {
							right-=inc;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
					case WXK_RIGHT:
						if (right+inc < iw) {
							right+=inc;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
					case WXK_UP:
						if (bottom-inc > top) {
							bottom-=inc;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
					case WXK_DOWN:
						if (bottom+inc < ih) {
							bottom+=inc;
							Refresh(); Update(); t->Start(500,wxTIMER_ONE_SHOT);
						}
						break;
				}
			}
		}
*/

	private:
		cmsHTRANSFORM hTransform;
		wxCheckBox *enablebox;
		CropPane *cpane;

};


PicProcessorCrop::PicProcessorCrop(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	if (command.IsEmpty()) {
		c= "0.0,0.0,1.0,1.0";
	}
	else {
		wxArrayString p = split(command, ",");
		float l = atof(p[0]);
		float t = atof(p[1]);
		float r = atof(p[2]);
		float b = atof(p[3]);

		if (l > 1.0 | t > 1.0 | r > 1.0 | b > 1.0) {  // old image-sized based crop
			int iw = getPreviousPicProcessor()->getProcessedPic().getWidth();
			int ih = getPreviousPicProcessor()->getProcessedPic().getHeight();
			l = l / (float) iw;
			t = t / (float) ih;
			r = r / (float) iw;
			b = b / (float) ih;
			c = wxString::Format("%f,%f,%f,%f", l, t, r, b);
		}
	}
}

PicProcessorCrop::PicProcessorCrop(wxString name, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, "0.0,0.0,1.0,1.0",  tree, display) 
{

}

void PicProcessorCrop::createPanel(wxSimplebook* parent)
{
	toolpanel = new CropPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
}

bool PicProcessorCrop::processPicture(gImage *processdib) 
{
	((wxFrame*) m_display->GetParent())->SetStatusText("crop...");
	wxArrayString p = split(getParams(),",");
	float l = atof(p[0]);
	float t = atof(p[1]);
	float r = atof(p[2]);
	float b = atof(p[3]);

	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str());

	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);

	dib = processdib;
	if (!global_processing_enabled) return true;

	int iw = dib->getWidth();
	int ih = dib->getHeight();
	int left    = iw * l;
	int top     = ih * t;
	int right   = iw * r;
	int bottom  = ih * b;
	
	if (processingenabled) {
		mark();
		dib->ApplyCrop(left, top, right, bottom, threadcount);
		m_display->SetModified(true);
		wxString d = duration();

		if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.crop.log","0") == "1"))
			log(wxString::Format("tool=crop,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));
	}

	dirty = false;


	((wxFrame*) m_display->GetParent())->SetStatusText("");
	
	return result;
}



