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

#include "PicProcessorGamma.h"
#include "PicProcessorBright.h"
#include "PicProcessorContrast.h"
#include "PicProcessorSaturation.h"
#include "PicProcessorExposure.h"
#include "PicProcessorShadow.h"
#include "PicProcessorHighlight.h"
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
#ifdef USE_LENSFUN
#include "PicProcessorLensCorrection.h"
#include <locale.h>
#include <lensfun/lensfun.h>
#endif
#include "myHistogramDialog.h"
#include "myEXIFDialog.h"
#include "myConfig.h"
#include "util.h"
#include "lcms2.h"
#include <omp.h>
#include <exception>

#include "unchecked.xpm"
#include "checked.xpm"

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
	////Manual Code Start
	EVT_TREE_SEL_CHANGED(ID_COMMANDTREE,rawprocFrm::CommandTreeSelChanged)
	////Manual Code End

	EVT_SIZE(rawprocFrm::OnSize)
	EVT_CLOSE(rawprocFrm::OnClose)
	EVT_MENU(ID_MNU_OPEN, rawprocFrm::Mnuopen1003Click)
	EVT_MENU(ID_MNU_REOPEN, rawprocFrm::Mnureopen1033Click)
	EVT_MENU(ID_MNU_OPENSOURCE, rawprocFrm::Mnuopensource1004Click)
	EVT_MENU(ID_MNU_SAVE, rawprocFrm::Mnusave1009Click)
	EVT_MENU(ID_MNU_EXIT, rawprocFrm::MnuexitClick)
	EVT_MENU(ID_MNU_GAMMA, rawprocFrm::Mnugamma1006Click)
	EVT_MENU(ID_MNU_BRIGHT, rawprocFrm::Mnubright1007Click)
	EVT_MENU(ID_MNU_CONTRAST, rawprocFrm::Mnucontrast1008Click)
	EVT_MENU(ID_MNU_SATURATION, rawprocFrm::MnusaturateClick)
	EVT_MENU(ID_MNU_EXPOSURE, rawprocFrm::MnuexposureClick)
	EVT_MENU(ID_MNU_SHADOW, rawprocFrm::MnuShadow1015Click)
	EVT_MENU(ID_MNU_HIGHLIGHT, rawprocFrm::MnuHighlightClick)
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
	EVT_MENU(ID_MNU_ABOUT,rawprocFrm::MnuAbout1011Click)
	EVT_MENU(ID_MNU_VIEWHELP,rawprocFrm::MnuHelpClick)
	EVT_MENU(ID_MNU_PROPERTIES,rawprocFrm::MnuProperties)
	EVT_MENU(ID_MNU_EXIF,rawprocFrm::MnuEXIF)
	EVT_MENU(ID_MNU_COLORSPACE, rawprocFrm::MnuColorSpace)
#ifdef USE_LENSFUN
	EVT_MENU(ID_MNU_LENSCORRECTION, rawprocFrm::MnuLensCorrection)
#endif
	EVT_TREE_KEY_DOWN(ID_COMMANDTREE,rawprocFrm::CommandTreeKeyDown)
	//EVT_TREE_DELETE_ITEM(ID_COMMANDTREE, rawprocFrm::CommandTreeDeleteItem)
	EVT_TREE_BEGIN_DRAG(ID_COMMANDTREE, rawprocFrm::CommandTreeBeginDrag)
	EVT_TREE_END_DRAG(ID_COMMANDTREE, rawprocFrm::CommandTreeEndDrag)
	EVT_TREE_STATE_IMAGE_CLICK(ID_COMMANDTREE, rawprocFrm::CommandTreeStateClick)
	//EVT_TREE_SEL_CHANGING(ID_COMMANDTREE, rawprocFrm::CommandTreeSelChanging)
	EVT_TREE_ITEM_MENU(ID_COMMANDTREE, rawprocFrm::CommandTreePopup)
END_EVENT_TABLE()
////Event Table End

void MyLogErrorHandler(cmsContext ContextID, cmsUInt32Number code, const char *text)
{
	//wxMessageBox(wxString::Format("CMS Error %d: %s", code, text));
	//printf("CMS Error %d: %s\n", code, text);
}

rawprocFrm::rawprocFrm(wxWindow *parent, wxWindowID id, const wxString &title, const wxPoint &position, const wxSize& size, long style)
: wxFrame(parent, id, title, position, size, style)
{
	wxString startpath = wxString(myConfig::getConfig().getValueOrDefault("app.start.path",""));
	if (startpath != "") 
		if (wxFileName::DirExists(startpath))
			openfilepath = startpath;

	CreateGUIControls();
#if defined(_OPENMP)
	omp_set_dynamic(0);
#endif
	deleting = false;
	displayitem.Unset();

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
	ret = help.AddBook(wxFileName(helpfile));
	if (! ret)
		wxMessageBox(wxString::Format("Failed adding %s",helpfile.GetFullPath()));

	cmsSetLogErrorHandler(MyLogErrorHandler);

	diag == NULL;
	
}

