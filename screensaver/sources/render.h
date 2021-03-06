//-----------------------------------------------------------------------------
// File: Render.h
//
// Desc: Frame move and render
//
// Note: This code uses the D3D Framework helper library.
//
//
// Copyright (c) 2001 Black Sphere Corp.
//-----------------------------------------------------------------------------

#include <math.h>
#include <time.h>
#include <d3dx.h>
#include <d3dxmath.h>

#include "D3DFrame.h"
#include "D3DTextr.h"
#include "D3DUtil.h"
#include "D3DFile.h"
#include "D3DMath.h"


#include "screensaver.h"

#include "resource.h"



//-----------------------------------------------------------------------------
// Declare the application globals for use in WinMain.cpp
//-----------------------------------------------------------------------------
extern TCHAR* g_strAppTitle;
extern BOOL   g_bAppUseZBuffer;    // Create/use a z-buffer




//-----------------------------------------------------------------------------
// Defines, constants, and global variables
//-----------------------------------------------------------------------------
extern CD3DFile* g_pPP1Object;
extern CD3DFile* g_pPP2Object;
extern CD3DFile* g_pPP3Object;
extern CD3DFile* g_pPP4Object;

extern D3DMATRIX g_matPP1Matrix;
extern D3DMATRIX g_matPP2Matrix;
extern D3DMATRIX g_matPP3Matrix;
extern D3DMATRIX g_matPP4Matrix;

extern CD3DFile* g_pSphereObject;
extern D3DMATRIX g_matSphereMatrix;



extern HRESULT hr;
//extern DWORD dwCoopFlags;

//extern RECT rc;

extern DWORD dwRenderWidth;
extern DWORD dwRenderHeight;
extern D3DVIEWPORT7 vp;

//extern DDSURFACEDESC2 ddsd;
//extern LPDIRECTDRAWSURFACE7 pddsCoverSurface;

//extern LPDIRECTDRAWCLIPPER pcClipper;
//extern D3DMATRIX matBsc;

// extern D3DMATRIX matView;
// extern D3DVECTOR vEyePt;
// extern D3DVECTOR vLookatPt;
// extern D3DVECTOR vUpVec;

extern D3DXMATRIX matAll;
extern D3DXMATRIX matTrans;
extern D3DXMATRIX matScale;
extern D3DXMATRIX matRot;
extern D3DXQUATERNION qRot;

extern float fX;
extern float fY;
extern float fZ;
extern float fRadsX;
extern float fRadsY;
extern float fRadsZ;

extern bool moveXUpPP1;
extern bool moveYUpPP1;
extern bool moveZUpPP1;

extern bool moveXUpPP2;
extern bool moveYUpPP2;
extern bool moveZUpPP2;

extern bool moveXUpPP3;
extern bool moveYUpPP3;
extern bool moveZUpPP3;

extern bool moveXUpPP4;
extern bool moveYUpPP4;
extern bool moveZUpPP4;

//extern bool bLButtonDown;
//extern bool bRButtonDown;

extern bool bHitRightPP1;
extern bool bHitRightPP2;
extern bool bHitRightPP3;
extern bool bHitRightPP4;

// alpha-blending
extern int iAlpha1;
extern int iAlpha2;
extern bool fadeIn1;
extern bool fadeIn2;

extern int count1;
extern int count2;


extern bool bStartTumble;

extern int i;
extern bool tumblePP1;
extern bool tumblePP2;
extern bool tumblePP3;
extern bool tumblePP4;


extern float fAspect;
extern D3DMATRIX matProj;

//extern bool bRenderTargetTooSmall;


// options
// TODO: put these in ScreenSaverOptions struct
extern float g_fSpeed;
extern float g_fRotation;

// NOTE: using BOOL (i.e. int) instead of bool will prevent C4800 warnings when
// assigning it a DWORD reg value
extern BOOL g_bSound;
extern BOOL g_bShowTrace;

extern float g_fXLimit;
extern float g_fYLimit;
extern float g_fZLimit;


extern float fRandomPP1DeltaX;
extern float fRandomPP1DeltaY;
extern float fRandomPP2DeltaX;
extern float fRandomPP2DeltaY;
extern float fRandomPP3DeltaX;
extern float fRandomPP3DeltaY;
extern float fRandomPP4DeltaX;
extern float fRandomPP4DeltaY;

extern int iRandomPP1X;
extern int iRandomPP1Y;
extern int iRandomPP2X;
extern int iRandomPP2Y;
extern int iRandomPP3X;
extern int iRandomPP3Y;
extern int iRandomPP4X;
extern int iRandomPP4Y;





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
enum {
    TEX_SPHERE0, TEX_SPHERE1, TEX_SPHERE2, TEX_SPHERE3,
    NUM_TEXTURES
};

extern char* g_szTexName[NUM_TEXTURES];

extern LPDIRECTDRAWSURFACE7 g_ppTex[NUM_TEXTURES];








//-----------------------------------------------------------------------------
// Function prototypes and global (or static) variables
//-----------------------------------------------------------------------------
VOID    AppPause( BOOL );
VOID    RotateVertexInX( FLOAT, DWORD, D3DVERTEX*, D3DVERTEX* );
BOOL    GenerateSphere( FLOAT, DWORD, DWORD, FLOAT, FLOAT, FLOAT, D3DVERTEX**,
                        DWORD*, WORD**, DWORD* );
VOID    BlendObjects( DWORD, D3DVERTEX*, D3DVERTEX*, D3DVERTEX* );
