#ifndef __PICPANEL_H__
#define __PICPANEL_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include <wx/image.h>
#include <wx/dcbuffer.h>
#include <wx/string.h>
#include <wx/treectrl.h>
#include <wx/filename.h>
//#include <wx/generic/statbmpg.h>
#include "myHistogramPane.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <lcms2.h>
#include "gimage/gimage.h"


class PicPanel: public wxPanel
{
	public:
		PicPanel(wxFrame *parent, wxTreeCtrl *tree, myHistogramPane *hgram);
		~PicPanel();
		void OnSize(wxSizeEvent& event);
		void PaintNow();
		void OnPaint(wxPaintEvent& event);
		void BlankPic();
		void render(wxDC &dc);
		void drawBox(wxDC &dc, int x, int y, int w,int h);
		void RefreshPic();
		void SetPic(gImage * dib, GIMAGE_CHANNEL channel=CHANNEL_RGB);
		void SetColorManagement(bool b);
		bool GetColorManagement();
		void SetProfile(gImage * dib);
		void SetImageProfile(cmsHPROFILE hImgProf);
		cmsHTRANSFORM GetDisplayTransform();
		void SetThumbMode(int mode);
		void ToggleThumb();
		wxString getHistogramString();
		double GetScale();
		void SetScale(double s);
		void SetScaleToWidth();
		void SetScaleToHeight();
		void SetScaleToWidth(double percentofwidth);
		coord GetImgCoords();
		void SetDrawList(wxString list);
		void FitMode(bool f);
		void OnMouseWheel(wxMouseEvent& event);
		void OnMouseMove(wxMouseEvent& event);
		void OnLeftUp(wxMouseEvent& event);
		void OnRightDown(wxMouseEvent& event);
		void OnLeftDown(wxMouseEvent& event);
		void OnLeftDoubleClicked(wxMouseEvent& event);
		void OnKey(wxKeyEvent& event);
		void OnMouseLeave(wxMouseEvent& event);
		void OnTimer(wxTimerEvent& event);
        
    private:

		int mousex, mousey;

		double scale;
		wxImage img;
		wxBitmap *scaleimg, *viewimg;

		double imgctrx, imgctry;
		int viewposx, viewposy;
		
		wxTimer *t;
    
};



#endif
