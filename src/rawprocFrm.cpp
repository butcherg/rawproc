///-----------------------------------------------------------------
///
/// @file      rawprocFrm.cpp
/// @author    Glenn
/// Created:   11/18/2015 7:04:06 PM
/// @section   DESCRIPTION
///            rawprocFrm class implementation
///
///------------------------------------------------------------------

#include "rawprocFrm.h"
#include <wx/filedlg.h>
#include <wx/bitmap.h>
#include <wx/artprov.h>
#include <wx/image.h>
#include <wx/imaglist.h>
#include <wx/clipbrd.h>
#include <wx/aboutdlg.h> 
#include <wx/stdpaths.h>
#include <wx/statline.h>
#include <wx/display.h>


#include "PicProcessorSaturation.h"
#include "PicProcessorExposure.h"
#include "PicProcessorCurve.h"
#include "PicProcessorGray.h"
#include "PicProcessorCrop.h"
#include "PicProcessorResize.h"
#include "PicProcessorBlackWhitePoint.h"
#include "PicProcessorSharpen.h"
#include "PicProcessorRotate.h"
#include "PicProcessorDenoise.h"
#include "PicProcessorRedEye.h"
#include "PicProcessorColorSpace.h"
#include "PicProcessorWhiteBalance.h"
#include "PicProcessorTone.h"
#include "PicProcessorSubtract.h"
#include "PicProcessorGroup.h"
#include "PicProcessorCACorrect.h"
#include "PicProcessorHLRecover.h"
#include "PicProcessorAdd.h"
#include "PicProcessorScript.h"
#include "PicProcessorSpot.h"
#include "PicProcessorLensDistortion.h"
#include "PicProcessorLensVignetting.h"

#ifdef USE_GMIC
#include "PicProcessorGMIC.h"
#endif

#include "PicProcessorLensCorrection.h"
#include <locale.h>

#ifdef USE_LENSFUN_DBUPDATE
#include <lensfun/lensfun.h>
#endif

#include "lensfun_dbupdate.h"
#include "PicProcessorDemosaic.h"
#include "myHistogramDialog.h"
#include "myBatchDialog.h"
#include "myEXIFDialog.h"
#include "myConfig.h"
#include "myListDialog.h"
#include "myDataDialog.h"
#include "util.h"
#include "strutil.h"
#include "fileutil.h"
#include "lcms2.h"
#ifdef USE_LCMS_FASTFLOAT
#include "lcms2_fast_float.h"
#endif
#include <omp.h>
#include <exception>

#include "unchecked.xpm"
#include "checked.xpm"
#include "icon.xpm"

#define nullptr 0

//Do not add custom headers between
//Header Include Start and Header Include End
//wxDev-C++ designer will remove them
////Header Include Start
////Header Include End

//----------------------------------------------------------------------------
// rawprocFrm
//----------------------------------------------------------------------------
//Add Custom Events only in the appropriate block.
//Code added in other places will be removed by wxDev-C++
////Event Table Start
BEGIN_EVENT_TABLE(rawprocFrm,wxFrame)
	EVT_SIZE(rawprocFrm::OnSize)
	EVT_MOVE(rawprocFrm::OnMove)
	EVT_CLOSE(rawprocFrm::OnClose)
	EVT_MENU(ID_MNU_OPEN, rawprocFrm::Mnuopen1003Click)
	EVT_MENU(ID_MNU_REOPEN, rawprocFrm::Mnureopen1033Click)
	EVT_MENU(ID_MNU_OPENSOURCE, rawprocFrm::Mnuopensource1004Click)
	EVT_MENU(ID_MNU_SAVE, rawprocFrm::MnuSave)
	EVT_MENU(ID_MNU_SAVEAS, rawprocFrm::MnusaveAs1009Click)
	EVT_MENU(ID_MNU_EXIT, rawprocFrm::MnuexitClick)
	EVT_MENU(ID_MNU_SATURATION, rawprocFrm::MnusaturateClick)
	EVT_MENU(ID_MNU_EXPOSURE, rawprocFrm::MnuexposureClick)
	EVT_MENU(ID_MNU_CURVE, rawprocFrm::Mnucurve1010Click)
	EVT_MENU(ID_MNU_GRAY, rawprocFrm::MnuGrayClick)
	EVT_MENU(ID_MNU_CROP, rawprocFrm::MnuCropClick)
	EVT_MENU(ID_MNU_RESIZE, rawprocFrm::MnuResizeClick)
	EVT_MENU(ID_MNU_BLACKWHITEPOINT, rawprocFrm::MnuBlackWhitePointClick)
	EVT_MENU(ID_MNU_SHARPEN, rawprocFrm::MnuSharpenClick)
	EVT_MENU(ID_MNU_ROTATE, rawprocFrm::MnuRotateClick)
	EVT_MENU(ID_MNU_DENOISE, rawprocFrm::MnuDenoiseClick)
	EVT_MENU(ID_MNU_REDEYE, rawprocFrm::MnuRedEyeClick)
	EVT_MENU(ID_MNU_Cut,rawprocFrm::MnuCut1201Click)
	EVT_MENU(ID_MNU_Copy,rawprocFrm::MnuCopy1202Click)
	EVT_MENU(ID_MNU_Paste,rawprocFrm::MnuPaste1203Click)
	EVT_MENU(ID_MNU_SHOWCOMMAND,rawprocFrm::MnuShowCommand1010Click)
	EVT_MENU(ID_MNU_BATCH,rawprocFrm::MnuBatchClick)
	EVT_MENU(ID_MNU_ABOUT,rawprocFrm::MnuAbout1011Click)
	EVT_MENU(ID_MNU_VIEWHELP,rawprocFrm::MnuHelpClick)
	EVT_MENU(ID_MNU_PROPERTIES,rawprocFrm::MnuProperties)
	EVT_MENU(ID_MNU_EXIF,rawprocFrm::MnuEXIF)
	EVT_MENU(ID_MNU_COLORSPACE, rawprocFrm::MnuColorSpace)
	EVT_MENU(ID_MNU_WHITEBALANCE, rawprocFrm::MnuWhiteBalance)
	EVT_MENU(ID_MNU_TONE, rawprocFrm::MnuTone)
	EVT_MENU(ID_MNU_SUBTRACT, rawprocFrm::MnuSubtract)
	EVT_MENU(ID_MNU_GROUP, rawprocFrm::MnuGroup)
	EVT_MENU(ID_MNU_CACORRECT, rawprocFrm::MnuCACorrect)
	EVT_MENU(ID_MNU_HLRECOVER, rawprocFrm::MnuHLRecover)
	EVT_MENU(ID_MNU_ADDITION, rawprocFrm::MnuAdd)
	EVT_MENU(ID_MNU_SCRIPT, rawprocFrm::MnuScript)
	EVT_MENU(ID_MNU_SPOT, rawprocFrm::MnuSpot)
	EVT_MENU(ID_MNU_LENSDISTORTION, rawprocFrm::MnuLensDistortion)
	EVT_MENU(ID_MNU_LENSVIGNETTING, rawprocFrm::MnuLensVignetting)
	
#ifdef USE_GMIC
	EVT_MENU(ID_MNU_GMIC, rawprocFrm::MnuGMIC)
#endif
	EVT_MENU(ID_MNU_LENSCORRECTION, rawprocFrm::MnuLensCorrection)
#ifdef USE_LENSFUN_DBUPDATE
	EVT_MENU(ID_MNU_DATAUPDATE,rawprocFrm::MnuData)
#endif
	EVT_MENU(ID_MNU_DEMOSAIC, rawprocFrm::MnuDemosaic)
	EVT_MENU(ID_MNU_TOOLLIST, rawprocFrm::MnuToolList)
	EVT_MENU(ID_MNU_EDITMETADATA, rawprocFrm::MnuEditMetadata)
	EVT_TREE_KEY_DOWN(ID_COMMANDTREE,rawprocFrm::CommandTreeKeyDown)
	//EVT_CHAR(rawprocFrm::CharEvent)
	//EVT_CHAR_HOOK(rawprocFrm::CharEvent)
	//EVT_TREE_DELETE_ITEM(ID_COMMANDTREE, rawprocFrm::CommandTreeDeleteItem)
	EVT_TREE_BEGIN_DRAG(ID_COMMANDTREE, rawprocFrm::CommandTreeBeginDrag)
	EVT_TREE_END_DRAG(ID_COMMANDTREE, rawprocFrm::CommandTreeEndDrag)
	EVT_TREE_STATE_IMAGE_CLICK(ID_COMMANDTREE, rawprocFrm::CommandTreeStateClick)
	EVT_TREE_SEL_CHANGED(ID_COMMANDTREE,rawprocFrm::CommandTreeSelChanged)
	EVT_TREE_SEL_CHANGING(ID_COMMANDTREE, rawprocFrm::CommandTreeSelChanging)
	EVT_TREE_ITEM_MENU(ID_COMMANDTREE, rawprocFrm::CommandTreePopup)
END_EVENT_TABLE()
////Event Table End

class myFileDropTarget: public wxFileDropTarget
{
	public:
		myFileDropTarget(rawprocFrm *frm) {
			frame = frm;
		}

		bool OnDropFiles (wxCoord x, wxCoord y, const wxArrayString &filenames)
		{
			if (frame) {
				wxFileName f(filenames[0]);
				f.MakeAbsolute();
				wxSetWorkingDirectory (f.GetPath());
				frame->SetStartPath(f.GetPath());
				//frame->OpenFile(f.GetFullPath());

				if (ImageContainsRawprocCommand(f.GetFullPath())) {
					if (wxMessageBox("Image contains rawproc script.  Open the script?", "Contains Script", wxYES_NO | wxCANCEL | wxNO_DEFAULT) == wxYES)
						frame->OpenFileSource(f.GetFullPath());
					else	
						frame->OpenFile(f.GetFullPath());
				}
				else frame->OpenFile(f.GetFullPath());
				return true;
			}
			return false;
		}

	private:
		rawprocFrm *frame;

};


void MyLogErrorHandler(cmsContext ContextID, cmsUInt32Number code, const char *text)
{
	if ((myConfig::getConfig().getValueOrDefault("app.cms.errorlog","0") == "1")) 
		log(wxString::Format(_("LittleCMS error %d: %s"),code, wxString(text)));
}

rawprocFrm::rawprocFrm(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{
	d = NULL;
	img = NULL;
	pic = NULL;

#ifdef USE_LCMS_FASTFLOAT	
	if (cmsPlugin(cmsFastFloatExtensions()) == 0)
		printf("LittleCMS fast_float plugin load failed.\n");
#endif
	
	wxString startpath = wxString(myConfig::getConfig().getValueOrDefault("app.start.path",""));
	if (startpath != "") 
		if (wxFileName::DirExists(startpath))
			openfilepath = startpath;

	SetDropTarget(new myFileDropTarget(this));

	CreateGUIControls();
#if defined(_OPENMP)
	omp_set_dynamic(0);
#endif
	deleting = false;
	opensource = false;
	open=false;
	displayitem.Unset();
	display_number = wxDisplay::GetFromWindow(this);

	wxImageList *states;
        wxIcon icons[2];
        icons[0] = wxIcon(unchecked_xpm);
        icons[1] = wxIcon(checked_xpm);

        int width  = icons[0].GetWidth(),
            height = icons[0].GetHeight();

        // Make a state image list containing small icons
        states = new wxImageList(width, height, true);
	for ( size_t i = 0; i < WXSIZEOF(icons); i++ )
            states->Add(icons[i]);
	commandtree->AssignStateImageList(states);

	configfile = "(none)";

	//help.UseConfig(wxConfig::Get());
	bool ret;
	help.SetTempDir(wxStandardPaths::Get().GetTempDir());
	wxFileName helpfile( wxStandardPaths::Get().GetExecutablePath());
	helpfile.SetFullName("rawprocdoc.zip");
	//ret = help.AddBook(wxFileName(helpfile));
	ret = help.Initialize(helpfile.GetFullPath());
	if (! ret)
		wxMessageBox(wxString::Format("Failed adding %s",helpfile.GetFullPath()));

	//parm app.cms.errorlog: 0|1, turns on/off LittleCMS error logging to the log file specified in log.filename.  Default=0
	if (myConfig::getConfig().getValueOrDefault("app.cms.errorlog","0") == "1")
		cmsSetLogErrorHandler(MyLogErrorHandler);

	propdiag = nullptr;

	//parm app.window.lastremembered: Modified when rawproc is exited to record the last window state.  Either a [width]x[height] or 'maximized'.  If param.window.rememberlast is 1, rawproc will update this parameter when it is exited.
	std::string lastremembered = myConfig::getConfig().getValueOrDefault("app.window.lastremembered","");
	if (lastremembered != std::string()) {
		if (lastremembered == "maximized") Maximize();
		else if (lastremembered.find('x') != std::string::npos) {
			std::vector<std::string> dim = bifurcate(lastremembered,'x');
			if (dim.size() >=2 && (isInt(dim[0]) & isInt(dim[1]))) {
				unsigned w = atoi(dim[0].c_str());
				unsigned h = atoi(dim[1].c_str());
				SetSize(w,h);
				Refresh();
			}
		}
	}
	Bind(wxEVT_MOVE, &rawprocFrm::OnMove, this);
}

void rawprocFrm::CreateGUIControls()
{
#ifndef SIZERLAYOUT
	mgr.SetManagedWindow(this);
#ifdef WIN32 //until I can figure out why tab traversal doesn't work in Windows...
	mgr.SetFlags(wxAUI_MGR_DEFAULT);
#else
	mgr.SetFlags(wxAUI_MGR_ALLOW_ACTIVE_PANE | wxAUI_MGR_DEFAULT);
#endif
#endif
	//Do not add custom code between
	//GUI Items Creation Start and GUI Items Creation End
	//wxDev-C++ designer will remove them.
	//Add the custom code before or after the blocks
	////GUI Items Creation Start

	WxMenuBar1 = new wxMenuBar();
	wxMenu *ID_MNU_FILEMnu_Obj = new wxMenu();
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_OPEN, _("Open..."), _(""), wxITEM_NORMAL);
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_REOPEN, _("Re-open"), _(""), wxITEM_NORMAL);
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_OPENSOURCE, _("Open Source..."), _(""), wxITEM_NORMAL);
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_SAVE, _("Save\tCtrl+S"), _(""), wxITEM_NORMAL);
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_SAVEAS, _("Save As..."), _(""), wxITEM_NORMAL);
	ID_MNU_FILEMnu_Obj->AppendSeparator();
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_EXIT, _("Exit"), _(""), wxITEM_NORMAL);
	WxMenuBar1->Append(ID_MNU_FILEMnu_Obj, _("File"));

	wxMenu *ID_MNU_EDITMnu_Obj = new wxMenu();
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_Cut,_("Cut"), _(""), wxITEM_NORMAL);
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_Copy,_("Copy"), _(""), wxITEM_NORMAL);
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_Paste,_("Paste"), _(""), wxITEM_NORMAL);
	ID_MNU_EDITMnu_Obj->AppendSeparator();
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_PROPERTIES,_("Properties..."), _(""), wxITEM_NORMAL);
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_EXIF, _("EXIF..."), _(""), wxITEM_NORMAL);
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_EDITMETADATA, _("Edit Metadata..."), _(""), wxITEM_NORMAL);
#ifdef USE_LENSFUN_DBUPDATE
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_DATAUPDATE, _("Data update..."), _(""), wxITEM_NORMAL);
#endif
	ID_MNU_EDITMnu_Obj->Append(ID_MNU_BATCH, _("Batch..."), _(""), wxITEM_NORMAL);
	WxMenuBar1->Append(ID_MNU_EDITMnu_Obj, _("Edit"));

	
	wxMenu *ID_MNU_ADDMnu_Obj = new wxMenu();
	
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_ADDITION,	_("Add"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_BLACKWHITEPOINT,	_("Black/White Point"), _(""), wxITEM_NORMAL);
#ifdef USE_LIBRTPROCESS
	//disable - figure out cacorrect
	//ID_MNU_ADDMnu_Obj->Append(ID_MNU_CACORRECT,	_("CACorrect"), _(""), wxITEM_NORMAL);
