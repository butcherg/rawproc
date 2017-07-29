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
#include <wx/fileconf.h>
#include <wx/stdpaths.h>

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
#include "myHistogramDialog.h"
#include "myEXIFDialog.h"
#include "util.h"
#include "lcms2.h"
#include <omp.h>
#include <exception>

#include "unchecked.xpm"
#include "checked.xpm"

wxString version = "0.6Dev";

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

	CreateGUIControls();
#if defined(_OPENMP)
	omp_set_dynamic(0);
#endif
	deleting = false;

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

	help.UseConfig(wxConfig::Get());
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
	
	//histogram = new wxGenericStaticBitmap(this, wxID_ANY, wxBitmap(), wxDefaultPosition, wxSize(285,150));
	//histogram->SetDoubleBuffered(true);
	
	histogram = new myHistogramPane(this, wxDefaultPosition,wxSize(285,150));
	
	wxAuiPaneInfo pinfo = wxAuiPaneInfo().Left().CloseButton(false);
	mgr.SetManagedWindow(this);
	
	commandtree = new wxTreeCtrl(this, ID_COMMANDTREE, wxDefaultPosition, wxSize(280,200), wxTR_DEFAULT_STYLE);
	
	pic = new PicPanel(this, commandtree, histogram);


	parameters = new myParameters(this, -1, wxDefaultPosition, wxSize(285,300));
	parameters->SetMinSize(wxSize(285,300));
	parameters->SetMaxSize(wxSize(1200,750));
	//parameters->SetAutoLayout(true); 
	
	//histogram = new wxGenericStaticBitmap(this, wxID_ANY, wxDefaultPosition, wxSize(285,200));

	mgr.AddPane(pic, wxCENTER);
	mgr.AddPane(commandtree, pinfo.Caption(wxT("Commands")).Position(0));
	mgr.AddPane(histogram, pinfo.Caption(wxT("Histogram")).Position(1));  //.GripperTop());
	mgr.AddPane(parameters, pinfo.Caption(wxT("Parameters")).Position(2).MinSize(wxSize(285,300))); //.GripperTop());

	mgr.Update();
}

void rawprocFrm::SetBackground()
{
	//parm app.backgroundcolor: r,g,b (0-255), set at startup.  Default=(255,255,255)
	wxString bk = wxConfigBase::Get()->Read("app.backgroundcolor","255,255,255");
	if (bk == "") bk = "255,255,255";
	wxArrayString bkgnd = split(bk,",");
	int r = atoi(bkgnd[0].c_str());
	int g = atoi(bkgnd[1].c_str());
	int b = atoi(bkgnd[2].c_str());
	parameters->SetBackgroundColour(wxColour(r,g,b));
	parameters->Refresh();
	pic->SetBackgroundColour(wxColour(r,g,b));
	pic->Refresh();
	commandtree->SetBackgroundColour(wxColour(r,g,b));
	commandtree->Refresh();
	histogram->SetBackgroundColour(wxColour(r,g,b));
	histogram->Refresh();
	SetBackgroundColour(wxColour(r,g,b));
	Refresh();
	
}

void rawprocFrm::OnClose(wxCloseEvent& event)
{
	commandtree->DeleteAllItems();
	commandtree->Update();
	pic->BlankPic();
	histogram->BlankPic();
	parameters->DestroyChildren();
	if ( help.GetFrame() ) // returns NULL if no help frame active
		help.GetFrame()->Close(true);
	// now we can safely delete the config pointer
	event.Skip();
	delete wxConfig::Set(NULL);
	mgr.UnInit();
	Destroy();
}

