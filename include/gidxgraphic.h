/**********************************************************
	gidxgraphic.h - Header-Datei
-----------------------------------------------------------
	begin				: 3.10.2004
	by					: Halit Ogul
	e-mail				: halit.ogul@icloud.com

	last changed		: 25.12.2019
	by					: Halit Ogul
	e-mail				: halit.ogul@icloud.com

	copyright			:(c)2004 Halit Ogul

**********************************************************/

#ifndef GIDXGRAPHIC_H
#define GIDXGRAPHIC_H

#define DIRECTDRAW_VERSION   0x0700

#define D3D_OVERLOADS

#pragma comment(lib,"dxguid.lib")
#pragma comment(lib,"ddraw.lib")
#pragma comment(lib,"d3dim.lib")

#include <ddraw.h>
#include <d3d.h>

const int IMG_NONE     = 0;
const int IMG_FROMFILE = 1;
const int IMG_FROMIMG  = 2;
const int IMG_FROMUSER = 3;

typedef struct IMG 
{
	// DirectDraw Oberflächen
	LPDIRECTDRAWSURFACE7		img;			     

	// Colorkey
	bool                        usecolorkey;	
	unsigned long				colorkey;       

	// Farbe
	unsigned long				r;             
	unsigned long               g;
	unsigned long               b;

	// Größe
	RECT						kollision;
	RECT						rect;
	RECT                        org;

	// Direct3D
	D3DTLVERTEX					square[4];

	// Texturkoordinaten
	bool						istextur;
	float						ftu;
	float						ftv;

	// Animation
	unsigned long               width;          
	unsigned long               height;
	unsigned long				frame;          
	unsigned long				reihe;			

	// Position
	unsigned long               x;				
	unsigned long               y;

	// Speicherverwaltung
	IMG							*prev;			
	IMG							*next;
	wchar_t						filename[256];
	ULONG_PTR                   resource;

	// Wie/Woher wurde das Bitmap in die Surface kopiert?
	unsigned char				type;
	unsigned char				memtype;
	IMG                         *source;  

	// Kollision
	bool						*bitfield;
	bool                        pixelgenau;

}IMG, *LPIMAGE, **LPLPIMAGE;

namespace gdx
{
	class CGidxGraphic
	{
	public:
		CGidxGraphic();
		~CGidxGraphic();

		void  Reset(void);
		void  ShutDown(void);
		void  Restore(void);
		bool  Check(void);
		bool  IsWindow(void);
		void  RestoreSurfaces(void);
		void  CloseWindow(void);

		void  Init(HWND,int);
		void  Release(void);
		int   Graphic(int,int,int,int);
		int   Cls(void);
		void  Print(int,int,const wchar_t*);
		void  SetFont(HFONT);
		void  FontColor(int,int,int);
		int   LoadImg(const wchar_t*,LPIMAGE,int);
		int   CreateImage(LPIMAGE, int,int,int);
		void  ColorKey(LPIMAGE img, unsigned int r, unsigned int g, unsigned b);
		int   DrawImage(int,int,LPIMAGE,int,unsigned char);
		int   DrawRect(int,int,LPIMAGE,int,int,int,int,int,unsigned char);
		int   CopyRect(int,int,int,int,int,int,LPIMAGE,LPIMAGE);
		void  GetBackBuffer(LPLPIMAGE);
		int   UnlockBuffer(LPIMAGE);
		ULONG Color(UINT,UINT,UINT);
		HFONT CreateFont(int,int,int,wchar_t*);
		HWND  GetHWND();
        int   ImageWidth(LPIMAGE);
		int   ImageHeight(LPIMAGE);

		void  GetMemory(LPLPIMAGE lplpimage);
		void  FreeImage(LPIMAGE lpimage);

		int   GetRed(ULONG color);
		int   GetGreen(ULONG color);
		int   GetBlue(ULONG color);

		int   GetX(void);
		int   GetY(void);
		int   GetBPP(void);
		int   GetModus(void);

		void  SetPixel16(int,int);
		ULONG GetPixel16(int,int);
		void  SetPixel32(int,int);
		ULONG GetPixel32(int,int);
		int   LockBuffer16(LPIMAGE);
		int   LockBuffer32(LPIMAGE);
		int   FlipFullscreen(ULONG);
		int	  FlipWindowed(ULONG);
		bool  CollisionRect(int,int,LPIMAGE,int,int,LPIMAGE); 
		bool  Collision16(int,int,LPIMAGE,unsigned int,
			              int,int,LPIMAGE,unsigned int);
		bool  Collision32(int,int,LPIMAGE,unsigned int,
						  int,int,LPIMAGE,unsigned int);

		ULONG(CGidxGraphic::*GetPixel)(int,int);
		void(CGidxGraphic::*SetPixel)(int,int);
		int (CGidxGraphic::*Flip)(ULONG);	
		bool(CGidxGraphic::*Collision)(int,int,LPIMAGE,unsigned int,
			                           int,int,LPIMAGE,unsigned int);
		int (CGidxGraphic::*LockBuffer)(LPIMAGE);

		int GetMode(LPDIRECTDRAWSURFACE7);

	private:
		int InitFullscreen();
		int InitWindowed();
		unsigned int BitCount(unsigned long);

		int							m_rbit,m_gbit,m_bbit;

		bool						m_bActive;
		bool                        m_bClose;
		bool						m_bInitialized;
		int							m_windowed;
		HWND						m_hwnd;
		LPDIRECTDRAW7				m_lpDD;
		LPDIRECTDRAWSURFACE7		m_lpDDSPrimary;
		LPDIRECTDRAWSURFACE7		m_lpDDSBack;
		LPDIRECT3D7                 m_lpDirect3D7;
	    LPDIRECT3DDEVICE7		    m_lpDirect3DDevice7;
		LPDIRECTDRAWCLIPPER         m_lpDClip;
		USHORT*						m_pVram16;
		ULONG*						m_pVram32;
		RECT						m_rect;
		LONG						m_iPitch;
		HFONT						m_font;
		int							m_bpp;
		int							m_xoffset;     
		int							m_yoffset;
		int							m_Format;
		int                         m_r;
		int                         m_g;
		int                         m_b;
		unsigned long				m_fontcolor;
		unsigned long				m_clscolor;
		unsigned long				m_color;
		unsigned long				m_MaskRed;
		unsigned long				m_MaskGreen;
		unsigned long				m_MaskBlue;

		RECT						sor_rect;
		LPIMAGE						m_img;

		bool						m_use3D;
	};
	typedef CGidxGraphic*	LPGIDXGRAPHIC;
}

extern gdx::LPGIDXGRAPHIC	pGraphic;

int BltAlpha( LPDIRECTDRAWSURFACE7 lpDDSDest, LPDIRECTDRAWSURFACE7 lpDDSSource, 
			 int iDestX, int iDestY, LPRECT lprcSource, int iAlpha, DWORD dwMode);

#endif//GIDXGRAPHIC_H
