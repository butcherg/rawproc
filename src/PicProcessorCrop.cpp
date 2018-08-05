#include "PicProcessor.h"
#include "PicProcessorCrop.h"
#include "PicProcPanel.h"
#include "myConfig.h"
#include "util.h"

class CropPanel: public PicProcPanel
{
	public:
		CropPanel(wxWindow *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetDoubleBuffered(true);
			node = 0;
			cropmode = 1; //0=freeform; 1=maintain aspect
			//parm tool.crop.controlpointradius: Radius of the rectangle displayed to indicate a control point.  Default=7
			cpradius = atoi(myConfig::getConfig().getValueOrDefault("tool.crop.controlpointradius","7").c_str());
			//parm tool.crop.landingradius: radius of control point area sensitive to mouseclicks.  Doesn't have to be the radius of the control point rectangle.  Default=7
			landingradius = atoi(myConfig::getConfig().getValueOrDefault("tool.crop.landingradius","7").c_str());
			isaspect = true;

			int indent = atoi(myConfig::getConfig().getValueOrDefault("tool.crop.initialindent","0").c_str());

			wxArrayString p = split(params,",");
			left = atoi(p[0])+indent;
			top = atoi(p[1])+indent;
			right = atoi(p[2])-indent;
			bottom = atoi(p[3])-indent;

			img = gImage2wxImage(proc->getPreviousPicProcessor()->getProcessedPic());

			GetSize(&ww, &wh);
			iw = img.GetWidth();
			ih = img.GetHeight();
			wa = (double) ww/ (double) iw;
			ha = (double) wh/ (double) ih;
			aspect = wa > ha? ha : wa;

			hTransform = proc->getDisplay()->GetDisplayTransform();
			if (hTransform)
				cmsDoTransform(hTransform, img.GetData(), img.GetData(), iw*ih);

			iwa = (double) img.GetWidth() / (double) img.GetHeight();
			iha = (double) img.GetHeight() / (double) img.GetWidth();

			t = new wxTimer(this);
			SetFocus();

			Bind(wxEVT_SIZE,&CropPanel::OnSize, this);
			Bind(wxEVT_PAINT,&CropPanel::OnPaint, this);
			Bind(wxEVT_LEFT_DOWN, &CropPanel::OnMouseDown, this);
			Bind(wxEVT_MOTION , &CropPanel::OnMouseMove, this);
			Bind(wxEVT_LEFT_UP, &CropPanel::OnMouseUp, this);
			Bind(wxEVT_LEAVE_WINDOW, &CropPanel::OnMouseUp, this);
			Bind(wxEVT_CHAR, &CropPanel::OnKey, this);
			Bind(wxEVT_TIMER, &CropPanel::OnTimer,  this);
			q->getCommandTree()->Bind(wxEVT_TREE_SEL_CHANGED, &CropPanel::OnCommandtreeSelChanged, this);
		}

		~CropPanel()
		{

		}
		
		
		void OnCommandtreeSelChanged(wxTreeEvent& event)
		{
			event.Skip();
			img = gImage2wxImage(q->getPreviousPicProcessor()->getProcessedPic());

			GetSize(&ww, &wh);
			iw = img.GetWidth();
			ih = img.GetHeight();
			wa = (double) ww/ (double) iw;
			ha = (double) wh/ (double) ih;
			aspect = wa > ha? ha : wa;

			hTransform = q->getDisplay()->GetDisplayTransform();
			if (hTransform)
				cmsDoTransform(hTransform, img.GetData(), img.GetData(), iw*ih);

			iwa = (double) img.GetWidth() / (double) img.GetHeight();
			iha = (double) img.GetHeight() / (double) img.GetWidth();
			Refresh();
		}

		void OnSize(wxSizeEvent& event) 
		{
			wxSize s = GetParent()->GetSize();
			SetSize(s);

			GetSize(&ww, &wh);
			iw = img.GetWidth();
			ih = img.GetHeight();
			wa = (double) ww/ (double) iw;
			ha = (double) wh/ (double) ih;
			aspect = wa > ha? ha : wa;

			iwa = (double) img.GetWidth() / (double) img.GetHeight();
			iha = (double) img.GetHeight() / (double) img.GetWidth();
			Refresh();
		}

