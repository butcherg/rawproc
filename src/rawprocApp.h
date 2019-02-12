//---------------------------------------------------------------------------
//
// Name:        rawprocApp.h
// Author:      Glenn
// Created:     11/18/2015 7:04:05 PM
// Description: 
//
//---------------------------------------------------------------------------

#ifndef __RAWPROCFRMApp_h__
#define __RAWPROCFRMApp_h__

#ifdef __BORLANDC__
	#pragma hdrstop
#endif

#ifndef WX_PRECOMP
	#include <wx/wx.h>
#else
	#include <wx/wxprec.h>
#endif

class rawprocFrmApp : public wxApp
{
	public:
		bool OnInit();
		int OnExit();
		int OnRun();
		void OnFatalException();
};

#endif
