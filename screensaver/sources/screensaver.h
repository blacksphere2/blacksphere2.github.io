//-----------------------------------------------------------------------------
// File: Screensaver.h
//
// Desc: Windows code for Direct3D screensaver.
//
// Note: This code uses the Direct3D sample framework.
//
//
// Copyright (c) 2001 Black Sphere Corp.
//-----------------------------------------------------------------------------
#pragma once


// Declarations only!!!
//-----------------------------------------------------------------------------
// Short name for this screen saver
//-----------------------------------------------------------------------------
extern TCHAR g_strScreenSaverName[];

extern TCHAR g_strMediaPath[256];




//-----------------------------------------------------------------------------
// Name: struct ScreenSaverOptions
// Desc: Structure to hold the options for the screensaver. A custom D3D screen
//       saver should put all of its options in this struct. Note: the first
//       two variables are required as they are used in ScreenSaver.cpp.
//-----------------------------------------------------------------------------
struct ScreenSaverOptions
{
	// Required options
	BOOL  bUse640x480Mode;
	BOOL  bUseHardware;

	// Custom options
	float fSpeed;
	float fRotation;
	float fSize;
	float fZoom;
	float fRandomFactor;
	BOOL bSound;
	BOOL bGameLoop;
	BOOL bTrace;
	BOOL bShowLogo;
	BOOL bMark1;
	BOOL bMark2;
	BOOL bMark3;
	BOOL bMark4;
	BOOL bLogo1;
	BOOL bLogo2;
	BOOL bLogo3;
	BOOL bLogo4;
	INT	 iPreset;
};



extern ScreenSaverOptions g_CurrentOptions;

extern HWND g_hWnd;
extern HINSTANCE g_hInst;




