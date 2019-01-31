
#include "PicPanel.h"
#include "util.h"
#include <vector>
#include "PicProcessor.h"
#include "myConfig.h"
#include <wx/clipbrd.h>


PicPanel::PicPanel(wxFrame *parent, wxTreeCtrl *tree, myHistogramPane *hgram): wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(1000,740)) 
{

	SetDoubleBuffered(true);  //watch this one... tricksy...

	scale = 1.0;
	imgctrx = 0.5; imgctry = 0.5;
	mousex = 0; mousey=0;

	Bind(wxEVT_PAINT, &PicPanel::OnPaint,  this);
	Bind(wxEVT_LEFT_DOWN, &PicPanel::OnLeftDown,  this);
	Bind(wxEVT_RIGHT_DOWN, &PicPanel::OnRightDown,  this);
	Bind(wxEVT_LEFT_DCLICK, &PicPanel::OnLeftDoubleClicked,  this);
	Bind(wxEVT_LEFT_UP, &PicPanel::OnLeftUp,  this);
	Bind(wxEVT_MOTION, &PicPanel::OnMouseMove,  this);
	Bind(wxEVT_MOUSEWHEEL, &PicPanel::OnMouseWheel,  this);
	Bind(wxEVT_LEAVE_WINDOW, &PicPanel::OnMouseLeave,  this);
	Bind(wxEVT_KEY_DOWN, &PicPanel::OnKey,  this);
	Bind(wxEVT_TIMER, &PicPanel::OnTimer,  this);
		
	t = new wxTimer(this);
}

PicPanel::~PicPanel()
{
	if (scaleimg) scaleimg->~wxBitmap();
	if (viewimg) viewimg->~wxBitmap();
	t->~wxTimer();
}
        
void PicPanel::OnSize(wxSizeEvent& event) 
{
	Refresh();
	event.Skip();
}

void PicPanel::SetPic(gImage * dib, GIMAGE_CHANNEL channel)
{
	img = gImage2wxImage(*dib);
	if (scaleimg) scaleimg->~wxBitmap();
	scaleimg = new wxBitmap(img.Rescale( img.GetWidth()*scale, img.GetHeight()*scale));
	//ToDo: cmsTransform()...

	Refresh();
}

void PicPanel::render(wxDC &dc)
{
	if (!scaleimg) return;
	int vieww, viewh; 
	//int viewposx=0, viewposy=0;  //now class members...
	int imgw, imgh; 
	int scalew, scaleh;

	GetSize(&vieww, &viewh);
	imgw = img.GetWidth(); imgh = img.GetHeight();
	scalew = scaleimg->GetWidth(); scaleh = scaleimg->GetHeight();

	//image position to be centered in the view, expressed as a percent:
	//imgctrx = (float) (1+ viewposx + mousex) / (float) scalew;
	//imgctry = (float) (1+ viewposy + mousey) / (float) scaleh;

	//((wxFrame *) GetParent())->SetStatusText(wxString::Format("imgctrx: %.2f imgctry: %.2f",imgctrx, imgctry));

	viewimg = scaleimg;  //all alternatives below just use scaleimg...

	if (scalew < vieww) {
		//center the image in the panel:
		viewposx = (vieww - scalew) / 2;
		viewposy = (viewh - scaleh) / 2;
		dc.DrawBitmap(*viewimg, viewposx, viewposy, false);
	}
	else {
		viewposx = (scalew/2 - vieww/2);
		viewposy = (scaleh/2 - viewh/2);

		//show the scaled image in its entirety, offset by viewposx/y:
		//viewposx = -viewposx;
		//viewposy = -viewposx;
		//dc.DrawBitmap(*viewimg, viewposx, viewposy, false);

		//extract the viewimage from the position (imgw*imgctrx)-vieww/2 and show it at 0,0:
		wxMemoryDC mdc;
		mdc.SelectObject(*scaleimg);
		dc.Blit(0, 0, vieww, viewh, &mdc, viewposx, viewposy);
		mdc.SelectObject(wxNullBitmap);
	}

}