void rawprocFrm::MnuexitClick(wxCommandEvent& event)
{
	commandtree->DeleteAllItems();
	commandtree->Update();
	pic->BlankPic();
	histogram->BlankPic();
	parameters->DestroyChildren();
	if ( help.GetFrame() ) // returns NULL if no help frame active
		help.GetFrame()->Close(true);
	// now we can safely delete the config pointer
	event.Skip();
	delete wxConfig::Set(NULL);
	mgr.UnInit();
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
	wxString exifcommand = wxConfigBase::Get()->Read("exif.command","");
	if (exifcommand == "") {
		wxMessageBox("No exiftool path defined in exif.command");
		return;
	}
	//parm exif.parameters: exiftool parameters used to format the exiftool output.  Default=-g -h, produces an HTML table, grouped by type.
	wxString exifparameters = wxConfigBase::Get()->Read("exif.parameters","-g -h");

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

	if      (name == "gamma")      		p = new PicProcessorGamma("gamma",command, commandtree,  pic,  parameters);
	else if (name == "bright")     		p = new PicProcessorBright("bright",command, commandtree, pic, parameters);
	else if (name == "contrast")   		p = new PicProcessorContrast("contrast",command, commandtree, pic, parameters);
	else if (name == "shadow")     		p = new PicProcessorShadow("shadow",command, commandtree, pic, parameters);
	else if (name == "highlight")  		p = new PicProcessorHighlight("highlight",command, commandtree, pic, parameters);
	else if (name == "saturation") 		p = new PicProcessorSaturation("saturation",command, commandtree, pic, parameters);
	else if (name == "curve")			p = new PicProcessorCurve("curve",command, commandtree, pic, parameters);
	else if (name == "gray")       		p = new PicProcessorGray("gray",command, commandtree, pic, parameters);
	else if (name == "crop")       		p = new PicProcessorCrop("crop",command, commandtree, pic, parameters);
	else if (name == "resize") 			p = new PicProcessorResize("resize",command, commandtree, pic, parameters);
	else if (name == "blackwhitepoint")	p = new PicProcessorBlackWhitePoint("blackwhitepoint",command, commandtree, pic, parameters);
	else if (name == "sharpen")     	p = new PicProcessorSharpen("sharpen",command, commandtree, pic, parameters);
	else if (name == "rotate")			p = new PicProcessorRotate("rotate",command, commandtree, pic, parameters);
	else if (name == "denoise")			p = new PicProcessorDenoise("denoise",command, commandtree, pic, parameters);
	else if (name == "redeye")			p = new PicProcessorRedEye("redeye",command, commandtree, pic, parameters);
	else return NULL;
	p->processPic();
	if (name == "resize") pic->SetScale(1.0);
	if (!commandtree->GetNextSibling(p->GetId()).IsOk()) CommandTreeSetDisplay(p->GetId());
	Refresh();
	Update();

	return p;
}




wxString rawprocFrm::AssembleCommand()
{
	SetStatusText("");
	wxString cmd = "rawproc-";
	cmd.Append(version);
	cmd.Append(" ");
	wxTreeItemIdValue cookie;
	wxTreeItemId root = commandtree->GetRootItem();
	wxString rootcmd = ((PicProcessor *)commandtree->GetItemData(root))->getCommand();
		cmd.Append(wxString::Format("%s ",rootcmd));
	wxTreeItemId iter = commandtree->GetFirstChild(root, cookie);
	if (iter.IsOk()) {
		cmd.Append(wxString::Format("%s ",((PicProcessor *)commandtree->GetItemData(iter))->getCommand()));
		iter = commandtree->GetNextChild(root, cookie);
		while (iter.IsOk()) {
			cmd.Append(wxString::Format("%s ",((PicProcessor *)commandtree->GetItemData(iter))->getCommand()));
			iter = commandtree->GetNextChild(root, cookie);
		}
	}

	return cmd;
}

