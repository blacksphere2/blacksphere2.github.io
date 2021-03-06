//-----------------------------------------------------------------------------
// File: ScreenSaver.cpp
//
// Desc: Windows code for Direct3D screensaver.
//
// Note: This code uses the Direct3D sample framework.
//
//
// Copyright (c) 2001 Black Sphere Corp.
//-----------------------------------------------------------------------------

#define STRICT

#include <windows.h>
#include <mmsystem.h>
#include <scrnsave.h>
#include <stdio.h>
#include <regstr.h>
#include <commctrl.h>
 


#include "D3DFrame.h"
#include "D3DEnum.h"
#include "D3DUtil.h"
#include "resource.h"

#include "screensaver.h"




//-----------------------------------------------------------------------------
// Global variables for using the D3D sample framework class
//-----------------------------------------------------------------------------
CD3DFramework7* g_pFramework           = NULL;
BOOL            g_bReady               = FALSE;
extern BOOL     g_bAppUseZBuffer;
extern BOOL     g_bAppUseBackBuffer;

BOOL            g_bActive = TRUE;


//-----------------------------------------------------------------------------
// Local function-prototypes
//-----------------------------------------------------------------------------
HRESULT Initialize3DEnvironment( HWND );
HRESULT Render3DEnvironment();
VOID    Cleanup3DEnvironment( HWND );
VOID    AppOutputText( LPDIRECT3DDEVICE7, DWORD, DWORD, CHAR* );

INT Run();
void GameLoop();
void SetDlgItems( HWND hDlg );
void DoPresets( int iItem );




//-----------------------------------------------------------------------------
// External function-prototypes
//-----------------------------------------------------------------------------
HRESULT App_ConfirmDevice( DDCAPS*, D3DDEVICEDESC* );
HRESULT App_OneTimeSceneInit();
VOID    App_DeleteDeviceObjects( HWND, LPDIRECT3DDEVICE7 );
HRESULT App_InitDeviceObjects( HWND, LPDIRECT3DDEVICE7 );
HRESULT App_FrameMove( LPDIRECT3DDEVICE7, FLOAT );
HRESULT App_Render( LPDIRECT3DDEVICE7 );
HRESULT App_RestoreSurfaces();
HRESULT App_FinalCleanup();



// Definitions
//-----------------------------------------------------------------------------
// Global options structure. Set this structure with default options values.
//-----------------------------------------------------------------------------
ScreenSaverOptions g_CurrentOptions =
{
    TRUE,     // bUse640x480Mode
    TRUE,     // bUseHardware
    0.033f,   // fSpeed: 0.01f to 0.1f;
    0.034f,   // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
    1.0f,     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
    2.0f,     // fZoom: 2.0f to 50.0f
    0.0f,     // fRandomFactor: 0.0f to 0.1f
    FALSE,    // bSound
    TRUE,     // bGameLoop
    FALSE,    // bTrace
    TRUE,     // bShowLogo
    TRUE,     // bMark1 
    TRUE,     // bMark2
    TRUE,     // bMark3
    TRUE,     // bMark4
    TRUE,     // bLogo1 
    FALSE,    // bLogo2
    FALSE,    // bLogo3
    FALSE,    // bLogo4
    0,        // iPreset
};





//-----------------------------------------------------------------------------
// Short name for this screen saver
//-----------------------------------------------------------------------------
TCHAR g_strScreenSaverName[] = TEXT("Black Sphere Corp.");

TCHAR g_strMediaPath[256] = "";

HWND g_hWnd;
HINSTANCE g_hInst;

HWND g_hDlg;








//-----------------------------------------------------------------------------
// Name: ReadSettings()
// Desc: 
//-----------------------------------------------------------------------------
VOID ReadSettings( ScreenSaverOptions* pOptions )
{
    HKEY  hKey; 
    DWORD dwType = REG_BINARY;
    DWORD dwSize = sizeof(ScreenSaverOptions);

    TCHAR strRegPath[80] = "";
    strcat( strRegPath, REGSTR_PATH_SCREENSAVE );
    strcat( strRegPath, TEXT("\\ScreenSaver.") );
    strcat( strRegPath, g_strScreenSaverName );

    // Read data from the registry
    if( ERROR_SUCCESS == RegOpenKeyEx( HKEY_CURRENT_USER, strRegPath,
                                       KEY_READ, NULL, &hKey ) )
    {
        // read options
        RegQueryValueEx( hKey, "Options", 0, &dwType, (BYTE*)pOptions, &dwSize );
        //RegCloseKey( hKey );

        // read media path (this key was created by InstallShield)
        dwSize = sizeof(g_strMediaPath);
        RegQueryValueEx( hKey, "MediaPath", 0, NULL, (BYTE*)g_strMediaPath, &dwSize );


        RegCloseKey( hKey );

        return;
    }

    // Else, keep current options w/their default values
}




