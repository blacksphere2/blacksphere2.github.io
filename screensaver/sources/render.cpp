//-----------------------------------------------------------------------------
// File: Render.cpp
//
// Desc: Frame move and render
//
// Note: This code uses the D3D Framework helper library.
//
//
// Copyright (c) 2001 Black Sphere Corp.
//-----------------------------------------------------------------------------
#define STRICT
#define D3D_OVERLOADS



#include "render.h"




//-----------------------------------------------------------------------------
// Declare the application globals for use in WinMain.cpp
//-----------------------------------------------------------------------------
TCHAR* g_strAppTitle       = TEXT( "Black Sphere Corp. 3D Screen Saver" );
BOOL   g_bAppUseZBuffer    = TRUE;    // Create/use a z-buffer




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
#define LERP(m,x0,x1)       ((x0) + (m)*((x1)-(x0)))
#define PI                  3.14159265358979323846f
#define ELLIPSE_RADIUS      1.5f
#define ELLIPSE_NUMRINGS    20
#define ELLIPSE_NUMSECTIONS 20
#define ELLIPSE_X_LENGTH    1.0f
#define ELLIPSE_Y_LENGTH    3.0f
#define ELLIPSE_Z_LENGTH    1.0f

D3DVERTEX* g_pvModelVertices1 = NULL;            //object's vertices
D3DVERTEX* g_pvModelVertices2 = NULL;            //object's vertices
D3DVERTEX* g_pvRenderVertices = NULL;            //object's vertices
WORD*      g_pwRenderIndices  = NULL;            //object's indices
DWORD      g_dwNumVertices;
DWORD      g_dwNumIndices;



CD3DFile* g_pPP1Object = NULL;
CD3DFile* g_pPP2Object = NULL;
CD3DFile* g_pPP3Object = NULL;
CD3DFile* g_pPP4Object = NULL;

D3DMATRIX g_matPP1Matrix;
D3DMATRIX g_matPP2Matrix;
D3DMATRIX g_matPP3Matrix;
D3DMATRIX g_matPP4Matrix;

CD3DFile* g_pSphereObject = NULL;
D3DMATRIX g_matSphereMatrix;



HRESULT hr;
//DWORD dwCoopFlags;

//RECT rc;

DWORD dwRenderWidth;
DWORD dwRenderHeight;
D3DVIEWPORT7 vp;

//DDSURFACEDESC2 ddsd;
//LPDIRECTDRAWSURFACE7 pddsCoverSurface;

//LPDIRECTDRAWCLIPPER pcClipper;
//D3DMATRIX matBsc;

// D3DMATRIX matView;
// D3DVECTOR vEyePt    = D3DVECTOR( 0.0f, 0.0f, 0.0f );
// D3DVECTOR vLookatPt = D3DVECTOR( 0.0f, 0.0f, 1.0f );
// D3DVECTOR vUpVec    = D3DVECTOR( 0.0f, 1.0f, 0.0f );

D3DXMATRIX matAll;
D3DXMATRIX matTrans;
D3DXMATRIX matScale;
D3DXMATRIX matRot;
D3DXQUATERNION qRot;

float fX = 0.0f;
float fY = 0.0f;
float fZ = 0.0f;
float fRadsX = 0.0f;
float fRadsY = 0.0f;
float fRadsZ = 0.0f;

bool moveXUpPP1 = true;
bool moveYUpPP1 = true;
bool moveZUpPP1 = true;

bool moveXUpPP2 = true;
bool moveYUpPP2 = true;
bool moveZUpPP2 = true;

bool moveXUpPP3 = true;
bool moveYUpPP3 = true;
bool moveZUpPP3 = true;

bool moveXUpPP4 = true;
bool moveYUpPP4 = true;
bool moveZUpPP4 = true;

//bool bLButtonDown = false;
//bool bRButtonDown = false;

bool bHitRightPP1 = false;
bool bHitRightPP2 = false;
bool bHitRightPP3 = false;
bool bHitRightPP4 = false;

// alpha-blending
int iAlpha1 = 0;
int iAlpha2 = 0;
bool fadeIn1 = true;
bool fadeIn2 = false;

int count1 = 0;
int count2 = 0;


bool bStartTumble = false;

int i = 0;
bool tumblePP1 = true;
bool tumblePP2 = true;
bool tumblePP3 = true;
bool tumblePP4 = true;


float fAspect;
D3DMATRIX matProj;

//bool bRenderTargetTooSmall = false;


// options
// TODO: put these in ScreenSaverOptions struct
float g_fSpeed;
float g_fRotation;

// NOTE: using BOOL (i.e. int) instead of bool will prevent C4800 warnings when
// assigning it a DWORD reg value
BOOL g_bSound = false;
BOOL g_bShowTrace = false;

float g_fXLimit;
float g_fYLimit;
float g_fZLimit;


float fRandomPP1DeltaX = 0.0f;
float fRandomPP1DeltaY = 0.0f;
float fRandomPP2DeltaX = 0.0f;
float fRandomPP2DeltaY = 0.0f;
float fRandomPP3DeltaX = 0.0f;
float fRandomPP3DeltaY = 0.0f;
float fRandomPP4DeltaX = 0.0f;
float fRandomPP4DeltaY = 0.0f;

int iRandomPP1X = 0;
int iRandomPP1Y = 0;
int iRandomPP2X = 0;
int iRandomPP2Y = 0;
int iRandomPP3X = 0;
int iRandomPP3Y = 0;
int iRandomPP4X = 0;
int iRandomPP4Y = 0;





extern CD3DFramework7* g_pFramework;


void TumblePP( D3DMATRIX& matPP, bool& moveXUp, bool& moveYUp, bool& bHitRight,
                          int& iRandomX, int& iRandomY, float& fRandomDeltaX, float& fRandomDeltaY );


void DrawLogo();
void DrawLogo1( LPDIRECT3DDEVICE7 pd3dDevice);
void DrawLogo2( LPDIRECT3DDEVICE7 pd3dDevice);
void DrawLogo3( LPDIRECT3DDEVICE7 pd3dDevice);
void DrawLogo4( LPDIRECT3DDEVICE7 pd3dDevice);


void BltAlphaFactor( LPDIRECT3DDEVICE7 pdev, LPDIRECTDRAWSURFACE7 pTex, 
                                        WORD x, WORD y, WORD x2, WORD y2, BYTE factor, bool clear );

// textures
//enum {
//    TEX_SPHERE0, TEX_SPHERE1, TEX_SPHERE2, TEX_SPHERE3,
//    NUM_TEXTURES
//};

char* g_szTexName[NUM_TEXTURES];

LPDIRECTDRAWSURFACE7 g_ppTex[NUM_TEXTURES];