void rawprocFrm::OpenFile(wxString fname) //, wxString params)
{
	filename.Assign(fname);
	sourcefilename.Clear();
	gImage *dib;
	GIMAGE_FILETYPE fif;
	fif = gImage::getFileType(fname.c_str());

	wxFileName profilepath;
	//parm cms.profilepath: Directory path where ICC colorspace profiles can be found.  Default: (none, implies current working directory)
	profilepath.AssignDir(wxConfigBase::Get()->Read("cms.profilepath",""));


	if (fif != FILETYPE_UNKNOWN) {

		wxString configparams, inputprofile;
		
		if (fif == FILETYPE_RAW) {
			//parm input.raw.libraw.*: Instead of, or in addition to input.raw.parameters, you can enter any libraw parameter individually, as in input.raw.libraw.bright=2.0.  Note that if you duplicate a parameter here and in input.raw.parameters, the latter will be what's used in processing.
			configparams = paramString("input.raw.libraw.");
			//parm input.raw.parameters: name=value list of parameters, separated by semicolons, to pass to the raw image reader.  Default=(none)
			configparams.Append(wxConfigBase::Get()->Read("input.raw.parameters",""));
			//parm input.raw.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			profilepath.SetFullName(wxConfigBase::Get()->Read("input.raw.cms.profile",""));
		}
		

		if (fif == FILETYPE_JPEG) {
			//parm input.jpeg.parameters: name=value list of parameters, separated by semicolons, to pass to the JPEG image reader.  Default=(none)
			configparams = wxConfigBase::Get()->Read("input.jpeg.parameters","");
			//parm input.jpeg.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			profilepath.SetFullName(wxConfigBase::Get()->Read("input.jpeg.cms.profile",""));
		}

		
		if (fif == FILETYPE_TIFF) {
			//parm input.tiff.parameters: name=value list of parameters, separated by semicolons, to pass to the TIFF image reader.  Default=(none)
			configparams = wxConfigBase::Get()->Read("input.tiff.parameters","");
			//parm input.tiff.cms.profile: ICC profile to use if the input image doesn't have one.  Default=(none)
			profilepath.SetFullName(wxConfigBase::Get()->Read("input.tiff.cms.profile",""));
		}

		SetStatusText(wxString::Format("Loading file:%s params:%s",filename.GetFullName(), configparams));

		if (!wxFileName::FileExists(fname)) {
			wxMessageBox(wxString::Format("Error: Source file %s not found", filename.GetFullName()));
			SetStatusText("");
			return;
		}

		commandtree->DeleteAllItems();
		commandtree->Update();
		pic->BlankPic();
		histogram->BlankPic();
		parameters->DestroyChildren();
		
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
		if (wxConfigBase::Get()->Read("input.log","0") == "1") 
			log(wxString::Format("file input,filename=%s,imagesize=%dx%d,time=%s",filename.GetFullName(),dib->getWidth(), dib->getHeight(),loadtime));

		//parm input.cms: When a file is input, enable or disable color management.  Default=0
		if (wxConfigBase::Get()->Read("input.cms","0") == "1") {

			if (wxConfigBase::Get()->Read("display.cms.displayprofile","") == "") wxMessageBox("CMS enabled, but no display profile was found.  Image will be displayed in its working profile.");

			cmsHPROFILE hImgProf;
			if (dib->getProfile() != NULL & dib->getProfileLength() > 0) {
				hImgProf = cmsOpenProfileFromMem(dib->getProfile(), dib->getProfileLength());
				pic->SetImageProfile(hImgProf);
				pic->SetColorManagement(true);
			}
			else {
				if (wxMessageBox(wxString::Format("Color management enabled, and no color profile was found in %s.  Apply the default input profile, %s?",filename.GetFullName(),profilepath.GetFullPath()),"foo",wxYES_NO) == wxYES) {
					hImgProf = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
					//hImgProf = gImage::makeLCMSProfile("srgb", 2.2);
					char * prof; cmsUInt32Number proflen;
					if (hImgProf) {
						gImage::makeICCProfile(hImgProf, prof, proflen);
						dib->setProfile(prof, proflen);
					}
					pic->SetImageProfile(hImgProf);
					pic->SetColorManagement(true);
				}
				else {
					pic->SetImageProfile(NULL);
					pic->SetColorManagement(false);
				}
			}
		}
		else {
			pic->SetImageProfile(NULL);
			pic->SetColorManagement(false);
		}

		pic->SetScaleToWidth();
		pic->FitMode(true);
		SetStatusText("scale: fit",2);
		PicProcessor *picdata = new PicProcessor(filename.GetFullName(), configparams, commandtree, pic, parameters, dib);
		picdata->showParams();
		picdata->processPic();
		CommandTreeSetDisplay(picdata->GetId());
		SetTitle(wxString::Format("rawproc: %s",filename.GetFullName()));
		SetStatusText("");

		//parm input.raw.default: Space-separated list of rawproc tools to apply to a raw image after it is input. If this parameter has an entry, application of the tools is prompted yes/no.  Default=(none)
		wxString raw_default = wxConfigBase::Get()->Read("input.raw.default","");
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
					wxMessageBox(wxString::Format("Error: Adding gamma tool failed: %s",e.what()));
				}
			}
		}
		
		opensource = false;

		Refresh();
		Update();
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

	GIMAGE_FILETYPE fif;
	fif = gImage::getFileType(fname.c_str());

	wxFileName profilepath;
	profilepath.AssignDir(wxConfigBase::Get()->Read("cms.profilepath",""));


	if (fif == FILETYPE_RAW) {
		profilepath.SetFullName(wxConfigBase::Get()->Read("input.raw.cms.profile",""));
	}
	if (fif == FILETYPE_JPEG) {
		profilepath.SetFullName(wxConfigBase::Get()->Read("input.jpeg.cms.profile",""));
	}
	if (fif == FILETYPE_TIFF) {
		profilepath.SetFullName(wxConfigBase::Get()->Read("input.tiff.cms.profile",""));
	}

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
				wxString subdir = wxConfigBase::Get()->Read("input.opensource.subdirectory",""); 
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
						if (wxConfigBase::Get()->Read("input.opensource.parentdirectory","0") == "1") {
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
			
			commandtree->DeleteAllItems();
			commandtree->Update();
			pic->BlankPic();
			histogram->BlankPic();
			parameters->DestroyChildren();
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
			
			if (wxConfigBase::Get()->Read("input.log","0") == "1") 
				log(wxString::Format("file input,filename=%s,imagesize=%dx%d,time=%s",filename.GetFullName(),dib->getWidth(), dib->getHeight(),loadtime));


			if (wxConfigBase::Get()->Read("input.cms","0") == "1") {

				if (wxConfigBase::Get()->Read("display.cms.displayprofile","") == "") wxMessageBox("CMS enabled, but no display profile was found.  Image will be displayed in its working profile.");
				cmsHPROFILE hImgProf;
				if (dib->getProfile() != NULL & dib->getProfileLength() > 0) {
					hImgProf = cmsOpenProfileFromMem(dib->getProfile(), dib->getProfileLength());
					pic->SetImageProfile(hImgProf);
					pic->SetColorManagement(true);
				}
				else {
					if (wxMessageBox(wxString::Format("Color management enabled, and no color profile was found in %s.  Apply the default input profile, %s?",filename.GetFullName(),profilepath.GetFullPath()),"foo",wxYES_NO) == wxYES) {
						hImgProf = cmsOpenProfileFromFile(profilepath.GetFullPath().c_str(), "r");
						//hImgProf = gImage::makeLCMSProfile("srgb", 2.2);
						char * prof; cmsUInt32Number proflen;
						if (hImgProf) {
							gImage::makeICCProfile(hImgProf, prof, proflen);
							dib->setProfile(prof, proflen);
						}
						pic->SetImageProfile(hImgProf);
						pic->SetColorManagement(true);
					}
					else {
						pic->SetImageProfile(NULL);
						pic->SetColorManagement(false);
					}
				}
			}
			else {
				pic->SetImageProfile(NULL);
				pic->SetColorManagement(false);
			}

			pic->SetScaleToWidth();
			pic->FitMode(true);
			SetStatusText("scale: fit",2);
			PicProcessor *picdata = new PicProcessor(filename.GetFullName(), oparams, commandtree, pic, parameters, dib);
			picdata->processPic();
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

			Refresh();
			Update();
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
	//parm cms.profilepath: Directory path where ICC colorspace profiles can be found.  Default: (none, implies current working directory)
	profilepath.AssignDir(wxConfigBase::Get()->Read("cms.profilepath",""));

	if (!sourcefilename.IsOk()) 
		fname = wxFileSelector("Save image...",filename.GetPath(),filename.GetName(),filename.GetExt(),"JPEG files (*.jpg)|*.jpg|TIFF files (*.tif)|*.tif",wxFD_SAVE);  // |PNG files (*.png)|*.png
	else
		fname = wxFileSelector("Save image...",sourcefilename.GetPath(),sourcefilename.GetName(),sourcefilename.GetExt(),"JPEG files (*.jpg)|*.jpg|TIFF files (*.tif)|*.tif",wxFD_SAVE);  // |PNG files (*.png)|*.png

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
			dib->setInfo("Software",(std::string) wxString::Format("rawproc %s",version).c_str());
			
			int rotation = atoi(dib->getInfoValue("Orientation").c_str());
			//parm output.orient: Rotate the image to represent the EXIF Orientation value originally inputted, then set the Orientation tag to 0.  Gets the image out of trying to tell other software how to orient it.  Default=0
			if ((wxConfigBase::Get()->Read("output.orient","0") == "1") & (rotation != 0)) {
				WxStatusBar1->SetStatusText(wxString::Format("Orienting image for output..."));
				if (rotation == 2) dib->ApplyHorizontalMirror();
				if (rotation == 3) dib->ApplyRotate180();
				if (rotation == 4) dib->ApplyVerticalMirror();
				if (rotation == 5) {dib->ApplyHorizontalMirror(); dib->ApplyRotate270();}
				if (rotation == 6) dib->ApplyRotate90();
				if (rotation == 7) {dib->ApplyHorizontalMirror(); dib->ApplyRotate90();}
				if (rotation == 8) dib->ApplyRotate270();
				dib->setInfo("Orientation","0");
			}

			wxString configparams;
			//parm output.jpeg.parameters: name=value list of parameters, separated by semicolons, to pass to the JPEG image writer.  Default=(none)
			if (filetype == FILETYPE_JPEG) configparams = wxConfigBase::Get()->Read("output.jpeg.parameters","");
			//parm output.tiff.parameters: name=value list of parameters, separated by semicolons, to pass to the TIFF image writer.  Default=(none)
			if (filetype == FILETYPE_TIFF) configparams = wxConfigBase::Get()->Read("output.tiff.parameters","");


			if (pic->GetColorManagement()) {

				wxString intentstr;
				cmsUInt32Number intent = INTENT_PERCEPTUAL;

				if (filetype == FILETYPE_JPEG) {
					//parm output.jpeg.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=srgb
					profilepath.SetFullName(wxConfigBase::Get()->Read("output.jpeg.cms.profile",""));
					//parm output.jpeg.cms.renderingintent: Specify the rendering intent for the JPEG output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
					intentstr = wxConfigBase::Get()->Read("output.jpeg.cms.renderingintent","perceptual");
				}
				else if (filetype == FILETYPE_TIFF) {
					//parm output.tiff.cms.profile: If color management is enabled, the specified profile is used to transform the output image and the ICC is stored in the image file.  Can be one of the internal profiles or the path/file name of an ICC profile. Default=prophoto
					profilepath.SetFullName(wxConfigBase::Get()->Read("output.tiff.cms.profile",""));
					//parm output.tiff.cms.renderingintent: Specify the rendering intent for the JPEG output transform, perceptual|saturation|relative_colorimetric|absolute_colorimetric.  Default=perceptual
					intentstr = wxConfigBase::Get()->Read("output.tiff.cms.renderingintent","perceptual");
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
	wxTreeItemIdValue cookie;
	wxTreeItemId root = commandtree->GetRootItem();
	if (root.IsOk()) commandtree->SetItemState(root,0);
	wxTreeItemId iter = commandtree->GetFirstChild(root, cookie);
	if (iter.IsOk()) {
		commandtree->SetItemState(iter,0);
		iter = commandtree->GetNextChild(root, cookie);
		while (iter.IsOk()) {
			commandtree->SetItemState(iter,0);
			iter = commandtree->GetNextChild(root, cookie);
		}
	}
	commandtree->SetItemState(item,1);
	((PicProcessor *) commandtree->GetItemData(item))->displayProcessedPic();

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
	wxTreeItemId item = event.GetItem();
	CommandTreeSetDisplay(item);
	
	event.Skip();
	Refresh();
	Update();
	
	if (isDownstream(displayitem, item)) {
		wxTreeItemId next = commandtree->GetNextSibling(displayitem);
		if (next.IsOk()) ((PicProcessor *) commandtree->GetItemData(next))->processPic();
	}
		

	displayitem = item;
}


void rawprocFrm::CommandTreeSelChanged(wxTreeEvent& event)
{
	SetStatusText("");
	wxTreeItemId item = event.GetItem();
	if (item.IsOk()) { 
		if ((PicProcessor *) commandtree->GetItemData(item))
			((PicProcessor *) commandtree->GetItemData(item))->showParams();
	}
	Update();
	Refresh();
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
       		commandtree->Delete(item);
		if (newitem.IsOk()) {
			((PicProcessor *) commandtree->GetItemData(newitem))->showParams();
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
	wxString fname = wxFileSelector("Open Image...", filename.GetPath());	
	if ( !fname.empty() ) { 
		wxFileName f(fname);
		wxSetWorkingDirectory (f.GetPath());
		OpenFile(fname);
	}
}

void rawprocFrm::Mnureopen1033Click(wxCommandEvent& event)
{
	if (filename.IsOk() && filename.FileExists()) {
		if (opensource)
			OpenFileSource(sourcefilename.GetFullPath());
		else
			OpenFile(filename.GetFullPath());
	}
	else {
		wxMessageBox("No file to re-open.");
	}
}

void rawprocFrm::Mnuopensource1004Click(wxCommandEvent& event)
{
	wxString fname = wxFileSelector("Open Image source...", filename.GetPath());	
	if ( !fname.empty() ) { 
		wxFileName f(fname);
		wxSetWorkingDirectory (f.GetPath());
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
		diag = new PropertyDialog(this, wxID_ANY, "Properties", (wxFileConfig *) wxConfigBase::Get());
		Bind(wxEVT_PG_CHANGED,&rawprocFrm::UpdateConfig,this);
	}
	diag->ClearModifiedStatus();
	diag->Show();
}

void rawprocFrm::MnuEXIF(wxCommandEvent& event)
{
	EXIFDialog(filename);
}

void rawprocFrm::UpdateConfig(wxPropertyGridEvent& event)
{
	SetStatusText(wxString::Format("Changed %s to %s.", event.GetPropertyName(), event.GetPropertyValue().GetString()));
	wxConfigBase::Get()->Write(event.GetPropertyName(), event.GetPropertyValue().GetString());
	wxConfigBase::Get()->Flush();
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
		wxString val = wxConfigBase::Get()->Read("tool.gamma.initialvalue","2.2");
		PicProcessorGamma *g = new PicProcessorGamma("gamma",val, commandtree, pic, parameters);	
		g->processPic();
		if (!commandtree->GetNextSibling(g->GetId()).IsOk()) CommandTreeSetDisplay(g->GetId());
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
		wxString val = wxConfigBase::Get()->Read("tool.bright.initialvalue","0");
		PicProcessorBright *g = new PicProcessorBright("bright",val, commandtree, pic, parameters);
		g->processPic();
		if (!commandtree->GetNextSibling(g->GetId()).IsOk()) CommandTreeSetDisplay(g->GetId());
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
		wxString val = wxConfigBase::Get()->Read("tool.contrast.initialvalue","0");
		PicProcessorContrast *c = new PicProcessorContrast("contrast",val, commandtree, pic, parameters);
		c->processPic();
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
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
		wxString val = wxConfigBase::Get()->Read("tool.saturate.initialvalue","1.0");
		PicProcessorSaturation *c = new PicProcessorSaturation("saturation",val, commandtree, pic, parameters);
		c->processPic();
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
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
		wxString val = wxConfigBase::Get()->Read("tool.exposure.initialvalue","0.0");
		PicProcessorExposure *c = new PicProcessorExposure("exposure",val, commandtree, pic, parameters);
		c->processPic();
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
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
		PicProcessorCurve *crv = new PicProcessorCurve("curve","0.0,0.0,255.0,255.0", commandtree, pic, parameters);
		crv->processPic();
		if (!commandtree->GetNextSibling(crv->GetId()).IsOk()) CommandTreeSetDisplay(crv->GetId());
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
		wxString level = wxConfigBase::Get()->Read("tool.shadow.level","0");
		//parm tool.shadow.threshold: The initial (and reset button) value of the shadow curve threshold.  Default=64
		wxString threshold = wxConfigBase::Get()->Read("tool.shadow.threshold","64");
		wxString cmd= wxString::Format("%s,%s",level,threshold);
		PicProcessorShadow *shd = new PicProcessorShadow("shadow",cmd, commandtree, pic, parameters);
		shd->processPic();
		if (!commandtree->GetNextSibling(shd->GetId()).IsOk()) CommandTreeSetDisplay(shd->GetId());
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
		wxString level = wxConfigBase::Get()->Read("tool.highlight.level","0");
		//parm tool.highlight.threshold: The initial (and reset button) value of the highlight curve threshold.  Default=192
		wxString threshold = wxConfigBase::Get()->Read("tool.highlight.threshold","192");
		wxString cmd= wxString::Format("%s,%s",level,threshold);
		PicProcessorHighlight *s = new PicProcessorHighlight("highlight",cmd, commandtree, pic, parameters);
		s->processPic();
		if (!commandtree->GetNextSibling(s->GetId()).IsOk()) CommandTreeSetDisplay(s->GetId());
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
		wxString r = wxConfigBase::Get()->Read("tool.gray.r","0.21");
		//parm tool.gray.g: The initial (and reset button) value of the green proportion for grayscale conversion. Default=0.72
		wxString g = wxConfigBase::Get()->Read("tool.gray.g","0.72");
		//parm tool.gray.b: The initial (and reset button) value of the blue proportion for grayscale conversion. Default=0.07
		wxString b = wxConfigBase::Get()->Read("tool.gray.b","0.07");
		wxString cmd= wxString::Format("%s,%s,%s",r,g,b);
		PicProcessorGray *gr = new PicProcessorGray("gray",cmd, commandtree, pic, parameters);
		gr->processPic();
		if (!commandtree->GetNextSibling(gr->GetId()).IsOk()) CommandTreeSetDisplay(gr->GetId());
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
		PicProcessorCrop *c = new PicProcessorCrop("crop", commandtree, pic, parameters);
		c->processPic();
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
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
		wxString x = wxConfigBase::Get()->Read("tool.resize.x","640");
		//parm tool.resize.y: Default resize of the height dimension.  Default=0 (calculate value to preserve aspect)
		wxString y = wxConfigBase::Get()->Read("tool.resize.y","0");
		//parm tool.resize.algorithm: Sets the algorithm used to interpolate resized pixels. Available algorithms are box, bilinear, bspline, bicubic, catmullrom, lanczos3.  Default=catmullrom
		wxString algo = wxConfigBase::Get()->Read("tool.resize.algorithm","catmullrom");
		wxString cmd= wxString::Format("%s,%s,%s",x,y,algo);
		PicProcessorResize *c = new PicProcessorResize("resize", cmd, commandtree, pic, parameters);
		c->processPic();
		pic->SetScale(1.0);
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
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
		PicProcessorBlackWhitePoint *c;
		//parm tool.blackwhitepoint.auto: Invoke auto calculation of inital black and white point values, based on a percent-pixels threshold.  Currently, this behavior is only invoked when the tool is added, so re-application requires deleting and re-adding the tool.  Default=0
		if (wxConfigBase::Get()->Read("tool.blackwhitepoint.auto","0") =="1")
			c = new PicProcessorBlackWhitePoint("blackwhitepoint", "", commandtree, pic, parameters);
		else
			c = new PicProcessorBlackWhitePoint("blackwhitepoint", "0,255", commandtree, pic, parameters);
		c->processPic();
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
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
		wxString defval = wxConfigBase::Get()->Read("tool.sharpen.initialvalue","0");
		PicProcessorSharpen *c = new PicProcessorSharpen("sharpen", defval, commandtree, pic, parameters);
		if (defval != "0") c->processPic();
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
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
		wxString defval = wxConfigBase::Get()->Read("tool.rotate.initialvalue","0.0");
		PicProcessorRotate *c = new PicProcessorRotate("rotate", defval, commandtree, pic, parameters);
		if (defval != "0.0") c->processPic();
		if (!commandtree->GetNextSibling(c->GetId()).IsOk()) CommandTreeSetDisplay(c->GetId());
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
		wxString sigma = wxConfigBase::Get()->Read("tool.denoise.initialvalue","0.0");
		//parm tool.denoise.local: Defines the initial (and reset button) size of the neigbor pixel array.  Default=3
		wxString local = wxConfigBase::Get()->Read("tool.denoise.local","3");
		//parm tool.denoise.patch: Defines the initial (and reset button) size of the patch pixel array.  Default=1
		wxString patch = wxConfigBase::Get()->Read("tool.denoise.patch","1");
		wxString cmd = wxString::Format("%s,%s,%s",sigma,local,patch);
		PicProcessorDenoise *d = new PicProcessorDenoise("denoise", cmd, commandtree, pic, parameters);
		if (sigma != "0") d->processPic();
		if (!commandtree->GetNextSibling(d->GetId()).IsOk()) CommandTreeSetDisplay(d->GetId());
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
		wxString threshold = wxConfigBase::Get()->Read("tool.redeye.threshold","1.5");
		//parm tool.redeye.radius: Defines the initial (and reset button) limit of the patch size.  Default=50
		wxString radius = wxConfigBase::Get()->Read("tool.redeye.radius","50");
		//parm tool.redeye.desaturation: The initial (and reset button) desaturation toggle.  Default=0
		wxString desat = wxConfigBase::Get()->Read("tool.redeye.desaturation","0");
		//parm tool.redeye.desaturationpercent: The initial (and reset button) desaturation percent.  Default=1.0
		wxString desatpct = wxConfigBase::Get()->Read("tool.redeye.desaturationpercent","1.0");
		
		wxString cmd = wxString::Format("%s,%s,%s,%s",threshold,radius,desat,desatpct);
		PicProcessorRedEye *d = new PicProcessorRedEye("redeye", cmd, commandtree, pic, parameters);
		//d->processPic();
		if (!commandtree->GetNextSibling(d->GetId()).IsOk()) CommandTreeSetDisplay(d->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding redeye tool failed: %s",e.what()));
	}
}

void rawprocFrm::MnuColorSpace(wxCommandEvent& event)
{
	if (commandtree->IsEmpty()) return;
	SetStatusText("");
	try {
		PicProcessorColorSpace *d = new PicProcessorColorSpace("colorspace", "", commandtree, pic, parameters);
		if (!commandtree->GetNextSibling(d->GetId()).IsOk()) CommandTreeSetDisplay(d->GetId());
	}
	catch (std::exception& e) {
		wxMessageBox(wxString::Format("Error: Adding colorspace tool failed: %s",e.what()));
	}
}



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
	wxMessageBox(AssembleCommand());
}

void rawprocFrm::MnuAbout1011Click(wxCommandEvent& event)
{
	wxAboutDialogInfo info;
	info.SetName(_("rawproc"));
	info.SetVersion(_(version));
	info.SetCopyright(wxT("(C) 2017 Glenn Butcher <glenn.butcher@gmail.com>"));
	
	wxString gImageVersion(gImage::Version().c_str());
	wxString WxWidgetsVersion = wxGetLibraryVersionInfo().GetVersionString();
	wxString libraries = wxString(gImage::LibraryVersions());
	wxString pixtype = wxString(gImage::getRGBCharacteristics());
#ifdef BUILDDATE
	wxString builddate = wxString(BUILDDATE);
	info.SetDescription(wxString::Format("Basic camera raw file and image editor.\n\nLibraries:\n%s\ngImage %s\n%s\n\nPixel Format: %s\n\nConfiguration file: %s\n\nBuild Date: %s", WxWidgetsVersion, gImageVersion, libraries.c_str(),pixtype, configfile, builddate));
#else
	info.SetDescription(wxString::Format("Basic camera raw file and image editor.\n\nLibraries:\n%s\ngImage %s\n%s\n\nPixel Format: %s\n\nConfiguration file: %s", WxWidgetsVersion, gImageVersion, libraries.c_str(),pixtype, configfile));
#endif

	wxAboutBox(info);

}

void rawprocFrm::MnuHelpClick(wxCommandEvent& event)
{
	help.DisplayContents();
}

#define ID_EXIF		2001
#define ID_HISTOGRAM	2002
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
 	mnu.Append(ID_HISTOGRAM, "Full Histogram...");
	mnu.AppendSeparator();
	mnu.Append(ID_DELETE, "Delete");
	switch (GetPopupMenuSelectionFromUser(mnu)) {
		case ID_EXIF:
			InfoDialog(event.GetItem());
			break;
		case ID_HISTOGRAM:
			showHistogram(event.GetItem());
			//wxMessageBox("Not there yet, press 't' to toggle the thumbnail histogram...");
			break;
		case ID_DELETE:
			CommandTreeDeleteItem(event.GetItem());
			break;
	}
}


void rawprocFrm::SetConfigFile(wxString cfile)
{
	configfile = cfile;
}


