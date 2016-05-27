#include "PicProcessor.h"
#include "PicProcessorCrop.h"
#include "PicProcPanel.h"
#include "FreeImage.h"
#include "FreeImage16.h"
#include <wx/fileconf.h>
#include "util.h"

class CropPanel: public PicProcPanel
{
	public:
		CropPanel(wxPanel *parent, PicProcessor *proc, wxString params): PicProcPanel(parent, proc, params)
		{
			SetDoubleBuffered(true);
			node = 0;
			cropmode = 1; //0=freeform; 1=maintain aspect
			wxConfigBase::Get()->Read("tool.crop.controlpointradius",&radius,7);
			isaspect = true;

			wxArrayString p = split(params,",");
			left = atoi(p[0]);
			top = atoi(p[1]);
			right = atoi(p[2]);
			bottom = atoi(p[3]);
			//wxSizerFlags flags = wxSizerFlags().Center().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM);
			img = ThreadedFreeImage2wxImage(proc->getPreviousPicProcessor()->getProcessedPic());

			GetSize(&ww, &wh);
			iw = img.GetWidth();
			ih = img.GetHeight();
			wa = (double) ww/ (double) iw;
			ha = (double) wh/ (double) ih;
			aspect = wa > ha? ha : wa;

			iwa = (double) img.GetWidth() / (double) img.GetHeight();
			iha = (double) img.GetHeight() / (double) img.GetWidth();

			Bind(wxEVT_SIZE,&CropPanel::OnSize, this);
			Bind(wxEVT_PAINT,&CropPanel::OnPaint, this);
			Bind(wxEVT_LEFT_DOWN, &CropPanel::OnMouseDown, this);
			Bind(wxEVT_MOTION , &CropPanel::OnMouseMove, this);
			Bind(wxEVT_LEFT_UP, &CropPanel::OnMouseUp, this);
			Bind(wxEVT_LEAVE_WINDOW, &CropPanel::OnMouseUp, this);
		}

		~CropPanel()
		{
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
			//int ww, iw, wh, ih;
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
			//dc.DrawCircle(left*aspect, top*aspect, radius);
			dc.DrawRectangle(left*aspect-radius, top*aspect-radius,radius*2,radius*2);
			dc.SetBrush(*wxRED_BRUSH);
			dc.SetPen(*wxRED_PEN);
			dc.DrawCircle(right*aspect-1, bottom*aspect, radius);
			//DrawRectangle (right*aspect-1-radius, bottom*aspect-radius, right*aspect-1+radius, bottom*aspect+radius)
		}

		void OnMouseDown(wxMouseEvent& event)
		{
			mousex = event.m_x;
			mousey = event.m_y;
			if ((mousex > left*aspect-radius) & (mousex < left*aspect+radius)) {
				if ((mousey > top*aspect-radius) & (mousey < top*aspect+radius)) {
					node = 1;
				}
			}
			else if ((mousex > right*aspect-radius) & (mousex < right*aspect+radius)) {
				if ((mousey > bottom*aspect-radius) & (mousey < bottom*aspect+radius)) {
					node = 2;
				}
			}
			else if ((mousex > left*aspect) * (mousex < right*aspect)) {
				if ((mousey > top*aspect) & (mousey < bottom*aspect)) {
					node = 3;
				}
			}
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
					case 1:
						if (left - dx/aspect < right) left -= dx/aspect;
						if (top - dy/aspect < bottom) top  -= dy/aspect;
						if (left < 0) {
							bottom - left;
							left = 0;
						}
						if (top < 0) {
							right - top;
							top = 0;
						}
						break;
					case 2:
						if (right - dx/aspect > left) right -= dx/aspect;
						if (bottom - dy/aspect > top) bottom  -= dy/aspect;
						if (right > iw) right = iw;
						if (bottom > ih) bottom = ih;
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
							//if (top+(dx/aspect)*iha > 0)
							//	top = top+(dx/aspect)*iha;
							//else, do something to preserve aspect...
						}
						else {
							if (bottom - dy/aspect >top) bottom -= dy/aspect;
							if (bottom > ih) {
								left -= bottom-ih;
								bottom = ih;
							}
							int height = bottom-top;
							//if (right + height*iwa < iw)
							//	right = left + height*iwa;
							//else, do something to preserve aspect...
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
				q->processPic();
			}
			event.Skip();
		}


	private:
		//wxTextCtrl *widthedit, *heightedit;
		wxImage img;
		int ww, iw, wh, ih;
		int node, cropmode, mousex, mousey, radius;
		double aspect, wa, ha, iwa, iha;
		int left, top , bottom, right;
		bool isaspect;

};


PicProcessorCrop::PicProcessorCrop(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, command,  tree, display, parameters) 
{
	showParams();
}

PicProcessorCrop::PicProcessorCrop(wxString name, wxTreeCtrl *tree, PicPanel *display, wxPanel *parameters): PicProcessor(name, "",  tree, display, parameters) 
{
	c = wxString::Format("0,0,%d,%d",FreeImage_GetWidth(getPreviousPicProcessor()->getProcessedPic()), FreeImage_GetHeight(getPreviousPicProcessor()->getProcessedPic()));
	showParams();
}

void PicProcessorCrop::showParams()
{
	if (!m_parameters) return;
	m_parameters->DestroyChildren();
	r = new CropPanel(m_parameters, this, c);
}


bool PicProcessorCrop::processPic() {
	((wxFrame*) m_display->GetParent())->SetStatusText("crop...");
	wxArrayString p = split(getParams(),",");
	int left = atoi(p[0]);
	int top = atoi(p[1]);
	int right = atoi(p[2]);
	int bottom = atoi(p[3]);
	bool result = true;

	if (dib) FreeImage_Unload(dib);
	dib = FreeImage_Copy(getPreviousPicProcessor()->getProcessedPic(), left, top, right, bottom);
	dirty = false;

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



