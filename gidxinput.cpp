#include <gidxinput.h>

using namespace gdx;

CGidxInput::CGidxInput()
{
	lpDI		   = NULL;           
	lpDIKeyboard   = NULL;
	lpDIMaus	   = NULL;
	lpDI		   = NULL;
	xoffset		   = 0;
	yoffset		   = 0;
	m_mousex       = 0;
	m_mousey       = 0;
	m_key          = 0;
	mousebutton[0] = false;
	mousebutton[1] = false;
	m_bPressedKey  = false;
	m_hwnd		   = NULL;
	m_hinst		   = NULL;
}

CGidxInput::~CGidxInput()
{
	ShutDown();
}

void
CGidxInput::ShutDown()
{
	if (lpDI != NULL)
	{		
			if(lpDIKeyboard != NULL)
			{
				lpDIKeyboard->Unacquire();
				lpDIKeyboard->Release();
				lpDIKeyboard=NULL;
			}
			if(lpDIMaus != NULL)
			{
				lpDIMaus->Unacquire();
				lpDIMaus->Release();
				lpDIMaus=NULL;
			}
	lpDI->Release();
	lpDI = NULL;
	}
}

void 
CGidxInput::SetScreenXY(int x, int y)
{
	xoffset=x;
	yoffset=y;
}

void 
CGidxInput::Init(HINSTANCE hinst, HWND hwnd)
{
	m_hinst    = hinst;
	m_hwnd     = hwnd;
	m_Pos      = (POINT*)malloc(sizeof(POINT));
}

int 
CGidxInput::InitInput(int mode)
{
	ShutDown();

	DIPROPDWORD	dipdw;

	//KEYBOARD INITIALISIEREN
	dirval = DirectInputCreateEx(m_hinst, DIRECTINPUT_VERSION,IID_IDirectInput7,(LPVOID*)&lpDI,NULL );
	if(dirval != DI_OK)
		return 10;
	
	dirval = lpDI->CreateDeviceEx(GUID_SysKeyboard,IID_IDirectInputDevice7,(LPVOID*)&lpDIKeyboard,NULL );
	if(dirval != DI_OK)
		return 20;

	dirval = lpDIKeyboard->SetDataFormat( &c_dfDIKeyboard );  
	if(dirval != DI_OK)
		return 30;

	dirval = lpDIKeyboard->SetCooperativeLevel( m_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY); 
	if(dirval != DI_OK)
		return 201;

	lpDIKeyboard->Acquire();
	
	if(mode==0)
	{
		//MAUS INITIALISIEREN
		dirval = lpDI->CreateDeviceEx(GUID_SysMouse,IID_IDirectInputDevice7,(LPVOID*)&lpDIMaus,NULL );
		if(dirval != DI_OK)
			return 4;

		dirval = lpDIMaus->SetDataFormat( &c_dfDIMouse );  
		if(dirval != DI_OK)
			return 5;

		dirval = lpDIMaus->SetCooperativeLevel( m_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE );
		if(dirval != DI_OK)
			return 6;

		dipdw.diph.dwSize         = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize   = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj          = 0;
		dipdw.diph.dwHow          = DIPH_DEVICE;
		dipdw.dwData              = 10;
		lpDIMaus->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
		lpDIMaus->Acquire();

		GetMouseX  = &CGidxInput::GetMouseXFull;
		GetMouseY  = &CGidxInput::GetMouseYFull;
		Statmouse  = &CGidxInput::StatmouseFull;
	}
	else if(mode==1)
	{
		//MAUS INITIALISIEREN
		dirval = lpDI->CreateDeviceEx(GUID_SysMouse,IID_IDirectInputDevice7,(LPVOID*)&lpDIMaus,NULL );
		if(dirval != DI_OK)
			return 4;

		dirval = lpDIMaus->SetDataFormat( &c_dfDIMouse );  
		if(dirval != DI_OK)
			return 5;

		dirval = lpDIMaus->SetCooperativeLevel( m_hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE );
		if(dirval != DI_OK)
			return 6;

		dipdw.diph.dwSize         = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize   = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj          = 0;
		dipdw.diph.dwHow          = DIPH_DEVICE;
		dipdw.dwData              = 10;
		lpDIMaus->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);
		lpDIMaus->Acquire();

		SetCursorPos(GetSystemMetrics(SM_CXSCREEN)/2,GetSystemMetrics(SM_CYSCREEN)/2);     
		GetMouseX  = &CGidxInput::GetMouseXWin;
		GetMouseY  = &CGidxInput::GetMouseYWin;
		Statmouse  = &CGidxInput::StatmouseWin;
	}

	return 0;
}