#endif
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_COLORSPACE,	_("Colorspace"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_CROP,		_("Crop"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_CURVE,		_("Curve"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_DEMOSAIC,		_("Demosaic"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_DENOISE,	_("Denoise"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_EXPOSURE,	_("Exposure Compensation"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_GAMMA,		_("Gamma"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_GRAY,		_("Gray"), _(""), wxITEM_NORMAL);
#ifdef USE_GMIC
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_GMIC,		_("G'MIC"), _(""), wxITEM_NORMAL);
#endif
#ifdef USE_LIBRTPROCESS
	//disable - figure out hlrecover
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_HLRECOVER,	_("HLRecover"), _(""), wxITEM_NORMAL);
#endif
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_LENSCORRECTION,_("Lens Correction"), _(""), wxITEM_NORMAL);
	//parm tool.lensdistortion.enable = 0/1: Set to 1 and restart rawproc to enable the Lens Distortion tool.  Default: 0
	if (myConfig::getConfig().getValueOrDefault("tool.lensdistortion.enable","0") == "1")
		ID_MNU_ADDMnu_Obj->Append(ID_MNU_LENSDISTORTION,_("Lens Distortion"), _(""), wxITEM_NORMAL);
	//parm tool.lensvignetting.enable = 0/1: Set to 1 and restart rawproc to enable the Lens Vignetting tool.  Default: 0
	if (myConfig::getConfig().getValueOrDefault("tool.lensvignetting.enable","0") == "1")
		ID_MNU_ADDMnu_Obj->Append(ID_MNU_LENSVIGNETTING,_("Lens Vignetting"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_REDEYE,	_("Redeye"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_RESIZE,	_("Resize"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_ROTATE,	_("Rotate"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SATURATION,	_("Saturation"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SCRIPT,	_("Script"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SHARPEN,	_("Sharpen"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SPOT,	_("Spot"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SUBTRACT,	_("Subtract"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_TONE,		_("Tone"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_WHITEBALANCE,	_("White Balance"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->AppendSeparator();
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_GROUP,	_("Group"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_TOOLLIST,	_("Tool List..."), _(""), wxITEM_NORMAL);
	
	
	WxMenuBar1->Append(ID_MNU_ADDMnu_Obj, _("Add"));
	
	wxMenu *ID_MNU_HELPMnu_Obj = new wxMenu();
	ID_MNU_HELPMnu_Obj->Append(ID_MNU_VIEWHELP, _("View Help..."), _(""), wxITEM_NORMAL);
	ID_MNU_HELPMnu_Obj->Append(ID_MNU_SHOWCOMMAND, _("Show Command..."), _(""), wxITEM_NORMAL);
	ID_MNU_HELPMnu_Obj->Append(ID_MNU_ABOUT, _("About..."), _(""), wxITEM_NORMAL);
	WxMenuBar1->Append(ID_MNU_HELPMnu_Obj, _("Help"));
	SetMenuBar(WxMenuBar1);

	WxStatusBar1 = new wxStatusBar(this, ID_WXSTATUSBAR1);
	int widths[4] = {-1,130, 130, 200};
	WxStatusBar1->SetFieldsCount (4, widths);

	SetStatusBar(WxStatusBar1);
	SetTitle(wxString::Format("%s %s",_("rawproc"),VERSION));
	SetIcon(wxNullIcon);
	SetSize(0,0,1200,820);
	Center();
	
	////GUI Items Creation End

	int fr=0, fg=0, fb=0;
	int fontsize = atoi(myConfig::getConfig().getValueOrDefault("app.parameters.fontsize","9").c_str());
	wxString fc = wxString(myConfig::getConfig().getValueOrDefault("app.parameters.fontcolor","0"));
	if (fc == "") fc = "0";
	wxArrayString fntc = split(fc,",");
	fr = atoi(fntc[0].c_str());
	if (fntc.GetCount() < 3) {
		fg = atoi(fntc[0].c_str());
		fb = atoi(fntc[0].c_str());
	}
	else if (fntc.GetCount() == 3) {
		fg = atoi(fntc[1].c_str());
		fb = atoi(fntc[2].c_str());
	}
	

	
	//parm app.dock.width: Specifies width of dock (command, histogram, parameters panels) at startup.  Default=300
	int dockwidth = atoi(wxString(myConfig::getConfig().getValueOrDefault("app.dock.width","300")).c_str());

	//parm app.toolchain.height: Specifies height of tool chain at startup. Change requires restarting rawproc.  Default=200
	int toolheight = atoi(wxString(myConfig::getConfig().getValueOrDefault("app.toolchain.height","200")).c_str());
	//parm app.histogram.height: Specifies height of histogram at startup. Change requires restarting rawproc.  Default=150
	int histheight = atoi(wxString(myConfig::getConfig().getValueOrDefault("app.histogram.height","150")).c_str());
	//parm app.parampane.height: Specifies height of parameter pane at startup. Change requires restarting rawproc. However, parameter pane will grow vertically if the rawproc application window is resized.  Default=350
	int parmheight = atoi(wxString(myConfig::getConfig().getValueOrDefault("app.parampane.height","350")).c_str());

	//Image manipulation panels:
	//commandtree = new wxTreeCtrl(this, ID_COMMANDTREE, wxDefaultPosition, wxSize(dockwidth,toolheight), wxTR_DEFAULT_STYLE | wxTR_HAS_VARIABLE_ROW_HEIGHT | wxTAB_TRAVERSAL | wxBORDER_RAISED);  
	commandtree = new wxTreeCtrl(this, ID_COMMANDTREE, wxDefaultPosition, wxSize(dockwidth,toolheight), wxBORDER_SUNKEN | wxTR_HAS_VARIABLE_ROW_HEIGHT);  
	histogram = new myHistogramPane(this, wxDefaultPosition,wxSize(dockwidth,histheight));
	parambook = new wxSimplebook(this, wxID_ANY, wxDefaultPosition,wxSize(dockwidth,parmheight), wxBORDER_SUNKEN | wxTAB_TRAVERSAL);
	//parambook = new wxSimplebook(this, wxID_ANY, wxDefaultPosition,wxSize(dockwidth,parmheight));

	//Main picture panel:
	pic = new PicPanel(this, commandtree, histogram);

	
	commandtree->SetForegroundColour(wxColour(fr,fg,fb));
	wxFont font(wxFontInfo(fontsize).Family(wxFONTFAMILY_SWISS));
	commandtree->SetFont(font);

#ifdef SIZERLAYOUT
	wxSizerFlags paneflags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5); //.FixedMinSize();  
	wxSizerFlags headerflags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT|wxTOP, 5); //.FixedMinSize();  
	wxSizerFlags paramflags = wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5).Expand(); //.FixedMinSize();  
	wxSizerFlags imageflags = wxSizerFlags(1).Left().Border(wxLEFT|wxRIGHT|wxTOP|wxBOTTOM, 5).Expand(); //.FixedMinSize(); 

	hs = new wxBoxSizer(wxHORIZONTAL);
	vs = new wxBoxSizer(wxVERTICAL);
	vs->Add(new wxStaticText(this,wxID_ANY, "Tool Chain:", wxDefaultPosition, wxSize(100,20)), headerflags);
	vs->Add(commandtree, paneflags);
	vs->Add(new wxStaticText(this,wxID_ANY, "Histogram:", wxDefaultPosition, wxSize(100,20)), headerflags);
	vs->Add(histogram, paneflags);
	vs->Add(new wxStaticText(this,wxID_ANY, "Parameters:", wxDefaultPosition, wxSize(100,20)), headerflags);
	vs->Add(parambook, paramflags);

	hs->Add(vs, wxSizerFlags().Expand());
	hs->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), headerflags.Expand());
	hs->Add(pic, imageflags);
	SetSizerAndFit(hs);
	Layout();
#else
	wxAuiPaneInfo pinfo = wxAuiPaneInfo().Left().CloseButton(false);
#ifdef WIN32
	mgr.AddPane(pic, wxAuiPaneInfo().Center().CloseButton(false).Movable(false));
#else
	mgr.AddPane(pic, wxAuiPaneInfo().Center().Caption("Image").CloseButton(false).Movable(false));
#endif
	mgr.AddPane(commandtree, pinfo.Caption(wxT("Tool Chain:")).Position(0));
	mgr.AddPane(histogram,   pinfo.Caption(wxT("Histogram")).Position(1).Fixed());  //.Resizable());  //Fixed());    //ToDo: myHistogramPane needs a sizer to preserve aspect...  ??
	mgr.AddPane(parambook,   pinfo.Caption(wxT("Parameters")).Position(2).Resizable().MinSize(285,320).FloatingSize(285,320));
	commandtree->SetFocus();
	mgr.Update();
	mgr.Bind(wxEVT_AUI_PANE_ACTIVATED, &rawprocFrm::OnAUIActivate, this);
	mgr.Bind(wxEVT_AUI_PANE_BUTTON, &rawprocFrm::OnPaneButton, this);
#endif



}

#ifndef SIZERLAYOUT
void rawprocFrm::OnPaneButton(wxAuiManagerEvent& event)
{
	//printf("OnAUIButton: %s\n", event.GetPane()->caption.ToStdString().c_str()); fflush(stdout);
}

void rawprocFrm::OnAUIActivate(wxAuiManagerEvent& event)
{
	//printf("OnAUIActivate: %s\n", event.GetPane()->caption.ToStdString().c_str()); fflush(stdout);
}
#endif

void rawprocFrm::ClearParamPane()
{
	while (parambook->GetPageCount()) {
		parambook->GetPage(0)->Freeze();  //maybe a better attempt to mitigate the occasional segfault buried in gtk...
		parambook->DeletePage(0);
		//wxMilliSleep(100); //attempt to mitigate the occasional segfault buried in gtk...
	}
}

void rawprocFrm::OnSize(wxSizeEvent& event)
{
	event.Skip();
	Refresh();
}

void rawprocFrm::OnMove(wxMoveEvent& event)
{
	int new_display_number = wxDisplay::GetFromWindow(this);
	if (display_number != new_display_number) {
		//printf("display changed: %d to %d\n",display_number, new_display_number); fflush(stdout);
		display_number = new_display_number;
		if (pic) pic->RefreshPic();
	}
	event.Skip();
}

bool rawprocFrm::isOpenSource()
{
	return opensource;
}

bool rawprocFrm::isOpen()
{
	return open;
}

void rawprocFrm::SetBackground()
{
	int pr, pg, pb;
	int dr, dg, db;
	wxString f;
	//parm app.backgroundcolor: r,g,b or t (0-255), set at startup. 'r,g,b' specifies a color, 't' specifies a gray tone.  Default=128
	wxString bk = wxString(myConfig::getConfig().getValueOrDefault("app.backgroundcolor","128"));
	if (bk == "") bk = "128";
	wxArrayString bkgnd = split(bk,",");
	pr = atoi(bkgnd[0].c_str());
	if (bkgnd.GetCount() < 3) {
		pg = atoi(bkgnd[0].c_str());
		pb = atoi(bkgnd[0].c_str());
	}
	else {
		pg = atoi(bkgnd[1].c_str());
		pb = atoi(bkgnd[2].c_str());
	}
	dr = pr; dg = pg; db = pb;
	
	//parm app.picpanel.backgroundcolor: r,g,b or t (0-255), set at startup. Same value rules as app.backgroundcolor, overrides it for the picture panel.  Default=(128)
	if (myConfig::getConfig().exists("app.picpanel.backgroundcolor")) {
		wxString pbk = wxString(myConfig::getConfig().getValueOrDefault("app.picpanel.backgroundcolor","128"));
		if (pbk == "") pbk = "128";
		wxArrayString picbkgnd = split(pbk,",");
		pr = atoi(picbkgnd[0].c_str());
		if (picbkgnd.GetCount() < 3) {
			pg = atoi(picbkgnd[0].c_str());
			pb = atoi(picbkgnd[0].c_str());
		}
		else {
			pg = atoi(picbkgnd[1].c_str());
			pb = atoi(picbkgnd[2].c_str());
		}
	}
	
	//parm app.dock.backgroundcolor: r,g,b or t (0-255), set at startup. Same value rules as app.backgroundcolor, overrides it for the command/histogram/parameters dock.  Default=(128)
	if (myConfig::getConfig().exists("app.dock.backgroundcolor")) {
		wxString dbk = wxString(myConfig::getConfig().getValueOrDefault("app.dock.backgroundcolor","128"));
		if (dbk == "") dbk = "128";
		wxArrayString dockbkgnd = split(dbk,",");
		dr = atoi(dockbkgnd[0].c_str());
		if (dockbkgnd.GetCount() < 3) {
			dg = atoi(dockbkgnd[0].c_str());
			db = atoi(dockbkgnd[0].c_str());
		}
		else {
			dg = atoi(dockbkgnd[1].c_str());
			db = atoi(dockbkgnd[2].c_str());
		}
	}

	commandtree->SetBackgroundColour(wxColour(dr,dg,db));
	commandtree->Refresh();
	histogram->SetBackgroundColour(wxColour(dr,dg,db));
	histogram->Refresh();
	parambook->SetBackgroundColour(wxColour(dr,dg,db));
	parambook->Refresh();
	pic->SetBackgroundColour(wxColour(pr,pg,pb));
	pic->Refresh();
	SetBackgroundColour(wxColour(pr,pg,pb));
	Refresh();
	
}

void rawprocFrm::SetStartPath(wxString path)
{
	openfilepath = path;
}

bool rawprocFrm::Shutdown()
{
	if (pic->Modified())
		if (wxMessageBox("Image is modified, continue to exit?", "Confirm", wxYES_NO, this) == wxNO) 
			return false;
			
	//parm app.window.rememberlast: If 1, rawproc will remember the dimensions or maximized window state at exit and restore that when it's re-run. Default: 1
	if(myConfig::getConfig().getValueOrDefault("app.window.rememberlast","1") == "1") {
		if (IsMaximized()) {
			myConfig::getConfig().setValue("app.window.lastremembered","maximized");
		}
		else {
			int w, h;
			GetSize(&w,&h);
			std::string dim = string_format("%dx%d", w, h);
			myConfig::getConfig().setValue("app.window.lastremembered",dim);
		}
		myConfig::getConfig().flush();
	}
			
	commandtree->DeleteAllItems();
	pic->BlankPic();
	histogram->BlankPic();
	//parambook->DeleteAllPages();
	ClearParamPane();
	if ( help.GetFrame() ) // returns NULL if no help frame active
		help.GetFrame()->Close(true);
	// now we can safely delete the config pointer
	delete wxConfig::Set(NULL);
#ifndef SIZERLAYOUT
	mgr.UnInit();
#endif
	return true;
}

void rawprocFrm::OnClose(wxCloseEvent& event)
{
	if (event.CanVeto() & !Shutdown()) {
		event.Veto();
		return;
		
	}
	Destroy();
}

void rawprocFrm::MnuexitClick(wxCommandEvent& event)
{
	event.Skip();
	if (Shutdown())
		Destroy();
}

void rawprocFrm::SetThumbMode(int mode)
{
	pic->SetThumbMode(mode);
}

PicProcessor * rawprocFrm::GetItemProcessor(wxTreeItemId item)
{
	if (item.IsOk()) {
		return (PicProcessor *) commandtree->GetItemData(item);
	}
	else {
		wxMessageBox("bad item");
		return NULL;
	}
}



void rawprocFrm::InfoDialog(wxTreeItemId item)
{
	bool floatstats = false;
	float exp = 0.0, apt = 0.0;
	//parm imageinfo.floatstats: Show image statistic as floats corresponding to the internal image, or integers corresponding to the unsigned 16-bit integer of raw formats.  Default=1
	if (myConfig::getConfig().getValueOrDefault("imageinfo.floatstats","1") == "1")
		floatstats = true;
	wxString exif="";
	gImage dib = ((PicProcessor *) commandtree->GetItemData(item))->getProcessedPic();

	//parm imageinfo.libraw: Include/exclude Libraw-inserted parameters in EXIF.  Default=0 (exclude)
	int librawinclude = atoi(wxString(myConfig::getConfig().getValueOrDefault("imageinfo.libraw","0")).c_str());

	exif.Append(wxString::Format("<b>Image Information:</b><br>\nWidth: %d Height: %d<br>\nColors: %d Original Image Bits: %s<br>\n<br>\n",dib.getWidth(), dib.getHeight(), dib.getColors(), dib.getBitsStr()));

	std::map<std::string,std::string> e =  dib.getInfo();
	for (std::map<std::string,std::string>::iterator it=e.begin(); it!=e.end(); ++it) {
		if (it->first == "ExifTag") continue;
		if (it->first.find("Libraw") != std::string::npos) if (librawinclude == 0) continue;
		if (it->first == "FNumber") apt = atof(it->second.c_str()); 
		
		if (it->first == "ExposureTime") {
			exp = atof(it->second.c_str());
			if (exp >= 1.0)
				exif.Append(wxString::Format("<b>%s:</b> %f sec<br>\n",it->first.c_str(),exp));
			else
				exif.Append(wxString::Format("<b>%s:</b> 1/%d sec<br>\n",it->first.c_str(),int(1.0/exp)));
		}
		else if (it->first == "PhotometricInterpretation") {
			if (it->second == "2") exif.Append(wxString::Format("<b>%s:</b> %s<br>\n",it->first.c_str(),"RGB"));
			else if (it->second == "32803") exif.Append(wxString::Format("<b>%s:</b> %s<br>\n",it->first.c_str(),"CFA"));
			else exif.Append(wxString::Format("<b>%s:</b> %s<br>\n",it->first.c_str(),it->second.c_str()));
		}
		else 
			exif.Append(wxString::Format("<b>%s:</b> %s<br>\n",it->first.c_str(),it->second.c_str()));
		
	}
	char buff[4096];

	exif.Append(wxString::Format("<br><b>Camera EV:</b> %0.2f<br>\n",log2(pow(apt,2)/exp)));


	char *profile = dib.getProfile();
	unsigned profile_length = dib.getProfileLength();

	if (profile_length && profile) {
		cmsHPROFILE icc = cmsOpenProfileFromMem(profile,profile_length);
		if (icc) {
			cmsUInt32Number n =  cmsGetProfileInfoASCII(icc, cmsInfoDescription, "en", "us", buff, 4096);
			exif.Append(wxString::Format("<br>\n<b>ICC Profile:</b> %s (%d)<br>\n", wxString(buff), profile_length));
			cmsCloseProfile(icc);
		}
		else exif.Append(wxString::Format("<br>\n<b>ICC Profile:</b> failed (%d)<br>\n",profile_length));
	}
	else exif.Append(wxString::Format("<br>\n<b>ICC Profile:</b> None (%d)<br>\n",profile_length));

	exif.Append("<hr><b>Image Stats:</b><pre>\n");
	exif.Append(dib.Stats(floatstats).c_str());
	exif.Append("</pre>\n");

	myEXIFDialog dlg(this, wxID_ANY, "Image Information", exif,  wxDefaultPosition, wxSize(400,500));
	dlg.ShowModal();
}

void rawprocFrm::EXIFDialog(wxFileName filename)
{
	if (!filename.FileExists()) return;
	//parm exif.format: text|html - If 'html', exif is displayed with HTML formatting.  If 'text', exif is presented in a filterable list box.  Default=html
	std::string exifformat = myConfig::getConfig().getValueOrDefault("exif.format","html");
	//parm exif.command: Full path/filename to the Phil Harvey exiftool.exe program.  Default=(none), won't work without a valid program.
	wxString exifcommand = wxString(myConfig::getConfig().getValueOrDefault("exif.command",""));
	if (exifcommand == "") {
		wxMessageBox("No exiftool path defined in exif.command");
		return;
	}
	//parm exif.parameters: exiftool parameters used to format the exiftool output.  If the parameter doesn't exist, parameters suitable for the exif.format value will be used.
	wxString exifparameters;
	if (myConfig::getConfig().exists("exif.parameters"))
		exifparameters = wxString(myConfig::getConfig().getValueOrDefault("exif.parameters",""));
	else if (exifformat == "text")
		exifparameters = "-G -S";
	else if (exifformat == "html")
		exifparameters = "-g -h";

	wxString command = wxString::Format("%s %s \"%s\"",exifcommand, exifparameters, filename.GetFullPath());
	wxArrayString output;
	wxArrayString errors;
	SetStatusText(wxString::Format(_("Loading metadata using \"%s\"..."),command));
	wxExecute (command, output, errors, wxEXEC_NODISABLE);
	wxString exif;
	for (int i=0; i<output.GetCount(); i++) exif.Append(output[i]);
	SetStatusText(filename.GetFullName());
	
	if (exifformat == "html") {
		myEXIFDialog dlg(this, wxID_ANY, filename.GetFullName(), exif,  wxDefaultPosition, wxSize(500,500));
		dlg.ShowModal();
	}
	else if (exifformat == "text") {
		myDataDialog dlg(this, wxID_ANY, filename.GetFullName(), output,  wxDefaultPosition, wxSize(500,500));
		dlg.ShowModal();
	}
	else wxMessageBox(wxString::Format("Invalid exif.format value: %s",exifformat));
}

//ToDo: expand to include camconst.json, elle's profiles?  Maybe even a rawproc.conf...
void rawprocFrm::MnuData(wxCommandEvent& event)
{
#ifdef USE_LENSFUN_DBUPDATE
	SetStatusText("Updating lensfun database...");
	std::string lensfundbpath;
	
	lensfundbpath = myConfig::getConfig().getValueOrDefault("tool.lenscorrection.databasepath",getAppConfigDir());
	
	switch (lensfun_dbupdate(LF_MAX_DATABASE_VERSION, lensfundbpath)) {
		case LENSFUN_DBUPDATE_OK:		wxMessageBox("Lens correction database update successful."); break;
		case LENSFUN_DBUPDATE_CURRENTVERSION:	wxMessageBox(wxString::Format(_("Lens correction: Version %d database is current."), 	LF_MAX_DATABASE_VERSION)); break;
		case LENSFUN_DBUPDATE_NOVERSION:	wxMessageBox(wxString::Format(_("Error: Lens correction - Version %d database not available from server."), LF_MAX_DATABASE_VERSION)); break;
		case LENSFUN_DBUPDATE_RETRIEVE_INITFAILED: wxMessageBox(_("Error: Lens correction database retrieve failed (init).")); break;
		case LENSFUN_DBUPDATE_RETRIEVE_FILEOPENFAILED: wxMessageBox(_("Error: Lens correction database retrieve failed (file).")); break;
		case LENSFUN_DBUPDATE_RETRIEVE_RETRIEVEFAILED: wxMessageBox(_("Error: Lens correction database retrieve failed (retrieve).")); break;
	}
		
	SetStatusText("");
#endif
}


//PicProcessor * rawprocFrm::AddItem(wxString name, wxString command, bool display)
wxTreeItemId rawprocFrm::AddItem(wxString name, wxString command, bool display)
{
	SetStatusText("");
	//bool result = true;
	wxTreeItemId id;
	id.Unset();
	PicProcessor *p;
	name.Trim(); command.Trim();

	if (name == "saturation") 			p = new PicProcessorSaturation("saturation",command, commandtree, pic);
	else if (name == "curve")			p = new PicProcessorCurve("curve",command, commandtree, pic);
	else if (name == "cacorrect")		p = new PicProcessorCACorrect("cacorrect",command, commandtree, pic);
	else if (name == "hlrecover")		p = new PicProcessorHLRecover("hlrecover",command, commandtree, pic);
	else if (name == "gray")       		p = new PicProcessorGray("gray",command, commandtree, pic);
	else if (name == "crop")       		p = new PicProcessorCrop("crop",command, commandtree, pic);
	else if (name == "resize")			p = new PicProcessorResize("resize",command, commandtree, pic);
	else if (name == "blackwhitepoint")	p = new PicProcessorBlackWhitePoint("blackwhitepoint",command, commandtree, pic);
	else if (name == "sharpen")     	p = new PicProcessorSharpen("sharpen",command, commandtree, pic);
	else if (name == "rotate")			p = new PicProcessorRotate("rotate",command, commandtree, pic);
	else if (name == "denoise")			p = new PicProcessorDenoise("denoise",command, commandtree, pic);
	else if (name == "redeye")			p = new PicProcessorRedEye("redeye",command, commandtree, pic);
	else if (name == "exposure")		p = new PicProcessorExposure("exposure", command, commandtree, pic);
	else if (name == "colorspace")		p = new PicProcessorColorSpace("colorspace", command, commandtree, pic);
	else if (name == "whitebalance")	p = new PicProcessorWhiteBalance("whitebalance", command, commandtree, pic);
	else if (name == "tone")			p = new PicProcessorTone("tone", command, commandtree, pic);
	else if (name == "add")				p = new PicProcessorAdd("add", command, commandtree, pic);
	else if (name == "subtract")		p = new PicProcessorSubtract("subtract", command, commandtree, pic);
	else if (name == "group")			p = new PicProcessorGroup("group", command, commandtree, pic);
	else if (name == "script")			p = new PicProcessorScript("script", command, commandtree, filename.GetFullName(), pic); 
	else if (name == "spot")			p = new PicProcessorSpot("spot", command, commandtree, pic);
	else if (name == "lensdistortion")	p = new PicProcessorLensDistortion("lensdistortion", command, commandtree, pic);
	else if (name == "lensvignetting")	p = new PicProcessorLensVignetting("lensvignetting", command, commandtree, pic);
#ifdef USE_GMIC
	else if (name == "gmic")			p = new PicProcessorGMIC("gmic", command, commandtree, pic);
#endif
	else if (name == "lenscorrection")	{
		lfDatabase *lfdb =  PicProcessorLensCorrection::findLensfunDatabase();
		if (lfdb)
			p = new PicProcessorLensCorrection(lfdb, "lenscorrection", command, commandtree, pic);
		else
			return id;
	}
	else if (name == "demosaic")		p = new PicProcessorDemosaic("demosaic", command, commandtree, pic);
	else return id;
	id = p->GetId();
	p->createPanel(parambook);
	if (name != "group") p->processPic();  
	if (name == "colorspace") pic->SetProfile(p->getProcessedPicPointer());
	if (name == "resize") pic->SetScale(1.0);
	if (display) CommandTreeSetDisplay(id, 592);
	//if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(), 593);
	Refresh();
	//Update();

	return id;
}

void rawprocFrm::ApplyOps(gImage &dib, wxString operations)
{
	wxArrayString ops = split(operations, " ");
	for (int i=0; i<ops.GetCount(); i++) {
		wxArrayString cmd = split(ops[i], ":");
		if (cmd.GetCount() == 2) {
			if (cmd[0] == "resize") {
				dib.ApplyResize(cmd[1].ToStdString());
			}
			else if (cmd[0] == "sharpen") {
				dib.ApplySharpen(atoi(cmd[1].c_str()));
			}
		}
	}
}



wxString rawprocFrm::AssembleCommand()
{
	SetStatusText("");
	wxString cmd = "rawproc";
	#ifdef VERSION
		cmd.Append(wxString::Format("-%s", VERSION));
	#endif
	cmd.Append(" ");
	wxTreeItemIdValue cookie;
	wxTreeItemId root = commandtree->GetRootItem();
	wxString rootcmd = ((PicProcessor *)commandtree->GetItemData(root))->getCommand();
		cmd.Append(wxString::Format("%s ",rootcmd));
	wxTreeItemId iter = commandtree->GetFirstChild(root, cookie);
	if (iter.IsOk()) {
		cmd.Append(wxString::Format("%s",((PicProcessor *)commandtree->GetItemData(iter))->getCommand()));
		iter = commandtree->GetNextChild(root, cookie);
		while (iter.IsOk()) {
			if (((PicProcessor *)commandtree->GetItemData(iter))->isEnabled()) 
				cmd.Append(wxString::Format("%s",((PicProcessor *)commandtree->GetItemData(iter))->getCommand()));
			iter = commandtree->GetNextChild(root, cookie);
		}
	}

	return cmd;
}

wxString rawprocFrm::getOpenFilePath()
{
	return openfilepath;
}

wxFileName rawprocFrm::getFileName()
{
	return filename;
}

wxFileName rawprocFrm::getSourceFileName()
{
	return sourcefilename;
}

wxString rawprocFrm::getRootTool()
{
	wxTreeItemId root = commandtree->GetRootItem();
	if (root.IsOk()) return ((PicProcessor *)commandtree->GetItemData(root))->getCommand();
	return "";
}

wxString rawprocFrm::getToolChain()
{
	wxString cmd;
	wxTreeItemIdValue cookie;
	wxTreeItemId root = commandtree->GetRootItem();

	wxTreeItemId iter = commandtree->GetFirstChild(root, cookie);
	if (iter.IsOk()) {
		cmd.Append(wxString::Format("%s",((PicProcessor *)commandtree->GetItemData(iter))->getCommand()));
		iter = commandtree->GetNextChild(root, cookie);
		while (iter.IsOk()) {
			if (((PicProcessor *)commandtree->GetItemData(iter))->isEnabled()) 
				cmd.Append(wxString::Format("%s",((PicProcessor *)commandtree->GetItemData(iter))->getCommand()));
			iter = commandtree->GetNextChild(root, cookie);
		}
	}

	return cmd;
}

void rawprocFrm::OpenFile(wxString fname) //, wxString params)
{
#ifdef USE_DCRAW
	gImage::setdcrawPath(myConfig::getConfig().getValueOrDefault("input.raw.dcraw.path","foo"));
#endif
	filename.Assign(fname);
	if (!filename.FileExists()) {
		wxMessageBox(wxString::Format(_("Error: %s not found."),filename.GetFullName()));
		return;
	}
	sourcefilename.Clear();
	gImage *dib = NULL;
	GIMAGE_FILETYPE fif;
	fif = gImage::getFileType(fname.c_str());

	wxFileName profilepath;
	//parm cms.profilepath: Directory path where ICC colorspace profiles can be found.  Default: (none, implies current working directory)
	profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath",openfilepath.ToStdString())));

	if (fif != FILETYPE_UNKNOWN) {

		wxString configparams, inputprofile;
		
		if (fif == FILETYPE_RAW) {
			//parm input.raw.libraw.*: Instead of, or in addition to input.raw.parameters, you can enter any libraw parameter individually, as in input.raw.libraw.bright=2.0.  Note that if you duplicate a parameter here and in input.raw.parameters, the latter will be what's used in processing.
#ifdef USE_DCRAW
			if (myConfig::getConfig().exists("input.raw.libraw.cameraprofile")) {
				wxString camprofile = wxString(myConfig::getConfig().getValueOrDefault("input.raw.libraw.cameraprofile",""));
				if (camprofile != "") {
					configparams.Append("cameraprofile=");
					configparams.Append(camprofile);
					configparams.Append(";");
				}
			}
			configparams.Append("params=");
			configparams.Append(wxString(myConfig::getConfig().getValueOrDefault("input.raw.dcraw.params","")));
#else
			//rawParamString removes all but rawdata and cameraprofile if rawdata=1:
			configparams = rawParamString("input.raw.libraw.");
			//parm input.raw.parameters: name=value list of parameters, separated by semicolons, to pass to the raw image reader.  Default=(none)
			configparams.Append(wxString(myConfig::getConfig().getValueOrDefault("input.raw.parameters","")));
#endif
			//parm input.raw.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			//template input.raw.cms.profile=iccfile
			profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.raw.cms.profile","")));
			
			//parm tool.lenscorrection.camera.enable = 0|1: Enable/disable loading lenscorrection camera metadata and including camera lenscorrection in the lenscorrecton tool. Default = 0;
			int uselenscorrection = atoi(myConfig::getConfig().getValueOrDefault("tool.lenscorrection.camera.enable","0").c_str());
			if (uselenscorrection == 1) {
				configparams.Append("uselenscorrection=1;");
			}
		}
		

		if (fif == FILETYPE_JPEG) {
			//parm input.jpeg.parameters: name=value list of parameters, separated by semicolons, to pass to the JPEG image reader.  Default=(none)
			configparams = wxString(myConfig::getConfig().getValueOrDefault("input.jpeg.parameters",""));
			//parm input.jpeg.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			//template input.jpeg.cms.profile=iccfile
			profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.jpeg.cms.profile","")));
		}

		if (fif == FILETYPE_TIFF) {
			//parm input.tiff.parameters: name=value list of parameters, separated by semicolons, to pass to the TIFF image reader.  Default=(none)
			configparams = wxString(myConfig::getConfig().getValueOrDefault("input.tiff.parameters",""));
			//parm input.tiff.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			//template input.tiff.cms.profile=iccfile
			profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.tiff.cms.profile","")));
		}
		if (fif == FILETYPE_PNG) {
			//parm input.png.parameters: name=value list of parameters, separated by semicolons, to pass to the PNG image reader.  Default=(none)
			configparams = wxString(myConfig::getConfig().getValueOrDefault("input.png.parameters",""));
			//parm input.png.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			//template input.png.cms.profile=iccfile
			profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.png.cms.profile","")));
		}

		SetStatusText(wxString::Format(_("Loading file:%s params:%s"),filename.GetFullName(), configparams));

		if (!wxFileName::FileExists(fname)) {
			wxMessageBox(wxString::Format(_("Error: Source file %s not found"), filename.GetFullName()));
			SetStatusText("");
			return;
		}

		pic->BlankPic();
		histogram->BlankPic();
		//parambook->DeleteAllPages();
		ClearParamPane();
		
		mark();
		if (dib) delete dib;
		dib = new gImage(gImage::loadImageFile(fname.c_str(), (std::string) configparams.c_str()));
		wxString loadtime = duration();
		if (dib->getWidth() == 0) {
			wxMessageBox(wxString::Format(_("Error: File %s not loaded successfully"), filename.GetFullName()));
			SetStatusText("");
			return;
		}
		
		//parm input.[jpeg|tiff|png].orient: Rotate the image to represent the EXIF Orientation value originally inputted, then set the Orientation tag to 1.  Gets the image out of trying to tell other software how to orient it.  Can be set for jpeg, tiff, or png images only; raw images have to wait until after demosaic to preserve the CFA orientation (see tool.demosaic.orient).  Default=0
		if (fif == FILETYPE_JPEG) {
			if (myConfig::getConfig().getValueOrDefault("input.jpeg.orient","0") == "1") {
				WxStatusBar1->SetStatusText(wxString::Format(_("Normalizing jpeg image orientation...")));
				dib->NormalizeRotation();
			}
		}
		else if (fif == FILETYPE_TIFF) {
			if (myConfig::getConfig().getValueOrDefault("input.tiff.orient","0") == "1") {
				WxStatusBar1->SetStatusText(wxString::Format(_("Normalizing tiff image orientation...")));
				dib->NormalizeRotation();
			}
		}
		else if (fif == FILETYPE_PNG) {
			if (myConfig::getConfig().getValueOrDefault("input.png.orient","0") == "1") {
				WxStatusBar1->SetStatusText(wxString::Format(_("Normalizing png image orientation...")));
				dib->NormalizeRotation();
			}
		}
		
		//wxString flagstring(params.c_str());
		//parm input.log: log the file input operation.  Default=0
		if (myConfig::getConfig().getValueOrDefault("input.log","0") == "1")
			log(wxString::Format("file input,filename=%s,imagesize=%dx%d,time=%s",filename.GetFullName(),dib->getWidth(), dib->getHeight(),loadtime));


		PicProcessor *picdata = new PicProcessor(filename.GetFullName(), configparams, commandtree, pic, dib);
		picdata->createPanel(parambook);
		//pic->SetScaleToWidth();
		if (pic->GetSize().GetWidth() > dib->getWidth()) {
			pic->SetScale(1.0);
		}
		else {
			pic->FitMode(true);
			SetStatusText("scale: fit",STATUS_SCALE);
		}
		CommandTreeSetDisplay(picdata->GetId(), 790);
		SetTitle(wxString::Format("%s %s: %s",_("rawproc"),VERSION,filename.GetFullName()));
		SetStatusText("");

		//parm input.raw.default: Space-separated list of rawproc tools to apply to a raw image after it is input. If this parameter has an entry, application of the tools is prompted yes/no.  Default=(none). <ul><li>Camera-specific default processing can be specified by appending '.Make_Model', or just '.Make' to the property name, where make and model identify the camera as these values appear in the raw metadata.  Put an underscore between the make and model, and substitute underscore for any spaces that occur in either value, e.g., Nikon_Z_6.</li><li>If a raw file was originally opened with this parameter, if it is re-opened, you'll be prompted to apply the input.raw.default.commands, then prompted to re-apply the processing chain.  In this case, say 'no' to the first one, and 'yes' to the second, otherwise you'll duplicate the input.raw.default commands.</li></ul>"
		//template input.raw.default=longstring
		//template input.raw.default.*=longstring
		std::string makemodel = std::string("input.raw.default")+"."+underscore(dib->getInfoValue("Make")) + "_" + underscore(dib->getInfoValue("Model"));
		std::string make = std::string("input.raw.default")+"."+underscore(dib->getInfoValue("Make"));
		wxString raw_default, raw_default_source; 
		if (myConfig::getConfig().exists(makemodel)) {
			raw_default = wxString(myConfig::getConfig().getValueOrDefault(makemodel, ""));
			raw_default_source = wxString(makemodel);
		}
		else if (myConfig::getConfig().exists(make)) {
			raw_default = wxString(myConfig::getConfig().getValueOrDefault(make, ""));
			raw_default_source = wxString(make);
		}
		else {
			raw_default = wxString(myConfig::getConfig().getValueOrDefault("input.raw.default","colorspace:camera,assign subtract:camera whitebalance:camera demosaic:proof blackwhitepoint:rgb,data"));
			raw_default_source = "input.raw.default";
		}
		
		pic->SetModified(false);

		//parm app.incrementaldisplay: 0|1, for addition of tool sequences, if 1, the display is rendered as each tool is added. if 0, the display render is deferred until the last tool is added.  Default=0;
		bool incdisplay=false;
		if (myConfig::getConfig().getValueOrDefault("app.incrementaldisplay","0") == "1") incdisplay = true;

		if ((fif == FILETYPE_RAW) & (raw_default != "")) {
			int applydefault = wxYES;
			//parm input.raw.default.prompt: 1|0, enable/disable prompt to apply input.raw.default.  Default=1 
			if (myConfig::getConfig().getValueOrDefault("input.raw.default.prompt","1") == "1") 
				applydefault = wxMessageBox(wxString::Format(_("Apply %s to raw file?"),raw_default), raw_default_source, wxYES_NO, this);
			if (applydefault == wxYES) {
				wxArrayString token = split(raw_default, " ");
				try {
					for (int i=0; i<token.GetCount(); i++) {
						std::vector<std::string> cmd = bifurcate(token[i].ToStdString(),':');
						if (cmd.size() == 2)
							AddItem(wxString(cmd[0]), wxString(cmd[1]), incdisplay);
						else
							AddItem(wxString(cmd[0]), "", incdisplay);
						wxSafeYield(this);
					}
					if (!incdisplay) CommandTreeSetDisplay(commandtree->GetLastChild(commandtree->GetRootItem()),787);
				}
				catch (std::exception& e) {
					wxMessageBox(wxString::Format(_("Error: Adding tool failed: %s"),e.what()));
				}
			}
		}
		
		opensource = false;
		open = true;

		SetStatusText(wxString::Format(_("File:%s opened."),filename.GetFullName()));
		commandtree->SetFocus();
	}
	else {
		SetStatusText(wxString::Format(_("%s file type unknown."),filename.GetFullName() ));
	}
}

void rawprocFrm::OpenFileSource(wxString fname)
{
#ifdef USE_DCRAW
	gImage::setdcrawPath(myConfig::getConfig().getValueOrDefault("input.raw.dcraw.path","foo"));
#endif
	gImage *dib;
	wxString ofilename, inputprofile;
	wxString oparams;

	SetStatusText("Retrieving source script...");
	std::map<std::string,std::string> info =  gImage::getInfo(fname.c_str());

	if(info.find("ImageDescription") != info.end() && info["ImageDescription"].find("rawproc") != std::string::npos ) {
		wxString script = info["ImageDescription"];
		wxArrayString token = split(script, " ");
			
		if (token[1].Contains(":")) {
			//wxArrayString fparams = split(token[1],":");
			//std::vector<std::string> fparams = bifurcate(token[1].ToStdString(), ':', true);
			wxArrayString fparams = inputfilecommand(token[1]);
			if (fparams.GetCount() >1) {
			//if (fparams.size() >1) {
				oparams = wxString(fparams[1]);
			}
			ofilename = wxString(fparams[0]);
		}
		else ofilename = token[1];
				
		if (token[0].Find("rawproc") == wxNOT_FOUND) {
			wxMessageBox(wxString::Format(_("Source script not found in %s, aborting Open Source."), filename.GetFullName().c_str()) );
		}
		else {
			SetStatusText(wxString::Format(_("Source script found, loading source file %s..."),ofilename) );


			//Now, searches in the current directory, the parent directory, and all subdirectories for the source file.  
			//uses fileutil.cpp::find_filepath().  Needs C++17...
			//input.opensource.parentdirectory and input.opensource.subdirectory are no longer needed...
			std::string ofile =  find_filepath(ofilename.ToStdString());
			if (ofile == "(none)") {
				wxMessageBox(wxString::Format(_("Error: Source file %s not found in source, parent or sub directory"), ofilename));
				SetStatusText("");
				return;
			}
			ofilename = wxString(ofile);

			GIMAGE_FILETYPE fif;
			fif = gImage::getFileType(ofilename.c_str());

			wxFileName profilepath;
			profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath",openfilepath.ToStdString())));

			if (fif == FILETYPE_RAW) {
				profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.raw.cms.profile","")));
				int uselenscorrection = atoi(myConfig::getConfig().getValueOrDefault("tool.lenscorrection.camera.enable","0").c_str());
				if (uselenscorrection == 1) {
					oparams.Append("uselenscorrection=1;");
				}
			}
			else if (fif == FILETYPE_JPEG) {
				profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.jpeg.cms.profile","")));
			}
			else if (fif == FILETYPE_TIFF) {
				profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.tiff.cms.profile","")));
			}
			else if (fif == FILETYPE_PNG) {
				profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.png.cms.profile","")));
			}

			pic->BlankPic();
			histogram->BlankPic();
			//parambook->DeleteAllPages();
			ClearParamPane();
			SetStatusText(wxString::Format(_("Loading file:%s params:%s"),ofilename, oparams));

			mark();
			dib = new gImage(gImage::loadImageFile(ofilename.c_str(), (std::string) oparams.c_str()));
			wxString loadtime = duration();
			if (dib->getWidth() == 0) {
				wxMessageBox(wxString::Format(_("Error: File %s load failed"), ofilename));
				SetStatusText("");
				return;
			}
			
			if (fif == FILETYPE_JPEG) {
				if (myConfig::getConfig().getValueOrDefault("input.jpeg.orient","0") == "1") {
					WxStatusBar1->SetStatusText(wxString::Format(_("Normalizing jpeg image orientation...")));
					dib->NormalizeRotation();
				}
			}
			else if (fif == FILETYPE_TIFF) {
				if (myConfig::getConfig().getValueOrDefault("input.tiff.orient","0") == "1") {
					WxStatusBar1->SetStatusText(wxString::Format(_("Normalizing tiff image orientation...")));
					dib->NormalizeRotation();
				}
			}
			else if (fif == FILETYPE_PNG) {
				if (myConfig::getConfig().getValueOrDefault("input.png.orient","0") == "1") {
					WxStatusBar1->SetStatusText(wxString::Format(_("Normalizing png image orientation...")));
					dib->NormalizeRotation();
				}
			}
			
			filename.Assign(ofilename);
			sourcefilename.Assign(fname);
			
			if (myConfig::getConfig().getValueOrDefault("input.log","0") == "1")
				log(wxString::Format(_("file input,filename=%s,imagesize=%dx%d,time=%s"),filename.GetFullName(),dib->getWidth(), dib->getHeight(),loadtime));


			//pic->SetScaleToWidth();
			if (pic->GetSize().GetWidth() > dib->getWidth()) {
				pic->SetScale(1.0);
			}
			else {
				pic->FitMode(true);
				SetStatusText(_("scale: fit"),STATUS_SCALE);
			}

			
			bool incdisplay=false;
			if (myConfig::getConfig().getValueOrDefault("app.incrementaldisplay","0") == "1") incdisplay = true;

			PicProcessor *picdata = new PicProcessor(filename.GetFullName(), oparams, commandtree, pic, dib);
			picdata->createPanel(parambook);
			if (incdisplay) CommandTreeSetDisplay(picdata->GetId(),933);
			SetTitle(wxString::Format("%s %s: %s (%s)",_("rawproc"), VERSION, filename.GetFullName(), sourcefilename.GetFullName()));

			for (int i=2; i<token.GetCount(); i++) {
				std::vector<std::string> cmd = bifurcate(token[i].ToStdString(),':');
				if (cmd.size() == 2) 
					AddItem(wxString(cmd[0]), wxString(cmd[1]), incdisplay);
				else
					AddItem(wxString(cmd[0]), "", incdisplay);
				wxSafeYield(this);
				
			}
			if (!incdisplay) CommandTreeSetDisplay(commandtree->GetLastChild(commandtree->GetRootItem()),945);
			
			opensource = true;
			open = false;
			
			pic->SetModified(false);

			SetStatusText(wxString::Format(_("Source of file:%s opened."),sourcefilename.GetFullName()));
			commandtree->SetFocus();
		}
			
	}
	else {
		wxMessageBox(wxString::Format(_("No source script found in %s, aborting Open Source."),fname ));
	}

	SetStatusText("");
}

void rawprocFrm::MnuSave(wxCommandEvent& event)
{
	wxString fname;
	gImage * dib;
	cmsHPROFILE profile;

	wxFileName profilepath;
	profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","")));
	std::string iccfile;

	if (sourcefilename.IsOk()) {
		fname = sourcefilename.GetFullPath();
	}
	else if (filename.IsOk()) {
		if (filename.GetExt() == wxEmptyString)
			filename.SetExt(wxString(myConfig::getConfig().getValueOrDefault("output.defaultfileextension","jpg")));
		fname = filename.GetFullPath();
	}
	else {
		WxStatusBar1->SetStatusText(wxString::Format(_("No file to save."),fname));
		return;
	}

	if ( !fname.empty() )
	{
		//if (wxFileName::FileExists(fname)) 
		//	if (wxMessageBox(_("File exists; overwrite?"), _("Confirm"), wxYES_NO, this) == wxNO)
		//		return;
			
			GIMAGE_FILETYPE filetype = gImage::getFileNameType(fname.ToStdString().c_str());
			
			if (filetype == FILETYPE_UNKNOWN) {
				wxMessageBox(_("Error: invalid file type"));
				return;
			}
			if (commandtree->ItemHasChildren(commandtree->GetRootItem()))
				dib = ((PicProcessor *) commandtree->GetItemData( commandtree->GetLastChild(commandtree->GetRootItem())))->getProcessedPicPointer();
			else
				dib = ((PicProcessor *) commandtree->GetItemData( commandtree->GetRootItem()))->getProcessedPicPointer();

			dib->setInfo("ImageDescription",(std::string) AssembleCommand().c_str());
			wxString versionstr = _("(dev build)");
			#ifdef VERSION
				versionstr = VERSION;
			#endif
			dib->setInfo("Software",(std::string) wxString::Format("rawproc %s",versionstr).c_str());
			
			//parm output.orient: Rotate the image to represent the EXIF Orientation value originally inputted, then set the Orientation tag to 0.  Gets the image out of trying to tell other software how to orient it.  Default=0
			if (myConfig::getConfig().getValueOrDefault("output.orient","0") == "1") {
				WxStatusBar1->SetStatusText(wxString::Format(_("Orienting image for output...")));
				dib->NormalizeRotation();
			}

			wxString configparams;
			//parm output.*.thumbnails.directory: *=all|jpeg|tiff|png, specifies a directory subordinate to the image directory where a thumbnail depiction is to be stored.  Default="", which inhibits thumbnail creation.  "all" is trumped by presence of any of the others.
			wxString thumbdir = myConfig::getConfig().getValueOrDefault("output.all.thumbnails.directory","");
			//parm output.*.thumbnails.parameters: *=all|jpeg|tiff|png, specifies space-separated list of rawproc tools to be applied to the image to make the thumbnail.  Default="resize:120 sharpen=1". "all" is trumped by presence of any of the others.
			wxString thumbparams = myConfig::getConfig().getValueOrDefault("output.all.thumbnails.parameters","");
			if (filetype == FILETYPE_JPEG) {
				//parm output.jpeg.parameters: name=value list of parameters, separated by semicolons, to pass to the JPEG image writer.  Applicable parameters: <ul><li>quality=n, 0-100: Specifies the image compression in terms of a percent.</li></ul> Default:quality=95.
				configparams = myConfig::getConfig().getValueOrDefault("output.jpeg.parameters","quality=95");
				thumbdir = myConfig::getConfig().getValueOrDefault("output.jpeg.thumbnails.directory",thumbdir.ToStdString());
				thumbparams = myConfig::getConfig().getValueOrDefault("output.jpeg.thumbnails.parameters",thumbparams.ToStdString());
			}

			if (filetype == FILETYPE_TIFF) {
				//parm output.tiff.parameters: name=value list of parameters, separated by semicolons, to pass to the TIFF image writer. Applicable parameters: <ul><li>channelformat=8bit|16bit|float: Specifies the output numeric format.  For float TIFFs, the data is saved 'unbounded', that is, not clipped to 0.0-1.0 IF the output.tiff.cms.profile is set to a matrix profile.</li></ul>
				configparams =  myConfig::getConfig().getValueOrDefault("output.tiff.parameters","");
				thumbdir = myConfig::getConfig().getValueOrDefault("output.tiff.thumbnails.directory",thumbdir.ToStdString());
				thumbparams = myConfig::getConfig().getValueOrDefault("output.tiff.thumbnails.parameters",thumbparams.ToStdString());
			}

			if (filetype == FILETYPE_PNG) {
				//parm output.png.parameters: name=value list of parameters, separated by semicolons, to pass to the PNG image writer.  Applicable parameters: <ul><li>channelformat=8bit|16bit:   Specifies the output numeric format.</li></ul>
				configparams =  myConfig::getConfig().getValueOrDefault("output.png.parameters","");
				thumbdir = myConfig::getConfig().getValueOrDefault("output.png.thumbnails.directory",thumbdir.ToStdString());
				thumbparams = myConfig::getConfig().getValueOrDefault("output.png.thumbnails.parameters",thumbparams.ToStdString());
			}
			if (filetype == FILETYPE_DATA) {
				//parm output.data.parameters: name=value list of parameters, separated by semicolons, to pass to the data writer.   Applicable parameters: <ul><li>outputmode=rgb|split|channelsummary: Specifies the output format. Default: rgb</li></ul>
				configparams =  myConfig::getConfig().getValueOrDefault("output.data.parameters","");
			}

			GIMAGE_ERROR result;
			//parmdontuse output.embedprofile: Embed/don't embed ICC profile with image, 0|1. If an ouput.*.cms.profile is specified, the internal image is converted to that profile and that file is embedded with the profile, otherwise, if a profile is assigned in the internal image, that profile is embedded.   Default=1  (Note: use excludeicc in the output params property instead...)
			//if (myConfig::getConfig().getValueOrDefault("output.embedprofile","1") == "1") {

				wxString intentstr;
				cmsUInt32Number intent = INTENT_PERCEPTUAL;

				if (filetype == FILETYPE_JPEG) {
					//parm output.jpeg.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=srgb
					//template output.jpeg.cms.profile=iccfile
					iccfile = myConfig::getConfig().getValueOrDefault("output.jpeg.cms.profile","");
					
					//parm output.jpeg.cms.renderingintent: Specify the rendering intent for the JPEG output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=relative_colorimetric
					//template output.jpeg.cms.renderingintent=relative_colorimetric|absolute_colorimetric|perceptual|saturation
					intentstr = wxString(myConfig::getConfig().getValueOrDefault("output.jpeg.cms.renderingintent","relative_colorimetric"));

				}
				else if (filetype == FILETYPE_TIFF) {
					//parm output.tiff.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=prophoto
					//template output.tiff.cms.profile=iccfile
					iccfile = myConfig::getConfig().getValueOrDefault("output.tiff.cms.profile","");
					
					//parm output.tiff.cms.renderingintent: Specify the rendering intent for the TIFF output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=relative_colorimetric
					//template output.tiff.cms.renderingintent=relative_colorimetric|absolute_colorimetric|perceptual|saturation
					intentstr = wxString(myConfig::getConfig().getValueOrDefault("output.tiff.cms.renderingintent","relative_colorimetric"));
				}
				else if (filetype == FILETYPE_PNG) {
					//parm output.png.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=prophoto
					//template output.png.cms.profile=iccfile
					iccfile = myConfig::getConfig().getValueOrDefault("output.png.cms.profile","");
					
					//parm output.png.cms.renderingintent: Specify the rendering intent for the PNG output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=relative_colorimetric
					//template output.png.cms.renderingintent=relative_colorimetric|absolute_colorimetric|perceptual|saturation
					intentstr = wxString(myConfig::getConfig().getValueOrDefault("output.png.cms.renderingintent","relative_colorimetric"));
				}
				
				if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
				if (intentstr == "saturation") intent = INTENT_SATURATION;
				if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
				if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

				//profile = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
				
				if (iccfile == "srgb" | iccfile == "wide" | iccfile == "adobe" | iccfile == "prophoto" | iccfile == "identity")
					profile = gImage::makeLCMSProfile(iccfile, 1.0);
				else if (iccfile == "aces2065-1-v4-g10" | iccfile == "adobergb-v4-g10" | iccfile == "bt709-v4-g10" | iccfile == "prophoto-v4-g10" | iccfile == "rec2020-v4-g10" | iccfile == "srgb-v4-g10" | iccfile == "srgb-v2-g22" | iccfile == "srgb-output")
					profile = gImage::makeLCMSStoredProfile(iccfile);
				else {
					profilepath.SetFullName(wxString(iccfile));
					profile = gImage::myCmsOpenProfileFromFile(profilepath.GetFullPath().ToStdString());
				}
				
				if (dib->getProfile()) {
					if (profile) {
						WxStatusBar1->SetStatusText(wxString::Format(_("Saving %s converting to color profile %s, rendering intent %s..."),fname, profilepath.GetFullName(), intentstr));
						result = dib->saveImageFile(fname.ToStdString().c_str(), std::string(configparams.c_str()), profile, intent);
					}
					else {
						//wxMessageBox(wxString::Format(_("No CMS profile file found, saving with the assigned internal color profile...")));
						WxStatusBar1->SetStatusText(wxString::Format(_("Saving %s with working profile..."),fname));
						result = dib->saveImageFile(fname.ToStdString().c_str(), std::string(configparams.c_str()));
					}
				}
				else {
					//wxMessageBox(wxString::Format(_("No internal working profile found, saving without a color profile...")));
					WxStatusBar1->SetStatusText(wxString::Format(_("Saving %s without a color profile..."),fname));
					result = dib->saveImageFile(fname.ToStdString().c_str(), std::string(configparams.c_str()));
				}
			//}
			//else {
			//	WxStatusBar1->SetStatusText(wxString::Format(_("Saving %s (no embedded profile)..."),fname));
			//	dib->saveImageFileNoProfile(fname, std::string(configparams.c_str()));
			//}
			if (result == GIMAGE_UNSUPPORTED_FILEFORMAT) {
				wxMessageBox("Error: Unsupported format.");
				return;
			}
			if (result == GIMAGE_EXIV2_METADATAWRITE_FAILED) {
				wxMessageBox("Image saved, but metadata insertion failed.");
			}
			
			pic->SetModified(false);
			
			wxFileName tmpname(fname);
			
			if (thumbdir != "") {
				wxFileName thumb(tmpname.GetPath(wxPATH_GET_SEPARATOR)+thumbdir, tmpname.GetFullName());
				if (thumb.DirExists()) {
					WxStatusBar1->SetStatusText(wxString::Format(_("Saving thumbnail %s..."),fname));
					gImage thumbdib = *dib;
					ApplyOps(thumbdib, thumbparams);
					thumbdib.saveImageFile(thumb.GetFullPath().ToStdString().c_str());
				}					
			}

			if (tmpname.GetFullName().compare(filename.GetFullName()) != 0) {
				sourcefilename.Assign(fname);
				SetTitle(wxString::Format("rawproc: %s (%s)",filename.GetFullName().c_str(), sourcefilename.GetFullName().c_str()));
				SetTitle(wxString::Format("%s %s: %s (%s)",_("rawproc"), VERSION, filename.GetFullName(), sourcefilename.GetFullName()));
			}
			
		WxStatusBar1->SetStatusText("");
	}
}

void rawprocFrm::MnusaveAs1009Click(wxCommandEvent& event)
{
	wxString fname;
	gImage * dib;
	cmsHPROFILE profile;

	wxFileName profilepath;
	profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","")));
	std::string iccfile;
	
	if (filename.GetExt() == wxEmptyString)\
		//param output.defaultfileextension: The default file extension to use in the File->Save dialog.  Default=jpg
		filename.SetExt(wxString(myConfig::getConfig().getValueOrDefault("output.defaultfileextension","jpg")));

	if (!sourcefilename.IsOk()) 
		fname = wxFileSelector(_("Save image..."),filename.GetPath(),filename.GetName(),filename.GetExt(),_("JPEG files (*.jpg)|*.jpg|TIFF files (*.tif)|*.tif|PNG files (*.png)|*.png |Data files (*.csv)|*.csv"),wxFD_SAVE);  // 
	else
		fname = wxFileSelector(_("Save image..."),sourcefilename.GetPath(),sourcefilename.GetName(),sourcefilename.GetExt(),_("JPEG files (*.jpg)|*.jpg|TIFF files (*.tif)|*.tif|PNG files (*.png)|*.png"),wxFD_SAVE);  // 

	if ( !fname.empty() )
	{
		if (wxFileName::FileExists(fname)) 
			if (wxMessageBox(_("File exists; overwrite?"), _("Confirm"), wxYES_NO, this) == wxNO)
				return;
			
			GIMAGE_FILETYPE filetype = gImage::getFileNameType(fname.ToStdString().c_str());
			
			if (filetype == FILETYPE_UNKNOWN) {
				wxMessageBox(_("Error: invalid file type"));
				return;
			}
			if (commandtree->ItemHasChildren(commandtree->GetRootItem()))
				dib = ((PicProcessor *) commandtree->GetItemData( commandtree->GetLastChild(commandtree->GetRootItem())))->getProcessedPicPointer();
			else
				dib = ((PicProcessor *) commandtree->GetItemData( commandtree->GetRootItem()))->getProcessedPicPointer();

			dib->setInfo("ImageDescription",(std::string) AssembleCommand().c_str());
			wxString versionstr = _("(dev build)");
			#ifdef VERSION
				versionstr = VERSION;
			#endif
			dib->setInfo("Software",(std::string) wxString::Format("rawproc %s",versionstr).c_str());
			
			//parm output.orient: Rotate the image to represent the EXIF Orientation value originally inputted, then set the Orientation tag to 0.  Gets the image out of trying to tell other software how to orient it.  Default=0
			if (myConfig::getConfig().getValueOrDefault("output.orient","0") == "1") {
				WxStatusBar1->SetStatusText(wxString::Format(_("Orienting image for output...")));
				dib->NormalizeRotation();
			}

			wxString configparams;
			//parm output.*.thumbnails.directory: *=all|jpeg|tiff|png, specifies a directory subordinate to the image directory where a thumbnail depiction is to be stored.  Default="", which inhibits thumbnail creation.  "all" is trumped by presence of any of the others.
			wxString thumbdir = myConfig::getConfig().getValueOrDefault("output.all.thumbnails.directory","");
			//parm output.*.thumbnails.parameters: *=all|jpeg|tiff|png, specifies space-separated list of rawproc tools to be applied to the image to make the thumbnail.  Default="resize:120 sharpen=1". "all" is trumped by presence of any of the others.
			wxString thumbparams = myConfig::getConfig().getValueOrDefault("output.all.thumbnails.parameters","");
			if (filetype == FILETYPE_JPEG) {
				//parm output.jpeg.parameters: name=value list of parameters, separated by semicolons, to pass to the JPEG image writer.  Applicable parameters: <ul><li>quality=n, 0-100: Specifies the image compression in terms of a percent.</li></ul> Default:quality=95.
				configparams = myConfig::getConfig().getValueOrDefault("output.jpeg.parameters","quality=95");
				thumbdir = myConfig::getConfig().getValueOrDefault("output.jpeg.thumbnails.directory",thumbdir.ToStdString());
				thumbparams = myConfig::getConfig().getValueOrDefault("output.jpeg.thumbnails.parameters",thumbparams.ToStdString());
			}

			if (filetype == FILETYPE_TIFF) {
				//parm output.tiff.parameters: name=value list of parameters, separated by semicolons, to pass to the TIFF image writer. Applicable parameters: <ul><li>channelformat=8bit|16bit|float: Specifies the output numeric format.  For float TIFFs, the data is saved 'unbounded', that is, not clipped to 0.0-1.0 IF the output.tiff.cms.profile is set to a matrix profile.</li></ul>
				configparams =  myConfig::getConfig().getValueOrDefault("output.tiff.parameters","");
				thumbdir = myConfig::getConfig().getValueOrDefault("output.tiff.thumbnails.directory",thumbdir.ToStdString());
				thumbparams = myConfig::getConfig().getValueOrDefault("output.tiff.thumbnails.parameters",thumbparams.ToStdString());
			}

			if (filetype == FILETYPE_PNG) {
				//parm output.png.parameters: name=value list of parameters, separated by semicolons, to pass to the PNG image writer.  Applicable parameters: <ul><li>channelformat=8bit|16bit:   Specifies the output numeric format.</li></ul>
				configparams =  myConfig::getConfig().getValueOrDefault("output.png.parameters","");
				thumbdir = myConfig::getConfig().getValueOrDefault("output.png.thumbnails.directory",thumbdir.ToStdString());
				thumbparams = myConfig::getConfig().getValueOrDefault("output.png.thumbnails.parameters",thumbparams.ToStdString());
			}
			if (filetype == FILETYPE_DATA) {
				//parm output.data.parameters: name=value list of parameters, separated by semicolons, to pass to the data writer.   Applicable parameters: <ul><li>outputmode=rgb|split|channelsummary: Specifies the output format. Default: rgb</li></ul>
				configparams =  myConfig::getConfig().getValueOrDefault("output.data.parameters","");
			}

			GIMAGE_ERROR result;
			//parmdontuse output.embedprofile: Embed/don't embed ICC profile with image, 0|1. If an ouput.*.cms.profile is specified, the internal image is converted to that profile and that file is embedded with the profile, otherwise, if a profile is assigned in the internal image, that profile is embedded.   Default=1  (Note: use excludeicc in the output params property instead...)
			//if (myConfig::getConfig().getValueOrDefault("output.embedprofile","1") == "1") {

				wxString intentstr;
				cmsUInt32Number intent = INTENT_PERCEPTUAL;

				if (filetype == FILETYPE_JPEG) {
					//parm output.jpeg.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=srgb
					//template output.jpeg.cms.profile=iccfile
					iccfile = myConfig::getConfig().getValueOrDefault("output.jpeg.cms.profile","");
					
					//parm output.jpeg.cms.renderingintent: Specify the rendering intent for the JPEG output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=relative_colorimetric
					//template output.jpeg.cms.renderingintent=relative_colorimetric|absolute_colorimetric|perceptual|saturation
					intentstr = wxString(myConfig::getConfig().getValueOrDefault("output.jpeg.cms.renderingintent","relative_colorimetric"));

				}
				else if (filetype == FILETYPE_TIFF) {
					//parm output.tiff.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=prophoto
					//template output.tiff.cms.profile=iccfile
					iccfile = myConfig::getConfig().getValueOrDefault("output.tiff.cms.profile","");
					
					//parm output.tiff.cms.renderingintent: Specify the rendering intent for the TIFF output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=relative_colorimetric
					//template output.tiff.cms.renderingintent=relative_colorimetric|absolute_colorimetric|perceptual|saturation
					intentstr = wxString(myConfig::getConfig().getValueOrDefault("output.tiff.cms.renderingintent","relative_colorimetric"));
				}
				else if (filetype == FILETYPE_PNG) {
					//parm output.png.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=prophoto
					//template output.png.cms.profile=iccfile
					iccfile = myConfig::getConfig().getValueOrDefault("output.png.cms.profile","");
					
					//parm output.png.cms.renderingintent: Specify the rendering intent for the PNG output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=relative_colorimetric
					//template output.png.cms.renderingintent=relative_colorimetric|absolute_colorimetric|perceptual|saturation
					intentstr = wxString(myConfig::getConfig().getValueOrDefault("output.png.cms.renderingintent","relative_colorimetric"));
				}
				
				if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
				if (intentstr == "saturation") intent = INTENT_SATURATION;
				if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
				if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

				//profile = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
				
				if (iccfile == "srgb" | iccfile == "wide" | iccfile == "adobe" | iccfile == "prophoto" | iccfile == "identity")
					profile = gImage::makeLCMSProfile(iccfile, 1.0);
				else if (iccfile == "aces2065-1-v4-g10" | iccfile == "adobergb-v4-g10" | iccfile == "bt709-v4-g10" | iccfile == "prophoto-v4-g10" | iccfile == "rec2020-v4-g10" | iccfile == "srgb-v4-g10" | iccfile == "srgb-v2-g22" | iccfile == "srgb-output")
					profile = gImage::makeLCMSStoredProfile(iccfile);
				else {
					profilepath.SetFullName(wxString(iccfile));
					profile = gImage::myCmsOpenProfileFromFile(profilepath.GetFullPath().ToStdString());
				}
				
				if (dib->getProfile()) {
					if (profile) {
						WxStatusBar1->SetStatusText(wxString::Format(_("Saving %s converting to color profile %s, rendering intent %s..."),fname, profilepath.GetFullName(), intentstr));
						result = dib->saveImageFile(fname.ToStdString().c_str(), std::string(configparams.c_str()), profile, intent);
					}
					else {
						wxMessageBox(wxString::Format(_("No CMS profile file found, saving with the assigned internal color profile...")));
						WxStatusBar1->SetStatusText(wxString::Format(_("Saving %s with working profile..."),fname));
						result = dib->saveImageFile(fname.ToStdString().c_str(), std::string(configparams.c_str()));
					}
				}
				else {
					wxMessageBox(wxString::Format(_("No internal working profile found, saving without a color profile...")));
					WxStatusBar1->SetStatusText(wxString::Format(_("Saving %s without a color profile..."),fname));
					result = dib->saveImageFile(fname.ToStdString().c_str(), std::string(configparams.c_str()));
				}
			//}
			//else {
			//	WxStatusBar1->SetStatusText(wxString::Format(_("Saving %s (no embedded profile)..."),fname));
			//	dib->saveImageFileNoProfile(fname, std::string(configparams.c_str()));
			//}
			if (result == GIMAGE_UNSUPPORTED_FILEFORMAT) {
				wxMessageBox("Error: Unsupported format.");
				return;
			}
			if (result == GIMAGE_EXIV2_METADATAWRITE_FAILED) {
				wxMessageBox("Image saved, but metadata insertion failed.");
			}
			
			pic->SetModified(false);
			
			wxFileName tmpname(fname);
			
			if (thumbdir != "") {
				wxFileName thumb(tmpname.GetPath(wxPATH_GET_SEPARATOR)+thumbdir, tmpname.GetFullName());
				if (thumb.DirExists()) {
					WxStatusBar1->SetStatusText(wxString::Format(_("Saving thumbnail %s..."),fname));
					gImage thumbdib = *dib;
					ApplyOps(thumbdib, thumbparams);
					thumbdib.saveImageFile(thumb.GetFullPath().ToStdString().c_str());
				}					
			}

			if (tmpname.GetFullName().compare(filename.GetFullName()) != 0) {
				sourcefilename.Assign(fname);
				SetTitle(wxString::Format("rawproc: %s (%s)",filename.GetFullName().c_str(), sourcefilename.GetFullName().c_str()));
				SetTitle(wxString::Format("%s %s: %s (%s)",_("rawproc"), VERSION, filename.GetFullName(), sourcefilename.GetFullName()));
			}
			
		WxStatusBar1->SetStatusText("");
	}
}

void rawprocFrm::MnuToolList(wxCommandEvent& event)
{
	wxFileName toollistpath;
	//parm app.toollistpath: Directory path where tool list files can be found.  Default: (none, implies current working directory)
	toollistpath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("app.toollistpath","")));

	bool incdisplay=false;
	if (myConfig::getConfig().getValueOrDefault("app.incrementaldisplay","0") == "1") incdisplay = true;

	wxString fname = wxFileSelector(_("Open Tool List..."), toollistpath.GetPath());
	if (fname == "") return;

	wxTextFile toolfile(fname);
	if (toolfile.Open()) {
		myConfig::getConfig().enableTempConfig(true);
		wxString token = toolfile.GetFirstLine();
		while (!toolfile.Eof())  {
			if (token == "") {
				token = toolfile.GetNextLine();
				continue;
			}
			std::vector<std::string> cmd = bifurcate(token.ToStdString(), ':');
			if (cmd.size() > 0) {
				wxString params;
				if (cmd.size() >=2) params = wxString(cmd[1]);
				if (cmd[0] == "set") {
					wxArrayString prop = split(params,"=");
					if (prop.GetCount() >=2) myConfig::getConfig().setValue(std::string(prop[0].c_str()),std::string(prop[1].c_str()));
				}
				else if (AddItem(wxString(cmd[0]), params, incdisplay).IsOk()) {
					wxSafeYield(this);
				}
				else {
					wxMessageBox(wxString::Format(_("Unknown command: %s.  Aborting tool list insertion."),wxString(cmd[0])));
					myConfig::getConfig().enableTempConfig(false);
					toolfile.Close();
					return;
				}
			}

			token = toolfile.GetNextLine();
		}
		if (!incdisplay) CommandTreeSetDisplay(commandtree->GetLastChild(commandtree->GetRootItem()),1154);
		myConfig::getConfig().enableTempConfig(false);
		toolfile.Close();
	}
	else wxMessageBox(_("Error: tool file not found."));

}


void rawprocFrm::CommandTreeSetDisplay(wxTreeItemId item, int src)
{
//printf("set display called from %d\n", src);  //used for group tool debugging, keep around just in case...
	SetStatusText("");
	if (!item.IsOk()) return;
	if (displayitem.IsOk()) commandtree->SetItemState(displayitem,0);
	commandtree->SetItemState(item,1);
	displayitem = item;
	pic->SetPic( ((PicProcessor *) commandtree->GetItemData(item))->getProcessedPicPointer(), ((PicProcessor *) commandtree->GetItemData(item))->getChannel() );
	//pic->SetDrawList(((PicProcessor *) commandtree->GetItemData(item))->getDrawList() );
}

bool rawprocFrm::isDownstream(wxTreeItemId here, wxTreeItemId down)
{
	wxTreeItemId at = here;
	while (at.IsOk()) {
		if (at == down) return true;
		at = commandtree->GetNextSibling(at);
	}
	return false;
}

//wxEVT_TREE_STATE_IMAGE_CLICK 
void rawprocFrm::CommandTreeStateClick(wxTreeEvent& event)
{
	SetStatusText("");
	wxTreeItemId item = event.GetItem();
	wxTreeItemId prev = displayitem;
	CommandTreeSetDisplay(item,1191);
	if (isDownstream(prev, item)) {
		wxTreeItemId next = commandtree->GetNextSibling(prev);
		if (next.IsOk()) ((PicProcessor *) commandtree->GetItemData(next))->processPic();
	}
	event.Skip();
}

//wxEVT_TREE_SEL_CHANGING
void rawprocFrm::CommandTreeSelChanging(wxTreeEvent& event)
{
	event.Skip();
}

//wxEVT_TREE_SEL_CHANGED
void rawprocFrm::CommandTreeSelChanged(wxTreeEvent& event)
{
	SetStatusText("");
	wxTreeItemId item = event.GetItem();
	if (item.IsOk()) { 
		wxTreeItemId parentitem = commandtree->GetItemParent(event.GetItem());
		std::string parentitemlabel = bifurcate(commandtree->GetItemText(parentitem).ToStdString(), ':')[0];
		if (parentitemlabel == "group") {
			//event.Veto();
			commandtree->SelectItem(parentitem);
		}
		if ((PicProcessor *) commandtree->GetItemData(item))
			if (parambook->FindPage(((PicProcessor *) commandtree->GetItemData(item))->getPanel()) != wxNOT_FOUND) {
				parambook->SetSelection(parambook->FindPage(((PicProcessor *) commandtree->GetItemData(item))->getPanel()));
				//pic->SetDrawList(((PicProcessor *) commandtree->GetItemData(item))->getDrawList() );
			}
	}
	event.Skip();
}

void rawprocFrm::CommandTreeDeleteItem(wxTreeItemId item, bool selectprevious)
{
	wxTreeItemId prev, next, newitem;
	if (commandtree->GetItemParent(item).IsOk()) {  //not root
		prev = commandtree->GetPrevSibling(item);
		next = commandtree->GetNextSibling(item);
		if (!prev.IsOk()) prev = commandtree->GetRootItem();
		if (commandtree->GetItemState(item) == 1) CommandTreeSetDisplay(prev,1221);
		if (next.IsOk())
			newitem = next;
		else
			newitem = prev;
		commandtree->SelectItem(newitem);
		parambook->DeletePage(parambook->FindPage(((PicProcessor *) commandtree->GetItemData(item))->getPanel()));
		if (selectprevious) commandtree->SelectItem(prev);
		if (commandtree->ItemHasChildren(item)) commandtree->DeleteChildren(item);
       		commandtree->Delete(item);
		if (newitem.IsOk()) {
			((PicProcessor *) commandtree->GetItemData(newitem))->processPic();
		}
		deleting = true;
	}
}

void rawprocFrm::CommandTreeDeleteSubsequent(wxTreeItemId item)
{
	wxTreeItemId next;
	wxTreeItemIdValue cookie;
	commandtree->SelectItem(item);
	CommandTreeSetDisplay(item,1242);
	if (item == commandtree->GetRootItem())
		next = commandtree->GetFirstChild(item, cookie);
	else
		next = commandtree->GetNextSibling(item);
	while (next.IsOk()) {
		wxTreeItemId following = commandtree->GetNextSibling(next);
		parambook->DeletePage(parambook->FindPage(((PicProcessor *) commandtree->GetItemData(next))->getPanel())); 
	      	commandtree->Delete(next);
		next = following;
	}
}

void rawprocFrm::CommandTreeKeyDown(wxTreeEvent& event)
{
	wxString cmd;
	SetStatusText("");
	//wxTreeItemId item, prev, next, newitem;
	event.Skip();
	switch (event.GetKeyCode()) {
        case 127:  //Delete
	//case 8: //Backspace
		CommandTreeDeleteItem(commandtree->GetSelection());
        	break;
        //case 315: //Up Arrow
        //	MoveBefore(commandtree->GetSelection());
        //	break;
        //case 317: //Down Arrow
        //	MoveAfter(commandtree->GetSelection());
        //	break;
	case 102: //f
	case 70: //F - fit image to window
		pic->SetScaleToWidth();
		pic->FitMode(true);
		SetStatusText(_("scale: fit"),STATUS_SCALE);
		pic->Refresh();
		break;
	case 67: //c - copy selected command and its parameters to the clipboard
		if (commandtree->IsEmpty()) return;
		if (event.GetKeyEvent().ControlDown()) {
			cmd = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getCommand();
			if (wxTheClipboard->Open()) {
				wxTheClipboard->SetData( new wxTextDataObject(cmd) );
				wxTheClipboard->Close();
				SetStatusText(wxString::Format(_("%s copied to clipboard."),cmd));
			}
		}
		break;
	case 86: //v - paste command and its parameters from the clipboard
		if (commandtree->IsEmpty()) return;
		if (event.GetKeyEvent().ControlDown()) {
			if (wxTheClipboard->Open()) {
				if (wxTheClipboard->IsSupported( wxDF_TEXT )) {
					wxTextDataObject data;
					wxTheClipboard->GetData( data );
					wxArrayString s = split(data.GetText(), ":");
					if (AddItem(s[0], s[1]).IsOk())
						SetStatusText(wxString::Format(_("%s pasted to command tree."),data.GetText()));
					else
						SetStatusText(wxString::Format(_("Error: %s not a valid command."),data.GetText()));
				}
				wxTheClipboard->Close();
			}
		}
		break;
	case 88: //x - cut command from tree and copy to clipboard
		if (commandtree->IsEmpty()) return;
		if (commandtree->GetSelection() == commandtree->GetRootItem()) return;
		if (event.GetKeyEvent().ControlDown()) {
			cmd = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getCommand();
			if (wxTheClipboard->Open()) {
				wxTheClipboard->SetData( new wxTextDataObject(cmd) );
				wxTheClipboard->Close();
				CommandTreeDeleteItem(commandtree->GetSelection());
				SetStatusText(wxString::Format(_("%s cut from command tree and copied to clipboard."),cmd));
			}
		}
		break;
	case 80: //p - process command
		WxStatusBar1->SetStatusText(_("processing..."));
		((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->processPic();
		WxStatusBar1->SetStatusText("");
		break;
	}
	//wxMessageBox(wxString::Format("keycode: %d", event.GetKeyCode()));
}


void rawprocFrm::CommandTreeBeginDrag(wxTreeEvent& event)
{
	event.Allow();
}

void rawprocFrm::CommandTreeEndDrag(wxTreeEvent& event)
{

}


//Menu Items (keep last in file)



void rawprocFrm::Mnuopen1003Click(wxCommandEvent& event)

{
	wxString fname = wxFileSelector(_("Open Image..."), openfilepath);	
	if ( !fname.empty() ) { 
		wxFileName f(fname);
		wxSetWorkingDirectory (f.GetPath());
		openfilepath = f.GetPath();
		OpenFile(fname);
	}
}

void rawprocFrm::Mnureopen1033Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	wxString cmdstring = AssembleCommand();
	if (filename.IsOk() && filename.FileExists()) {
		if (opensource) {
			int result = wxMessageBox(wxString::Format(_("Open source file %s with the old processing?\n\nSelecting No will open the source file with current Properties parameters and the current processing chain.\n\nCancel aborts the whole thing."),filename.GetFullName()), _("Re-Open"), wxYES_NO |wxCANCEL, this);
			if (result == wxYES) {
				OpenFileSource(sourcefilename.GetFullPath());
			}
			else if (result == wxNO) {
				OpenFile(filename.GetFullPath());
			}
		}
		else {
			OpenFile(filename.GetFullPath());
			wxArrayString token = split(cmdstring, " ");
			if (token.GetCount() > 2) {
				if (wxMessageBox(_("Re-apply processing chain?"), _("Re-Open"), wxYES_NO, this) == wxYES) {
					for (int i=2; i<token.GetCount(); i++) {
						wxArrayString cmd = split(token[i], ":");					
						if (AddItem(cmd[0], cmd[1]).IsOk()) 
							wxSafeYield(this);
						else
							wxMessageBox(wxString::Format(_("Unknown command: %s"),cmd[0]));
					}
				}
			}
		}
	}
	else {
		wxMessageBox(_("No file to re-open."));
	}
}


void rawprocFrm::Mnuopensource1004Click(wxCommandEvent& event)
{
	wxString fname = wxFileSelector(_("Open Image source..."), openfilepath);	
	if ( !fname.empty() ) { 
		wxFileName f(fname);
		wxSetWorkingDirectory (f.GetPath());
		openfilepath = f.GetPath();
		OpenFileSource(fname);
		opensource = true;
	}

}

void rawprocFrm::MnuProperties(wxCommandEvent& event)
{
	if (configfile == "(none)") {
		wxMessageBox(_("No configuration file found."));
		return;
	}
	if (propdiag == nullptr) {
		propdiag = new PropertyDialog(this, wxID_ANY, _("Properties"), wxDefaultPosition, wxSize(640,480));
		propdiag->LoadConfig();
		Bind(wxEVT_PG_CHANGED,&rawprocFrm::UpdateConfig,this);
	}
	if (propdiag != nullptr) {
		propdiag->ClearModifiedStatus();
		propdiag->Show();
	}
	else {
		wxMessageBox(_("Failed to create Properties dialog"));
	}
}

void rawprocFrm::MnuEXIF(wxCommandEvent& event)
{
	EXIFDialog(filename);
}

void rawprocFrm::MnuEditMetadata(wxCommandEvent& event)
{
	gImage *dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPicPointer();
	//tagdiag = new MetadataDialog(this, wxID_ANY, _("Metadata"), wxDefaultPosition, wxSize(640,480));
	//tagdiag->LoadMetadata(dib);
	//tagdiag->Show();
	
	MetadataDialog tagdiag(this, wxID_ANY, _("Metadata"), wxDefaultPosition, wxSize(640,480));
	tagdiag.LoadMetadata(dib);
	tagdiag.ShowModal();
}

void rawprocFrm::MnuBatchClick(wxCommandEvent& event)
{
	myBatchDialog dlg(this, wxID_ANY, _("Batch"));
	dlg.ShowModal();
}

void rawprocFrm::UpdateConfig(wxPropertyGridEvent& event)
{
	wxPGProperty * prop = event.GetProperty();
	wxString propname = event.GetPropertyName();
	wxString propval;
	wxPGChoices ch = prop->GetChoices();

	if (ch.IsOk()) 
		propval = ch.GetLabel(prop->GetChoiceSelection());
	else if (myConfig::getConfig().getValue("Templates",std::string(propname.mb_str())) == "iccfile") 
		propval = wxFileName(event.GetPropertyValue().GetString()).GetFullName();
	else 
		propval = event.GetPropertyValue().GetString();

	SetStatusText(wxString::Format(_("Changed %s to %s."), propname, propval));
	myConfig::getConfig().setValue((const char  *) propname.mb_str(), (const char  *) propval.mb_str());
	if (!myConfig::getConfig().flush()) SetStatusText(_("Write to configuration file failed."));

	//check for properties that should update immediately:
	if (propname.Find("display.") != wxNOT_FOUND)
		pic->RefreshPic();
	if (propname.Find("histogram") != wxNOT_FOUND)
		pic->RefreshPic();
	if (propname.Find("app.start.path") != wxNOT_FOUND)
		openfilepath = wxString(myConfig::getConfig().getValueOrDefault("app.start.path",""));

	//not ready for prime time
	//if (propname.Find("backgroundcolor") != wxNOT_FOUND) SetBackground();
}


void rawprocFrm::MnuTone(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.tone.initialvalue: The initial value of the tone tool, 1.0=no change (linear).  Default=gamma,1.0
		wxString val = wxString(myConfig::getConfig().getValueOrDefault("tool.tone.initialvalue","gamma,1.0"));
		PicProcessorTone *p = new PicProcessorTone("tone",val, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1510);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding tone tool failed: %s"),e.what()));
	}

}

void rawprocFrm::MnuCACorrect(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "0") {
		wxMessageBox(_("Error: CACorrect can only be applied to mosaic data, before demosaic."));
		return;
	}
	SetStatusText("");
	try {
		PicProcessorCACorrect *p = new PicProcessorCACorrect("cacorrect","auto,1", commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1510);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding cacorrect tool failed: %s"),e.what()));
	}

}

void rawprocFrm::MnuHLRecover(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
		wxMessageBox(_("Error: HLRecover can only be applied to RGB data."));
		return;
	}
	SetStatusText("");
	try {
		PicProcessorHLRecover *p = new PicProcessorHLRecover("hlrecover","", commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1510);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding hlrecover tool failed: %s"),e.what()));
	}

}



void rawprocFrm::MnusaturateClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
		wxMessageBox(_("Error: Saturation can only be applied to RGB data."));
		return;
	}
	SetStatusText("");
	try {
		//parm tool.saturate.initialvalue: The initial (and reset button) value of the saturation tool, 1.0=no change.  Default=1.0
		wxString val = wxString(myConfig::getConfig().getValueOrDefault("tool.saturate.initialvalue","1.0"));
		PicProcessorSaturation *p = new PicProcessorSaturation("saturation",val, commandtree, pic);
		p->createPanel(parambook);
		if (val != "1.0") p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1572);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding saturation tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuexposureClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.exposure.initialvalue: The initial (and reset button) value of the exposure tool, 0.0=no change.  Default=0.0
		wxString val = wxString(myConfig::getConfig().getValueOrDefault("tool.exposure.initialvalue","0.0"));
		PicProcessorExposure *p = new PicProcessorExposure("exposure",val, commandtree, pic);
		p->createPanel(parambook);
		if (val != "0.0") p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1589);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding exposure tool failed: %s"),e.what()));
	}
}


void rawprocFrm::Mnucurve1010Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorCurve *p = new PicProcessorCurve("curve","rgb,0.0,0.0,255.0,255.0", commandtree, pic);
		p->createPanel(parambook);
		//p->processPic();  //comment out, don't need to process new tool
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1605);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding curve tool failed: %s"),e.what()));
	}
}