//-----------------------------------------------------------------------------
// Function prototypes and global (or static) variables
//-----------------------------------------------------------------------------
VOID    AppPause( BOOL );
VOID    RotateVertexInX( FLOAT, DWORD, D3DVERTEX*, D3DVERTEX* );
BOOL    GenerateSphere( FLOAT, DWORD, DWORD, FLOAT, FLOAT, FLOAT, D3DVERTEX**,
                        DWORD*, WORD**, DWORD* );
VOID    BlendObjects( DWORD, D3DVERTEX*, D3DVERTEX*, D3DVERTEX* );




//-----------------------------------------------------------------------------
// Name: App_OneTimeSceneInit()
// Desc: Called during initial app startup, this function performs all the
//       permanent initialization.
//-----------------------------------------------------------------------------
HRESULT App_OneTimeSceneInit()
{
/*
    // Generate the object data
    GenerateSphere( ELLIPSE_RADIUS, ELLIPSE_NUMRINGS, ELLIPSE_NUMSECTIONS, 
                    ELLIPSE_X_LENGTH, ELLIPSE_Y_LENGTH, ELLIPSE_Z_LENGTH, 
                    &g_pvRenderVertices, &g_dwNumVertices, 
                    &g_pwRenderIndices, &g_dwNumIndices );
    RotateVertexInX( (FLOAT)(PI/2), g_dwNumVertices, g_pvRenderVertices,
                     g_pvRenderVertices );

    // Make two copies of the object (for modification of the vertices)
    g_pvModelVertices1 = new D3DVERTEX[g_dwNumVertices];
    g_pvModelVertices2 = new D3DVERTEX[g_dwNumVertices];

    for( DWORD i=0; i<g_dwNumVertices; i++ )
    {
        g_pvModelVertices1[i] = g_pvRenderVertices[i];
        g_pvModelVertices2[i] = g_pvRenderVertices[i];
    }


    // Create textures
    //D3DTextr_CreateTextureFromFile( "Banana.bmp" );

        // ??? No
        //D3DTextr_CreateTextureFromFile( "IDB_BITMAP1" );
        //D3DTextr_CreateTextureFromFile( "IDB_BITMAP2" );
        //D3DTextr_RestoreAllTextures( pd3dDevice );
*/

        // kludge
        //SetCurrentDirectory( "C:\\resources" );
        //SetCurrentDirectory( "..\\media" );
        //SetCurrentDirectory( "F:\\mssdk\\samples\\Multimedia\\D3DIM\\src\\ScreenSaver\\media" );
        SetCurrentDirectory( g_strMediaPath );

        // Scr should be self-contained app, i.e. a single file. Therefore the
        // textures should be resources, and the x file too. On app start up we
        // first create files from the resources and scr loads these
        // Create the files in C:\ because it would be ugly for the user to see
        // all those files being created when double-clicking the scr (or would it)
        // Delete the files after the scr exits.

        // Geen self-contained app: zo krijgt user kans wat met de media files te spelen.
        // InstallShield en save program directory in reg zodat scr de media dir kan vinden.

        // Load the pilot position marks
		g_pPP1Object = new CD3DFile();
		if( FAILED( g_pPP1Object->Load( "pp1.x" ) ) ) {
                MessageBox(NULL,"Can't find X file.",
                        "Black Sphere Corp. 3D Screensaver Error Message",MB_OK|MB_ICONERROR);
        return E_FAIL;
        }

        // Load the pilot position marks
		g_pPP2Object = new CD3DFile();
		if( FAILED( g_pPP2Object->Load( "pp2.x" ) ) ) {
                MessageBox(NULL,"Can't find X file.",
                        "Black Sphere Corp. 3D Screensaver Error Message",MB_OK|MB_ICONERROR);
        return E_FAIL;
        }

        // Load the pilot position marks
		g_pPP3Object = new CD3DFile();
		if( FAILED( g_pPP3Object->Load( "pp3.x" ) ) ) {
                MessageBox(NULL,"Can't find X file.",
                        "Black Sphere Corp. 3D Screensaver Error Message",MB_OK|MB_ICONERROR);
        return E_FAIL;
        }

        // Load the pilot position marks
		g_pPP4Object = new CD3DFile();
		if( FAILED( g_pPP4Object->Load( "pp4.x" ) ) ) {
                MessageBox(NULL,"Can't find X file.",
                        "Black Sphere Corp. 3D Screensaver Error Message",MB_OK|MB_ICONERROR);
        return E_FAIL;
        }


        // Load the sphere
		g_pSphereObject = new CD3DFile();
		if( FAILED( g_pSphereObject->Load( "sphere.x" ) ) ) {
                MessageBox(NULL,"Can't find X file.",
                        "Black Sphere Corp. 3D Screensaver Error Message",MB_OK|MB_ICONERROR);
        return E_FAIL;
        }


        // set matrix
        D3DUtil_SetIdentityMatrix(g_matPP1Matrix);
        D3DUtil_SetScaleMatrix(matScale, g_CurrentOptions.fSize, g_CurrentOptions.fSize, g_CurrentOptions.fSize);
        D3DMath_MatrixMultiply(g_matPP1Matrix, matScale, g_matPP1Matrix);       // order is crucial!!!

        g_matPP1Matrix._41 = -15.0f;
        g_matPP1Matrix._43 = g_CurrentOptions.fZoom; //2.0f;    

        
        g_matPP2Matrix=g_matPP3Matrix=g_matPP4Matrix=g_matPP1Matrix;

        //g_matPP2Matrix._41 = -20.0f;
        //g_matPP3Matrix._41 = -30.0f;
        //g_matPP4Matrix._41 = -40.0f;


        // logo4 sphere
        D3DUtil_SetIdentityMatrix(g_matSphereMatrix);

        float fScale = 0.92f; //0.9f;
        D3DUtil_SetScaleMatrix(matScale, 1.0f*fScale, 1.25f*fScale, 1.0f*fScale);
        D3DMath_MatrixMultiply(g_matSphereMatrix, matScale, g_matSphereMatrix); // order is crucial!!!

        D3DUtil_SetRotateZMatrix(matRot, -g_PI_DIV_2-0.3f);
        D3DMath_MatrixMultiply(g_matSphereMatrix, matRot, g_matSphereMatrix);

        
        

        g_matSphereMatrix._43 = 0.0f; //0.0f;




        // Seed the random-number generator with current time so that
        // the numbers will be different every time we run.
        srand( (unsigned)time( NULL ) );


    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: App_FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT App_FrameMove( LPDIRECT3DDEVICE7 pd3dDevice, FLOAT fTimeKey )
{
    // Compute the bend and rotate angles for this frame
    FLOAT fRotateAngle = (FLOAT)( fTimeKey / 3 );
    FLOAT fBendAngle   = (FLOAT)( (sin(fTimeKey)+1.0f)*0.6f );

    // Setup the world spin matrix
    D3DMATRIX matWorldSpin;
    D3DUtil_SetRotateYMatrix( matWorldSpin, fRotateAngle );
    pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &matWorldSpin );

    if (g_CurrentOptions.bGameLoop) {
            // gameloop
            g_fSpeed = g_CurrentOptions.fSpeed*3; // 0.1f;
            g_fRotation = g_CurrentOptions.fRotation*10; // 0.34f;
    } else {
            // timerloop
            g_fSpeed = g_CurrentOptions.fSpeed*6; // 0.2f;
            g_fRotation = g_CurrentOptions.fRotation*20; // 1.0f;
    }



    // kludge for cursor
    if (g_CurrentOptions.bGameLoop) {
            //SetCursor(NULL);
            //ShowCursor( FALSE );
    }


    g_bSound = g_CurrentOptions.bSound;

    g_fXLimit = 6.5f;
    g_fYLimit = 6.5f;
    //g_fZLimit = 8.0f;

    // Mmm, nothing like a proper gameloop: timer causes erratic framerate
    // In 3D About Box not really noticable but here unacceptable
    //MessageBeep(-1);




    // tumble PP1
    if (tumblePP1 && g_CurrentOptions.bMark1) 
            TumblePP( g_matPP1Matrix, moveXUpPP1, moveYUpPP1, bHitRightPP1,
                                    iRandomPP1X, iRandomPP1Y, fRandomPP1DeltaX, fRandomPP1DeltaY );

    // tumble PP2
    if (tumblePP2 && g_CurrentOptions.bMark2) 
            TumblePP( g_matPP2Matrix, moveXUpPP2, moveYUpPP2, bHitRightPP2,
                                    iRandomPP2X, iRandomPP2Y, fRandomPP2DeltaX, fRandomPP2DeltaY );

    // tumble PP3
    if (tumblePP3 && g_CurrentOptions.bMark3) 
            TumblePP( g_matPP3Matrix, moveXUpPP3, moveYUpPP3, bHitRightPP3,
                                    iRandomPP3X, iRandomPP3Y, fRandomPP3DeltaX, fRandomPP3DeltaY );

    // tumble PP4
    if (tumblePP4 && g_CurrentOptions.bMark4) 
            TumblePP( g_matPP4Matrix, moveXUpPP4, moveYUpPP4, bHitRightPP4,
                                    iRandomPP4X, iRandomPP4Y, fRandomPP4DeltaX, fRandomPP4DeltaY );


        
        



    // Bend two copies of the object in different directions and 
    // merge (blend) them into one set of vertex data.
    //RotateVertexInX( fBendAngle, g_dwNumVertices, g_pvModelVertices2,
    //                 g_pvModelVertices1 );
    //BlendObjects( g_dwNumVertices, g_pvModelVertices1, g_pvModelVertices2,
    //              g_pvRenderVertices );

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: App_Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT App_Render( LPDIRECT3DDEVICE7 pd3dDevice )
{

        // TODO: only in timerloop will scr be visible in Display Properties
        // but when moving that dialog scr will be fucked. This is a bug in the
        // original app. Fix it!!!

        // TODO: solve artifacts in gameloop:
        // - not visible in Display Properties
        // - desktop flickers through
        // - cursor shows
        // - mousemove does not kill saver
        // - when Display Properties dialog is up and scr appears after the
        //   set minutes have elapsed scr will be very slow and erratic

        // NOTE: mousemove does kill scr when in game loop if we are in another resolution
        // than 640x480 and let the scr run at 640x480
        //
        // NOTE2: 640x480x16 is de veiligste mode


        
    // trace werkt alleen goed in 640x480x16 + gameloop
        // Don't switch trace on here, or logos will be painted incorrectly
        // switch it on when we start tumbling
        //g_bShowTrace = g_CurrentOptions.bTrace;
        
        
        // Clear the viewport    
        if (g_bShowTrace) {
                pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER,
               0x00000000, 1.0f, 0L ); // black
        } else {
                pd3dDevice->Clear( 0, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
               0x00000000, 1.0f, 0L ); // black
        }


        // GDI blitting is too slow...///////////////////////////////////////
        //if (g_CurrentOptions.bLogo)
        //      DrawLogo();
        /////////////////////////////////////////////////////////////////////


        


        // TODO: draw black background with DrawPrimitive()
        // TODO: draw bsc logo with DrawPrimitive()
        // But make sure your textures are square and a power of 2
        // De juiste D3D manier van bitmap drawing (GDI Blt is too slow):
        // See: Flare Sample
        // Draw the background
    //    m_pd3dDevice->SetTexture( 0, D3DTextr_GetSurface("dx5_logo.bmp") );
    //    m_pd3dDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, D3DFVF_TLVERTEX,
    //                                 m_Background, 4, 0 );



        //Sleep(100);

        //SetPriorityClass( GetCurrentProcess(), REALTIME_PRIORITY_CLASS ); 
        //SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );


    // Begin the scene 
    if( SUCCEEDED( pd3dDevice->BeginScene() ) )
    {
        //Display the object
        //pd3dDevice->DrawIndexedPrimitive( D3DPT_TRIANGLELIST, D3DFVF_VERTEX,
        //                  g_pvRenderVertices, g_dwNumVertices, 
        //                  g_pwRenderIndices, g_dwNumIndices, NULL );


                
                if( !g_CurrentOptions.bShowLogo ) {
                        g_CurrentOptions.bLogo1 = false;
                        g_CurrentOptions.bLogo2 = false;
                        g_CurrentOptions.bLogo3 = false;
                        g_CurrentOptions.bLogo4 = false;

                        bStartTumble = true;
                }



                // D3D blitting for raw speed.../////////////////////////////////////
                if (g_CurrentOptions.bLogo1) DrawLogo1( pd3dDevice );           
                /////////////////////////////////////////////////////////////////////



                // D3D blitting for raw speed.../////////////////////////////////////
                if (g_CurrentOptions.bLogo2) DrawLogo2( pd3dDevice );
                /////////////////////////////////////////////////////////////////////



                // D3D blitting for raw speed.../////////////////////////////////////
                if (g_CurrentOptions.bLogo3) DrawLogo3( pd3dDevice );           
                /////////////////////////////////////////////////////////////////////


                // D3D blitting for raw speed.../////////////////////////////////////
                if (g_CurrentOptions.bLogo4) DrawLogo4( pd3dDevice );
                /////////////////////////////////////////////////////////////////////



                tumblePP1=tumblePP2=tumblePP3=tumblePP4=false;

                if (bStartTumble) {
                        static int count = 0;
                        count++;                        
                        if (count > 0)   tumblePP1 = true;
                        if (count > 111) tumblePP2 = true;
                        if (count > 222) tumblePP3 = true;
                        if (count > 333) tumblePP4 = true;

                        // only now, after logo painting, switch on trace (if user has set it)
                        g_bShowTrace = g_CurrentOptions.bTrace;
                }


                //pd3dDevice->SetTextureStageState( 0, D3DTSS_ADDRESS, D3DTADDRESS_WRAP );
                
                // xfile sphere
                if ( g_CurrentOptions.bLogo4 ) {
                        pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &g_matSphereMatrix );
                        g_pSphereObject->Render( pd3dDevice );
                }


                if ( g_CurrentOptions.bMark1 && g_matPP1Matrix._41 > -(g_fXLimit+1.0f) ) {
                        pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &g_matPP1Matrix );
                        g_pPP1Object->Render( pd3dDevice );
                }
                
                if ( g_CurrentOptions.bMark2 && g_matPP2Matrix._41 > -(g_fXLimit+1.0f) ) {
                        pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &g_matPP2Matrix );
                        g_pPP2Object->Render( pd3dDevice );
                }       
                
                if ( g_CurrentOptions.bMark3 && g_matPP3Matrix._41 > -(g_fXLimit+1.0f) ) {
                        pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &g_matPP3Matrix );
                        g_pPP3Object->Render( pd3dDevice );
                }       
                
                if ( g_CurrentOptions.bMark4 && g_matPP4Matrix._41 > -(g_fXLimit+1.0f) ) {
                        pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD, &g_matPP4Matrix );
                        g_pPP4Object->Render( pd3dDevice );
                }

        // End the scene.
        pd3dDevice->EndScene();
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: App_InitDeviceObjects()
// Desc: Initialize scene objects.
//-----------------------------------------------------------------------------
HRESULT App_InitDeviceObjects( HWND hWnd, LPDIRECT3DDEVICE7 pd3dDevice )
{
    // Check parameters
    if( NULL==pd3dDevice )
        return E_INVALIDARG;

    // Get the device caps
    D3DDEVICEDESC7 ddDesc;
    if( FAILED( pd3dDevice->GetCaps( &ddDesc ) ) )
        return E_FAIL;

    // Setup the material
    D3DMATERIAL7      mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 1.0f, 1.0f );
    mtrl.power = 40.0f;
    pd3dDevice->SetMaterial( &mtrl );

    // Set up the textures
    D3DTextr_RestoreAllTextures( pd3dDevice );
    pd3dDevice->SetTexture( 0, D3DTextr_GetSurface("Banana.bmp") );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR );
    pd3dDevice->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFG_LINEAR );


        // new textures
        // NOTE: got to set the current directory right
        //
        // CD3DFile::Load() searches as follows:
        // 1. D3DUtil_GetDXSDKMediaPath()
        // 2. current directory
        //
        // D3DTextr_CreateTextureFromFile() searches are done by:
        // TextureContainer::LoadImageData() as follows:
        // 1. executable's resource (so it must be possible to put all the bmp's in the .exe)
        // 2. current directory/global texture path (can be set by D3DTextr_SetTexturePath(),
        //              initially set to current directory)
        // 3. D3DUtil_GetDXSDKMediaPath()

        // already there
        //SetCurrentDirectory( "..\\media" );

        D3DTextr_CreateTextureFromFile( "sphere0.bmp" );
    D3DTextr_CreateTextureFromFile( "sphere1.bmp" );
    //D3DTextr_CreateTextureFromFile( "sphere2.bmp" );
    //D3DTextr_CreateTextureFromFile( "sphere3.bmp" );

        D3DTextr_RestoreAllTextures( pd3dDevice );

        g_ppTex[TEX_SPHERE0] = D3DTextr_GetSurface( "sphere0.bmp" );
        g_ppTex[TEX_SPHERE1] = D3DTextr_GetSurface( "sphere1.bmp" );
        //g_ppTex[TEX_SPHERE2] = D3DTextr_GetSurface( "sphere2.bmp" );
        //g_ppTex[TEX_SPHERE3] = D3DTextr_GetSurface( "sphere3.bmp" );



    // Miscellaneous render states
    pd3dDevice->SetRenderState( D3DRENDERSTATE_AMBIENT,        0x40404040 );
    pd3dDevice->SetRenderState( D3DRENDERSTATE_DITHERENABLE,   TRUE );
    pd3dDevice->SetRenderState( D3DRENDERSTATE_SPECULARENABLE, TRUE );
    pd3dDevice->SetRenderState( D3DRENDERSTATE_ZENABLE,        TRUE );

    // Set the transform matrices
    D3DVECTOR vEyePt    = D3DVECTOR( 0.0f, 0.0f, -6.5f );
    D3DVECTOR vLookatPt = D3DVECTOR( 0.0f, 0.0f,  0.0f );
    D3DVECTOR vUpVec    = D3DVECTOR( 0.0f, 1.0f,  0.0f );
    D3DMATRIX matWorld, matView, matProj;

    D3DUtil_SetIdentityMatrix( matWorld );
    D3DUtil_SetViewMatrix( matView, vEyePt, vLookatPt, vUpVec );
    D3DUtil_SetProjectionMatrix( matProj, 1.57f, 1.0f, 1.0f, 100.0f );

    pd3dDevice->SetTransform( D3DTRANSFORMSTATE_WORLD,      &matWorld );
    pd3dDevice->SetTransform( D3DTRANSFORMSTATE_VIEW,       &matView );
    pd3dDevice->SetTransform( D3DTRANSFORMSTATE_PROJECTION, &matProj );

    // Set up lighting states
    if( ddDesc.dwVertexProcessingCaps & D3DVTXPCAPS_DIRECTIONALLIGHTS )
    {
        D3DLIGHT7 light;
        D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, 0.0f, -1.0f, 0.0f );
        pd3dDevice->SetLight( 0, &light );
        pd3dDevice->LightEnable( 0, TRUE );
        pd3dDevice->SetRenderState( D3DRENDERSTATE_LIGHTING, TRUE );
    }

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: App_DeleteDeviceObjects()
// Desc: Called when the app is exitting, or the device is being changed,
//       this function deletes any device dependant objects.
//-----------------------------------------------------------------------------
VOID App_DeleteDeviceObjects( HWND hWnd, LPDIRECT3DDEVICE7 pd3dDevice )
{
    D3DTextr_InvalidateAllTextures();
}