//-----------------------------------------------------------------------------
// Name: WriteSettings()
// Desc:
// Note: HKEY_CURRENT_USER\Control Panel\Desktop\ScreenSaver.Black Sphere Corp.
//-----------------------------------------------------------------------------
HRESULT WriteSettings( ScreenSaverOptions* pOptions )
{
    HKEY  hKey; 
    DWORD dwDisposition;
    DWORD dwType = REG_BINARY;
    DWORD dwSize = sizeof(ScreenSaverOptions);

    // Build the registry path
    TCHAR strRegPath[80] = "";
    strcat( strRegPath, REGSTR_PATH_SCREENSAVE );
    strcat( strRegPath, TEXT("\\ScreenSaver.") );
    strcat( strRegPath, g_strScreenSaverName );

    // Open the registry
    if( ERROR_SUCCESS != RegCreateKeyEx( HKEY_CURRENT_USER, strRegPath, 0, "",
                                         REG_OPTION_NON_VOLATILE, 
                                         KEY_ALL_ACCESS, NULL, &hKey,
                                         &dwDisposition ) )
        return E_FAIL;

    RegSetValueEx( hKey, "Options",  0, dwType, (BYTE*)pOptions, dwSize );
    RegFlushKey( hKey );
    RegCloseKey( hKey ); 

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: ScreenSaverProc()
// Desc: 
//-----------------------------------------------------------------------------
LRESULT CALLBACK ScreenSaverProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                                  LPARAM lParam )
{
    static UINT  uTimer = 0;
    static DWORD dwFakeMouseMoveMsgAllowed = 5;
    HRESULT      hr;

    // always handy
    g_hWnd = hWnd;

    
    // Command Lines (%1 is screensaver program name)
    // See: Explorer|View|Folder Options|File Types|Screen Saver
    // Install: C:\WINDOWS\rundll32.exe desk.cpl,InstallScreenSaver %l
    // Configure: "%1"
    // Test: "%1" /S


    switch( uMsg )
    {
        case WM_CREATE:
            // always handy
            g_hInst = LPCREATESTRUCT(lParam)->hInstance;

            // Read data from the registry
            ReadSettings( &g_CurrentOptions );

            // TODO: write 0 to ScreenSaveUsePassword to disable password
            // as we still don't support flip to GDI
            
            // Initialize the 3D environment for the app
            if( FAILED( hr = Initialize3DEnvironment( hWnd ) ) )
                return 0;

            g_bReady = TRUE;

            // TODO: option timerloop/gameloop
            // gameloop introduces too many artifacts, unfortunately...
            if (g_CurrentOptions.bGameLoop) {
                // Gameloop
                // Yyyyeeeesssss!!! nothing like a gameloop for raw speed!
                // NOTE: waarschijnlijk is er met GDI ook een behoorlijke snelheid te halen
                // als je maar een gameloop gebruikt, en niet een timerloop zoals je altijd
                // nog gedaan hebt...
                // NOTE2: The static-link library SCRNSAVE.LIB contains the main function and 
                // other startup code required for a screen saver.
                // Therefore we go into a gameloop here
                
                //SetCursor( NULL );
                //ShowCursor( FALSE );
                
                // set-up gameloop
                Run();
            } 
            else 
            {
                // Timerloop
                // timerloop is too erratic, and too slow
                // Create a timer
                //uTimer = SetTimer( hWnd, 1, 0, NULL );
                SetTimer( hWnd, 1, 0, NULL );
            }

            return 0;
            break;
        
        case WM_MOUSEMOVE:
            // Ignore any fake msgs triggered by DDraw::SetDisplayMode()
            if( dwFakeMouseMoveMsgAllowed )
            {
                dwFakeMouseMoveMsgAllowed--;
                return 0;
            }

            //PostQuitMessage(0);
            //Cleanup3DEnvironment( hWnd );
            ///return 0;
            break;

        // for taking screenshots with Alt+PrintScreen
        // use Alt+Tab to minimize, Alt+Ctrl+Del or PView to kill
        case WM_CLOSE:
            //return 0;
            break;

        case SCRM_VERIFYPW:
            // message to validate passwords.
            // The default action is to call the Windows Master Password Router.
            // TODO: We should handle this to switch to GDI
            //g_pFramework->GetDirectDraw()->FlipToGDISurface();
            break;

        case WM_ERASEBKGND:
            break;
        
        case WM_TIMER:
            // Draw the scene
            if( g_bReady )
                Render3DEnvironment();
            return 0;
            break;
        
        case WM_DESTROY:
            // Clean everything up and exit
            KillTimer( hWnd, 1 );
            Cleanup3DEnvironment( hWnd );
            break;
    }

    // Pass all unhandled msgs to the default msg handler
    return DefScreenSaverProc( hWnd, uMsg, wParam, lParam );
}
            



