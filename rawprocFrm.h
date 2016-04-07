///-----------------------------------------------------------------
///
/// @file      rawprocFrm.h
/// @author    Glenn
/// Created:   11/18/2015 7:04:06 PM
/// @section   DESCRIPTION
///            rawprocFrm class declaration
///
///------------------------------------------------------------------

#ifndef __RAWPROCFRM_H__
#define __RAWPROCFRM_H__

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

//Do not add custom headers between 
//Header Include Start and Header Include End.
//wxDev-C++ designer will remove them. Add custom headers after the block.
////Header Include Start
#include <wx/statusbr.h>
#include <wx/menu.h>
////Header Include End

////Dialog Style Start
#undef rawprocFrm_STYLE
#define rawprocFrm_STYLE wxCAPTION | wxRESIZE_BORDER | wxSYSTEM_MENU | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX
////Dialog Style End

#include <wx/filename.h>
#include <wx/aui/aui.h>
#include <wx/treectrl.h>
//#include <wx/treelist.h>
#include "FreeImage.h"
#include "PicPanel.h"
#include "PicProcessor.h"


class rawprocFrm : public wxFrame
{
	private:
		DECLARE_EVENT_TABLE();
		
	public:
		rawprocFrm(wxWindow *parent, wxWindowID id = 1, const wxString &title = wxT("rawproc"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = rawprocFrm_STYLE);
		virtual ~rawprocFrm();
		void Mnuopen1003Click(wxCommandEvent& event);
		void Mnuopensource1004Click(wxCommandEvent& event);
		
	private:
		//Do not add custom control declarations between
		//GUI Control Declaration Start and GUI Control Declaration End.
		//wxDev-C++ will remove them. Add custom code after the block.
		////GUI Control Declaration Start
		wxStatusBar *WxStatusBar1;
		wxMenuBar *WxMenuBar1;
		////GUI Control Declaration End
		
	private:
		//Note: if you receive any error with these enum IDs, then you need to
		//change your old form code that are based on the #define control IDs.
		//#defines may replace a numeric value for the enum names.
		//Try copy and pasting the below block in your old form header files.
		enum
		{
			////GUI Enum Control ID Start
			ID_WXSTATUSBAR1 = 1001,
			ID_MNU_FILE = 1002,
			ID_MNU_OPEN = 1003,
			ID_MNU_OPENSOURCE = 1004,
			ID_MNU_SAVE = 1005,
			ID_MNU_ADD = 1006,
			ID_MNU_GAMMA = 1010,
			ID_MNU_BRIGHT = 1011,
			ID_MNU_CONTRAST = 1012,
			ID_MNU_CURVE = 1013,
			ID_MNU_SHADOW = 1014,
			ID_MNU_HIGHLIGHT = 1015,
			ID_MNU_SATURATION = 1016,
			ID_MNU_GRAY = 1017,
			ID_MNU_CROP = 1018,
			ID_MNU_RESIZE = 1019,
			ID_MNU_HELP = 1020,
			ID_MNU_SHOWCOMMAND = 1021,
			ID_MNU_ABOUT = 1022,
			ID_MNU_EXIT = 1023,
			ID_MNU_BLACKWHITEPOINT = 1024,
			ID_MNU_SHARPEN = 1025,
			
			////GUI Enum Control ID End
			ID_COMMANDTREE = 2000,
			ID_MNU_Cut =  1201,
			ID_MNU_Copy = 1202,
			ID_MNU_Paste = 1203,
			ID_DUMMY_VALUE_ //don't remove this value unless you have other enum values
		};
		
	public:
        //void SetPic(FIBITMAP *dib);
	PicProcessor *  AddItem(wxString name, wxString command);
	void CommandTreeSelChanging(wxTreeEvent& event);
        void CommandTreeSelChanged(wxTreeEvent& event);
	void CommandTreeKeyDown(wxTreeEvent& event);
	void CommandTreeDeleteItem(wxTreeEvent& event);
	void CommandTreeBeginDrag(wxTreeEvent& event);
	void CommandTreeEndDrag(wxTreeEvent& event);
	void CommandTreeStateClick(wxTreeEvent& event);
	void CommandTreeSetDisplay(wxTreeItemId item);
	void CommandTreePopup(wxTreeEvent& event);
	void CommandTreeDeleteItem(wxTreeItemId item);

	bool MoveBefore(wxTreeItemId item);
	bool MoveAfter(wxTreeItemId item);
	PicProcessor * GetItemProcessor(wxTreeItemId item);
	wxString AssembleCommand();
	void EXIFDialog(wxTreeItemId item);
	void OpenFile(wxString fname, int flag);
	void OpenFileSource(wxString fname);
	void Mnuadd1005Click(wxCommandEvent& event);
	void Mnugamma1006Click(wxCommandEvent& event);
	void Mnubright1007Click(wxCommandEvent& event);
	void Mnucontrast1008Click(wxCommandEvent& event);
	void MnusaturateClick(wxCommandEvent& event);
	void Mnucurve1010Click(wxCommandEvent& event);
	void MnuShadow1015Click(wxCommandEvent& event);
	void MnuHighlightClick(wxCommandEvent& event);
	void MnuGrayClick(wxCommandEvent& event);
	void MnuCropClick(wxCommandEvent& event);
	void MnuResizeClick(wxCommandEvent& event);
	void MnuBlackWhitePointClick(wxCommandEvent& event);
	void MnuSharpenClick(wxCommandEvent& event);
	void Mnusave1009Click(wxCommandEvent& event);
	void MnuexitClick(wxCommandEvent& event);
	void MnuCut1201Click(wxCommandEvent& event);
	void MnuCopy1202Click(wxCommandEvent& event);
	void MnuPaste1203Click(wxCommandEvent& event);
	void MnuShowCommand1010Click(wxCommandEvent& event);
	void MnuAbout1011Click(wxCommandEvent& event);
		
	private:
		void OnClose(wxCloseEvent& event);
		void CreateGUIControls();
		
		wxAuiManager mgr;
		
		wxTreeCtrl *commandtree;
		PicPanel *pic;
		wxPanel *parameters;
		FIBITMAP *d;
		wxImage *img;
		bool deleting;
		wxTreeItemId olditem;
		wxFileName filename, sourcefilename;
};

#endif