bool 
CGidxInput::Statkey()
{
   	if(DI_OK != lpDIKeyboard->GetDeviceState(256, &buffer))
	{
			dirval = lpDIKeyboard->Acquire();
			if(dirval != DI_OK)
				return false;
	}
	return true;
}

bool
CGidxInput::WaitKey()
{
	if(!this->Statkey())
		return false;

	for(int i=2; i<237; i++)
	{
		if(buffer[i]&0x80 && m_bPressedKey == false)
		{
			m_bPressedKey = true;
			m_key         = i;
			return false;
		}
	}

	if(!(buffer[m_key]&0x80))
	{
		m_bPressedKey = false;
		m_key         = 0;
	}

	return true;
}

bool
CGidxInput::KeyHit(int key)
{
	if((buffer[key]&0x80) && m_bPressedKey == false)
	{
		m_bPressedKey = true;
		m_key         = key;
		return true;
	}
	if(!(buffer[m_key]&0x80))
	{
		m_bPressedKey = false;
	}

	return false;
}

bool
CGidxInput::KeyDown(int key)
{
	if((buffer[key]&0x80))
		return true;

	return false;
}

bool 
CGidxInput::StatmouseFull()
{
	DIDEVICEOBJECTDATA diod;
	memset(&diod,0,sizeof(DIDEVICEOBJECTDATA));
	DWORD dwNum=1;

	bool running=false;
	while(!running)
	{
		if(lpDIMaus->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &diod, &dwNum, 0)!=DI_OK)
		{
			if(lpDIMaus->Acquire()==DI_OK)
				lpDIMaus->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),&diod, &dwNum,0);
			 // Wenn hier auch ein Fehler kommt kann mich die Anwendung am Arsch lecken
		}
		switch(diod.dwOfs)
			{
			case DIMOFS_X:
				{
					m_mousex += diod.dwData;
				}break;
			case DIMOFS_Y:
				{
					m_mousey += diod.dwData;
				}break;
			case DIMOFS_BUTTON0:
				{
					if(diod.dwData & 0x80)
					{
						mousebutton[0]=true;
					}
					else
					{
						mousebutton[0]=false;
					}
				}break;
			case DIMOFS_BUTTON1:
				{
					if(diod.dwData & 0x80)
					{
						mousebutton[1]=true;
					}
					else
					{
						mousebutton[1]=false;
					}
				}break;
			}
		if(dwNum == 0)
			{	
				running = true;
				break;
			}
		}

	if (m_mousex<0)m_mousex=0;
	if (m_mousex>xoffset) m_mousex=xoffset;
	if (m_mousey<0)m_mousey=0;
	if (m_mousey>yoffset) m_mousey=yoffset;
return true;
}

bool 
CGidxInput::StatmouseWin()
{ 
	DIDEVICEOBJECTDATA diod;
	memset(&diod,0,sizeof(DIDEVICEOBJECTDATA));
	DWORD dwNum=1;

	bool running=false;
	while(!running)
	{
		if(lpDIMaus->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &diod, &dwNum, 0)!=DI_OK)
		{
			if(lpDIMaus->Acquire()==DI_OK)
				lpDIMaus->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),&diod, &dwNum,0);
			 // Wenn hier auch ein Fehler kommt kann mich die Anwendung am Arsch lecken
		}
		switch(diod.dwOfs)
		{
			case DIMOFS_BUTTON0:
			{
				if(diod.dwData & 0x80)
				{
					mousebutton[0]=true;
				}
				else
				{
					mousebutton[0]=false;
				}
			}break;
			case DIMOFS_BUTTON1:
			{
				if(diod.dwData & 0x80)
				{
					mousebutton[1]=true;
				}
				else
				{
					mousebutton[1]=false;
				}
			}break;
		}
		if(dwNum == 0)
		{	
			running = true;
			break;
		}
	}

	GetCursorPos(m_Pos);
	ScreenToClient(m_hwnd,m_Pos);
	return 0;
}

int
CGidxInput::GetMouseXWin(void)
{
	return m_Pos->x;
}

int
CGidxInput::GetMouseYWin(void)
{
	return m_Pos->y;
}

int
CGidxInput::GetMouseXFull(void)
{
	return this->m_mousex;
}

int
CGidxInput::GetMouseYFull(void)
{
	return this->m_mousey;
}

bool
CGidxInput::MouseDown1(void)
{
	return this->mousebutton[1];
}

bool
CGidxInput::MouseDown0(void)
{
	return this->mousebutton[0];
}