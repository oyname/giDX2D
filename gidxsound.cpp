#include <gidxsound.h>

using namespace gdx;

CGidxSound::CGidxSound()
{
	lpDS   = NULL;
	m_hwnd = NULL;
	m_snd  = NULL;
}

CGidxSound::~CGidxSound()
{
	SND *temp = m_snd;

	if(temp!=NULL)
	{
 		while(temp->next!=NULL)
		{
 			temp = temp->next;

			if(temp->prev!=NULL)
			{
				if(temp->prev->snd)
				{
					temp->prev->snd->Release();
					temp->prev->snd=NULL;
				}
				delete temp->prev;
			}
			temp->prev = NULL;
		}

		if(temp!=NULL)
		{
			if(temp->snd)
			{
				temp->snd->Release();
				temp->snd=NULL;
			}
			delete temp->prev;
		}
	}

	if(lpDS!=NULL)
	{
		lpDS->Release();
		lpDS=NULL;
	}
}

void
CGidxSound::Init(HWND hwnd)
{
	m_hwnd = hwnd;
}

int 
CGidxSound::InitSound() 
{
	HRESULT	dsrval;

	dsrval = DirectSoundCreate(NULL, &lpDS, NULL);
	if(FAILED(dsrval))
		return 1;

	dsrval = lpDS->SetCooperativeLevel(m_hwnd, DSSCL_NORMAL);
	if(FAILED(dsrval))
		return 2;
	
	return 0;
}

int 
CGidxSound::LoadSnd(const wchar_t* filename, LPLPSOUND lplpSND)
{
   this->AddList(lplpSND);

   DSBUFFERDESC         dsbd;          
   BYTE                 *pDSBuffData;   
   WAVEFORMATEX         waveFormat;    
   DWORD                dwDataLength;  
   PBYTE                pbyWaveDaten;  
   HRESULT              dsrval;        

   pbyWaveDaten = NULL;

   if(0!=WaveLoad(filename, &waveFormat, &dwDataLength,&pbyWaveDaten))
	   return 1;

   ZeroMemory(&dsbd,sizeof(DSBUFFERDESC));
   dsbd.dwSize = sizeof(DSBUFFERDESC);
   dsbd.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME |
                  DSBCAPS_CTRLFREQUENCY | DSBCAPS_STATIC;
   dsbd.dwBufferBytes = dwDataLength;
   dsbd.lpwfxFormat = &waveFormat;

   dsrval = lpDS->CreateSoundBuffer(&dsbd, &(*lplpSND)->snd, NULL);
	if (FAILED(dsrval))
		return 2;

   dsrval = (*lplpSND)->snd->Lock(0,dwDataLength,(LPVOID *)&pDSBuffData,
                           &dwDataLength,NULL,0,0);
   if (FAILED(dsrval))
		return 3;

   memcpy(pDSBuffData,pbyWaveDaten,dwDataLength);
   free(pbyWaveDaten);

   dsrval = (*lplpSND)->snd->Unlock(pDSBuffData,dwDataLength,NULL,0);
   if (FAILED(dsrval))
		return 4;

   return 0;
}
void
CGidxSound::PlaySnd(LPDIRECTSOUNDBUFFER lpDSB, long pan, long vol, long loop) 
{
	if(pan<-10000 && pan> 10000) pan=0;		
	if(lpDSB)
	{							
		lpDSB->SetPan(pan);
		lpDSB->SetVolume(vol);
		lpDSB->Play(0,0,loop);
	}
}

void 
CGidxSound::StopSnd(LPDIRECTSOUNDBUFFER lpDSB, bool reset)
{
	if(reset)
		lpDSB->SetCurrentPosition(0);
	lpDSB->Stop();
}

void 
CGidxSound::Balance(LPDIRECTSOUNDBUFFER lpDSB, long pan)
{
	if(pan<-10000 && pan> 10000) pan=0;
	lpDSB->SetPan(pan);
}

void
CGidxSound::Frequency(LPDIRECTSOUNDBUFFER lpDSB, long freq)
{
	lpDSB->SetFrequency(freq);
}