		void OnPaint(wxPaintEvent& event)
		{
			GetSize(&ww, &wh);
			wxPaintDC dc(this);
			dc.DrawBitmap(wxBitmap(img.Scale(iw*aspect, ih*aspect)),0,0);
			if (isaspect)
				dc.SetPen(*wxYELLOW_PEN);
			else
				dc.SetPen(*wxRED_PEN);
			dc.SetBrush(*wxYELLOW_BRUSH);
			dc.DrawLine(left*aspect, top*aspect, right*aspect-1, top*aspect);
			dc.DrawLine(right*aspect-1, top*aspect, right*aspect-1, bottom*aspect);
			dc.DrawLine(right*aspect-1, bottom*aspect, left*aspect, bottom*aspect);
			dc.DrawLine(left*aspect, bottom*aspect, left*aspect, top*aspect);
			dc.SetBrush(*wxYELLOW_BRUSH);
			dc.SetPen(*wxYELLOW_PEN);
			dc.DrawRectangle(left*aspect+1, top*aspect+1,cpradius*2,cpradius*2);
			dc.SetBrush(*wxRED_BRUSH);
			dc.SetPen(*wxRED_PEN);
			dc.DrawRectangle((right*aspect-cpradius*2)-1, (bottom*aspect-cpradius*2),cpradius*2,cpradius*2);
		}

