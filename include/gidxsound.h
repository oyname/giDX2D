/**********************************************************
	gidxsound.h - Header-Datei
-----------------------------------------------------------
	begin				: 9.10.2004
	by					: Halit Ogul
	e-mail				: halit.ogul@icloud.com

	last changed		: 25.12.2019
	by					: Halit Ogul
	e-mail				: halit.ogul@icloud.com

	copyright			:(c)2004 Halit Ogul

**********************************************************/

#ifndef GIDXSOUND_H
#define GIDXSOUND_H

#define DIRECTSOUND_VERSION  0x0700

#pragma comment(lib,"dsound.lib")
#pragma comment(lib,"winmm.lib")

#include <dsound.h>
#include <windows.h>
#include <stdio.h>
#include <mmsystem.h>

typedef struct SOUND 
{
	LPDIRECTSOUNDBUFFER			snd;

	SOUND						*prev;
	SOUND						*next;
}SND, *LPSOUND, **LPLPSOUND, *LPSND, **LPLPSND;

namespace gdx
{
	class CGidxSound
	{
	private:
		LPDIRECTSOUND       lpDS;
		HWND				m_hwnd;
		LPSND				m_snd;

		int WaveLoad(const wchar_t*,WAVEFORMATEX*,DWORD*,PBYTE*);
		int WaveDaten(HMMIO, DWORD*, PBYTE*);
		int WaveFormat(HMMIO, WAVEFORMATEX*); 

		void AddList(LPLPSND lplpsnd);

	public:
		CGidxSound();
		~CGidxSound();
		void Init(HWND);
		int  InitSound();
		int  LoadSnd(const wchar_t*, LPLPSOUND);
		void PlaySnd(LPDIRECTSOUNDBUFFER,long,long,long);
		void StopSnd(LPDIRECTSOUNDBUFFER,bool);
		void Balance(LPDIRECTSOUNDBUFFER,long);
		void Frequency(LPDIRECTSOUNDBUFFER,long);

		void FreeSound(LPSND lpsnd);
	};
	typedef	CGidxSound*	LPGIDXSOUND;
}

extern gdx::LPGIDXSOUND	pSound;

#endif//GIDXSOUND_H
