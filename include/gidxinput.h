/**********************************************************
	gidxinput.h - Header-Datei
-----------------------------------------------------------
	begin				: 9.10.2004
	by					: Halit Ogul
	e-mail				: halit.ogul@icloud.com

	last changed		: 25.12.2019
	by					: Halit Ogul
	e-mail				: halit.ogul@icloud.com

	copyright			:(c)2004 Halit Ogul

**********************************************************/

#ifndef GIDXINPUT_H
#define GIDXINPUT_H

#define DIRECTINPUT_VERSION  0x0700

#pragma comment(lib,"dinput.lib")

#include <dinput.h>

namespace gdx
{
	class CGidxInput
	{
	private:
		LPDIRECTINPUT7       lpDI;           
		LPDIRECTINPUTDEVICE7 lpDIKeyboard;
		LPDIRECTINPUTDEVICE7 lpDIMaus;	
		HRESULT				 dirval;
		int					 xoffset;
		int				     yoffset;
		bool				 m_bPressedKey;
		int					 m_key;
		int					 m_mousex;
		int					 m_mousey;
		bool				 mousebutton[2];
		char				 buffer[256];
		HWND				 m_hwnd;
		HINSTANCE			 m_hinst;
		POINT				*m_Pos;

	public:

		CGidxInput();
		virtual ~CGidxInput();

		void ShutDown();

		void SetScreenXY(int,int);
		void Init(HINSTANCE,HWND);
		int  InitInput(int);
		bool KeyHit(int);
		bool KeyDown(int);
		bool Statkey(void);
		bool WaitKey(void);
		bool MouseDown1(void);
		bool MouseDown0(void);

		bool StatmouseWin(void);
		bool StatmouseFull(void);
		int  GetMouseXWin(void);
		int  GetMouseYWin(void);
		int  GetMouseXFull(void);
		int  GetMouseYFull(void);

		bool(CGidxInput::*Statmouse)(void);
		int(CGidxInput::*GetMouseX)(void);
		int(CGidxInput::*GetMouseY)(void);
	};
	typedef CGidxInput*	LPGIDXINPUT;
}

extern gdx::LPGIDXINPUT	pInput;

#endif//GIDXINPUT_H
