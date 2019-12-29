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
#include <wx/propgrid/propgrid.h>

#include <wx/image.h>
#include <wx/html/helpfrm.h>
#include <wx/html/helpctrl.h>
#include <wx/filesys.h>
#include <wx/fs_zip.h>
#include <wx/generic/statbmpg.h>
#include <wx/dnd.h>

#include "gimage/gimage.h"
#include "PicPanel.h"
#include "PicProcessor.h"
#include "myPropertyDialog.h"


class rawprocFrm : public wxFrame //, wxFileDropTarget
{
	private:
		DECLARE_EVENT_TABLE();
		
	public:
		rawprocFrm(wxWindow *parent, wxWindowID id = 1, const wxString &title = wxT("rawproc"), const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = rawprocFrm_STYLE);
		
		void Mnuopen1003Click(wxCommandEvent& event);
		void Mnuopensource1004Click(wxCommandEvent& event);
		void Mnureopen1033Click(wxCommandEvent& event);
		
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
			ID_MNU_ROTATE = 1026,
			ID_MNU_DENOISE = 1027,
			ID_MNU_VIEWHELP = 1028,
			ID_MNU_PROPERTIES = 1029,
			ID_MNU_EXIF = 1030,
			ID_MNU_EXPOSURE = 1031,
			ID_MNU_REDEYE = 1032,
			ID_MNU_REOPEN = 1033,
			ID_MNU_COLORSPACE = 1034,
			ID_MNU_LENSCORRECTION = 1035,
			ID_MNU_WHITEBALANCE = 1036,
			ID_MNU_DEMOSAIC = 1037,
			ID_MNU_TONE = 1038,
			ID_MNU_SUBTRACT = 1039,
			ID_MNU_GROUP = 1040,
			ID_MNU_CACORRECT = 1041,
			ID_MNU_HLRECOVER = 1042,
			ID_MNU_BATCH = 1043,

			ID_MNU_TOOLLIST = 1100,
			////GUI Enum Control ID End
			ID_COMMANDTREE = 2000,
			ID_MNU_Cut =  1201,
			ID_MNU_Copy = 1202,
			ID_MNU_Paste = 1203,
			ID_DUMMY_VALUE_ //don't remove this value unless you have other enum values
		};
		
	public:
	void OnAUIActivate(wxAuiManagerEvent& event);
	void OnPaneButton(wxAuiManagerEvent& event);
	//PicProcessor *  AddItem(wxString name, wxString command, bool display=true);
	wxTreeItemId AddItem(wxString name, wxString command, bool display=true);
	void ApplyOps(gImage &dib, wxString operations);
	void CommandTreeSelChanging(wxTreeEvent& event);
	void CommandTreeSelChanged(wxTreeEvent& event);
	void CommandTreeKeyDown(wxTreeEvent& event);
	//void CommandTreeDeleteItem(wxTreeEvent& event);
	void CommandTreeBeginDrag(wxTreeEvent& event);
	void CommandTreeEndDrag(wxTreeEvent& event);
	void CommandTreeStateClick(wxTreeEvent& event);
	void CommandTreeSetDisplay(wxTreeItemId item, int src=0);
	void CommandTreePopup(wxTreeEvent& event);
	void CommandTreeDeleteItem(wxTreeItemId item, bool selectprevious=false);
	void CommandTreeDeleteSubsequent(wxTreeItemId item);

	void SetConfigFile(wxString cfile);
	void SetThumbMode(int mode);

	PicProcessor * GetItemProcessor(wxTreeItemId item);
	wxString AssembleCommand();
	void InfoDialog(wxTreeItemId item);
	void EXIFDialog(wxFileName filename);
	void showHistogram(wxTreeItemId item);
	void OpenFile(wxString fname);  //, wxString params="");
	void OpenFileSource(wxString fname);

	void MnusaturateClick(wxCommandEvent& event);
	void MnuexposureClick(wxCommandEvent& event);
	void Mnucurve1010Click(wxCommandEvent& event);
	void MnuGrayClick(wxCommandEvent& event);
	void MnuCropClick(wxCommandEvent& event);
	void MnuResizeClick(wxCommandEvent& event);
	void MnuBlackWhitePointClick(wxCommandEvent& event);
	void MnuSharpenClick(wxCommandEvent& event);
	void MnuRotateClick(wxCommandEvent& event);
	void MnuDenoiseClick(wxCommandEvent& event);
	void MnuRedEyeClick(wxCommandEvent& event);
	void MnuTone(wxCommandEvent& event);
	void MnuSubtract(wxCommandEvent& event);
	void MnuGroup(wxCommandEvent& event);

	void Mnusave1009Click(wxCommandEvent& event);
	void MnuexitClick(wxCommandEvent& event);
	void MnuCut1201Click(wxCommandEvent& event);
	void MnuCopy1202Click(wxCommandEvent& event);
	void MnuPaste1203Click(wxCommandEvent& event);
	void MnuShowCommand1010Click(wxCommandEvent& event);
	void MnuBatchClick(wxCommandEvent& event);
	void MnuAbout1011Click(wxCommandEvent& event);
	void MnuHelpClick(wxCommandEvent& event);
	void MnuProperties(wxCommandEvent& event);
	void MnuEXIF(wxCommandEvent& event);
	void MnuColorSpace(wxCommandEvent& event);
	void MnuCACorrect(wxCommandEvent& event);
	void MnuHLRecover(wxCommandEvent& event);
	void MnuWhiteBalance(wxCommandEvent& event);
	void MnuToolList(wxCommandEvent& event);
#ifdef USE_LENSFUN
	void MnuLensCorrection(wxCommandEvent& event);
#endif
	void MnuDemosaic(wxCommandEvent& event);
	void UpdateConfig(wxPropertyGridEvent& event);
	void SetStartPath(wxString path);
	void SetBackground();
	void OnSize(wxSizeEvent& event);
	void CharEvent(wxKeyEvent& event);
	wxString getOpenFilePath();

	bool isOpen();
	bool isOpenSource();
	wxFileName getFileName();
	wxFileName getSourceFileName();
	wxString getRootTool();  //returns input filename and any input processing params (':')
	wxString getToolChain();
		
	private:
		void OnClose(wxCloseEvent& event);
		void CreateGUIControls();
		
		bool isDownstream(wxTreeItemId here, wxTreeItemId down);
		
#ifdef SIZERLAYOUT
		wxBoxSizer *hs, *vs;
#else
		wxAuiManager mgr;
#endif
		
		wxTreeCtrl *commandtree;
		PicPanel *pic;
		wxPanel *preview;
		wxSimplebook* parambook;
		myHistogramPane *histogram;
		PropertyDialog *propdiag;

		gImage *d;
		wxImage *img;
		

		bool deleting;
		bool opensource;
		bool open;
		//wxTreeItemId olditem;
		wxTreeItemId displayitem;
		wxFileName filename, sourcefilename;
		wxString configfile;
		wxString openfilepath;

		wxHtmlHelpController help;
};

#endif