int
CGidxSound::WaveDaten(HMMIO hmmio, DWORD *pdwLaenge, PBYTE* ppbyWDat)
{
   MMCKINFO mmWave;
   MMCKINFO mmData;
   MMRESULT mmResult;
   ZeroMemory(&mmWave, sizeof(mmWave));
   ZeroMemory(&mmData, sizeof(mmData));
   mmResult = mmioSeek(hmmio, 0, SEEK_SET);
   mmWave.fccType = mmioStringToFOURCC(L"WAVE", 0);
   mmData.ckid    = mmioStringToFOURCC(L"data", 0);
   mmResult = mmioDescend(hmmio,&mmWave,NULL,MMIO_FINDRIFF);
   if(FAILED(mmResult))
		return 1;

   mmResult = mmioDescend(hmmio,&mmData,&mmWave,MMIO_FINDCHUNK);
   if(FAILED(mmResult))
		return 2;

   *pdwLaenge = mmData.cksize;
   *ppbyWDat = (PBYTE)malloc(*pdwLaenge);
   if(!*ppbyWDat)
		return 3;

   ZeroMemory(*ppbyWDat,*pdwLaenge);
   mmResult = mmioRead(hmmio,(HPSTR)*ppbyWDat,*pdwLaenge);
   if(FAILED(mmResult))
		return 4;

   return 0;
}

int 
CGidxSound::WaveFormat(HMMIO hmmio, WAVEFORMATEX *pWf) 
{
   MMCKINFO mmWave;
   MMCKINFO mmFmt;
   MMRESULT mmResult;
   ZeroMemory(&mmWave, sizeof(mmWave));
   ZeroMemory(&mmFmt, sizeof(mmFmt));
   mmResult = mmioSeek(hmmio, 0, SEEK_SET);
   mmWave.fccType = mmioStringToFOURCC(L"WAVE", 0);
   mmFmt.ckid = mmioStringToFOURCC(L"fmt", 0);
   mmResult = mmioDescend(hmmio,&mmWave,NULL,MMIO_FINDRIFF);
   if(FAILED(mmResult))
		return 1;

   mmResult = mmioDescend(hmmio,&mmFmt,&mmWave,MMIO_FINDCHUNK);
   if(FAILED(mmResult))
		return 2;

   mmResult = mmioRead(hmmio,(HPSTR)pWf,sizeof(WAVEFORMATEX));
   if(FAILED(mmResult))
		return 3;

   return 0;
} 

int 
CGidxSound::WaveLoad(const wchar_t* fileName, WAVEFORMATEX* pWf,DWORD* pdwLaenge,PBYTE* ppbyWDat) 
{
   HMMIO hmmio;
   int  result;
   hmmio = mmioOpen((wchar_t*)((LPCTSTR)fileName), NULL, MMIO_READ);
   if(!hmmio)
		return 1;

   result=WaveFormat(hmmio, pWf);
   if(result!=0)
		return 2;

   result=WaveDaten(hmmio, pdwLaenge, ppbyWDat);
   if(result!=0)
		return 3;

   mmioClose(hmmio, 0);

   return 0;
} 

void
CGidxSound::AddList(LPLPSND lplpsnd)
{
	SND	*temp = m_snd;

	if(m_snd==NULL)
	{
		(*lplpsnd)       = new SND;

		(*lplpsnd)->snd  = NULL;
		(*lplpsnd)->prev = NULL;
		(*lplpsnd)->next = NULL;

		m_snd = (*lplpsnd);
	}
	else
	{
		while(temp->next!=NULL)
			temp=temp->next;

		temp->next       = new SND;
		temp->next->snd  = NULL;
		temp->next->next = NULL;
		temp->next->prev = temp;

		(*lplpsnd) = temp->next;
	}

}

void
CGidxSound::FreeSound(LPSND lpsnd)
{
	if(lpsnd->prev==NULL)
	{
		m_snd = lpsnd->next;
				
		if(lpsnd->next!=NULL)
			lpsnd->next->prev=NULL;

		lpsnd->next=NULL;
		
		if(lpsnd!=NULL)
		{
			if(lpsnd->snd)
			{
				lpsnd->snd->Release();
				lpsnd->snd=NULL;
			}
			delete lpsnd;
		}
		
		lpsnd = NULL;
	}
	else if(lpsnd->next==NULL)
	{
		lpsnd->prev->next=NULL;
		
		if(lpsnd!=NULL)
		{
			if(lpsnd->snd)
			{
				lpsnd->snd->Release();
				lpsnd->snd=NULL;
			}
			delete lpsnd;
		}
		
		lpsnd = NULL;
	}
	else
	{
		lpsnd->next->prev = lpsnd->prev;
		lpsnd->prev->next = lpsnd->next;

		if(lpsnd!=NULL)
		{
			if(lpsnd->snd)
			{
				lpsnd->snd->Release();
				lpsnd->snd=NULL;
			}
			delete lpsnd;
		}
		
		lpsnd = NULL;
	}
}