//-----------------------------------------------------------------------------
// Name: RegisterDialogClasses()
// Desc: 
//-----------------------------------------------------------------------------
BOOL WINAPI RegisterDialogClasses( HANDLE hInstance )
{
    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: ScreenSaverConfigureDialog()
// Desc: 
//-----------------------------------------------------------------------------
BOOL WINAPI ScreenSaverConfigureDialog( HWND hDlg, UINT uMsg, WPARAM wParam,
                                        LPARAM lParam )
{

    static int idComboBox;
    static int iNewItem = 0;



    if( WM_INITDIALOG == uMsg )
    {
        // always handy
        g_hDlg = hDlg;

            
        // fill the combo box
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"(None)");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Firecracker Suite");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Lonely Planet");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Orbitters");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Big Mamas");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Hail Brabant");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Good Morning Croatia");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Screenshaver");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"Pointillism");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"1 is a Lonely Number");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"2 is a Lonely Number");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"3 is Company");
        SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_ADDSTRING, 0, (LPARAM)(LPCSTR)"4 is a Kraut");


        // Handle the initialization message
        ReadSettings( &g_CurrentOptions );

        // set items
        SetDlgItems( hDlg );

        return TRUE;
    }

    
    if( WM_COMMAND == uMsg )
    {
        if (HIWORD(wParam) == BN_CLICKED)
        {
            switch ( LOWORD(wParam) )
            {
				 case IDC_CHECK1:
					 break;
				 case IDC_CHECK2:
					 break;
				 case IDC_CHECK3:
					 g_CurrentOptions.bShowLogo = !g_CurrentOptions.bShowLogo;
					 if( !g_CurrentOptions.bShowLogo ) {
						 EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO1), FALSE );
						 EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO2), FALSE );
						 EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO3), FALSE );
						 EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO4), FALSE );
					 } else {
						 EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO1), TRUE );
						 EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO2), TRUE );
						 EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO3), TRUE );
						 EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO4), TRUE );
					 }
					 break;
             
				 case IDC_BUTTON_DEFAULT:
					 g_CurrentOptions.bUse640x480Mode    = TRUE; 
					 g_CurrentOptions.bUseHardware       = TRUE;

					 g_CurrentOptions.fSpeed             = 0.033f;           
					 g_CurrentOptions.fRotation          = 0.034f;
					 g_CurrentOptions.fSize              = 1.0f;
					 g_CurrentOptions.fZoom          = 2.0f;
					 g_CurrentOptions.fRandomFactor      = 0.0f;
					 g_CurrentOptions.bSound             = FALSE;
					 g_CurrentOptions.bGameLoop          = TRUE;
					 g_CurrentOptions.bTrace             = FALSE;
					 g_CurrentOptions.bShowLogo          = TRUE;
					 g_CurrentOptions.bMark1             = TRUE;
					 g_CurrentOptions.bMark2             = TRUE;
					 g_CurrentOptions.bMark3             = TRUE;
					 g_CurrentOptions.bMark4             = TRUE;
					 g_CurrentOptions.bLogo1             = TRUE;
					 g_CurrentOptions.bLogo2             = FALSE;
					 g_CurrentOptions.bLogo3             = FALSE;
					 g_CurrentOptions.bLogo4             = FALSE;
					 g_CurrentOptions.iPreset            = 0;


					 SetDlgItems( hDlg );
					 break;
            }
        }
	}


    if( WM_COMMAND == uMsg )
    {
        switch ( HIWORD(wParam) )
        {   
            case CBN_SELCHANGE:
                // enable Apply button
                //PropSheet_Changed( GetParent(hDlg), hDlg );
                break;

            case CBN_SELENDOK:
                idComboBox = (int) LOWORD(wParam);
                iNewItem = SendMessage( (HWND)lParam, CB_GETCURSEL, 0, 0 );
                if (idComboBox==IDC_COMBO_PRESETS)
                {
                    DoPresets(iNewItem);
                }
                break;
                
        }
    


	    if( IDOK == LOWORD(wParam) )
	    {
	        // Handle the case when the user hits the OK button
	        g_CurrentOptions.bUse640x480Mode = SendDlgItemMessage( hDlg, IDC_640x480_MODE, BM_GETCHECK, 0, 0 ); 
	        g_CurrentOptions.bUseHardware    = SendDlgItemMessage( hDlg, IDC_HARDWARE, BM_GETCHECK, 0, 0 ); 
	        
	        // get options
	        g_CurrentOptions.bGameLoop = SendDlgItemMessage( hDlg, IDC_GAMELOOP, BM_GETCHECK, 0, 0 );
	        
	        // misc
	        g_CurrentOptions.bSound    = SendDlgItemMessage( hDlg, IDC_CHECK1, BM_GETCHECK, 0, 0 );
	        g_CurrentOptions.bTrace    = SendDlgItemMessage( hDlg, IDC_CHECK2, BM_GETCHECK, 0, 0 );
	        g_CurrentOptions.bShowLogo = SendDlgItemMessage( hDlg, IDC_CHECK3, BM_GETCHECK, 0, 0 );
	
	        // marks
	        g_CurrentOptions.bMark1    = SendDlgItemMessage( hDlg, IDC_CHECK_PP1, BM_GETCHECK, 0, 0 );
	        g_CurrentOptions.bMark2    = SendDlgItemMessage( hDlg, IDC_CHECK_PP2, BM_GETCHECK, 0, 0 );
	        g_CurrentOptions.bMark3    = SendDlgItemMessage( hDlg, IDC_CHECK_PP3, BM_GETCHECK, 0, 0 );
	        g_CurrentOptions.bMark4    = SendDlgItemMessage( hDlg, IDC_CHECK_PP4, BM_GETCHECK, 0, 0 );
	
	        // logo
	        g_CurrentOptions.bLogo1    = SendDlgItemMessage( hDlg, IDC_RADIO_LOGO1, BM_GETCHECK, 0, 0 );
	        g_CurrentOptions.bLogo2    = SendDlgItemMessage( hDlg, IDC_RADIO_LOGO2, BM_GETCHECK, 0, 0 );
	        g_CurrentOptions.bLogo3    = SendDlgItemMessage( hDlg, IDC_RADIO_LOGO3, BM_GETCHECK, 0, 0 );
	        g_CurrentOptions.bLogo4    = SendDlgItemMessage( hDlg, IDC_RADIO_LOGO4, BM_GETCHECK, 0, 0 );
	    
	        // sliders
	        g_CurrentOptions.fSpeed         = (float)SendDlgItemMessage( hDlg, IDC_SLIDER_SPEED,    TBM_GETPOS, 0, 0 ) / 1000;
	        g_CurrentOptions.fRotation      = (float)SendDlgItemMessage( hDlg, IDC_SLIDER_ROTATION, TBM_GETPOS, 0, 0 ) / 1000;
	        g_CurrentOptions.fSize          = (float)SendDlgItemMessage( hDlg, IDC_SLIDER_SIZE,     TBM_GETPOS, 0, 0 ) / 100 + 1.0f;
	        g_CurrentOptions.fZoom          = (float)SendDlgItemMessage( hDlg, IDC_SLIDER_ZOOM, TBM_GETPOS, 0, 0 );
	        g_CurrentOptions.fRandomFactor  = (float)SendDlgItemMessage( hDlg, IDC_SLIDER_RANDOM,   TBM_GETPOS, 0, 0 ) / 1000;
	
	        // preset
	        g_CurrentOptions.iPreset = SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_GETCURSEL, 0, 0 );
	
	
	        
	        WriteSettings( &g_CurrentOptions );
	    
	        EndDialog( hDlg, IDOK );
	        return TRUE;
	    }
	    else 
		{	 
			if( IDCANCEL == LOWORD(wParam) )
			{
				 // Handle the case when the user hits the Cancel button
				 EndDialog( hDlg, IDCANCEL );
				 return TRUE;
			}
	    }
	    
    } // NOTE: there used to be a nasty bug here: if( WM_COMMAND == uMsg ) was not surrounding IDOK and IDCANCEL
    	 // This bug is still in the exec on the net and the html of this file!!!

    return FALSE;
}