//-----------------------------------------------------------------------------
// Name: App_FinalCleanup()
// Desc: Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT App_FinalCleanup()
{
    SAFE_DELETE( g_pvModelVertices1 );
    SAFE_DELETE( g_pvModelVertices2 );
    SAFE_DELETE( g_pvRenderVertices );
    SAFE_DELETE( g_pwRenderIndices );

    return S_OK;
}




//----------------------------------------------------------------------------
// Name: App_RestoreSurfaces
// Desc: Restores any previously lost surfaces. Must do this for all surfaces
//       (including textures) that the app created.
//----------------------------------------------------------------------------
HRESULT App_RestoreSurfaces()
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: App_ConfirmDevice()
// Desc: Called during device intialization, this code checks the device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT App_ConfirmDevice( DDCAPS* pddDriverCaps,
                           D3DDEVICEDESC7* pd3dDeviceDesc )
{
    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RotateVertexInX()
// Desc: Rotates an array of vertices by an amount theta about the x-axis.
//-----------------------------------------------------------------------------
VOID RotateVertexInX( FLOAT fTheta, DWORD dwCount,
                      D3DVERTEX* pvInVertices, D3DVERTEX* pvOutVertices )
{
    FLOAT fSin = (FLOAT)sin(fTheta); 
    FLOAT fCos = (FLOAT)cos(fTheta);
    
    for( DWORD i=0; i<dwCount; i++ )
    {
        FLOAT y = pvInVertices[i].y;
        FLOAT z = pvInVertices[i].z;
        pvOutVertices[i].y = fCos*y + fSin*z;
        pvOutVertices[i].z = -fSin*y + fCos*z;

        FLOAT ny = pvInVertices[i].ny;
        FLOAT nz = pvInVertices[i].nz;
        pvOutVertices[i].ny = fCos*ny + fSin*nz;
        pvOutVertices[i].nz = -fSin*ny + fCos*nz;
    }
}



//-----------------------------------------------------------------------------
// Name: GenerateSphere()
// Desc: Makes vertex and index data for a sphere.
//-----------------------------------------------------------------------------
BOOL GenerateSphere( FLOAT fRadius, DWORD dwNumRings, DWORD dwNumSections, 
                     FLOAT sx, FLOAT sy, FLOAT sz,
                     D3DVERTEX** ppvVertices, DWORD* pdwNumVertices,
                     WORD** ppwIndices, DWORD* pdwNumIndices )
{
    FLOAT x, y, z, v, rsintheta; // Temporary variables
    DWORD i, j;            // counters
    DWORD n, m;            // counters

    //Generate space for the required triangles and vertices.
    DWORD      dwNumTriangles = (dwNumRings+1) * dwNumSections * 2;
    DWORD      dwNumVertices  = (dwNumRings+1) * dwNumSections + 2;
    D3DVERTEX* pvVertices     = new D3DVERTEX[dwNumVertices];
    DWORD      dwNumIndices   = dwNumTriangles*3;
    WORD*      pwIndices      = new WORD[dwNumIndices];

    // Generate vertices at the top and bottom points.
    D3DVECTOR vPoint  = D3DVECTOR( 0.0f, sy*fRadius, 0.0f );
    D3DVECTOR vNormal = D3DVECTOR( 0.0f, 0.0f, 1.0f );
    pvVertices[0]               = D3DVERTEX(  vPoint,  vNormal, 0.0f, 0.0f );
    pvVertices[dwNumVertices-1] = D3DVERTEX( -vPoint, -vNormal, 0.0f, 0.0f );

    // Generate vertex points for rings
    FLOAT dtheta = (FLOAT)(PI / (dwNumRings + 2));     //Angle between each ring
    FLOAT dphi   = (FLOAT)(2*PI / dwNumSections); //Angle between each section
    FLOAT theta  = dtheta;
    n = 1; //vertex being generated, begins at 1 to skip top point

    dwNumRings += 1;
    dwNumRings -= 1;

    for( i = 0; i < (dwNumRings+1); i++ )
    {
        y = fRadius * (FLOAT)cos(theta); // y is the same for each ring
        v = theta / PI;     // v is the same for each ring
        rsintheta = fRadius * (FLOAT)sin(theta);
        FLOAT phi = 0.0f;

        for( j = 0; j < dwNumSections; j++ )
        {
            x = rsintheta * (FLOAT)sin(phi);
            z = rsintheta * (FLOAT)cos(phi);
        
            FLOAT u = (FLOAT)(1.0 - phi / (2*PI) );
            
            vPoint        = D3DVECTOR( sx*x, sy*y, sz*z );
            vNormal       = D3DVECTOR( x/fRadius, y/fRadius, z/fRadius );
            pvVertices[n] = D3DVERTEX( vPoint, vNormal, u, v );

            phi += dphi;
            ++n;
        }
        theta += dtheta;
    }

    // Generate triangles for top and bottom caps.
    for( i = 0; i < dwNumSections; i++ )
    {
        DWORD t1 = 3*i;
        DWORD t2 = 3*(dwNumTriangles - dwNumSections + i);

        pwIndices[t1+0] = (WORD)(0);
        pwIndices[t1+1] = (WORD)(i + 1);
        pwIndices[t1+2] = (WORD)(1 + ((i + 1) % dwNumSections));

        pwIndices[t2+0] = (WORD)( dwNumVertices - 1 );
        pwIndices[t2+1] = (WORD)( dwNumVertices - 2 - i );
        pwIndices[t2+2] = (WORD)( dwNumVertices - 2 - ((1 + i) % dwNumSections) );
    }

    // Generate triangles for the rings
    m = 1;            // 1st vertex begins at 1 to skip top point
    n = dwNumSections; // triangle being generated, skip the top cap 
    
    for( i = 0; i < dwNumRings; i++ )
    {
        for( j = 0; j < dwNumSections; j++ )
        {
            pwIndices[3*n+0] = (WORD)(m + j);
            pwIndices[3*n+1] = (WORD)(m + dwNumSections + j);
            pwIndices[3*n+2] = (WORD)(m + dwNumSections + ((j + 1) % dwNumSections));
            n++;
            
            pwIndices[3*n+0] = (WORD)(m + j);
            pwIndices[3*n+1] = (WORD)(m + dwNumSections + ((j + 1) % dwNumSections));
            pwIndices[3*n+2] = (WORD)(m + ((j + 1) % dwNumSections));
            n++;
        }
        m += dwNumSections;
    }

    (*pdwNumIndices)  = dwNumIndices;
    (*ppwIndices)     = pwIndices;
    (*pdwNumVertices) = dwNumVertices;
    (*ppvVertices)    = pvVertices;

    return TRUE;
}




//-----------------------------------------------------------------------------
// Name: BlendObjects()
// Desc: Merges two sets of vertices together
//-----------------------------------------------------------------------------
VOID BlendObjects( DWORD dwCount, D3DVERTEX* pvInputVertices1, 
                   D3DVERTEX* pvInputVertices2,
                   D3DVERTEX* pvOutputVertices )
{
    D3DVERTEX* p1 = pvInputVertices1;
    D3DVERTEX* p2 = pvInputVertices2;
    D3DVERTEX* p3 = pvOutputVertices;

    FLOAT fMinZ = -ELLIPSE_Y_LENGTH * ELLIPSE_RADIUS;
    FLOAT fMaxZ = +ELLIPSE_Y_LENGTH * ELLIPSE_RADIUS;

    for( DWORD i=0; i<dwCount; i++ )
    {
        FLOAT m;
        FLOAT a = ( p2->z - fMinZ ) / ( fMaxZ - fMinZ );

        if( a >= 0.75f )
            m = 0.0f;
        else if( a >= 0.5f )
        {
            FLOAT x = 4*(0.75f-a);
            m = (x*x)*0.5f;
        }
        else if( a >= 0.25f )
        {
            FLOAT x = 4*(a-0.25f);
            m = 1.0f-(x*x)*0.5f;
        }
        else
            m = 1.0f;

        p3->x = LERP( m, p1->x, p2->x );
        p3->y = LERP( m, p1->y, p2->y );
        p3->z = LERP( m, p1->z, p2->z );

        p1++; p2++; p3++;
    }
}







//-----------------------------------------------------------------------------
// Name: TumblePP()
// Desc: Tumble about PP's
//-----------------------------------------------------------------------------
void TumblePP( D3DMATRIX& matPP, bool& moveXUp, bool& moveYUp, bool& bHitRight,
                          int& iRandomX, int& iRandomY, float& fRandomDeltaX, float& fRandomDeltaY )
{
        // X
        if (moveXUp)
                matPP._41 += g_fSpeed;
        else
                matPP._41 -= g_fSpeed;
        
        if (matPP._41 >  g_fXLimit) {
                moveXUp = false;
                bHitRight = true;
                if (g_bSound)
                        PlaySound( "crash.wav", NULL, SND_FILENAME|SND_ASYNC );
        }
        if (matPP._41 < -g_fXLimit) {
                moveXUp = true;
                if (g_bSound && bHitRight)
                        PlaySound( "crash.wav", NULL, SND_FILENAME|SND_ASYNC );
        }


        // Y
        if (moveYUp)
                matPP._42 += g_fSpeed/2;
        else
                matPP._42 -= g_fSpeed;
        
        if (matPP._42 >  g_fYLimit) {
                moveYUp = false;
                if (g_bSound)
                        PlaySound( "crash.wav", NULL, SND_FILENAME|SND_ASYNC );
        }
        if (matPP._42 < -g_fYLimit) {
                moveYUp = true;
                if (g_bSound)
                        PlaySound( "crash.wav", NULL, SND_FILENAME|SND_ASYNC );
        }

/*      
                // Z
                if (moveZUp)
                        matPP._43 += 0.1f;
                else
                        matPP._43 -= 0.1f;
                
                if (matPP._43 > 11.0f) moveZUp = false; 
                if (matPP._43 <  9.0f) moveZUp = true;
*/

        fRadsX = -0.08f*g_fRotation;
        fRadsY = -0.15f*g_fRotation;
        fRadsZ = -0.2f*g_fRotation;

        
        // ook hier: gimbal lock!!!
        // Quaternions ///////////
        //D3DXMATRIX matTrans;
        //D3DXMatrixTranslation( &matTrans, fX, fY, fZ );
        //D3DMath_MatrixMultiply(matBsc, matTrans, matBsc);     // order is crucial!!!
        
        //D3DXMATRIX matRot;
        //D3DXQUATERNION qRot;
        //D3DXQuaternionRotationYawPitchRoll( &qRot, fRadsY, fRadsX, fRadsZ );  
        //D3DXMatrixRotationQuaternion( &matRot, &qRot );
        
        D3DXMatrixRotationYawPitchRoll( &matRot, fRadsY, fRadsX, fRadsZ );
        D3DMath_MatrixMultiply(matPP, matRot, matPP); // order is crucial!!!

        
        // Order!!!
        //D3DMath_MatrixMultiply( matBsc, matTrans, matBsc );
        //D3DMath_MatrixMultiply( matBsc, matRot, matBsc );
        //D3DMath_MatrixMultiply( matAll, matRot, matBsc );
        //D3DMath_MatrixMultiply( matBsc, matAll, matBsc );


        // reset!!!
        fX=fY=fZ=0.0f;
        fRadsX=fRadsY=fRadsZ=0.0f;


        // randomize //////////////////////////////////////////////////////////////
        // Only seed once per app run!!! Do in OneTimeSceneInit()
        // Seed the random-number generator with current time so that
    // the numbers will be different every time we run.
        // generate one rand per hit
        // srand( (unsigned)time( NULL ) );
        // RAND_MAX == 0x7fff == 32767

        float fRandomFactor = g_CurrentOptions.fRandomFactor; // 0.0f to 0.1f

        
        // only start randomizing if we have hit right
        if (bHitRight)
        {
                // randomize X
                if ( (matPP._41 >  g_fXLimit && moveXUp == false ) ||   // hit right
                         (matPP._41 < -g_fXLimit && moveXUp == true) )          // hit left 
                {
                        iRandomX = rand();
                        fRandomDeltaX = ( (float)iRandomX/RAND_MAX )*fRandomFactor;
                        //MessageBeep(-1);
                }

                if ( iRandomX%2 == 0 ) 
                        matPP._41 += fRandomDeltaX;
                else
                        matPP._41 -= fRandomDeltaX;


                // randomize Y
                if ( (matPP._42 >  g_fYLimit && moveYUp == false ) ||   // hit top
                         (matPP._42 < -g_fYLimit && moveYUp == true) )          // hit bottom
                {
                        iRandomY = rand();
                        fRandomDeltaY = ( (float)iRandomY/RAND_MAX )*fRandomFactor;
                        //MessageBeep(-1);
                }

                if ( iRandomY%2 == 0 ) 
                        matPP._42 += fRandomDeltaY;
                else
                        matPP._42 -= fRandomDeltaY;
        }
        ///////////////////////////////////////////////////////////////////////////
        
        



        // Helaas, maar waar: de Framework/D3D vervormen de objecten naarmate ze
        // zich verder buiten het gezichtsveld begeven. Dit is het duidelijkst zichtbaar
        // bij spherische objecten en bij een window van kleine hoogte: vervang PP1 met 
        // sphere1.x, maak de window klein in de hoogte, ga naar RT config en verplaats de sphere naar de randen van de
        // window: het wordt een ei...
        // Helaas is hBscWnd een window van kleine hoogte: in de midden niks aan de hand,
        // aan de randen vervorming.
        // We kunnen het dus wel vergeten om een 3D sphere vanuit de rand naar het midden
        // te verplaatsen: dat moet gewoon in 2D...

                
}



//-----------------------------------------------------------------------------
// Name: DrawLogo()
// Desc: Sluggish GDI
//-----------------------------------------------------------------------------
void DrawLogo()
{
        // GDI blitting is too slow...
        // only do this once ////////////////////////////////////////////////
        // NO: don't blit into desktop as that will f*ck up the screen if
        // scr is watched in Display Properties
        // So probably cannot do trace...
        // blit a black background otherwise trace won't work
        // GDI stuff so must do this outside scene
        DDSURFACEDESC2 ddsd;
        ddsd.dwSize = sizeof(ddsd);
        ddsd.dwFlags = DDSD_HEIGHT | DDSD_WIDTH;
        g_pFramework->GetRenderSurface()->GetSurfaceDesc(&ddsd);
        
        dwRenderWidth = ddsd.dwWidth;
        dwRenderHeight = ddsd.dwHeight;

        HDC hdcDest;
        g_pFramework->GetRenderSurface()->GetDC(&hdcDest);
        //hDC = GetDC(g_hWnd);

        HDC hdcBuffer = CreateCompatibleDC(hdcDest);
        HBITMAP hbmBack1 = LoadBitmap( g_hInst, MAKEINTRESOURCE(IDB_BITMAP3));
        SelectObject(hdcBuffer, hbmBack1);

        //static bool firsttime = true;
        //if (firsttime) {
        //      firsttime = false;
        BitBlt(hdcDest, (dwRenderWidth-256)/2, (dwRenderHeight-256)/2, 256, 256, hdcBuffer, 0, 0, SRCCOPY );
        //}

        // and release
        g_pFramework->GetRenderSurface()->ReleaseDC(hdcDest); // release!!!
        //ReleaseDC(g_hWnd, hdcDest);

        // Clean up
        DeleteDC( hdcBuffer );

        // laat het geheugen niet vollopen...
        DeleteObject(hbmBack1);
        /////////////////////////////////////////////////////////////////////

}




//-----------------------------------------------------------------------------
// Name: BltAlphaFactor()
// Desc: Draws a textured square which can be used for fade-in/fade out.
// Note: We must be in a scene.
//-----------------------------------------------------------------------------
void BltAlphaFactor( LPDIRECT3DDEVICE7 pdev, LPDIRECTDRAWSURFACE7 pTex, 
                                        WORD x, WORD y, WORD x2, WORD y2, BYTE factor, bool clear )
{
        D3DTLVERTEX square1[4];

        // we zetten de square zover mogelijk naar achter zodat PP1 ervoor blijft
        // NOTE: The largest allowable value for dvSZ is 0.99999 if you want the vertex to be 
        // within the range of z-values that are displayed:
        // Z-values are in device space rather than camera space
        square1[0]=D3DTLVERTEX( D3DVECTOR(x,y,  0.99999f) ,1.0,RGBA_MAKE(255,255,255,factor),0,0,0 );
        square1[1]=D3DTLVERTEX( D3DVECTOR(x2,y, 0.99999f) ,1.0,RGBA_MAKE(255,255,255,factor),0,1,0 );
        square1[2]=D3DTLVERTEX( D3DVECTOR(x,y2, 0.99999f) ,1.0,RGBA_MAKE(255,255,255,factor),0,0,1 );
        square1[3]=D3DTLVERTEX( D3DVECTOR(x2,y2,0.99999f) ,1.0,RGBA_MAKE(255,255,255,factor),0,1,1 );

        // we zetten de square zover mogelijk naar achter zodat PP1 ervoor blijft
        // NOTE: The largest allowable value for dvSZ is 0.99999 if you want the vertex to be 
        // within the range of z-values that are displayed:
        // Z-values are in device space rather than camera space
        //square1[0]=D3DTLVERTEX( D3DVECTOR(x,y,  0.99999f), 1.0,RGBA_MAKE(0,0,0,factor),0,0,0);
        //square1[1]=D3DTLVERTEX( D3DVECTOR(x2,y, 0.99999f), 1.0,RGBA_MAKE(0,0,0,factor),0,1,0);
        //square1[2]=D3DTLVERTEX( D3DVECTOR(x,y2, 0.99999f), 1.0,RGBA_MAKE(0,0,0,factor),0,0,1);
        //square1[3]=D3DTLVERTEX( D3DVECTOR(x2,y2,0.99999f), 1.0,RGBA_MAKE(0,0,0,factor),0,1,1);

        if (clear==true) pdev->Clear(0,NULL,D3DCLEAR_TARGET,0,0.0f,0);

        pdev->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,true);
        pdev->SetRenderState(D3DRENDERSTATE_SRCBLEND,D3DBLEND_SRCALPHA);
    pdev->SetRenderState(D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA);
        pdev->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_MODULATE);
        
        pdev->SetTexture(0,pTex);       
        

        //if( SUCCEEDED( pdev->BeginScene() ) ) {       
                
                pdev->DrawPrimitive(D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX,square1,4,0);
                pdev->SetTexture(0,NULL);       
                pdev->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE,false);
                
        //      pdev->EndScene();
        //}
}