		void OnMouseDown(wxMouseEvent& event)
		{
			mousex = event.m_x;
			mousey = event.m_y;

			int radius = atoi(myConfig::getConfig().getValueOrDefault("tool.crop.landingradius","7").c_str());

			if ((mousex > left*aspect) & (mousex < left*aspect+radius*2)) {
				if ((mousey > top*aspect) & (mousey < top*aspect+landingradius*2)) {
					node = 1;  //top left
					cropmode = 1;
					isaspect = true;
				}
			}
			else if ((mousex > right*aspect-landingradius*2) & (mousex < right*aspect)) {
				if ((mousey > bottom*aspect-landingradius*2) & (mousey < bottom*aspect)) {
					node = 2; //bottom right
					cropmode = 0;
					isaspect = false;
				}
			}
			else if ((mousex > left*aspect) * (mousex < right*aspect)) {
				if ((mousey > top*aspect) & (mousey < bottom*aspect)) {
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
			bool anchorx;
			if (node == 0) {
				event.Skip();
				return;
			}
			int dx = mousex-event.m_x;
			int dy = mousey-event.m_y;

			if (cropmode == 0) {
				isaspect = false;
				switch (node) {
					case 1:  //top left
						if (left - dx/aspect < right) left -= dx/aspect;
						if (top - dy/aspect < bottom) top  -= dy/aspect;
						if (left < 0) {
							bottom -= left;
							left = 0;
						}
						if (top < 0) {
							right -= top;
							top = 0;
						}
						break;
					case 2:  //bottom right
						if (right - dx/aspect > left) right -= dx/aspect;
						if (bottom - dy/aspect > top) bottom  -= dy/aspect;
						if (right > iw) right = iw;
						if (bottom > ih) bottom = ih;
						break;
					case 3:  //move
						if ((left - dx/aspect > 0) & (top - dy/aspect > 0) & (right - dx/aspect < iw) & (bottom - dy/aspect < ih)) {
							left -= dx/aspect;
							top  -= dy/aspect;
							right -= dx/aspect;
							bottom  -= dy/aspect;
						}
						break;
				}
			}
			else if (cropmode == 1) {
				if (abs(dx) > abs(dy))
					anchorx = true;  //x
				else 
					anchorx = false;  //y

				switch (node) {
					case 1:
						isaspect = true;
						if (anchorx) {
							if (left - dx/aspect < right) left -= dx/aspect;
							if (left < 0) {
								bottom -= left;
								left = 0;
							}
							int width = right-left;
							bottom = top + width*iha;
						}
						else {
							if (top - dy/aspect < bottom) top -= dy/aspect;
							if (top < 0) {
								right -= top;
								top = 0;
							}
							int height = bottom-top;
							if (left + height*iwa < iw)
								right = left + height*iwa;
							//else, do something to preserve aspect...
						}
						break;
					case 2:
						isaspect = false;
						if (anchorx) {
							if (right - dx/aspect > left) right -= dx/aspect;
							if (right > iw) {
								top -= right-iw;
								right = iw;
							}
							int width = right-left;
						}
						else {
							if (bottom - dy/aspect >top) bottom -= dy/aspect;
							if (bottom > ih) {
								left -= bottom-ih;
								bottom = ih;
							}
							int height = bottom-top;
						}
						break;
					case 3:
						if ((left - dx/aspect > 0) & (top - dy/aspect > 0) & (right - dx/aspect < iw) & (bottom - dy/aspect < ih)) {
							left -= dx/aspect;
							top  -= dy/aspect;
							right -= dx/aspect;
							bottom  -= dy/aspect;
						}
						break;
				}
				if (left < 0) left = 0;
				if (right > iw) right = iw;
				if (top < 0) top = 0;
				if (bottom > ih) bottom = ih;
			}
			mousemoved = true;
			mousex = event.m_x;
			mousey = event.m_y;
			Refresh();
			Update();
			event.Skip();
		}

		void OnMouseUp(wxMouseEvent& event)
		{
			if (node != 0) {
				node = 0;
				q->setParams(wxString::Format("%d,%d,%d,%d",left,top,right,bottom));
				if (mousemoved) q->processPic();
			}
			event.Skip();
		}

		void OnKey(wxKeyEvent& event)
		{
			int newleft, newright, newtop, newbottom;
			int k = event.GetKeyCode();

			int inc = 1;
			if (event.ShiftDown()) inc = 10;
			if (event.ControlDown()) inc = 100;

			if (event.AltDown()) {   //move
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

		void OnTimer(wxTimerEvent& event)
		{
			q->setParams(wxString::Format("%d,%d,%d,%d",left,top,right,bottom));
			q->processPic();
		}


	private:
		//wxTextCtrl *widthedit, *heightedit;
		wxImage img;
		int ww, iw, wh, ih;
		int node, cropmode, mousex, mousey;
		double aspect, wa, ha, iwa, iha;
		int left, top , bottom, right, cpradius, landingradius;
		bool isaspect, mousemoved;
		cmsHTRANSFORM hTransform;
		wxTimer *t;

};


PicProcessorCrop::PicProcessorCrop(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, command,  tree, display) 
{
	//showParams();
}

PicProcessorCrop::PicProcessorCrop(wxString name, wxTreeCtrl *tree, PicPanel *display): PicProcessor(name, "",  tree, display) 
{
	c = wxString::Format("0,0,%d,%d",getPreviousPicProcessor()->getProcessedPic().getWidth(), getPreviousPicProcessor()->getProcessedPic().getHeight());
	//showParams();
}

void PicProcessorCrop::createPanel(wxSimplebook* parent)
{
	toolpanel = new CropPanel(parent, this, c);
	parent->ShowNewPage(toolpanel);
	toolpanel->Refresh();
	toolpanel->Update();
}

bool PicProcessorCrop::processPic(bool processnext) {
	((wxFrame*) m_display->GetParent())->SetStatusText("crop...");
	wxArrayString p = split(getParams(),",");
	int left = atoi(p[0]);
	int top = atoi(p[1]);
	int right = atoi(p[2]);
	int bottom = atoi(p[3]);
	bool result = true;

	int threadcount =  atoi(myConfig::getConfig().getValueOrDefault("tool.crop.cores","0").c_str());

	if (threadcount == 0) 
		threadcount = gImage::ThreadCount();
	else if (threadcount < 0) 
		threadcount = std::max(gImage::ThreadCount() + threadcount,0);



	if (dib) delete dib;
	dib = new gImage(getPreviousPicProcessor()->getProcessedPic());
	//toolpanel->SetPic(this);
	
	mark();
	dib->ApplyCrop(left, top, right, bottom, threadcount);
	wxString d = duration();

	if ((myConfig::getConfig().getValueOrDefault("tool.all.log","0") == "1") || (myConfig::getConfig().getValueOrDefault("tool.crop.log","0") == "1"))
		log(wxString::Format("tool=crop,imagesize=%dx%d,threads=%d,time=%s",dib->getWidth(), dib->getHeight(),threadcount,d));

	dirty = false;

	((wxFrame*) m_display->GetParent())->SetStatusText("");
	if (processnext) processNext();
	
	return result;
}