//-----------------------------------------------------------------------------
// Note: From this point on, the code is DirectX specific support for the app.
//-----------------------------------------------------------------------------




//-----------------------------------------------------------------------------
// Name: AppInitialize()
// Desc: Initializes the sample framework, then calls the app-specific function
//       to initialize device specific objects. This code is structured to
//       handled any errors that may occur duing initialization
//-----------------------------------------------------------------------------
HRESULT AppInitialize( HWND hWnd, GUID guidDevice, DDSURFACEDESC2* pddsd )
{
    HRESULT hr;

    // Setup framework flags
    DWORD dwFlags = 0L;
    if( TRUE == g_bAppUseZBuffer )
        dwFlags |= D3DFW_ZBUFFER;

    // IMPORTANT: we need to determine whether the hWnd we got is the
    // child preview window or the real fullscreen screensaver window.
    if( 0 == ( GetWindowLong( hWnd, GWL_STYLE ) & WS_CHILD ) )
        dwFlags |= D3DFW_FULLSCREEN;
        
    // Initialize the D3D framework
    if( SUCCEEDED( hr = g_pFramework->Initialize( hWnd, NULL, &guidDevice,
                                                  pddsd, dwFlags ) ) )
    {
        // Let the app run its startup code which creates the 3d scene.
        if( SUCCEEDED( hr = App_InitDeviceObjects( hWnd, 
                                             g_pFramework->GetD3DDevice() ) ) )
        {
            return S_OK;
        }
        else
        {
            App_DeleteDeviceObjects( hWnd, g_pFramework->GetD3DDevice() );
            g_pFramework->DestroyObjects();
        }
    }

    return hr;
}