void rawprocFrm::CreateGUIControls()
{
#ifndef SIZERLAYOUT
	mgr.SetManagedWindow(this);
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
	ID_MNU_FILEMnu_Obj->Append(ID_MNU_SAVE, _("Save..."), _(""), wxITEM_NORMAL);
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
	WxMenuBar1->Append(ID_MNU_EDITMnu_Obj, _("Edit"));

	
	wxMenu *ID_MNU_ADDMnu_Obj = new wxMenu();
	
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_BLACKWHITEPOINT,	_("Black/White Point"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_BRIGHT,	_("Bright"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_COLORSPACE,	_("Colorspace"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_CONTRAST,	_("Contrast"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_CROP,		_("Crop"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_CURVE,		_("Curve"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_DENOISE,	_("Denoise"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_EXPOSURE,	_("Exposure"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_GAMMA,		_("Gamma"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_GRAY,		_("Gray"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_HIGHLIGHT,	_("Highlight"), _(""), wxITEM_NORMAL);
#ifdef USE_LENSFUN
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_LENSCORRECTION,_("Lens Correction"), _(""), wxITEM_NORMAL);
#endif
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_REDEYE,	_("Redeye"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_RESIZE,	_("Resize"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_ROTATE,	_("Rotate"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SATURATION,	_("Saturation"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SHADOW,	_("Shadow"), _(""), wxITEM_NORMAL);
	ID_MNU_ADDMnu_Obj->Append(ID_MNU_SHARPEN,	_("Sharpen"), _(""), wxITEM_NORMAL);
	
	
	WxMenuBar1->Append(ID_MNU_ADDMnu_Obj, _("Add"));
	
	wxMenu *ID_MNU_HELPMnu_Obj = new wxMenu();
	ID_MNU_HELPMnu_Obj->Append(ID_MNU_VIEWHELP, _("View Help..."), _(""), wxITEM_NORMAL);
	ID_MNU_HELPMnu_Obj->Append(ID_MNU_SHOWCOMMAND, _("Show Command..."), _(""), wxITEM_NORMAL);
	ID_MNU_HELPMnu_Obj->Append(ID_MNU_ABOUT, _("About..."), _(""), wxITEM_NORMAL);
	WxMenuBar1->Append(ID_MNU_HELPMnu_Obj, _("Help"));
	SetMenuBar(WxMenuBar1);

	WxStatusBar1 = new wxStatusBar(this, ID_WXSTATUSBAR1);
	int widths[3] = {-1,50, 100};
	WxStatusBar1->SetFieldsCount (3, widths);

	SetStatusBar(WxStatusBar1);
	SetTitle(_("rawproc"));
	SetIcon(wxNullIcon);
	SetSize(0,0,1200,750);
	Center();
	
	////GUI Items Creation End
	

	//Image manipulation panels:
	commandtree = new wxTreeCtrl(this, ID_COMMANDTREE, wxDefaultPosition, wxSize(280,200), wxTR_DEFAULT_STYLE | wxTR_HAS_VARIABLE_ROW_HEIGHT);  

	histogram = new myHistogramPane(this, wxDefaultPosition,wxSize(285,150));

	parambook = new wxSimplebook(this, wxID_ANY, wxDefaultPosition,wxSize(285,320), wxBORDER_SUNKEN);


	//Main picture panel:
	pic = new PicPanel(this, commandtree, histogram);


#ifdef SIZERLAYOUT
	wxSizerFlags flags = wxSizerFlags().Left().Border(wxLEFT|wxRIGHT).FixedMinSize();  
	hs = new wxBoxSizer(wxHORIZONTAL);
	vs = new wxBoxSizer(wxVERTICAL);
	vs->Add(new wxStaticText(this,wxID_ANY, "Commands:", wxDefaultPosition, wxSize(100,20)), flags);
	vs->Add(commandtree, flags);
	vs->Add(new wxStaticText(this,wxID_ANY, "Histogram:", wxDefaultPosition, wxSize(100,20)), flags);
	vs->Add(histogram, flags);
	vs->Add(new wxStaticText(this,wxID_ANY, "Parameters:", wxDefaultPosition, wxSize(100,20)), flags);
	vs->Add(parambook, flags);
	hs->Add(vs, flags);
	hs->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), flags.Expand());
	hs->Add(pic, flags);
	SetSizerAndFit(hs);
#else
	wxAuiPaneInfo pinfo = wxAuiPaneInfo().Left().CloseButton(false);
	mgr.AddPane(pic, wxCENTER);
	mgr.AddPane(commandtree, pinfo.Caption(wxT("Commands")).Position(0));
	mgr.AddPane(histogram, pinfo.Caption(wxT("Histogram")).Position(1).Fixed());  //.GripperTop());
	mgr.AddPane(parambook, pinfo.Caption(wxT("Parameters")).Position(2).Resizable().MinSize(285,320).FloatingSize(285,320));
	mgr.Update();
#endif

}

void rawprocFrm::OnSize(wxSizeEvent& event)
{
	event.Skip();
	Refresh();
}

void rawprocFrm::SetBackground()
{
	int pr, pg, pb;
	int dr, dg, db;
	wxString f;
	//parm app.backgroundcolor: r,g,b or t (0-255), set at startup. 'r,g,b' specifies a color, 't' specifies a gray tone.  Default=(119,119,119)
	wxString bk = wxString(myConfig::getConfig().getValueOrDefault("app.backgroundcolor","119,119,119"));
	if (bk == "") bk = "119,119,119";
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
	
	//parm app.picpanel.backgroundcolor: r,g,b or t (0-255), set at startup. Same value rules as app.backgroundcolor, overrides it for the picture panel.  Default=(119,119,119)
	if (myConfig::getConfig().exists("app.picpanel.backgroundcolor")) {
		wxString pbk = wxString(myConfig::getConfig().getValueOrDefault("app.picpanel.backgroundcolor","119,119,119"));
		if (pbk == "") pbk = "119,119,119";
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
	
	//parm app.dock.backgroundcolor: r,g,b or t (0-255), set at startup. Same value rules as app.backgroundcolor, overrides it for the command/histogram/parameters dock.  Default=(119,119,119)
	if (myConfig::getConfig().exists("app.dock.backgroundcolor")) {
		wxString dbk = wxString(myConfig::getConfig().getValueOrDefault("app.dock.backgroundcolor","119,119,119"));
		if (dbk == "") dbk = "119,119,119";
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

void rawprocFrm::OnClose(wxCloseEvent& event)
{
	commandtree->DeleteAllItems();
	pic->BlankPic();
	histogram->BlankPic();
	parambook->DeleteAllPages();
	if ( help.GetFrame() ) // returns NULL if no help frame active
		help.GetFrame()->Close(true);
	// now we can safely delete the config pointer
	event.Skip();
	delete wxConfig::Set(NULL);
#ifdef SIZERLAYOUT
	if (vs) vs->~wxBoxSizer();
	if (hs) hs->~wxBoxSizer();
#else
	mgr.UnInit();
#endif
	Destroy();
}

void rawprocFrm::MnuexitClick(wxCommandEvent& event)
{
	commandtree->DeleteAllItems();
	pic->BlankPic();
	histogram->BlankPic();
	parambook->DeleteAllPages();
	if ( help.GetFrame() ) // returns NULL if no help frame active
		help.GetFrame()->Close(true);
	// now we can safely delete the config pointer
	event.Skip();
	delete wxConfig::Set(NULL);
#ifdef SIZERLAYOUT
	if (vs) vs->~wxBoxSizer();
	if (hs) hs->~wxBoxSizer();
#else
	mgr.UnInit();
#endif
	Destroy();
}

void rawprocFrm::SetThumbMode(int mode)
{
	pic->SetThumbMode(mode);
}

PicProcessor * rawprocFrm::GetItemProcessor(wxTreeItemId item)
{
	if (item.IsOk())
		return (PicProcessor *) commandtree->GetItemData(item);
	else
		wxMessageBox("bad item");
}



void rawprocFrm::InfoDialog(wxTreeItemId item)
{
	wxString exif="";
	gImage dib = ((PicProcessor *) commandtree->GetItemData(item))->getProcessedPic();

	exif.Append(wxString::Format("<b>Image Information:</b><br>\nWidth: %d Height: %d<br>\nColors: %d Original Image Bits: %s<br>\n<br>\n",dib.getWidth(), dib.getHeight(), dib.getColors(), dib.getBitsStr()));

	std::map<std::string,std::string> e =  dib.getInfo();
	for (std::map<std::string,std::string>::iterator it=e.begin(); it!=e.end(); ++it) {
		if (it->first == "ExifTag") continue;
		if (it->first == "ExposureTime") {
			float exp = atof(it->second.c_str());
			if (exp > 1.0)
				exif.Append(wxString::Format("<b>%s:</b> 1/%f sec<br>\n",it->first.c_str(),exp));
			else
				exif.Append(wxString::Format("<b>%s:</b> 1/%d sec<br>\n",it->first.c_str(),int(1.0/exp)));
		}
		else 
			exif.Append(wxString::Format("<b>%s:</b> %s<br>\n",it->first.c_str(),it->second.c_str()));
	}
	char buff[4096];
	exif.Append("<br><b>Image Stats:</b><br><pre>\n");
	exif.Append(dib.Stats().c_str());
	exif.Append("</pre><br>\n");

	char *profile = dib.getProfile();
	unsigned profile_length = dib.getProfileLength();

	if (profile_length && profile) {
		cmsHPROFILE icc = cmsOpenProfileFromMem(profile,profile_length);
		if (icc) {
			cmsUInt32Number n =  cmsGetProfileInfoASCII(icc, cmsInfoDescription, "en", "us", buff, 4096);
			exif.Append(wxString::Format("<br>\n<b>ICC Profile:</b> %s<br>\n", wxString(buff)));
			cmsCloseProfile(icc);
		}
		else exif.Append(wxString::Format("<br>\nICC Profile: failed (%d)<br>\n",profile_length));
	}
	else exif.Append(wxString::Format("<br>\nICC Profile: None (%d)<br>\n",profile_length));

	myEXIFDialog dlg(this, wxID_ANY, "Image Information", exif,  wxDefaultPosition, wxSize(400,500));
	dlg.ShowModal();
}

void rawprocFrm::EXIFDialog(wxFileName filename)
{
	if (!filename.FileExists()) return;
	//parm exif.command: Full path/filename to the Phil Harvey exiftool.exe program.  Default=(none), won't work without a valid program.
	wxString exifcommand = wxString(myConfig::getConfig().getValueOrDefault("exif.command",""));
	if (exifcommand == "") {
		wxMessageBox("No exiftool path defined in exif.command");
		return;
	}
	//parm exif.parameters: exiftool parameters used to format the exiftool output.  Default=-g -h, produces an HTML table, grouped by type.
	wxString exifparameters = wxString(myConfig::getConfig().getValueOrDefault("exif.parameters","-g -h"));

	wxString command = wxString::Format("%s %s %s",exifcommand, exifparameters, filename.GetFullPath());
	wxArrayString output;
	wxArrayString errors;
	SetStatusText(wxString::Format("Loading metadata using \"%s\"...",command));
	wxExecute (command, output, errors, wxEXEC_NODISABLE);
	wxString exif;
	for (int i=0; i<output.GetCount(); i++) exif.Append(output[i]);
	SetStatusText("");
	myEXIFDialog dlg(this, wxID_ANY, filename.GetFullName(), exif,  wxDefaultPosition, wxSize(500,500));
	dlg.ShowModal();
}


PicProcessor * rawprocFrm::AddItem(wxString name, wxString command)
{
	SetStatusText("");
	bool result = true;
	PicProcessor *p;

	if      (name == "gamma")      		p = new PicProcessorGamma("gamma",command, commandtree,  pic);
	else if (name == "bright")     		p = new PicProcessorBright("bright",command, commandtree, pic);
	else if (name == "contrast")   		p = new PicProcessorContrast("contrast",command, commandtree, pic);
	else if (name == "shadow")     		p = new PicProcessorShadow("shadow",command, commandtree, pic);
	else if (name == "highlight")  		p = new PicProcessorHighlight("highlight",command, commandtree, pic);
	else if (name == "saturation") 		p = new PicProcessorSaturation("saturation",command, commandtree, pic);
	else if (name == "curve")		p = new PicProcessorCurve("curve",command, commandtree, pic);
	else if (name == "gray")       		p = new PicProcessorGray("gray",command, commandtree, pic);
	else if (name == "crop")       		p = new PicProcessorCrop("crop",command, commandtree, pic);
	else if (name == "resize")		p = new PicProcessorResize("resize",command, commandtree, pic);
	else if (name == "blackwhitepoint")	p = new PicProcessorBlackWhitePoint("blackwhitepoint",command, commandtree, pic);
	else if (name == "sharpen")     	p = new PicProcessorSharpen("sharpen",command, commandtree, pic);
	else if (name == "rotate")		p = new PicProcessorRotate("rotate",command, commandtree, pic);
	else if (name == "denoise")		p = new PicProcessorDenoise("denoise",command, commandtree, pic);
	else if (name == "redeye")		p = new PicProcessorRedEye("redeye",command, commandtree, pic);
	else if (name == "colorspace")		p = new PicProcessorColorSpace("colorspace", command, commandtree, pic);
#ifdef USE_LENSFUN
	else if (name == "lenscorrection")	p = new PicProcessorLensCorrection("lenscorrection", command, commandtree, pic);
#endif
	else return NULL;
	p->createPanel(parambook);
	p->processPic();
	if (name == "colorspace") pic->SetProfile(p->getProcessedPicPointer());
	if (name == "resize") pic->SetScale(1.0);
	if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	Refresh();
	//Update();

	return p;
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
			cmd.Append(wxString::Format("%s",((PicProcessor *)commandtree->GetItemData(iter))->getCommand()));
			iter = commandtree->GetNextChild(root, cookie);
		}
	}

	return cmd;
}

void rawprocFrm::OpenFile(wxString fname) //, wxString params)
{
	filename.Assign(fname);
	if (!filename.FileExists()) {
		wxMessageBox(wxString::Format("Error: %s not found.",filename.GetFullName()));
		return;
	}
	sourcefilename.Clear();
	gImage *dib;
	GIMAGE_FILETYPE fif;
	fif = gImage::getFileType(fname.c_str());

	wxFileName profilepath;
	//parm cms.profilepath: Directory path where ICC colorspace profiles can be found.  Default: (none, implies current working directory)
	profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","")));


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
			configparams = paramString("input.raw.libraw.");
			//parm input.raw.parameters: name=value list of parameters, separated by semicolons, to pass to the raw image reader.  Default=(none)
			configparams.Append(wxString(myConfig::getConfig().getValueOrDefault("input.raw.parameters","")));
#endif
			//parm input.raw.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.raw.cms.profile","")));
		}
		

		if (fif == FILETYPE_JPEG) {
			//parm input.jpeg.parameters: name=value list of parameters, separated by semicolons, to pass to the JPEG image reader.  Default=(none)
			configparams = wxString(myConfig::getConfig().getValueOrDefault("input.jpeg.parameters",""));
			//parm input.jpeg.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.jpeg.cms.profile","")));
		}

		
		if (fif == FILETYPE_TIFF) {
			//parm input.tiff.parameters: name=value list of parameters, separated by semicolons, to pass to the TIFF image reader.  Default=(none)
			configparams = wxString(myConfig::getConfig().getValueOrDefault("input.tiff.parameters",""));
			//parm input.tiff.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.tiff.cms.profile","")));
		}
		if (fif == FILETYPE_PNG) {
			//parm input.png.parameters: name=value list of parameters, separated by semicolons, to pass to the PNG image reader.  Default=(none)
			configparams = wxString(myConfig::getConfig().getValueOrDefault("input.png.parameters",""));
			//parm input.png.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.png.cms.profile","")));
		}

		SetStatusText(wxString::Format("Loading file:%s params:%s",filename.GetFullName(), configparams));

		if (!wxFileName::FileExists(fname)) {
			wxMessageBox(wxString::Format("Error: Source file %s not found", filename.GetFullName()));
			SetStatusText("");
			return;
		}

		pic->BlankPic();
		histogram->BlankPic();
		parambook->DeleteAllPages();
		
		mark();
		dib = new gImage(gImage::loadImageFile(fname.c_str(), (std::string) configparams.c_str()));
		wxString loadtime = duration();
		if (dib->getWidth() == 0) {
			wxMessageBox(wxString::Format("Error: File %s not loaded successfully", filename.GetFullName()));
			SetStatusText("");
			return;
		}
		//wxString flagstring(params.c_str());
		//parm input.log: log the file input operation.  Default=0
		if (myConfig::getConfig().getValueOrDefault("input.log","0") == "1")
			log(wxString::Format("file input,filename=%s,imagesize=%dx%d,time=%s",filename.GetFullName(),dib->getWidth(), dib->getHeight(),loadtime));

		//parm input.cms: When a file is input, enable or disable color management.  Default=0
		if (myConfig::getConfig().getValueOrDefault("input.cms","0") == "1") {

			if (myConfig::getConfig().getValueOrDefault("display.cms.displayprofile","") == "") {
				//parm display.cms.requireprofile: Enforce display profile requirement.  If 0, image will be displayed 'as-is' with no output profile transform.  Default=1
				if (myConfig::getConfig().getValueOrDefault("display.cms.requireprofile","1") == "1") {
					wxMessageBox("CMS enabled, but no display profile was found.  Color management is disabled.");
					pic->SetImageProfile(NULL);
					pic->SetColorManagement(false);
				}
				else {
					wxMessageBox("No display profile was found.  Image will be displayed 'as-is' with no display profile conversion.");
					pic->SetImageProfile(NULL);
				}
			}
			else {

				cmsHPROFILE hImgProf;
				if (dib->getProfile() != NULL & dib->getProfileLength() > 0) {
					hImgProf = cmsOpenProfileFromMem(dib->getProfile(), dib->getProfileLength());
					pic->SetImageProfile(hImgProf);
					pic->SetColorManagement(true);
				}
				else {
					if (profilepath.FileExists()) {
						if (wxMessageBox(wxString::Format("Color management enabled, and no color profile was found in %s.  Assign the default input profile, %s?",filename.GetFullName(),profilepath.GetFullPath()),"foo",wxYES_NO) == wxYES) {
							hImgProf = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
							//hImgProf = gImage::makeLCMSProfile("srgb", 2.2);
							char * prof; cmsUInt32Number proflen;
							if (hImgProf) {
								gImage::makeICCProfile(hImgProf, prof, proflen);
								dib->setProfile(prof, proflen);
								pic->SetImageProfile(hImgProf);
								pic->SetColorManagement(true);
							}
							else {
								wxMessageBox("Set profile failed, disabling color management.");
								pic->SetImageProfile(NULL);
								pic->SetColorManagement(false);
							}
						}
						else {
							SetStatusText("Disabling color management");
							pic->SetImageProfile(NULL);
							pic->SetColorManagement(false);
						}
					}
					else {
						wxMessageBox("No input profile found, disabling color management.");
						pic->SetImageProfile(NULL);
						pic->SetColorManagement(false);
					}
				}
			}
		}
		else {
			pic->SetImageProfile(NULL);
			pic->SetColorManagement(false);
		}


		PicProcessor *picdata = new PicProcessor(filename.GetFullName(), configparams, commandtree, pic, dib);
		picdata->createPanel(parambook);
		CommandTreeSetDisplay(picdata->GetId());
		//pic->SetScaleToWidth();
		pic->FitMode(true);
		SetStatusText("scale: fit",2);
		SetTitle(wxString::Format("rawproc: %s",filename.GetFullName()));
		SetStatusText("");

		//parm input.raw.default: Space-separated list of rawproc tools to apply to a raw image after it is input. If this parameter has an entry, application of the tools is prompted yes/no.  Default=(none).  Note: If a raw file is opened with this parameter, if it is re-opened, you'll be prompted to apply the input.raw.default.commands, then prompted to re-apply the processing chain.  In this case, say 'no' to the first one, and 'yes' to the second, otherwise you'll duplicate the input.raw.default commands."
		wxString raw_default = wxString(myConfig::getConfig().getValueOrDefault("input.raw.default",""));
		if ((fif == FILETYPE_RAW) & (raw_default != "")) {
			if (wxMessageBox(wxString::Format("Apply %s to raw file?",raw_default), "Confirm", wxYES_NO, this) == wxYES) {
				wxArrayString token = split(raw_default, " ");
				try {
					for (int i=0; i<token.GetCount(); i++) {
						wxArrayString cmd = split(token[i], ":");
						if (cmd.GetCount() == 2)
							AddItem(cmd[0], cmd[1]);
						else
							AddItem(cmd[0], "");
						wxSafeYield(this);
					}
				}
				catch (std::exception& e) {
					wxMessageBox(wxString::Format("Error: Adding tool failed: %s",e.what()));
				}
			}
		}
		
		opensource = false;

		SetStatusText(wxString::Format("File:%s opened.",filename.GetFullName()));
	}
	else {
		SetStatusText(wxString::Format("%s file type unknown.",filename.GetFullName() ));
	}
}

void rawprocFrm::OpenFileSource(wxString fname)
{
	gImage *dib;
	wxString ofilename, inputprofile;
	wxString oparams = "";

	SetStatusText("Retrieving source script...");
	std::map<std::string,std::string> info =  gImage::getInfo(fname.c_str());

	if(info.find("ImageDescription") != info.end() && info["ImageDescription"].find("rawproc") != std::string::npos ) {
		wxString script = info["ImageDescription"];
		wxArrayString token = split(script, " ");
			
		if (token[1].Contains(":")) {
			wxArrayString fparams = split(token[1],":");
			if (fparams.GetCount() >1) {
				oparams = fparams[1];
			}
			ofilename = fparams[0];
		}
		else ofilename = token[1];
				
		if (token[0].Find("rawproc") == wxNOT_FOUND) {
			wxMessageBox(wxString::Format("Source script not found in %s, aborting Open Source.", filename.GetFullName().c_str()) );
		}
		else {
			SetStatusText(wxString::Format("Source script found, loading source file %s...",ofilename) );

			if (!wxFileName::FileExists(ofilename)) {  //source file not found in same directory as destination file
				//parm input.opensource.subdirectory: Enable search of specific directory for source file.  
				wxString subdir = wxString(myConfig::getConfig().getValueOrDefault("input.opensource.subdirectory",""));
				if (subdir != "") {
					wxFileName findfilesubdir(ofilename);
					if (findfilesubdir.Normalize()) {
						findfilesubdir.AppendDir(subdir);  //see if source file is in the parent directory
						if (findfilesubdir.Exists()) {
							//SetStatusText(wxString::Format("Loading source file %s from %s directory...",ofilename,subdir) );
							ofilename = findfilesubdir.GetFullPath();
						}
					}
					else {
						//parm input.opensource.parentdirectory: Enable search of parent directory for source file.  Default=0 
						if (myConfig::getConfig().getValueOrDefault("input.opensource.parentdirectory","0") == "1") {
							wxFileName findfileparentdir(ofilename);
							if (findfileparentdir.Normalize()) {
								findfileparentdir.RemoveLastDir();  //see if source file is in the parent directory
								if (findfileparentdir.Exists()) {
									//SetStatusText(wxString::Format("Loading source file %s from parent directory...",ofilename) );
									ofilename = findfileparentdir.GetFullPath();
								}
								else {
									wxMessageBox(wxString::Format("Error: Source file %s not found in source, parent or sub directory", ofilename));
									SetStatusText("");
									return;
								}
							}
						}
					}
				}
//				else {
//					wxMessageBox(wxString::Format("Error: Source file %s not found", ofilename));
//					SetStatusText("");
//					return;
//				}
			}

			GIMAGE_FILETYPE fif;
			fif = gImage::getFileType(ofilename.c_str());

			wxFileName profilepath;
			profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","")));


			if (fif == FILETYPE_RAW) {
				profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.raw.cms.profile","")));
			}
			if (fif == FILETYPE_JPEG) {
				profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.jpeg.cms.profile","")));
			}
			if (fif == FILETYPE_TIFF) {
				profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.tiff.cms.profile","")));
			}
			if (fif == FILETYPE_PNG) {
				profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("input.png.cms.profile","")));
			}

			pic->BlankPic();
			histogram->BlankPic();
			parambook->DeleteAllPages();
			SetStatusText(wxString::Format("Loading file:%s params:%s",ofilename, oparams));

			mark();
			dib = new gImage(gImage::loadImageFile(ofilename.c_str(), (std::string) oparams.c_str()));
			wxString loadtime = duration();
			if (dib->getWidth() == 0) {
				wxMessageBox(wxString::Format("Error: File %s load failed", ofilename));
				SetStatusText("");
				return;
			}
			
			filename.Assign(ofilename);
			sourcefilename.Assign(fname);
			
			if (myConfig::getConfig().getValueOrDefault("input.log","0") == "1")
				log(wxString::Format("file input,filename=%s,imagesize=%dx%d,time=%s",filename.GetFullName(),dib->getWidth(), dib->getHeight(),loadtime));



			if (myConfig::getConfig().getValueOrDefault("input.cms","0") == "1") {

				if (myConfig::getConfig().getValueOrDefault("display.cms.displayprofile","") == "") {
					if (myConfig::getConfig().getValueOrDefault("display.cms.requireprofile","1") == "1") {
						wxMessageBox("CMS enabled, but no display profile was found.  Color management is disabled.");
						pic->SetImageProfile(NULL);
						pic->SetColorManagement(false);
					}
					else {
						wxMessageBox("No display profile was found.  Image will be displayed 'as-is' with no display profile conversion.");
						pic->SetImageProfile(NULL);
					}
				}
				else {

					cmsHPROFILE hImgProf;
					if (dib->getProfile() != NULL & dib->getProfileLength() > 0) {
						hImgProf = cmsOpenProfileFromMem(dib->getProfile(), dib->getProfileLength());
						pic->SetImageProfile(hImgProf);
						pic->SetColorManagement(true);
					}
					else {
						if (profilepath.FileExists()) {
							if (wxMessageBox(wxString::Format("Color management enabled, and no color profile was found in %s.  Assign the default input profile, %s?",filename.GetFullName(),profilepath.GetFullPath()),"foo",wxYES_NO) == wxYES) {
								hImgProf = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
								//hImgProf = gImage::makeLCMSProfile("srgb", 2.2);
								char * prof; cmsUInt32Number proflen;
								if (hImgProf) {
									gImage::makeICCProfile(hImgProf, prof, proflen);
									dib->setProfile(prof, proflen);
									pic->SetImageProfile(hImgProf);
									pic->SetColorManagement(true);
								}
								else {
									wxMessageBox("Set profile failed, disabling color management.");
									pic->SetImageProfile(NULL);
									pic->SetColorManagement(false);
								}
							}
							else {
								SetStatusText("Disabling color management");
								pic->SetImageProfile(NULL);
								pic->SetColorManagement(false);
							}
						}
						else {
							wxMessageBox("No input profile found, disabling color management.");
							pic->SetImageProfile(NULL);
							pic->SetColorManagement(false);
						}
					}
				}
			}
			else {
				pic->SetImageProfile(NULL);
				pic->SetColorManagement(false);
			}


			//pic->SetScaleToWidth();
			pic->FitMode(true);
			SetStatusText("scale: fit",2);
			PicProcessor *picdata = new PicProcessor(filename.GetFullName(), oparams, commandtree, pic, dib);
			picdata->createPanel(parambook);
			CommandTreeSetDisplay(picdata->GetId());
			SetTitle(wxString::Format("rawproc: %s (%s)",filename.GetFullName().c_str(), sourcefilename.GetFullName().c_str()));
			
			for (int i=2; i<token.GetCount(); i++) {
				wxArrayString cmd = split(token[i], ":");					
				if (AddItem(cmd[0], cmd[1])) 
					wxSafeYield(this);
				else
					wxMessageBox(wxString::Format("Unknown command: %s",cmd[0]));
			}
			
			opensource = true;

			SetStatusText(wxString::Format("Source of file:%s opened.",sourcefilename.GetFullName()));
		}
			
	}
	else {
		wxMessageBox(wxString::Format("No source script found in %s, aborting Open Source.",fname ));
	}

	SetStatusText("");
}


void rawprocFrm::Mnusave1009Click(wxCommandEvent& event)
{
	wxString fname;
	gImage * dib;
	cmsHPROFILE profile;

	wxFileName profilepath;
	profilepath.AssignDir(wxString(myConfig::getConfig().getValueOrDefault("cms.profilepath","")));

	if (!sourcefilename.IsOk()) 
		fname = wxFileSelector("Save image...",filename.GetPath(),filename.GetName(),filename.GetExt(),"JPEG files (*.jpg)|*.jpg|TIFF files (*.tif)|*.tif|PNG files (*.png)|*.png",wxFD_SAVE);  // 
	else
		fname = wxFileSelector("Save image...",sourcefilename.GetPath(),sourcefilename.GetName(),sourcefilename.GetExt(),"JPEG files (*.jpg)|*.jpg|TIFF files (*.tif)|*.tif|PNG files (*.png)|*.png",wxFD_SAVE);  // 

	if ( !fname.empty() )
	{
		if (wxFileName::FileExists(fname)) 
			if (wxMessageBox("File exists; overwrite?", "Confirm", wxYES_NO, this) == wxNO)
				return;
			
			GIMAGE_FILETYPE filetype = gImage::getFileNameType(fname);
			
			if (filetype == FILETYPE_UNKNOWN) {
				wxMessageBox("Error: invalid file type");
				return;
			}
			if (commandtree->ItemHasChildren(commandtree->GetRootItem()))
				dib = ((PicProcessor *) commandtree->GetItemData( commandtree->GetLastChild(commandtree->GetRootItem())))->getProcessedPicPointer();
			else
				dib = ((PicProcessor *) commandtree->GetItemData( commandtree->GetRootItem()))->getProcessedPicPointer();

			dib->setInfo("ImageDescription",(std::string) AssembleCommand().c_str());
			wxString versionstr = "(dev build)";
			#ifdef VERSION
				versionstr = VERSION;
			#endif
			dib->setInfo("Software",(std::string) wxString::Format("rawproc %s",versionstr).c_str());
			
			int rotation = atoi(dib->getInfoValue("Orientation").c_str());
			//parm output.orient: Rotate the image to represent the EXIF Orientation value originally inputted, then set the Orientation tag to 0.  Gets the image out of trying to tell other software how to orient it.  Default=0
			if ((myConfig::getConfig().getValueOrDefault("output.orient","0") == "1") & (rotation != 0)) {
				WxStatusBar1->SetStatusText(wxString::Format("Orienting image for output..."));
				if (rotation == 2) dib->ApplyHorizontalMirror();
				if (rotation == 3) dib->ApplyRotate180();
				if (rotation == 4) dib->ApplyVerticalMirror();
				if (rotation == 5) {dib->ApplyRotate90(); dib->ApplyHorizontalMirror();}
				if (rotation == 6) dib->ApplyRotate90();
				if (rotation == 7) {dib->ApplyRotate270(); dib->ApplyHorizontalMirror(); }
				if (rotation == 8) dib->ApplyRotate270();
				dib->setInfo("Orientation","0");
			}

			wxString configparams;
			//parm output.jpeg.parameters: name=value list of parameters, separated by semicolons, to pass to the JPEG image writer.  Default=(none)
			if (filetype == FILETYPE_JPEG) configparams = myConfig::getConfig().getValueOrDefault("output.jpeg.parameters","");

			//parm output.tiff.parameters: name=value list of parameters, separated by semicolons, to pass to the TIFF image writer.  Default=(none)
			if (filetype == FILETYPE_TIFF) configparams =  myConfig::getConfig().getValueOrDefault("output.tiff.parameters","");

			//parm output.png.parameters: name=value list of parameters, separated by semicolons, to pass to the PNG image writer.  Default=(none)
			if (filetype == FILETYPE_PNG) configparams =  myConfig::getConfig().getValueOrDefault("output.png.parameters","");


			if (pic->GetColorManagement()) {

				wxString intentstr;
				cmsUInt32Number intent = INTENT_PERCEPTUAL;

				if (filetype == FILETYPE_JPEG) {
					//parm output.jpeg.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=srgb
					profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("output.jpeg.cms.profile","")));
					//parm output.jpeg.cms.renderingintent: Specify the rendering intent for the JPEG output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
					intentstr = wxString(myConfig::getConfig().getValueOrDefault("output.jpeg.cms.renderingintent","perceptual"));
				}
				else if (filetype == FILETYPE_TIFF) {
					//parm output.tiff.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=prophoto
					profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("output.tiff.cms.profile","")));
					//parm output.tiff.cms.renderingintent: Specify the rendering intent for the JPEG output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
					intentstr = wxString(myConfig::getConfig().getValueOrDefault("output.tiff.cms.renderingintent","perceptual"));
				}
				else if (filetype == FILETYPE_PNG) {
					//parm output.png.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=prophoto
					profilepath.SetFullName(wxString(myConfig::getConfig().getValueOrDefault("output.png.cms.profile","")));
					//parm output.png.cms.renderingintent: Specify the rendering intent for the JPEG output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
					intentstr = wxString(myConfig::getConfig().getValueOrDefault("output.png.cms.renderingintent","perceptual"));
				}
				
				if (intentstr == "perceptual") intent = INTENT_PERCEPTUAL;
				if (intentstr == "saturation") intent = INTENT_SATURATION;
				if (intentstr == "relative_colorimetric") intent = INTENT_RELATIVE_COLORIMETRIC;
				if (intentstr == "absolute_colorimetric") intent = INTENT_ABSOLUTE_COLORIMETRIC;

				profile = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
				if (!profile) {
					wxMessageBox(wxString::Format("No CMS profile found, saving with the working profile..."));
					WxStatusBar1->SetStatusText(wxString::Format("Saving %s with working profile...",fname));
					dib->saveImageFile(fname, std::string(configparams.c_str()));
				}
				else {
					WxStatusBar1->SetStatusText(wxString::Format("Saving %s with icc profile %s, rendering intent %s...",fname, profilepath.GetFullName(), intentstr));
					dib->saveImageFile(fname, std::string(configparams.c_str()), profile, intent);
				}


			}
			else {
				WxStatusBar1->SetStatusText(wxString::Format("Saving %s...",fname));
				dib->saveImageFile(fname, std::string(configparams.c_str()));
			}
			wxFileName tmpname(fname);

			if (tmpname.GetFullName().compare(filename.GetFullName()) != 0) {
				sourcefilename.Assign(fname);
				SetTitle(wxString::Format("rawproc: %s (%s)",filename.GetFullName().c_str(), sourcefilename.GetFullName().c_str()));
			}
			
		WxStatusBar1->SetStatusText("");
	}
}

void rawprocFrm::CommandTreeSetDisplay(wxTreeItemId item)
{
	SetStatusText("");
	if (!item.IsOk()) return;
	if (displayitem.IsOk()) commandtree->SetItemState(displayitem,0);
	commandtree->SetItemState(item,1);
	displayitem = item;
	pic->SetPic( ((PicProcessor *) commandtree->GetItemData(item))->getProcessedPicPointer(), ((PicProcessor *) commandtree->GetItemData(item))->getChannel() );
	pic->SetDrawList(((PicProcessor *) commandtree->GetItemData(item))->getDrawList() );
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


void rawprocFrm::CommandTreeStateClick(wxTreeEvent& event)
{
	SetStatusText("");
	wxTreeItemId item = event.GetItem();
	wxTreeItemId prev = displayitem;
	CommandTreeSetDisplay(item);
	if (isDownstream(prev, item)) {
		wxTreeItemId next = commandtree->GetNextSibling(prev);
		if (next.IsOk()) ((PicProcessor *) commandtree->GetItemData(next))->processPic();
	}
	event.Skip();
}


void rawprocFrm::CommandTreeSelChanged(wxTreeEvent& event)
{
	SetStatusText("");
	wxTreeItemId item = event.GetItem();
	if (item.IsOk()) { 
		if ((PicProcessor *) commandtree->GetItemData(item))
			if (parambook->FindPage(((PicProcessor *) commandtree->GetItemData(item))->getPanel()) != wxNOT_FOUND)
				parambook->SetSelection(parambook->FindPage(((PicProcessor *) commandtree->GetItemData(item))->getPanel()));
	}
	event.Skip();
}

void rawprocFrm::CommandTreeDeleteItem(wxTreeItemId item)
{
	wxTreeItemId prev, next, newitem;
	if (commandtree->GetItemParent(item).IsOk()) {  //not root
		prev = commandtree->GetPrevSibling(item);
		next = commandtree->GetNextSibling(item);
		if (!prev.IsOk()) prev = commandtree->GetRootItem();
		if (commandtree->GetItemState(item) == 1) CommandTreeSetDisplay(prev);
		if (next.IsOk())
			newitem = next;
		else
			newitem = prev;
		commandtree->SelectItem(newitem);
		parambook->DeletePage(parambook->FindPage(((PicProcessor *) commandtree->GetItemData(item))->getPanel()));
       		commandtree->Delete(item);
		if (newitem.IsOk()) {
			((PicProcessor *) commandtree->GetItemData(newitem))->processPic();
		}
		deleting = true;
	}
}

void rawprocFrm::CommandTreeKeyDown(wxTreeEvent& event)
{
	SetStatusText("");
	//wxTreeItemId item, prev, next, newitem;
	switch (event.GetKeyCode()) {
        case 127:  //Delete
	case 8: //Backspace
		//CommandTreeDeleteItem(commandtree->GetSelection());
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
		SetStatusText("scale: fit",2);
		break;
	case 67: //c - test cropmode
		//pic->ToggleCropMode();
		break;
	case 80: //p - process command
		WxStatusBar1->SetStatusText("processing...");
		((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->processPic();
		WxStatusBar1->SetStatusText("");
		break;
	case 116: //t
	case 84: //T - toggle display thumbnail
		pic->ToggleThumb();
		break;
    	}
	//wxMessageBox(wxString::Format("keycode: %d", event.GetKeyCode()));
	event.Skip();
}

void rawprocFrm::CommandTreeDeleteItem(wxTreeEvent& event)
{
	wxTreeItemId s = commandtree->GetSelection();
	if (s)
		((PicProcessor *) commandtree->GetItemData(s))->processPic();
	((PicProcessor *) commandtree->GetItemData(event.GetItem()))->processPic();
	event.Skip();
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
	wxString fname = wxFileSelector("Open Image...", openfilepath);	
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
			int result = wxMessageBox(wxString::Format("Open source file %s with the old processing?\n\nSelecting No will open the source file with current Properties parameters and the current processing chain.\n\nCancel aborts the whole thing.",filename.GetFullName()), "Confirm", wxYES_NO |wxCANCEL, this);
			if (result == wxYES) {
				OpenFileSource(sourcefilename.GetFullPath());
			}
			else if (result == wxNO) {
				OpenFile(filename.GetFullPath());
				wxArrayString token = split(cmdstring, " ");
				for (int i=2; i<token.GetCount(); i++) {
					wxArrayString cmd = split(token[i], ":");					
					if (AddItem(cmd[0], cmd[1])) 
						wxSafeYield(this);
					else
						wxMessageBox(wxString::Format("Unknown command: %s",cmd[0]));
				}
			}
		}
		else {
			OpenFile(filename.GetFullPath());
			wxArrayString token = split(cmdstring, " ");
			if (token.GetCount() > 2) {
				if (wxMessageBox("Re-apply processing chain?", "Confirm", wxYES_NO, this) == wxYES) {
					for (int i=2; i<token.GetCount(); i++) {
						wxArrayString cmd = split(token[i], ":");					
						if (AddItem(cmd[0], cmd[1])) 
							wxSafeYield(this);
						else
							wxMessageBox(wxString::Format("Unknown command: %s",cmd[0]));
					}
				}
			}
		}
	}
	else {
		wxMessageBox("No file to re-open.");
	}
}


void rawprocFrm::Mnuopensource1004Click(wxCommandEvent& event)
{
	wxString fname = wxFileSelector("Open Image source...", openfilepath);	
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
		wxMessageBox("No configuration file found.");
		return;
	}
	if (diag == NULL) {
		diag = new PropertyDialog(this, wxID_ANY, "Properties");
		Bind(wxEVT_PG_CHANGED,&rawprocFrm::UpdateConfig,this);
	}
	if (diag) {
		diag->ClearModifiedStatus();
		diag->Show();
	}
	else {
		wxMessageBox("Failed to create Properties dialog");
	}
}

void rawprocFrm::MnuEXIF(wxCommandEvent& event)
{
	EXIFDialog(filename);
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

	SetStatusText(wxString::Format("Changed %s to %s.", propname, propval));
	myConfig::getConfig().setValue((const char  *) propname.mb_str(), (const char  *) propval.mb_str());
	myConfig::getConfig().flush();

	//check for properties that should update immediately:
	if (propname.Find("display.cms") != wxNOT_FOUND)
		if (!commandtree->IsEmpty())
			pic->RefreshPic();
	if (propname.Find("display.outofbound") != wxNOT_FOUND)
		pic->RefreshPic();
	if (propname.Find("histogram.singlechannel") != wxNOT_FOUND)
		pic->RefreshPic();
	//not ready for prime time
	//if (propname.Find("backgroundcolor") != wxNOT_FOUND) SetBackground();
}


/*
 * Mnugamma1006Click
*/
void rawprocFrm::Mnugamma1006Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.gamma.initialvalue: The initial (and reset button) value of the gamma tool, 1.0=no change (linear).  Default=2.2
		wxString val = wxString(myConfig::getConfig().getValueOrDefault("tool.gamma.initialvalue","2.2"));
		PicProcessorGamma *p = new PicProcessorGamma("gamma",val, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding gamma tool failed: %s",e.what()));
	}
}


/*
 * Mnubright1007Click
*/
void rawprocFrm::Mnubright1007Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.bright.initialvalue: The initial (and reset button) value of the bright tool, 0=no change.  Default=0
		wxString val = wxString(myConfig::getConfig().getValueOrDefault("tool.bright.initialvalue","0"));
		PicProcessorBright *p = new PicProcessorBright("bright",val, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding bright tool failed: %s",e.what()));
	}
}