void rawprocFrm::MnuGrayClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
		wxMessageBox(_("Error: Gray can only be applied to RGB data."));
		return;
	}
	SetStatusText("");
	try {
		//parm tool.gray.r: The initial (and reset button) value of the red proportion for grayscale conversion. Default=0.21
		wxString r =  wxString(myConfig::getConfig().getValueOrDefault("tool.gray.r","0.21"));
		//parm tool.gray.g: The initial (and reset button) value of the green proportion for grayscale conversion. Default=0.72
		wxString g =  wxString(myConfig::getConfig().getValueOrDefault("tool.gray.g","0.72"));
		//parm tool.gray.b: The initial (and reset button) value of the blue proportion for grayscale conversion. Default=0.07
		wxString b =  wxString(myConfig::getConfig().getValueOrDefault("tool.gray.b","0.07"));
		wxString cmd= wxString::Format("%s,%s,%s",r,g,b);
		PicProcessorGray *p = new PicProcessorGray("gray",cmd, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1669);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding grayscale tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuCropClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	//gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	//if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
	//	wxMessageBox(_("Error: Crop can only be applied to RGB data."));
	//	return;
	//}
	SetStatusText("");
	try {
		PicProcessorCrop *p = new PicProcessorCrop("crop", commandtree, pic);
		p->createPanel(parambook);
		//p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1689);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding crop tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuResizeClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
		wxMessageBox(_("Error: Resize can only be applied to RGB data."));
		return;
	}
	SetStatusText("");
	try {
		PicProcessorResize *p = new PicProcessorResize("resize", "", commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		pic->SetScale(1.0);
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1710);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding resize tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuBlackWhitePointClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorBlackWhitePoint *p;
		//parm tool.blackwhitepoint.auto: Invoke auto calculation of inital black and white point values, based on a percent-pixels threshold.  Currently, this behavior is only invoked when the tool is added, so re-application requires deleting and re-adding the tool.  Default=0
		if (myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.auto","0") == "1") {
			p = new PicProcessorBlackWhitePoint("blackwhitepoint", "rgb", commandtree, pic);
			p->createPanel(parambook);
			p->processPic();
		}
		else {
			p = new PicProcessorBlackWhitePoint("blackwhitepoint", "rgb,0,255", commandtree, pic);
			p->createPanel(parambook);
		}

		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1734);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding blackwhitepoint tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuSharpenClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
		wxMessageBox(_("Error: Sharpen can only be applied to RGB data."));
		return;
	}
	SetStatusText("");
	try {
		//parm tool.sharpen.initialvalue: The initial (and reset button) value of the sharpen tool, 0=no change.  Default=0
		wxString defval =  wxString(myConfig::getConfig().getValueOrDefault("tool.sharpen.initialvalue","0"));
		PicProcessorSharpen *p = new PicProcessorSharpen("sharpen", defval, commandtree, pic);
		p->createPanel(parambook);
		if (defval != "0") p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1751);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding sharpen tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuSpot(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
		wxMessageBox(_("Error: Spot can only be applied to RGB data."));
		return;
	}
	SetStatusText("");
	try {
		//parm tool.spot.initialmode: The initial mode of the spot tool, Default=(blank), which gives list mode
		wxString defmode =  wxString(myConfig::getConfig().getValueOrDefault("tool.spot.initialmode",""));
		PicProcessorSpot *p = new PicProcessorSpot("spot", defmode, commandtree, pic);
		p->createPanel(parambook);
		//parm tool.spot.initialdisplay: The initial display mode of the spot tool, Default=1, check the display box
		if (myConfig::getConfig().getValueOrDefault("tool.spot.initialdisplay","1") == "1") 
			CommandTreeSetDisplay(p->GetId(),1750);
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1751);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding spot tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuRotateClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
		wxMessageBox(_("Error: Rotate can only be applied to RGB data."));
		return;
	}
	if (dib.getInfoValue("Orientation") != "1") {
		wxMessageBox(_("Rotate tool isn't designed to work work if the image orientation isn't normalized prior to demosaic.  Put it in the tool chain after Orientation=1"));
		return;
	}
	SetStatusText("");
	try {
		//parm tool.rotate.initialvalue: The initial (and reset button) angle of the rotate tool, 0=no change.  Default=0
		wxString defval =  wxString(myConfig::getConfig().getValueOrDefault("tool.rotate.initialvalue","0.0"));
		PicProcessorRotate *p = new PicProcessorRotate("rotate", defval, commandtree, pic);
		p->createPanel(parambook);
		//p->processPic();  //not sure why rotate has an initial value.....
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1733);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding rotate tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuDenoiseClick(wxCommandEvent& event)
{
	wxString algorithm, sigma, local, patch, threshold, cmd;
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
		wxMessageBox(_("Error: Denoise can only be applied to RGB data."));
		return;
	}
	SetStatusText("");
	try {
		//parm tool.denoise.algorithm: nlmeans|wavelet. The default algorithm to use when adding a denoise tool.  Default=wavelet
		algorithm =  wxString(myConfig::getConfig().getValueOrDefault("tool.denoise.algorithm","wavelet"));
		
		if (algorithm == "nlmeans") {
			//parm tool.denoise.initialvalue: The initial (and reset button) sigma value used to calculate the denoised pixel.  Default=tool.denoise.sigma (initialvalue is deprecated)
			//parm tool.denoise.sigma: The initial sigma value used to calculate the denoised pixel.  Default=1
			sigma =  wxString(myConfig::getConfig().getValueOrDefault("tool.denoise.initialvalue",
				myConfig::getConfig().getValueOrDefault("tool.denoise.sigma","0")));
			//parm tool.denoise.local: Defines the initial (and reset button) size of the neigbor pixel array.  Default=3
			local =  wxString(myConfig::getConfig().getValueOrDefault("tool.denoise.local","3"));
			//parm tool.denoise.patch: Defines the initial (and reset button) size of the patch pixel array.  Default=1
			patch =  wxString(myConfig::getConfig().getValueOrDefault("tool.denoise.patch","1"));
			cmd = wxString::Format("nlmeans,%s,%s,%s",sigma,local,patch);
		}
		if (algorithm == "wavelet") {
			//parm tool.denoise.threshold: Defines the initial value of the threshold for wavelet denoise.  Default=0.0
			threshold =  wxString(myConfig::getConfig().getValueOrDefault("tool.denoise.threshold","0.0"));
			cmd = wxString::Format("wavelet,%s",threshold);
		}
		PicProcessorDenoise *p = new PicProcessorDenoise("denoise", cmd, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1806);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding denoise tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuRedEyeClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
		wxMessageBox(_("Error: Redeye can only be applied to RGB data."));
		return;
	}
	SetStatusText("");
	try {
		//parm tool.redeye.threshold: The initial (and reset button) red intensity threshold.  Default=1.5
		wxString threshold =  wxString(myConfig::getConfig().getValueOrDefault("tool.redeye.threshold","1.5"));
		//parm tool.redeye.radius: Defines the initial (and reset button) limit of the patch size.  Default=50
		wxString radius =  wxString(myConfig::getConfig().getValueOrDefault("tool.redeye.radius","50"));
		//parm tool.redeye.desaturation: The initial (and reset button) desaturation toggle.  Default=0
		wxString desat =  wxString(myConfig::getConfig().getValueOrDefault("tool.redeye.desaturation","0"));
		//parm tool.redeye.desaturationpercent: The initial (and reset button) desaturation percent.  Default=1.0
		wxString desatpct =  wxString(myConfig::getConfig().getValueOrDefault("tool.redeye.desaturationpercent","1.0"));
		
		wxString cmd = wxString::Format("%s,%s,%s,%s",threshold,radius,desat,desatpct);
		PicProcessorRedEye *p = new PicProcessorRedEye("redeye", cmd, commandtree, pic);
		p->createPanel(parambook);
		//p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1831);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding redeye tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuColorSpace(wxCommandEvent& event)
{
	wxString cmd = "(none)";

	if (commandtree->IsEmpty()) return;

	if (PicProcessor::getSelectedPicProcessor(commandtree)->getProcessedPic().getProfile() == NULL) {
		wxMessageBox(_("Note: Image does not have a source profile, only 'assign' is valid"));
		cmd = "(none),assign";
	}

	SetStatusText("");
	try {
		PicProcessorColorSpace *p = new PicProcessorColorSpace("colorspace", cmd, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		p->setOpenFilePath(openfilepath);
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1855);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding colorspace tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuLensCorrection(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "1") {
		wxMessageBox(_("Error: Lens correction can only be applied to RGB data."));
		return;
	}
	SetStatusText("");
	try {
		lfDatabase *lfdb =  PicProcessorLensCorrection::findLensfunDatabase();
		if (lfdb) {
			//parm tool.lenscorrection.default: The corrections to automatically apply. Default: mode=lensfun.
			wxString defaults =  wxString(myConfig::getConfig().getValueOrDefault("tool.lenscorrection.default","mode=lensfun"));
			PicProcessorLensCorrection *p = new PicProcessorLensCorrection(lfdb, "lenscorrection", defaults, commandtree, pic);
			p->createPanel(parambook);
			if (defaults != "") p->processPic();
			if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1884);
		}
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding lenscorrection tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuLensDistortion(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorLensDistortion *p = new PicProcessorLensDistortion("lensdistortion", "ptlens,0.0,0.0,0.0,1.0", commandtree, pic);
		p->createPanel(parambook);
		//p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1936);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding lensdistortion tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuLensVignetting(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorLensVignetting *p = new PicProcessorLensVignetting("lensvignetting", "pa,0.0,0.0,0.0", commandtree, pic);
		p->createPanel(parambook);
		//p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1936);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding lensvignetting tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuDemosaic(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	gImage dib = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getProcessedPic();
	if (dib.getInfoValue("Libraw.Mosaiced") == "0") {
		wxMessageBox(_("Error: Demosaix can only be applied to mosaic data."));
		return;
	}
	SetStatusText("");
	try {
		//parm tool.demosaic.default: Demosaic algorithm default.  Default=ahd, if librtprocess is present, else Default=half.
#ifdef USE_LIBRTPROCESS
		wxString d =  wxString(myConfig::getConfig().getValueOrDefault("tool.demosaic.default","ahd"));
#else
		wxString d =  wxString(myConfig::getConfig().getValueOrDefault("tool.demosaic.default","half"));
#endif
		PicProcessorDemosaic *p = new PicProcessorDemosaic("demosaic", d, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1906);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding demosaic tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuWhiteBalance(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorWhiteBalance *p = new PicProcessorWhiteBalance("whitebalance", "", commandtree, pic);
		p->createPanel(parambook);
		//p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1921);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding white balance tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuAdd(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorAdd *p = new PicProcessorAdd("add", "rgb,0.0", commandtree, pic);
		p->createPanel(parambook);
		//p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1936);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding add tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuSubtract(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorSubtract *p = new PicProcessorSubtract("subtract", "rgb,0.0", commandtree, pic);
		p->createPanel(parambook);
		//p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1936);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding subtract tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuGroup(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorGroup *p = new PicProcessorGroup("group", "", commandtree, pic);
		p->createPanel(parambook);
		p->selectFile();
		//p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1951);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding group tool failed: %s"),e.what()));
	}
}

void rawprocFrm::MnuScript(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorScript *p = new PicProcessorScript("script", "", commandtree, filename.GetFullName(), pic);
		p->createPanel(parambook);
		//p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1951);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding script tool failed: %s"),e.what()));
	}
}

#ifdef USE_GMIC
void rawprocFrm::MnuGMIC(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorGMIC *p = new PicProcessorGMIC("gmic", "", commandtree, pic);
		p->createPanel(parambook);
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId(),1936);
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format(_("Error: Adding gmic tool failed: %s"),e.what()));
	}
}
#endif