void PicPanel::OnMouseWheel(wxMouseEvent& event)
{
	mousex = event.m_x;
	mousey = event.m_y;

	double increment = 0.05;

	if (event.GetWheelRotation() > 0)
		scale += increment;
	else
		scale -= increment;

	if (scale < 0.1) 
		scale = 0.1;
	else if (scale > 3) 
		scale = 3; 


	((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %.0f%%", scale*100),2);
	event.Skip();

	if (event.ShiftDown()) {
		t->Start(400,wxTIMER_ONE_SHOT);
	}
	else {
		if (scaleimg) scaleimg->~wxBitmap();
		wxImage im = img;
		scaleimg = new wxBitmap(im.Rescale( img.GetWidth()*scale, img.GetHeight()*scale));
		Refresh();
	}
}

void PicPanel::OnTimer(wxTimerEvent& event)
{
	if (scaleimg) scaleimg->~wxBitmap();
	wxImage im = img;
	scaleimg = new wxBitmap(im.Rescale( img.GetWidth()*scale, img.GetHeight()*scale));
	Refresh();
}

void PicPanel::OnLeftDoubleClicked(wxMouseEvent& event)
{
	mousex = event.m_x;
	mousey = event.m_y;

	scale = 1.0;

	if (scaleimg) scaleimg->~wxBitmap();
	wxImage im = img;
	scaleimg = new wxBitmap(im.Rescale( img.GetWidth()*scale, img.GetHeight()*scale));

	((wxFrame *) GetParent())->SetStatusText(wxString::Format("scale: %.0f%%", scale*100),2);
	//((wxFrame *) GetParent())->SetStatusText(wxString::Format("imgctrx: %.2f imgctry: %.2f",imgctrx, imgctry));

	event.Skip();
	Refresh();
}





        
void PicPanel::drawBox(wxDC &dc, int x, int y, int w,int h)
{
	dc.DrawLine(x, y, x+w, y);
	dc.DrawLine(x+w, y, x+w, y+h);
	dc.DrawLine(x+w, y+h, x, y+h);
	dc.DrawLine(x, y+h, x,y);
}

void PicPanel::SetThumbMode(int mode)
{

}

void PicPanel::ToggleThumb()
{

}
	
void PicPanel::BlankPic()
{

}

void PicPanel::RefreshPic()
{

}


	
void PicPanel::SetDrawList(wxString list)
{

}
   

void PicPanel::SetColorManagement(bool b)
{

}

bool PicPanel::GetColorManagement()
{
	return false;
}


void PicPanel::SetProfile(gImage * dib)
{

}

void PicPanel::SetImageProfile(cmsHPROFILE hImgProf)
{

}

cmsHTRANSFORM PicPanel::GetDisplayTransform()
{
	return NULL;
}

wxString PicPanel::getHistogramString()
{
	return "";
}
        

void PicPanel::SetScaleToWidth()
{

}
	
void PicPanel::SetScaleToHeight()
{

}

void PicPanel::SetScaleToWidth(double percentofwidth)
{
	int w, h;
}

void PicPanel::SetScale(double s)
{
	scale = s;
}

void PicPanel::FitMode(bool f)
{

}


double PicPanel::GetScale()
{
	return scale;
}
	
coord PicPanel::GetImgCoords()
{
	return coord {0,0};
}
	
void PicPanel::PaintNow()
{
	wxClientDC dc(this);
	render(dc);
}

void PicPanel::OnPaint(wxPaintEvent & event)
{
	wxPaintDC dc(this);
	render(dc);
}

void PicPanel::OnLeftDown(wxMouseEvent& event)
{
	mousex = event.m_x;
	mousey = event.m_y;

	int imgw, imgh;
	int scalew, scaleh;

	//GetSize(&vieww, &viewh);
	imgw = img.GetWidth(); imgh = img.GetHeight();
	scalew = scaleimg->GetWidth(); scaleh = scaleimg->GetHeight();

	imgctrx = (float) (1+ viewposx + mousex) / (float) scalew;
	imgctry = (float) (1+ viewposy + mousey) / (float) scaleh;

	((wxFrame *) GetParent())->SetStatusText(wxString::Format("imgctrx: %.2f imgctry: %.2f",imgctrx, imgctry));

}

void PicPanel::OnRightDown(wxMouseEvent& event)
{

}

void PicPanel::OnLeftUp(wxMouseEvent& event)
{

}

void PicPanel::OnMouseMove(wxMouseEvent& event)
{
	mousex = event.m_x;
	mousey = event.m_y;
	Refresh();
}

void PicPanel::OnMouseLeave(wxMouseEvent& event)
{

}

void PicPanel::OnKey(wxKeyEvent& event)
{

}