//-----------------------------------------------------------------------------
// Name: DrawLogo1()
// Desc: Draw BSC logo using D3D blitting
// Note: Must be done in scene
//-----------------------------------------------------------------------------
void DrawLogo1( LPDIRECT3DDEVICE7 pd3dDevice)
{
        
        // alpha blending
        if (fadeIn1) {          
                //count1++;
                //if ( count1%2==0 )
                        iAlpha1++;
        }

        if (iAlpha1 > 255) {
                iAlpha1 = 255;
                fadeIn1 = false;
                fadeIn2 = true;
        }

        if (fadeIn2) {          
                //count2++;
                //if ( count2%2==0 )
                        iAlpha2+=3;
        }

        if (iAlpha2 > 255) {
                iAlpha2 = 255;
                fadeIn2 = false;
                bStartTumble = true;
        }

        //static int i = 0;
        //if (!fadeIn1 && !fadeIn2)
        //      i++;
        //if (i > 30) tumblePP1 = true;


        //static int i = 0;
        //static bool up = true;
        //if (up)       i+=7;
        //else i-=1;
        //if(i>50) up=false;
        //if(i<0) up=true;

        // sphere
        int x = 580;
        int y = 200;
        int i = -20;

        if ( g_CurrentOptions.bUse640x480Mode ) {
                x = 360;
                y = 120;
                i = -100;
        }

        // Do D3D blit
        if (!g_bShowTrace) {
                BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE0], 
                                        x, y, x+256+i, y+256+i, iAlpha1, false );
        } else {
                if (!bStartTumble)
                        BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE0], 
                                        x, y, x+256+i, y+256+i, iAlpha1, false );
        }

        
        // text
        x = 150;
        y = 450;
        i = 315;

        if ( g_CurrentOptions.bUse640x480Mode ) {
                x = 60;
                y = 275;
                i = 110;
        }

        if (!g_bShowTrace) {
                BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE1], 
                                        x, y, x+256+i, y+256+i, iAlpha2, false );
        } else {
                if (!bStartTumble)
                        BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE1], 
                                                x, y, x+256+i, y+256+i, iAlpha2, false );
        }

}