void rawprocFrm::MnuCut1201Click(wxCommandEvent& event)
{
	wxTreeItemId prev;
	if (commandtree->IsEmpty()) return;
	if (commandtree->GetSelection() == commandtree->GetRootItem()) return;
	wxString cmd = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getCommand();
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(cmd) );
		wxTheClipboard->Close();
		CommandTreeDeleteItem(commandtree->GetSelection());
		SetStatusText(wxString::Format(_("%s cut from command tree and copied to clipboard."),cmd));
	}
}

void rawprocFrm::MnuCopy1202Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	wxString cmd = ((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getCommand();
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(cmd) );
		wxTheClipboard->Close();
		SetStatusText(wxString::Format(_("%s copied to clipboard."),cmd));
	}
}

void rawprocFrm::MnuPaste1203Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported( wxDF_TEXT ))
		{
			wxTextDataObject data;
			wxTheClipboard->GetData( data );
			wxArrayString s = split(data.GetText(), ":");
			if (AddItem(s[0], s[1]).IsOk())
				SetStatusText(wxString::Format(_("%s pasted to command tree."),data.GetText()));
			else
				SetStatusText(wxString::Format(_("Error: %s not a valid command."),data.GetText()));
		}
		wxTheClipboard->Close();
	}
}

void rawprocFrm::MnuShowCommand1010Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	//parm menu.showcommand.type: Specify dialog box type in which to show the cumulative command, text|html. Dialog type 'html' can be copied to the clipboard.  Default=text
	wxString diagtype = myConfig::getConfig().getValueOrDefault("menu.showcommand.type","text");
	if (diagtype == "text") {
		wxMessageBox(AssembleCommand());
	}
	else if (diagtype == "html") {
		wxString cmd = "<p>" + AssembleCommand() + "</p>";
		myEXIFDialog dlg(this, wxID_ANY, _("Image Command"), cmd,  wxDefaultPosition, wxSize(400,200));
		dlg.ShowModal();
	}
	else wxMessageBox(_("Invalid dialog type for Show Command... (menu.showcommand.type)"));
}