//-----------------------------------------------------------------------------
// Name: Initialize3DEnvironment()
// Desc: Called when the app window is initially created, this triggers
//       creation of the remaining portion (the 3D stuff) of the app.
//-----------------------------------------------------------------------------
HRESULT Initialize3DEnvironment( HWND hWnd )
{
    HRESULT        hr;
    DDSURFACEDESC2 ddsd;
    ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );

    // Initialize the app
    if( FAILED( hr = App_OneTimeSceneInit() ) )
        return E_FAIL;

    // Create a new CD3DFramework class. This class does all of our D3D
    // initialization and manages the common D3D objects.
    if( NULL == ( g_pFramework = new CD3DFramework7() ) )
        return E_OUTOFMEMORY;

    
    // TODO: enumerate device resolutions (see: R/C Sim Window Prop Page)
    // DONE: this is not necessary
    // NOTE: we are always in Fullscreen mode. De Use Desktop Settings
    // option is verwarrend. Ook daar zitten we in Fullscreen mode
    // NOTE2: very fickle: only 640x480x16 is reliable and not even that:
    // Only when coming from another res than 640x480x16.
    // Also when current res is 8 bit: no go.
    if( g_CurrentOptions.bUse640x480Mode )
    {
        ddsd.dwWidth  = 640;
        ddsd.dwHeight = 480;
        ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
    }
    else
    {
        // this code sucks??? or this mode sucks...
        DEVMODE dm;
        dm.dmSize = sizeof(DEVMODE);
        EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &dm );
        ddsd.dwWidth  = dm.dmPelsWidth;
        ddsd.dwHeight = dm.dmPelsHeight;
        ddsd.ddpfPixelFormat.dwRGBBitCount = dm.dmBitsPerPel;
        
    }

    // Initialize the framework and scene. Try hardware first
    if( TRUE == g_CurrentOptions.bUseHardware )
    {
        if( SUCCEEDED( AppInitialize( hWnd, IID_IDirect3DHALDevice, &ddsd ) ) )
            return S_OK;

        // If that failed, try forcing a 16-bit depth
        ddsd.ddpfPixelFormat.dwRGBBitCount = 16;
        if( SUCCEEDED( AppInitialize( hWnd, IID_IDirect3DHALDevice, &ddsd ) ) )
            return S_OK;

        // If that failed too, try 640x480 mode
        ddsd.dwWidth  = 640;
        ddsd.dwHeight = 480;
        if( SUCCEEDED( AppInitialize( hWnd, IID_IDirect3DHALDevice, &ddsd ) ) )
            return S_OK;
    }

    // Resort to a software rasterizer
    if( SUCCEEDED( AppInitialize( hWnd, IID_IDirect3DRGBDevice, &ddsd ) ) )
        return S_OK;

    // All attempts failed, so return an error
    return E_FAIL;
}

    