/*
 * Mnucontrast1008Click
 */
void rawprocFrm::Mnucontrast1008Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.contrast.initialvalue: The initial (and reset button) value of the contrast tool, 0=no change.  Default=0
		wxString val = wxString(myConfig::getConfig().getValueOrDefault("tool.contrast.initialvalue","0"));
		PicProcessorContrast *p = new PicProcessorContrast("contrast",val, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding contrast tool failed: %s",e.what()));
	}
}



void rawprocFrm::MnusaturateClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.saturate.initialvalue: The initial (and reset button) value of the saturation tool, 1.0=no change.  Default=1.0
		wxString val = wxString(myConfig::getConfig().getValueOrDefault("tool.saturate.initialvalue","1.0"));
		PicProcessorSaturation *p = new PicProcessorSaturation("saturation",val, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding saturation tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuexposureClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.exposure.initialvalue: The initial (and reset button) value of the saturation tool, 1.0=no change.  Default=0.0
		wxString val = wxString(myConfig::getConfig().getValueOrDefault("tool.exposure.initialvalue","0.0"));
		PicProcessorExposure *p = new PicProcessorExposure("exposure",val, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding exposure tool failed: %s",e.what()));
	}
}


void rawprocFrm::Mnucurve1010Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorCurve *p = new PicProcessorCurve("curve","0.0,0.0,255.0,255.0", commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding curve tool failed: %s",e.what()));
	}
}