void rawprocFrm::MnuAbout1011Click(wxCommandEvent& event)
{	
	wxAboutDialogInfo info;

	#ifdef VERSION
		info.SetName(_("rawproc"));
		info.SetVersion(VERSION);
	#else
		info.SetName(_("rawproc\n(development build)"));
	#endif

	info.SetIcon(icon_xpm);
	info.SetCopyright(_("(C) 2016-2021 Glenn Butcher <glenn.butcher@gmail.com>"));
	
	//wxString WxWidgetsVersion = wxGetLibraryVersionInfo().GetVersionString();
	wxVersionInfo wxversion = wxGetLibraryVersionInfo();

	wxString WxWidgetsVersion = wxString::Format("%s %d.%d.%d", wxversion.GetName(), wxversion.GetMajor(), wxversion.GetMinor(), wxversion.GetMicro());
	wxString libraries = wxString(gImage::LibraryVersions());

	libraries.Append(wxString::Format("\nLensfun: %d.%d.%d",LF_VERSION_MAJOR,LF_VERSION_MINOR,LF_VERSION_MICRO));
	wxString pixtype = wxString(gImage::getRGBCharacteristics());

	wxString description(wxString::Format(_("Basic camera raw file and image editor.\n\nLibraries:\n%s\n%s\n\nPixel Format: %s\n\nConfiguration file: %s"), WxWidgetsVersion, libraries.c_str(),pixtype, configfile));

	std::string lensfundbpath = myConfig::getConfig().getValueOrDefault("tool.lenscorrection.databasepath",getAppConfigDir());
	wxString lensfundb = "Lensfun Database";
	lensfundb.Append(wxString::Format(": %s",wxString(lensfundbpath)));

#ifdef USE_LENSFUN_DBUPDATE
	//parm app.about.lensdatabasecheck: 1|0, if set, the lensfun database version will be checked against the server. Default: 1
	if (myConfig::getConfig().getValueOrDefault("app.about.lensdatabasecheck","1") == "1") {
		switch (lensfun_dbcheck(LF_MAX_DATABASE_VERSION, lensfundbpath)) {
			case LENSFUN_DBUPDATE_NOVERSION:	lensfundb.Append(wxString::Format(_("- version %d not available from server."), LF_MAX_DATABASE_VERSION)); break;
			case LENSFUN_DBUPDATE_NODATABASE:	lensfundb.Append(wxString::Format(_("- no Version %d database at this path."),LF_MAX_DATABASE_VERSION)); break;
			case LENSFUN_DBUPDATE_OLDVERSION:	lensfundb.Append(wxString::Format(_("- Version %d, Not current."), LF_MAX_DATABASE_VERSION)); break;
			case LENSFUN_DBUPDATE_CURRENTVERSION:	lensfundb.Append(wxString::Format(_("- Version %d, Current."), LF_MAX_DATABASE_VERSION)); break;
		}
	}
#endif
	description.Append(wxString::Format(_("\n%s"), lensfundb));

#ifdef BUILDDATE
	wxString builddate = wxString(BUILDDATE);
	description.Append(wxString::Format(_("\n\nBuild Date: %s"), builddate));
#endif

	info.SetDescription(description);
	wxAboutBox(info, this);

}