//-----------------------------------------------------------------------------
// Name: Render3DEnvironment()
// Desc: Draws the scene
//-----------------------------------------------------------------------------
HRESULT Render3DEnvironment()
{
    // Check the cooperative level before rendering
    if( FAILED( g_pFramework->GetDirectDraw()->TestCooperativeLevel() ) )
        return S_OK;

    // Get the relative time, in seconds
    FLOAT fTime = timeGetTime() * 0.001f;

    // FrameMove (animate) the scene
    if( FAILED( App_FrameMove( g_pFramework->GetD3DDevice(), fTime ) ) )
        return E_FAIL;

    //Render the scene
    if( FAILED( App_Render( g_pFramework->GetD3DDevice() ) ) )
        return E_FAIL;
    
    // Show the frame on the primary surface.
    if( DDERR_SURFACELOST == g_pFramework->ShowFrame() )
    {
        g_pFramework->RestoreSurfaces();
        App_RestoreSurfaces();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Cleanup3DEnvironment()
// Desc: Cleanup scene objects
//-----------------------------------------------------------------------------
VOID Cleanup3DEnvironment( HWND hWnd )
{
    if( g_pFramework )
    {
        App_DeleteDeviceObjects( hWnd, g_pFramework->GetD3DDevice() );
        App_FinalCleanup();

        SAFE_DELETE( g_pFramework );
    }

    g_bReady  = FALSE;
}


  

//-----------------------------------------------------------------------------
// Name: AppOutputText()
// Desc: Draws text on the window.
//-----------------------------------------------------------------------------
VOID AppOutputText( LPDIRECT3DDEVICE3 pd3dDevice, DWORD x, DWORD y, CHAR* str )
{
    LPDIRECTDRAWSURFACE4 pddsRenderSurface;
    if( FAILED( pd3dDevice->GetRenderTarget( &pddsRenderSurface ) ) )
        return;

    // Get a DC for the surface. Then, write out the buffer
    HDC hDC;
    if( SUCCEEDED( pddsRenderSurface->GetDC(&hDC) ) )
    {
        SetTextColor( hDC, RGB(255,255,0) );
        SetBkMode( hDC, TRANSPARENT );
        ExtTextOut( hDC, x, y, 0, NULL, str, strlen(str), NULL );
    
        pddsRenderSurface->ReleaseDC(hDC);
    }
    pddsRenderSurface->Release();
}




//-----------------------------------------------------------------------------
// Name: Run()
// Desc: Message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT Run()
{
    // Special message pump
    BOOL bGotMsg;
    MSG  msg;
    PeekMessage( &msg, NULL, 0U, 0U, PM_NOREMOVE );

    while( WM_QUIT != msg.message  )
    {
        // Use PeekMessage() if the app is active, so we can use idle time to
        // render the scene. Else, use GetMessage() to avoid eating CPU time.
        if( g_bActive )
            bGotMsg = PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE );
        else
            bGotMsg = GetMessage( &msg, NULL, 0U, 0U );
        
        if( bGotMsg )
        {      
            //if ( !IsDialogMessage(hWnd, &msg) ) {     // this is for dialogs!!! 
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            //}
        }
        else 
        {
            // Render a frame during idle time (no messages are waiting)
            if( g_bActive && g_bReady )
                Render3DEnvironment();
                //GameLoop();
        }
    }

    return msg.wParam;
}



//-----------------------------------------------------------------------------
// Name: GameLoop()
// Desc: loop-the-loop
//-----------------------------------------------------------------------------
void GameLoop()
{

}




//-----------------------------------------------------------------------------
// Name: SetDlgItems()
// Desc: 
//-----------------------------------------------------------------------------
void SetDlgItems( HWND hDlg )
{
    // this is done to remove radio button checks for Default
    SendDlgItemMessage( hDlg, IDC_640x480_MODE, BM_SETCHECK, BST_UNCHECKED, 0 );
    SendDlgItemMessage( hDlg, IDC_DEFAULT_MODE, BM_SETCHECK, BST_UNCHECKED, 0 );
    SendDlgItemMessage( hDlg, IDC_HARDWARE,     BM_SETCHECK, BST_UNCHECKED, 0 );
    SendDlgItemMessage( hDlg, IDC_SOFTWARE,     BM_SETCHECK, BST_UNCHECKED, 0 );
    SendDlgItemMessage( hDlg, IDC_GAMELOOP,     BM_SETCHECK, BST_UNCHECKED, 0 );
    SendDlgItemMessage( hDlg, IDC_TIMERLOOP,    BM_SETCHECK, BST_UNCHECKED, 0 );


    if( TRUE == g_CurrentOptions.bUse640x480Mode )
        SendDlgItemMessage( hDlg, IDC_640x480_MODE, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_DEFAULT_MODE, BM_SETCHECK, BST_CHECKED, 0 ); 

    if( TRUE == g_CurrentOptions.bUseHardware )
        SendDlgItemMessage( hDlg, IDC_HARDWARE, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_SOFTWARE, BM_SETCHECK, BST_CHECKED, 0 );

    // gameloop
    if( TRUE == g_CurrentOptions.bGameLoop )
        SendDlgItemMessage( hDlg, IDC_GAMELOOP, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_TIMERLOOP, BM_SETCHECK, BST_CHECKED, 0 ); 
    

    // sound
    if( TRUE == g_CurrentOptions.bSound )
        SendDlgItemMessage( hDlg, IDC_CHECK1, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_CHECK1, BM_SETCHECK, BST_UNCHECKED, 0 );

    // trace
    if( TRUE == g_CurrentOptions.bTrace )
        SendDlgItemMessage( hDlg, IDC_CHECK2, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_CHECK2, BM_SETCHECK, BST_UNCHECKED, 0 );

    // logo
    if( TRUE == g_CurrentOptions.bShowLogo )
        SendDlgItemMessage( hDlg, IDC_CHECK3, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_CHECK3, BM_SETCHECK, BST_UNCHECKED, 0 );

    if( !g_CurrentOptions.bShowLogo ) {
        EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO1), FALSE );
        EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO2), FALSE );
        EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO3), FALSE );
        EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO4), FALSE );

    } else {
        EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO1), TRUE );
        EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO2), TRUE );
        EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO3), TRUE );
        EnableWindow( GetDlgItem(hDlg,IDC_RADIO_LOGO4), TRUE );
    }


    // mark1
    if( TRUE == g_CurrentOptions.bMark1 )
        SendDlgItemMessage( hDlg, IDC_CHECK_PP1, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_CHECK_PP1, BM_SETCHECK, BST_UNCHECKED, 0 );

    // mark2
    if( TRUE == g_CurrentOptions.bMark2 )
        SendDlgItemMessage( hDlg, IDC_CHECK_PP2, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_CHECK_PP2, BM_SETCHECK, BST_UNCHECKED, 0 );

    // mark3
    if( TRUE == g_CurrentOptions.bMark3 )
        SendDlgItemMessage( hDlg, IDC_CHECK_PP3, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_CHECK_PP3, BM_SETCHECK, BST_UNCHECKED, 0 );

    // mark4
    if( TRUE == g_CurrentOptions.bMark4 )
        SendDlgItemMessage( hDlg, IDC_CHECK_PP4, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_CHECK_PP4, BM_SETCHECK, BST_UNCHECKED, 0 );


    // logo1
    if( TRUE == g_CurrentOptions.bLogo1 )
        SendDlgItemMessage( hDlg, IDC_RADIO_LOGO1, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_RADIO_LOGO1, BM_SETCHECK, BST_UNCHECKED, 0 );

    // logo2
    if( TRUE == g_CurrentOptions.bLogo2 )
        SendDlgItemMessage( hDlg, IDC_RADIO_LOGO2, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_RADIO_LOGO2, BM_SETCHECK, BST_UNCHECKED, 0 );

    // logo3
    if( TRUE == g_CurrentOptions.bLogo3 )
        SendDlgItemMessage( hDlg, IDC_RADIO_LOGO3, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_RADIO_LOGO3, BM_SETCHECK, BST_UNCHECKED, 0 );

    // logo4
    if( TRUE == g_CurrentOptions.bLogo4 )
        SendDlgItemMessage( hDlg, IDC_RADIO_LOGO4, BM_SETCHECK, BST_CHECKED, 0 ); 
    else
        SendDlgItemMessage( hDlg, IDC_RADIO_LOGO4, BM_SETCHECK, BST_UNCHECKED, 0 );


    InitCommonControls(); // loads common controlís DLL

    // sliders
    SendDlgItemMessage( hDlg, IDC_SLIDER_SPEED,    TBM_SETRANGE, TRUE, MAKELONG(10,100) );
    SendDlgItemMessage( hDlg, IDC_SLIDER_ROTATION, TBM_SETRANGE, TRUE, MAKELONG(-100,100) );
    SendDlgItemMessage( hDlg, IDC_SLIDER_SIZE,     TBM_SETRANGE, TRUE, MAKELONG(-99,100) );
    SendDlgItemMessage( hDlg, IDC_SLIDER_ZOOM,     TBM_SETRANGE, TRUE, MAKELONG(2,50) );
    SendDlgItemMessage( hDlg, IDC_SLIDER_RANDOM,   TBM_SETRANGE, TRUE, MAKELONG(0,100) );

    SendDlgItemMessage( hDlg, IDC_SLIDER_SPEED,    TBM_SETPOS, TRUE, INT(g_CurrentOptions.fSpeed*1000) );
    SendDlgItemMessage( hDlg, IDC_SLIDER_ROTATION, TBM_SETPOS, TRUE, INT(g_CurrentOptions.fRotation*1000) );
    SendDlgItemMessage( hDlg, IDC_SLIDER_SIZE,     TBM_SETPOS, TRUE, INT(g_CurrentOptions.fSize*100)-100 );
    SendDlgItemMessage( hDlg, IDC_SLIDER_ZOOM,     TBM_SETPOS, TRUE, INT(g_CurrentOptions.fZoom) );
    SendDlgItemMessage( hDlg, IDC_SLIDER_RANDOM,   TBM_SETPOS, TRUE, INT(g_CurrentOptions.fRandomFactor*1000) );

    
    // preset
    SendDlgItemMessage( hDlg, IDC_COMBO_PRESETS, CB_SETCURSEL, g_CurrentOptions.iPreset, 0);

}