//-----------------------------------------------------------------------------
// Name: DrawLogo2()
// Desc: Draw BSC logo using D3D blitting
// Note: Must be done in scene
//-----------------------------------------------------------------------------
void DrawLogo2( LPDIRECT3DDEVICE7 pd3dDevice)
{
        
        // alpha blending
        if (fadeIn1) {          
                count1++;
                if ( count1%3==0 )
                        iAlpha1++;
        }

        if (iAlpha1 > 255) {
                iAlpha1 = 255;
                fadeIn1 = false;
                fadeIn2 = true;
        }

        if (fadeIn2) {          
                //count2++;
                //if ( count2%2==0 )
                        iAlpha2+=2;
        }

        if (iAlpha2 > 255) {
                iAlpha2 = 255;
                fadeIn2 = false;
                bStartTumble = true;
        }

        
        static int j = 0;
        static bool up = true;

        // pulsate
        if (fadeIn1) {
                if (up) j+=7;
                else j-=1;
                if(j>50) up=false;
                if(j<0) up=true;
        } else {
                j = 0;
        }

        if (g_bSound) {
                if (j>54 && up==false)          
                        PlaySound( "p_bang.wav", NULL, SND_FILENAME|SND_ASYNC );
        }

        // sphere
        int x = 400;
        int y = 210;
        int i = 0;

        if ( g_CurrentOptions.bUse640x480Mode ) {
                x = 215;
                y = 120;
                i -= 80;
        }

        // Do D3D blit                  
        if (!g_bShowTrace) {
                BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE0], 
                                        x-j, y-j, (x+256+i)+j, (y+256+i)+j, iAlpha1, false );
        } else {
                if (!bStartTumble)
                        BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE0], 
                                        x-j, y-j, (x+256+i)+j, (y+256+i)+j, iAlpha1, false );
        }


        // text
        x = 245;
        y = 490;
        i = 350;

        if ( g_CurrentOptions.bUse640x480Mode ) {
                x = 110;
                y = 300;
                i = 180;
        }
        
        // Do D3D blit
        if (!g_bShowTrace) {
                BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE1], 
                                        x, y, x+256+i, y+256+i, iAlpha2, false );
        } else {
                if (!bStartTumble)
                        BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE1], 
                                        x, y, x+256+i, y+256+i, iAlpha2, false );
        }

}



