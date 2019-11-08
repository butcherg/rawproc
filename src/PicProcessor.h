#ifndef __PICPROCESSOR_H__
#define __PICPROCESSOR_H__

#ifndef WX_PRECOMP
	#include <wx/wx.h>
	#include <wx/frame.h>
#else
	#include <wx/wxprec.h>
#endif

#include <wx/image.h>
#include <wx/treectrl.h>
#include <wx/sizer.h>
#include <wx/event.h>
#include <wx/simplebook.h>

#include "PicPanel.h"
#include "PicProcPanel.h"
//#include <deque>
#include "gimage/gimage.h"
#include <string>
#include <map>

class PicProcPanel;
class PicPanel;


class PicProcessor: public wxTreeItemData//, public wxEvtHandler
{

	public:
		PicProcessor(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, gImage * startipc);
		PicProcessor(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display) ;
		PicProcessor(wxString name, wxString command, wxTreeCtrl *tree, PicPanel *display, wxTreeItemId parent);  //for group items

		~PicProcessor();

		virtual bool processPic(bool processnext=true);
		virtual bool processPicture(gImage *processdib);

		void processNext();
		wxString getCommand();
		wxString getParams();
		wxString getName();
		virtual void createPanel(wxSimplebook* parent, PicProcessor* proc);
		virtual PicProcPanel* getPanel();
		PicProcessor *getPreviousPicProcessor();
		gImage& getProcessedPic();
		gImage* getProcessedPicPointer();
		PicPanel *getDisplay();
		virtual std::map<std::string,std::string> paramMap(std::string params, std::string positionnames="");
		wxString getDrawList();
		wxTreeCtrl *getCommandTree();
		void enableProcessing(bool e);
		bool isEnabled();
		GIMAGE_CHANNEL getChannel();
		//void setDirty();
		//bool isDirty();
		virtual void displayProcessedPic();
		virtual void setParams(wxString params);
		virtual void displayDraw(wxDC &dc);

		bool copyParamsToClipboard();
		bool pasteParamsFromClipboard();

		static PicProcessor *getSelectedPicProcessor(wxTreeCtrl *tree);
		static void enableGlobalProcessing(bool e);

	protected:
		gImage *dib;
		PicPanel *m_display;
		
		wxTreeCtrl *m_tree;
		wxString n, c;
		wxTreeItemId id;

		GIMAGE_CHANNEL channel;

		PicProcPanel *r, *toolpanel;

		bool dirty, processingenabled, groupitem;
		
		wxString dcList;

		static bool global_processing_enabled;

};



#endif

