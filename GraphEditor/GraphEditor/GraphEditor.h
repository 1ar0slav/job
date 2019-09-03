
// GraphEditor.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#include <afxwin.h>


// CGraphEditorApp:
// See GraphEditor.cpp for the implementation of this class
//

class CGraphEditorApp : public CWinApp
{
public:
	CGraphEditorApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CGraphEditorApp theApp;

class GraphEditor;
