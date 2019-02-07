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
		void SetThumbMode(int mode);
		void SetColorManagement(bool b);
		bool GetColorManagement();
		void SetProfile(gImage * dib);
		void SetImageProfile(cmsHPROFILE hImgProf);
		cmsHTRANSFORM GetDisplayTransform();
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
		void OnMouseEnter(wxMouseEvent& event);
		void OnMouseLeave(wxMouseEvent& event);
		void OnTimer(wxTimerEvent& event);

		void setStatusBar();
        
    private:
		gImage * display_dib;
		GIMAGE_CHANNEL ch;

		int mousex, mousey;
		bool dragging, thumbdragging, thumbvisible, fit;
		int skipmove;
		int oob;

		double scale;
		wxBitmap *image, *thumbnail;

		double imgctrx, imgctry;
		int viewposx, viewposy;
		int imageposx, imageposy, imagex, imagey, imagew, imageh, vieww, viewh;
		int thumbw, thumbh;
		double thumbwscale, thumbhscale;
		
		wxString dcList;
		
		bool colormgt;
		cmsHPROFILE displayProfile;
		cmsHTRANSFORM displayTransform;
		
		myHistogramPane *histogram;

    
};



#endif