//-----------------------------------------------------------------------------
// Name: DoPresets()
// Desc: 
//-----------------------------------------------------------------------------
void DoPresets( int iItem )
{
    switch(iItem)
    {
        case 0: // (None)
            g_CurrentOptions.iPreset            = 0;        // iPreset
            break;

        case 1: // Firecracker Suite
            g_CurrentOptions.fSpeed             = 0.1f;     // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.1f;     // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 1.0f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 2.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.1f;     // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = TRUE;     // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = FALSE;    // bTrace
            g_CurrentOptions.bShowLogo          = FALSE;    // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = TRUE;     // bMark2
            g_CurrentOptions.bMark3             = TRUE;     // bMark3
            g_CurrentOptions.bMark4             = TRUE;     // bMark4
            g_CurrentOptions.bLogo1             = TRUE;     // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 1;        // iPreset
            break;

        case 2: // Lonely Planet
            g_CurrentOptions.fSpeed             = 0.1f;     // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.1f;     // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 1.0f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 2.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.1f;     // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = FALSE;    // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = FALSE;    // bTrace
            g_CurrentOptions.bShowLogo          = TRUE;     // bShowLogo
            g_CurrentOptions.bMark1             = FALSE;    // bMark1   
            g_CurrentOptions.bMark2             = FALSE;    // bMark2
            g_CurrentOptions.bMark3             = FALSE;    // bMark3
            g_CurrentOptions.bMark4             = FALSE;    // bMark4
            g_CurrentOptions.bLogo1             = FALSE;    // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = TRUE;     // bLogo4
            g_CurrentOptions.iPreset            = 2;        // iPreset
            break;

        case 3: // Orbitters
            g_CurrentOptions.fSpeed             = 0.03f;    // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.04f;    // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 1.0f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 28.0f;    // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.0f;     // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = FALSE;    // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = FALSE;    // bTrace
            g_CurrentOptions.bShowLogo          = TRUE;     // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = TRUE;     // bMark2
            g_CurrentOptions.bMark3             = TRUE;     // bMark3
            g_CurrentOptions.bMark4             = TRUE;     // bMark4
            g_CurrentOptions.bLogo1             = FALSE;    // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = TRUE;     // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 3;        // iPreset
            break;

        case 4: // Big Mamas
            g_CurrentOptions.fSpeed             = 0.01f;    // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.01f;    // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 2.0f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 2.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.01f;    // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = FALSE;    // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = TRUE;     // bTrace
            g_CurrentOptions.bShowLogo          = FALSE;    // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = TRUE;     // bMark2
            g_CurrentOptions.bMark3             = FALSE;    // bMark3
            g_CurrentOptions.bMark4             = FALSE;    // bMark4
            g_CurrentOptions.bLogo1             = TRUE;     // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 4;        // iPreset
            break;

        case 5: // Hail Brabant
            g_CurrentOptions.fSpeed             = 0.03f;    // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.04f;    // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 1.0f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 3.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.08f;    // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = FALSE;    // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = TRUE;     // bTrace
            g_CurrentOptions.bShowLogo          = FALSE;    // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = TRUE;     // bMark2
            g_CurrentOptions.bMark3             = TRUE;     // bMark3
            g_CurrentOptions.bMark4             = TRUE;     // bMark4
            g_CurrentOptions.bLogo1             = TRUE;     // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 5;        // iPreset
            break;

        case 6: // Good Morning Croatia
            g_CurrentOptions.fSpeed             = 0.1f;     // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = -0.05f;   // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 1.0f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 2.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.1f;     // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = TRUE;     // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = FALSE;    // bTrace
            g_CurrentOptions.bShowLogo          = FALSE;    // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = FALSE;    // bMark2
            g_CurrentOptions.bMark3             = TRUE;     // bMark3
            g_CurrentOptions.bMark4             = FALSE;    // bMark4
            g_CurrentOptions.bLogo1             = TRUE;     // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 6;        // iPreset
            break;

        case 7: // Screenshaver
            g_CurrentOptions.fSpeed             = 0.08f;    // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.09f;    // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 0.2f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 2.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.03f;    // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = FALSE;    // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = TRUE;     // bTrace
            g_CurrentOptions.bShowLogo          = FALSE;    // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = FALSE;    // bMark2
            g_CurrentOptions.bMark3             = FALSE;    // bMark3
            g_CurrentOptions.bMark4             = FALSE;    // bMark4
            g_CurrentOptions.bLogo1             = TRUE;     // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 7;        // iPreset
            break;

        case 8: // Pointillism
            g_CurrentOptions.fSpeed             = 0.1f;     // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.1f;     // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 0.08f;    // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 2.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.1f;     // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = FALSE;    // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = TRUE;     // bTrace
            g_CurrentOptions.bShowLogo          = FALSE;    // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = TRUE;     // bMark2
            g_CurrentOptions.bMark3             = TRUE;     // bMark3
            g_CurrentOptions.bMark4             = TRUE;     // bMark4
            g_CurrentOptions.bLogo1             = TRUE;     // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 8;        // iPreset
            break;

        case 9: // 1 is a Lonely Number
            g_CurrentOptions.fSpeed             = 0.01f;    // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.0f;     // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 1.0f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 2.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.0f;     // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = FALSE;    // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = FALSE;    // bTrace
            g_CurrentOptions.bShowLogo          = FALSE;    // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = FALSE;    // bMark2
            g_CurrentOptions.bMark3             = FALSE;    // bMark3
            g_CurrentOptions.bMark4             = FALSE;    // bMark4
            g_CurrentOptions.bLogo1             = TRUE;     // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 9;        // iPreset
            break;

        case 10: // 2 is a Lonely Number
            g_CurrentOptions.fSpeed             = 0.02f;    // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.0f;     // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 1.0f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 2.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.0f;     // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = FALSE;    // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = FALSE;    // bTrace
            g_CurrentOptions.bShowLogo          = FALSE;    // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = TRUE;     // bMark2
            g_CurrentOptions.bMark3             = FALSE;    // bMark3
            g_CurrentOptions.bMark4             = FALSE;    // bMark4
            g_CurrentOptions.bLogo1             = TRUE;     // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 10;       // iPreset
            break;

        case 11: // 3 is Company
            g_CurrentOptions.fSpeed             = 0.02f;    // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.02f;    // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 1.0f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 2.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.0f;     // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = FALSE;    // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = FALSE;    // bTrace
            g_CurrentOptions.bShowLogo          = FALSE;    // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = TRUE;     // bMark2
            g_CurrentOptions.bMark3             = TRUE;     // bMark3
            g_CurrentOptions.bMark4             = FALSE;    // bMark4
            g_CurrentOptions.bLogo1             = TRUE;     // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 11;       // iPreset
            break;

        case 12: // 4 is a Kraut
            g_CurrentOptions.fSpeed             = 0.1f;     // fSpeed: 0.01f to 0.1f;           
            g_CurrentOptions.fRotation          = 0.1f;     // fRotation: -0.1f to 0.1f (Neg==CCW and Pos==CW)
            g_CurrentOptions.fSize              = 2.0f;     // fSize: 0.01f to 2.0f (1.0f == no scaling) 
            g_CurrentOptions.fZoom              = 2.0f;     // fZoom: 2.0f to 50.0f
            g_CurrentOptions.fRandomFactor      = 0.1f;     // fRandomFactor: 0.0f to 0.1f
            g_CurrentOptions.bSound             = TRUE;     // bSound
            g_CurrentOptions.bGameLoop          = TRUE;     // bGameLoop
            g_CurrentOptions.bTrace             = TRUE;     // bTrace
            g_CurrentOptions.bShowLogo          = FALSE;    // bShowLogo
            g_CurrentOptions.bMark1             = TRUE;     // bMark1   
            g_CurrentOptions.bMark2             = TRUE;     // bMark2
            g_CurrentOptions.bMark3             = TRUE;     // bMark3
            g_CurrentOptions.bMark4             = TRUE;     // bMark4
            g_CurrentOptions.bLogo1             = TRUE;     // bLogo1   
            g_CurrentOptions.bLogo2             = FALSE;    // bLogo2
            g_CurrentOptions.bLogo3             = FALSE;    // bLogo3
            g_CurrentOptions.bLogo4             = FALSE;    // bLogo4
            g_CurrentOptions.iPreset            = 12;       // iPreset
            break;

    }


    // and set
    SetDlgItems( g_hDlg );
}