void rawprocFrm::MnuHelpClick(wxCommandEvent& event)
{
	help.Display("Introduction");
	help.Display("rawproc Image Processor");
}

#define ID_EXIF			2001
#define ID_DELETESUBSEQUENT	2002
#define ID_DELETE		2003
#define ID_HISTOGRAM		2004
#define ID_GROUPTOTOOLLIST	2005
#define ID_GROUPLASTTOTOOL	2006

void rawprocFrm::showHistogram(wxTreeItemId item)
{
	gImage *g = ((PicProcessor *) commandtree->GetItemData(item))->getProcessedPicPointer();
	SetStatusText(_("Building histogram..."));
	myHistogramDialog hdiag(this, wxID_ANY, "Histogram", g , wxDefaultPosition, wxDefaultSize);  //wxSize(500,300));
	SetStatusText("");
	hdiag.ShowModal();
}

void rawprocFrm::CommandTreePopup(wxTreeEvent& event)
{
	wxTreeItemId disp, id, first, prev, next;
	bool displaylast = false;
	wxString pstr, newlist, last;
	wxArrayString p, cmd;
	PicProcessorGroup *g;
	if (commandtree->GetItemText(commandtree->GetItemParent(event.GetItem())) == "group") return;  
	wxMenu mnu;
 	mnu.Append(ID_EXIF, _("Image Information..."));
 	mnu.Append(ID_HISTOGRAM, _("Full Histogram..."));
	mnu.AppendSeparator();
	mnu.Append(ID_DELETE, _("Delete"));
	mnu.Append(ID_DELETESUBSEQUENT, _("Delete subsequent..."));
	//if (commandtree->GetItemText(event.GetItem()) == "group") {
	if (bifurcate(commandtree->GetItemText(event.GetItem()).ToStdString(), ':')[0] == "group") {
		mnu.AppendSeparator();
		mnu.Append(ID_GROUPTOTOOLLIST, _("Convert group to toollist"));
		mnu.Append(ID_GROUPLASTTOTOOL, _("Convert last group item to tool"));
	}	
	switch (GetPopupMenuSelectionFromUser(mnu)) {
		case ID_EXIF:
			InfoDialog(event.GetItem());
			break;
		case ID_HISTOGRAM:
			showHistogram(event.GetItem());
			//wxMessageBox(_("Not there yet, press 't' to toggle the thumbnail histogram..."));
			break;
		case ID_DELETE:
			CommandTreeDeleteItem(event.GetItem());
			break;
		case ID_DELETESUBSEQUENT:
			CommandTreeDeleteSubsequent(event.GetItem());
			break;
		case ID_GROUPTOTOOLLIST:
			if (event.GetItem() == displayitem) displaylast = true;
			commandtree->SelectItem(event.GetItem());
			prev = commandtree->GetPrevSibling(event.GetItem());
			//next = commandtree->GetNextSibling(event.GetItem());
			if (!prev.IsOk()) prev = commandtree->GetRootItem();
			pstr = ((PicProcessor *) commandtree->GetItemData(event.GetItem()))->getParams();
			p = split(pstr, ";");
			CommandTreeDeleteItem(event.GetItem(), true);
			//PicProcessor::enableGlobalProcessing(false);  //not ready for prime time
			for (int i=0; i<p.GetCount(); i++) {
				cmd = split(p[i], ":");	
				id = AddItem(cmd[0], cmd[1], false);
				if (id.IsOk()) 
					wxSafeYield(this);
				else
					wxMessageBox(wxString::Format(_("Unknown command: %s"),cmd[0]));
				if (i==0) first = id;
			}
			//PicProcessor::enableGlobalProcessing(true);  //not ready for prime time
			((PicProcessor *) commandtree->GetItemData(id))->processPic();
			if (displaylast && id.IsOk()) CommandTreeSetDisplay(id,2128);
			break;
		case ID_GROUPLASTTOTOOL:
			disp = displayitem;
			g = ((PicProcessorGroup *) commandtree->GetItemData(event.GetItem()));
			pstr = g->getParams();
			p = split(pstr, ";");
			last = p.Last();
			p.RemoveAt(p.GetCount()-1);
			if (p.GetCount() > 0) {
				newlist = p[0];
				for (int i=1; i<p.GetCount(); i++) newlist.Append(wxString::Format(";%s",p[i]));
				g->loadCommands(newlist);
				g->processPic(false);
			}
			else {
				CommandTreeDeleteItem(event.GetItem(), true);
			}
			cmd = split(last,":");
			wxTreeItemId id = AddItem(cmd[0], cmd[1]);
			if (disp == event.GetItem()) CommandTreeSetDisplay(id,2148);
			break;
	}
}


void rawprocFrm::SetConfigFile(wxString cfile)
{
	configfile = cfile;
}

void rawprocFrm::CharEvent(wxKeyEvent& event)
{
	wxChar uc = event.GetUnicodeKey();
	if ( uc != WXK_NONE )
	{
		// It's a "normal" character. Notice that this includes
		// control characters in 1..31 range, e.g. WXK_RETURN or
		// WXK_BACK, so check for them explicitly.
		if ( uc >= 32 )
		{
			switch (uc) {
			}
		}
		else
		{
			// It's a control character, < WXK_START
			switch (uc)
			{
				case WXK_TAB:
					//NavigateIn(); //maybe needed for Win32...  ??
					break;
			}
		}
	}
	else // No Unicode equivalent.
	{
		// It's a special key, > WXK_START, deal with all the known ones:
		switch ( event.GetKeyCode() )
		{
		}
	}
	event.Skip();
}