void rawprocFrm::MnuShadow1015Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.shadow.level: The initial (and reset button) value of the shadow tool, 0=no change.  Default=0
		wxString level = wxString(myConfig::getConfig().getValueOrDefault("tool.shadow.level","0"));
		//parm tool.shadow.threshold: The initial (and reset button) value of the shadow curve threshold.  Default=64
		wxString threshold = wxString(myConfig::getConfig().getValueOrDefault("tool.shadow.threshold","64"));
		wxString cmd= wxString::Format("%s,%s",level,threshold);
		PicProcessorShadow *p = new PicProcessorShadow("shadow",cmd, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding shadow tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuHighlightClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.highlight.level: The initial (and reset button) value of the highlight tool, 0=no change.  Default=0
		wxString level = wxString(myConfig::getConfig().getValueOrDefault("tool.highlight.level","0"));
		//parm tool.highlight.threshold: The initial (and reset button) value of the highlight curve threshold.  Default=192
		wxString threshold = wxString(myConfig::getConfig().getValueOrDefault("tool.highlight.threshold","192"));
		wxString cmd= wxString::Format("%s,%s",level,threshold);
		PicProcessorHighlight *p = new PicProcessorHighlight("highlight",cmd, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding highlight tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuGrayClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
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
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding grayscale tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuCropClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorCrop *p = new PicProcessorCrop("crop", commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding crop tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuResizeClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.resize.x: Default resize of the width dimension.  Default=640
		wxString x =  wxString(myConfig::getConfig().getValueOrDefault("tool.resize.x","640"));
		//parm tool.resize.y: Default resize of the height dimension.  Default=0 (calculate value to preserve aspect)
		wxString y =  wxString(myConfig::getConfig().getValueOrDefault("tool.resize.y","0"));
		//parm tool.resize.algorithm: Sets the algorithm used to interpolate resized pixels. Available algorithms are box, bilinear, bspline, bicubic, catmullrom, lanczos3.  Default=catmullrom
		wxString algo =  wxString(myConfig::getConfig().getValueOrDefault("tool.resize.algorithm","catmullrom"));
		wxString cmd= wxString::Format("%s,%s,%s",x,y,algo);
		PicProcessorResize *p = new PicProcessorResize("resize", cmd, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		pic->SetScale(1.0);
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding resize tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuBlackWhitePointClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorBlackWhitePoint *p;
		//parm tool.blackwhitepoint.auto: Invoke auto calculation of inital black and white point values, based on a percent-pixels threshold.  Currently, this behavior is only invoked when the tool is added, so re-application requires deleting and re-adding the tool.  Default=0
		if (myConfig::getConfig().getValueOrDefault("tool.blackwhitepoint.auto","0") == "1")
			p = new PicProcessorBlackWhitePoint("blackwhitepoint", "", commandtree, pic);
		else
			p = new PicProcessorBlackWhitePoint("blackwhitepoint", "0,255", commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding blackwhitepoint tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuSharpenClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.sharpen.initialvalue: The initial (and reset button) value of the sharpen tool, 0=no change.  Default=0
		wxString defval =  wxString(myConfig::getConfig().getValueOrDefault("tool.sharpen.initialvalue","0"));
		PicProcessorSharpen *p = new PicProcessorSharpen("sharpen", defval, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding sharpen tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuRotateClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.rotate.initialvalue: The initial (and reset button) angle of the rotate tool, 0=no change.  Default=0
		wxString defval =  wxString(myConfig::getConfig().getValueOrDefault("tool.rotate.initialvalue","0.0"));
		PicProcessorRotate *p = new PicProcessorRotate("rotate", defval, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding rotate tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuDenoiseClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		//parm tool.denoise.initialvalue: The initial (and reset button) sigma value used to calculate the denoised pixel.  Default=0
		wxString sigma =  wxString(myConfig::getConfig().getValueOrDefault("tool.denoise.initialvalue","0"));
		//parm tool.denoise.local: Defines the initial (and reset button) size of the neigbor pixel array.  Default=3
		wxString local =  wxString(myConfig::getConfig().getValueOrDefault("tool.denoise.local","3"));
		//parm tool.denoise.patch: Defines the initial (and reset button) size of the patch pixel array.  Default=1
		wxString patch =  wxString(myConfig::getConfig().getValueOrDefault("tool.denoise.patch","1"));
		wxString cmd = wxString::Format("%s,%s,%s",sigma,local,patch);
		PicProcessorDenoise *p = new PicProcessorDenoise("denoise", cmd, commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding denoise tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuRedEyeClick(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
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
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding redeye tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuColorSpace(wxCommandEvent& event)
{
	//if (!pic->GetColorManagement()) {
	//	if (myConfig::getConfig().getValueOrDefault("display.cms.requireprofile","1") == "1") {
	//		wxMessageBox("Color management disabled, no input profile for colorspace");
	//		return;
	//	}
	//}

	if (commandtree->IsEmpty()) return;

	if (PicProcessor::getSelectedPicProcessor(commandtree)->getProcessedPic().getProfile() == NULL) {
		wxMessageBox("Note: Image does not have a source profile, only 'assign' is valid");
	}

	SetStatusText("");
	try {
		PicProcessorColorSpace *p = new PicProcessorColorSpace("colorspace", "(none),-,-", commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding colorspace tool failed: %s",e.what()));
	}
}

#ifdef USE_LENSFUN
void rawprocFrm::MnuLensCorrection(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	
	lfError e;
	struct lfDatabase *ldb = lf_db_new ();
	if (lf_db_load (ldb) != LF_NO_ERROR) {
		wxMessageBox("Error: Cannot open lens correction database.") ;
		if (ldb) lf_db_destroy (ldb);
		return;
	}
	if (ldb) lf_db_destroy (ldb);

	SetStatusText("");
	try {
		PicProcessorLensCorrection *p = new PicProcessorLensCorrection("lenscorrection", "", commandtree, pic);
		p->createPanel(parambook);
		p->processPic();
		if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding lenscorrection tool failed: %s",e.what()));
	}
}
#endif


void rawprocFrm::MnuCut1201Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	if (commandtree->GetSelection() == commandtree->GetRootItem()) return;
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getCommand() ) );
		wxTheClipboard->Close();
		commandtree->Delete(commandtree->GetSelection());
	}
}

void rawprocFrm::MnuCopy1202Click(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData( new wxTextDataObject(((PicProcessor *) commandtree->GetItemData(commandtree->GetSelection()))->getCommand() ) );
		wxTheClipboard->Close();
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
			AddItem(s[0], s[1]);
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
		myEXIFDialog dlg(this, wxID_ANY, "Image Command", cmd,  wxDefaultPosition, wxSize(400,200));
		dlg.ShowModal();
	}
	else wxMessageBox("Invalid dialog type for Show Command... (menu.showcommand.type)");
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

	info.SetCopyright(wxT("(C) 2017 Glenn Butcher <glenn.butcher@gmail.com>"));
	
	//wxString WxWidgetsVersion = wxGetLibraryVersionInfo().GetVersionString();
	wxVersionInfo wxversion = wxGetLibraryVersionInfo();

	wxString WxWidgetsVersion = wxString::Format("%s %d.%d.%d", wxversion.GetName(), wxversion.GetMajor(), wxversion.GetMinor(), wxversion.GetMicro());
	wxString libraries = wxString(gImage::LibraryVersions());
	wxString pixtype = wxString(gImage::getRGBCharacteristics());
#ifdef BUILDDATE
	wxString builddate = wxString(BUILDDATE);
	info.SetDescription(wxString::Format("Basic camera raw file and image editor.\n\nLibraries:\n%s\n%s\n\nPixel Format: %s\n\nConfiguration file: %s\n\nBuild Date: %s", WxWidgetsVersion, libraries.c_str(),pixtype, configfile, builddate));
#else
	info.SetDescription(wxString::Format("Basic camera raw file and image editor.\n\nLibraries:\n%s\n%s\n\nPixel Format: %s\n\nConfiguration file: %s", WxWidgetsVersion, libraries.c_str(),pixtype, configfile));
#endif

	wxAboutBox(info);

}

void rawprocFrm::MnuHelpClick(wxCommandEvent& event)
{
	help.DisplayContents();
}

#define ID_EXIF		2001
//#define ID_HISTOGRAM	2002
#define ID_DELETE	2003
#define ID_ICC		2004

void rawprocFrm::showHistogram(wxTreeItemId item)
{
	gImage &g = ((PicProcessor *) commandtree->GetItemData(item))->getProcessedPic();
	SetStatusText("Building histogram...");
	myHistogramDialog hdiag(this, wxID_ANY, "Histogram", g , wxDefaultPosition, wxDefaultSize);  //wxSize(500,300));
	SetStatusText("");
	hdiag.ShowModal();
}

void rawprocFrm::CommandTreePopup(wxTreeEvent& event)
{
	wxMenu mnu;
 	mnu.Append(ID_EXIF, "Image Information...");
 	//mnu.Append(ID_HISTOGRAM, "Full Histogram...");
	mnu.AppendSeparator();
	mnu.Append(ID_DELETE, "Delete");
	switch (GetPopupMenuSelectionFromUser(mnu)) {
		case ID_EXIF:
			InfoDialog(event.GetItem());
			break;
	//	case ID_HISTOGRAM:
	//		showHistogram(event.GetItem());
	//		//wxMessageBox("Not there yet, press 't' to toggle the thumbnail histogram...");
	//		break;
		case ID_DELETE:
			CommandTreeDeleteItem(event.GetItem());
			break;
	}
}


void rawprocFrm::SetConfigFile(wxString cfile)
{
	configfile = cfile;
}