//-----------------------------------------------------------------------------
// Name: DrawLogo3()
// Desc: Draw BSC logo using D3D blitting
// Note: Must be done in scene
//-----------------------------------------------------------------------------
void DrawLogo3( LPDIRECT3DDEVICE7 pd3dDevice)
{

        count1++;

        // alpha blending
        if (fadeIn1) {          
                if ( count1%2==0 )
                        iAlpha1++;
        }

        if (iAlpha1 > 255) {
                iAlpha1 = 255;
                fadeIn1 = false;
                fadeIn2 = true;
        }

        if (fadeIn2) {          
                //count2++;
                //if ( count2%2==0 )
                        iAlpha2+=2;
        }

        if (iAlpha2 > 255) {
                iAlpha2 = 255;
                fadeIn2 = false;
                bStartTumble = true;
        }


        //iAlpha1 = 255;
        static int j = -100;
        static bool up = true;

        // sphere
        static int x1 = 650; //130;
        static int y1 = 0; //140;
        int i = -80;

        //if ( g_CurrentOptions.bUse640x480Mode ) {
        //      x = 130;
        //      y = 140;
        //      i -= 80;
        //}

        // swoosh in
        if (up) {
                if ( count1%5==0 ) 
                        j+=1;
                //if ( count1%4==0 ) 
                        x1-=1;
                if ( count1%4==0 ) 
                        y1+=1;
        }
        if (x1<130) up=false;

        
        // Do D3D blit                  
        if (!g_bShowTrace) {
                BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE0], 
                                        x1-j, y1-j, (x1+256+i)+j, (y1+256+i)+j, iAlpha1, false );
        } else {
                if (!bStartTumble)
                        BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE0], 
                                        x1-j, y1-j, (x1+256+i)+j, (y1+256+i)+j, iAlpha1, false );
        }




        // text
        int x = 250;
        int y = 300;
        i = 110;

        //if ( g_CurrentOptions.bUse640x480Mode ) {
        //      x = 250;
        //      y = 300;
        //      i = 110;
        //}

        if (!g_bShowTrace) {
                BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE1], 
                                        x, y, x+256+i, y+256+i, iAlpha2, false );
        } else {
                if (!bStartTumble)
                        BltAlphaFactor( pd3dDevice, g_ppTex[TEX_SPHERE1], 
                                                x, y, x+256+i, y+256+i, iAlpha2, false );
        }


}


//-----------------------------------------------------------------------------
// Name: DrawLogo4()
// Desc: Draw BSC logo using D3D blitting
// Note: Must be done in scene
//-----------------------------------------------------------------------------
void DrawLogo4( LPDIRECT3DDEVICE7 pd3dDevice)
{
        static int count = 0;
        count++;
        if (count>200)
                bStartTumble = true;
        
        float fRotation = 1.0f;
        
        fRadsX = 0.02f*fRotation;
        //fRadsY = -0.05f*fRotation;
        //fRadsZ = -0.2f*fRotation;

        
        // ook hier: gimbal lock!!!
        // Quaternions ///////////
        //D3DXMATRIX matTrans;
        //D3DXMatrixTranslation( &matTrans, fX, fY, fZ );
        //D3DMath_MatrixMultiply(matBsc, matTrans, matBsc);     // order is crucial!!!
        
        //D3DXMATRIX matRot;
        //D3DXQUATERNION qRot;
        //D3DXQuaternionRotationYawPitchRoll( &qRot, fRadsY, fRadsX, fRadsZ );  
        //D3DXMatrixRotationQuaternion( &matRot, &qRot );
        
        D3DXMatrixRotationYawPitchRoll( &matRot, fRadsY, fRadsX, fRadsZ );
        D3DMath_MatrixMultiply(g_matSphereMatrix, matRot, g_matSphereMatrix); // order is crucial!!!

        
        // Order!!!
        //D3DMath_MatrixMultiply( matBsc, matTrans, matBsc );
        //D3DMath_MatrixMultiply( matBsc, matRot, matBsc );
        //D3DMath_MatrixMultiply( matAll, matRot, matBsc );
        //D3DMath_MatrixMultiply( matBsc, matAll, matBsc );


        // reset!!!
        fX=fY=fZ=0.0f;
        fRadsX=fRadsY=fRadsZ=0.0f;
}