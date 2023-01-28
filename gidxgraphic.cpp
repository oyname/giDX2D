#include <gidxgraphic.h>
#include <stdio.h>

using namespace gdx;

CGidxGraphic::CGidxGraphic()
{
	m_img				= NULL;
	m_hwnd				= NULL;
	m_lpDD				= NULL;
	m_lpDirect3D7		= NULL;
	m_lpDirect3DDevice7 = NULL;
	m_lpDDSPrimary		= NULL;
	m_lpDDSBack			= NULL;
	m_lpDClip			= NULL;
	m_clscolor			= 0;
	m_color				= 0;
	m_font				= 0;
	m_fontcolor			= 0;
	m_bInitialized		= false;
	m_bClose			= false;
	m_bActive			= false;
	m_use3D             = false;
}	

CGidxGraphic::~CGidxGraphic()
{
	this->Release();
	m_bInitialized=false;
}

void
CGidxGraphic::Release()
{
	// Alle IMGs freigeben
	IMG *temp = m_img;

	if(temp!=NULL)
	{   
 		while(temp->next!=NULL)
		{	
 			temp = temp->next;

			if(temp->prev!=NULL)
			{
				if(temp->prev->img && temp->prev->img != m_lpDDSBack)
				{
					if(temp->prev->bitfield!=0)
						delete[] temp->prev->bitfield;

					temp->prev->img->Release();
					temp->prev->img=NULL;
				}
				delete temp->prev;
			}
			temp->prev = NULL;
		}

		if(temp!=NULL)
		{
			if(temp->img)
			{
				if(temp->bitfield!=0)
					delete[] temp->bitfield;

				temp->img->Release();
				temp->img=NULL;
			}
			delete temp->prev;
		}
	}

	// Direct3DDevice7
	if (m_lpDirect3DDevice7)
	{
		m_lpDirect3DDevice7->Release();
		m_lpDirect3DDevice7	= NULL;
	}

	// Direct3D7-Objekt freigeben
	if (m_lpDirect3D7)
	{
		m_lpDirect3D7->Release();
		m_lpDirect3D7	= NULL;
	}

	//DirectDraw beenden
	if(m_lpDD != NULL)
    {
		if(m_lpDDSPrimary != NULL) 
		{ 
			m_lpDDSPrimary->Release();
			m_lpDDSPrimary = NULL;
			if(m_windowed!=0)
				if(m_lpDDSBack != NULL)
					m_lpDDSBack->Release();
			m_lpDDSBack    = NULL;
		} 
		m_lpDD->Release();
		m_lpDD = NULL;
    }

	//Clipper für Fensteranwendung
	if(m_lpDClip != NULL)
	{
		m_lpDClip->Release();
		m_lpDClip = NULL;
	}
}

unsigned int 
CGidxGraphic::BitCount(unsigned long x) 
{ 
	// Zählt die Anzahl der Bits die 1 sind.
    unsigned int n = 0; 
    while (x != 0) 
    { 
        x &= x - 1; 
        ++n; 
    } 
    return n; 
}

void
CGidxGraphic::ShutDown()
{
	IMG *temp = m_img;
    // Alle DirectDraw Surface freigeben...
 	while(temp!=NULL)
	{	
		if(temp->img!=NULL)
		{
			temp->img->Release();
			temp->img=NULL;
		}
 		temp = temp->next;
	}

	// ...Direct3DDevice7 freigeben...
	if (m_lpDirect3DDevice7)
	{
		m_lpDirect3DDevice7->Release();
		m_lpDirect3DDevice7	= NULL;
	}

	// ...Direct3D7-Objekt freigeben...
	if (m_lpDirect3D7)
	{
		m_lpDirect3D7->Release();
		m_lpDirect3D7	= NULL;
	}

	//...und DirectDraw beenden
	if(m_lpDDSPrimary != NULL) 
	{ 
		m_lpDDSPrimary->Release();
		m_lpDDSPrimary = NULL;
		if(m_windowed!=0)
			m_lpDDSBack->Release();
		m_lpDDSBack    = NULL;
	}

	if(m_lpDD != NULL)
    {	
		m_lpDD->Release();
		m_lpDD = NULL;
    }

	// Den Clipper für Fensteranwendungen nicht vergessen.
	if(m_lpDClip != NULL)
	{
		m_lpDClip->Release();
		m_lpDClip = NULL;
	}
}

bool
CGidxGraphic::Check()
{
	if(m_lpDDSPrimary->IsLost()!= DDERR_SURFACELOST)
		return true;
	else
		return false;
}

void
CGidxGraphic::Restore()
{
	// Erst laden wir alle Bildinformationen die
	// aus einer Bitmapdatei geladen wurden.

	IMG *temp = m_img;

 	while(temp!=NULL)
	{	
		if(temp->type==IMG_FROMFILE)
		{
			LoadImg(temp->filename,temp,temp->memtype);
			
			if(temp->usecolorkey)
			{   // Wenn Colorkey nötig ist
				DDCOLORKEY      key;
				DDPIXELFORMAT	ddpf;

				ddpf.dwSize=sizeof(ddpf);
				m_lpDDSPrimary->GetPixelFormat(&ddpf);
				
				key.dwColorSpaceLowValue =Color(temp->r,temp->g,temp->b);
				key.dwColorSpaceHighValue=Color(temp->r,temp->g,temp->b);
				
				temp->img->SetColorKey(DDCKEY_SRCBLT,&key);
			}
		}
 		temp = temp->next;
	}

	// Jetzt Speicherherstellen die von User 
	// angefordert wurden
	temp = m_img;

 	while(temp!=NULL)
	{	
		if(temp->type==IMG_FROMUSER)
		{
			CreateImage(temp,temp->rect.right,temp->rect.bottom,temp->memtype );
			
			if(temp->usecolorkey)
			{   // Wenn Colorkey nötig ist
				DDCOLORKEY      key;
				DDPIXELFORMAT	ddpf;

				ddpf.dwSize=sizeof(ddpf);
				m_lpDDSPrimary->GetPixelFormat(&ddpf);
				
				key.dwColorSpaceLowValue =Color(temp->r,temp->g,temp->b);
				key.dwColorSpaceHighValue=Color(temp->r,temp->g,temp->b);
				
				temp->img->SetColorKey(DDCKEY_SRCBLT,&key);
			}
		}
 		temp = temp->next;
	}

	// Und jetzt noch alle Bilder in die man ein Bild 
	// reinkopiert hat
	temp = m_img;

 	while(temp!=NULL)
	{	
		if(temp->type==IMG_FROMIMG)
		{
			CreateImage(temp,temp->rect.right,temp->rect.bottom,temp->memtype);
			CopyRect(0,0,temp->rect.right,temp->rect.bottom,0,0,temp->source,temp);

			if(temp->usecolorkey)
			{   // Wenn Colorkey nötig ist
				DDCOLORKEY      key;
				DDPIXELFORMAT	ddpf;

				ddpf.dwSize=sizeof(ddpf);
				m_lpDDSPrimary->GetPixelFormat(&ddpf);
				
				key.dwColorSpaceLowValue =Color(temp->r,temp->g,temp->b);
				key.dwColorSpaceHighValue=Color(temp->r,temp->g,temp->b);
				
				temp->img->SetColorKey(DDCKEY_SRCBLT,&key);
			}
		}
 		temp = temp->next;
	}
	Color(0,0,0);
}

void
CGidxGraphic::RestoreSurfaces()
{
	m_lpDD->RestoreAllSurfaces();
}

void
CGidxGraphic::CloseWindow()
{
	m_bClose=true;
}

bool
CGidxGraphic::IsWindow()
{
	return m_bClose;
}

int
CGidxGraphic::GetMode(LPDIRECTDRAWSURFACE7 surface)
{
	DDSURFACEDESC2	ddsd;
	int iMode=0;

	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof DDSURFACEDESC2;
	surface->GetSurfaceDesc( &ddsd );

	int rbit = BitCount(ddsd.ddpfPixelFormat.dwRBitMask);
	int gbit = BitCount(ddsd.ddpfPixelFormat.dwGBitMask);
	int bbit = BitCount(ddsd.ddpfPixelFormat.dwBBitMask);

	if(ddsd.ddpfPixelFormat.dwRGBBitCount == 16)
	{
		// RGB 555
		if(rbit==5 && rbit==5 && rbit==5)
			iMode = 0;
		// RGB 565
		if(rbit==5 && rbit==6 && rbit==5)
			iMode = 1;
		// 16Bit
		else
			iMode = 2;
	}
	
	// 24Bit
	if(ddsd.ddpfPixelFormat.dwRGBBitCount == 24 )
		iMode = 3;

	// 32Bit
	if(ddsd.ddpfPixelFormat.dwRGBBitCount == 32)
		iMode = 4;

	return iMode;
}

void
CGidxGraphic::Init(HWND hwnd, int reso)
{
	m_hwnd = hwnd;
	m_bpp  = reso;
	m_bActive=true;
}

int
CGidxGraphic::InitFullscreen()
{
	HRESULT	ddrval;

	// Fensterstyle 
	SetWindowLong(m_hwnd,GWL_STYLE,WS_POPUP);

    ShowWindow    (m_hwnd, SW_SHOWNORMAL); 
    UpdateWindow  (m_hwnd); 

	// Direct Draw vorbereiten
	ddrval = DirectDrawCreateEx(NULL, (LPVOID *)&m_lpDD, IID_IDirectDraw7, NULL); 
	if(ddrval != DD_OK) 
		return 1;

	ddrval =m_lpDD->SetCooperativeLevel(m_hwnd,DDSCL_EXCLUSIVE|DDSCL_FULLSCREEN);
	if(ddrval != DD_OK) 
		return 2;

	ddrval = m_lpDD->SetDisplayMode(m_xoffset, m_yoffset, m_bpp, 0, 0);
	if(ddrval != DD_OK) 
		return 3; 

	// Primär Surface erstellen mit einem Backbuffer
	DDSURFACEDESC2			ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd); 
	ddsd.dwFlags = DDSD_CAPS | DDSD_BACKBUFFERCOUNT; 
	ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE |DDSCAPS_FLIP |DDSCAPS_COMPLEX |DDSCAPS_PRIMARYSURFACE |DDSCAPS_VIDEOMEMORY;
	ddsd.dwBackBufferCount = 2;
	ddrval = m_lpDD->CreateSurface(&ddsd, &m_lpDDSPrimary, NULL);
	
	if(ddrval != DD_OK) 
		return 4;

	DDSCAPS2				ddscaps;
	ZeroMemory(&ddscaps, sizeof(ddscaps));
	ddscaps.dwCaps = DDSCAPS_BACKBUFFER; 
	ddrval = m_lpDDSPrimary->GetAttachedSurface(&ddscaps, &m_lpDDSBack);
	if(ddrval != DD_OK) 
		return 5;

	// Farbmaske
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize=sizeof(ddsd);
	ddrval = m_lpDD->GetDisplayMode(&ddsd);
	if(ddrval != DD_OK)
		return 6;

	m_MaskRed   = ddsd.ddpfPixelFormat.dwRBitMask;
	m_MaskGreen = ddsd.ddpfPixelFormat.dwGBitMask;
	m_MaskBlue  = ddsd.ddpfPixelFormat.dwBBitMask;

	// Anzahl der Bits 
	m_rbit      = BitCount(ddsd.ddpfPixelFormat.dwRBitMask);
	m_gbit      = BitCount(ddsd.ddpfPixelFormat.dwGBitMask);
	m_bbit      = BitCount(ddsd.ddpfPixelFormat.dwBBitMask);

	
	if (m_lpDD->QueryInterface(IID_IDirect3D7,(LPVOID *) &m_lpDirect3D7)!=S_OK)
		return 10;

	// Direct3D7
	if (m_lpDirect3D7->CreateDevice(IID_IDirect3DTnLHalDevice,m_lpDDSBack,&m_lpDirect3DDevice7)!=D3D_OK)
		if (m_lpDirect3D7->CreateDevice(IID_IDirect3DHALDevice,m_lpDDSBack,&m_lpDirect3DDevice7)!=D3D_OK)
			if (m_lpDirect3D7->CreateDevice(IID_IDirect3DMMXDevice,m_lpDDSBack,&m_lpDirect3DDevice7)!=D3D_OK)
				if (m_lpDirect3D7->CreateDevice(IID_IDirect3DRGBDevice,m_lpDDSBack,&m_lpDirect3DDevice7)!=D3D_OK)
					return 11;


	D3DVIEWPORT7			D3DViewport7;
	ZeroMemory(&D3DViewport7,sizeof(D3DVIEWPORT7));

	D3DViewport7.dwX		= 0;
	D3DViewport7.dwY		= 0; 
	D3DViewport7.dwWidth	= m_xoffset;
	D3DViewport7.dwHeight	= m_yoffset; 
	D3DViewport7.dvMinZ		= 0.0f;
	D3DViewport7.dvMaxZ		= 1.0f;
	m_lpDirect3DDevice7->SetViewport(&D3DViewport7);
	
	// Device einstellen
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE,true);
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE,  true);
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_ZENABLE,         false);
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA);
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);

	// Schwarze Ränder werden nicht gezeichnet
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
	//

	m_bInitialized= true;

	this->Restore();

	// kein Fehler
	return 0;
}

int
CGidxGraphic::InitWindowed()
{
	HRESULT	ddrval;
	RECT    rc;

	SetRect(&rc,0,0,m_xoffset,m_yoffset);
	AdjustWindowRect(&rc,WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED,TRUE);

	MoveWindow (m_hwnd, 
				GetSystemMetrics(SM_CXSCREEN)/2-m_xoffset/2,
				GetSystemMetrics(SM_CYSCREEN)/2-m_yoffset/2,
				(rc.right-rc.left),(rc.bottom-rc.top),true);

	SetWindowLong(m_hwnd,GWL_STYLE,WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_OVERLAPPED);
    ShowWindow    (m_hwnd, SW_SHOWNORMAL); 
    UpdateWindow  (m_hwnd); 

	// Direct Draw vorbereiten
	ddrval = DirectDrawCreateEx(NULL, (VOID **)&m_lpDD, IID_IDirectDraw7, NULL); 
	if(ddrval != DD_OK) 
		return 1;

	ddrval =m_lpDD->SetCooperativeLevel(m_hwnd, DDSCL_NORMAL );
	if(ddrval != DD_OK) 
		return 2;
	
	DDSURFACEDESC2			ddsd;
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize = sizeof(ddsd); 
	ddsd.dwFlags = DDSD_CAPS; 
	ddsd.ddsCaps.dwCaps = DDSCAPS_3DDEVICE|DDSCAPS_PRIMARYSURFACE |DDSCAPS_VIDEOMEMORY;
	ddrval = m_lpDD->CreateSurface(&ddsd, &m_lpDDSPrimary, NULL);
	if(ddrval != DD_OK) 
		return 4;

    ddrval=m_lpDD->CreateClipper(0,&m_lpDClip, NULL);
	if(ddrval != DD_OK) 
		return 7;

    ddrval=m_lpDClip->SetHWnd(0, m_hwnd);
	if(ddrval != DD_OK) 
		return 8;

    ddrval=m_lpDDSPrimary->SetClipper(m_lpDClip);
	if(ddrval != DD_OK) 
		return 9;

	if(m_lpDClip != NULL)
	{
		m_lpDClip->Release();
		m_lpDClip = NULL;
	}

	ZeroMemory(&ddsd,sizeof(ddsd)) ; 
    ddsd.dwSize         = sizeof(DDSURFACEDESC2) ; 
	ddsd.dwFlags        = DDSD_CAPS| DDSD_WIDTH | DDSD_HEIGHT ;
	ddsd.ddsCaps.dwCaps	= DDSCAPS_OFFSCREENPLAIN | DDSCAPS_3DDEVICE |DDSCAPS_VIDEOMEMORY;
    ddsd.dwWidth        = m_xoffset ; 
    ddsd.dwHeight       = m_yoffset ;
	
	ddrval=m_lpDD->CreateSurface(&ddsd, &m_lpDDSBack,NULL); 
	if (ddrval!=DD_OK) 
		return 5;

	// Farbmaske
	ZeroMemory(&ddsd, sizeof(ddsd));
	ddsd.dwSize=sizeof(ddsd);
	ddrval = m_lpDD->GetDisplayMode(&ddsd);
	if(ddrval != DD_OK)
		return 6;

	m_MaskRed   = ddsd.ddpfPixelFormat.dwRBitMask;
	m_MaskGreen = ddsd.ddpfPixelFormat.dwGBitMask;
	m_MaskBlue  = ddsd.ddpfPixelFormat.dwBBitMask;

	m_rbit      = BitCount(ddsd.ddpfPixelFormat.dwRBitMask);
	m_gbit      = BitCount(ddsd.ddpfPixelFormat.dwGBitMask);
	m_bbit      = BitCount(ddsd.ddpfPixelFormat.dwBBitMask);

	if (m_lpDD->QueryInterface(IID_IDirect3D7,(LPVOID *) &m_lpDirect3D7)!=S_OK)
		return 9;

	// Direct3D7
	if (m_lpDirect3D7->CreateDevice(IID_IDirect3DTnLHalDevice,m_lpDDSBack,&m_lpDirect3DDevice7)!=D3D_OK)
		if (m_lpDirect3D7->CreateDevice(IID_IDirect3DHALDevice,m_lpDDSBack,&m_lpDirect3DDevice7)!=D3D_OK)
			if (m_lpDirect3D7->CreateDevice(IID_IDirect3DMMXDevice,m_lpDDSBack,&m_lpDirect3DDevice7)!=D3D_OK)
				if (m_lpDirect3D7->CreateDevice(IID_IDirect3DRGBDevice,m_lpDDSBack,&m_lpDirect3DDevice7)!=D3D_OK)	
					return 11;

	D3DVIEWPORT7			D3DViewport7;
	ZeroMemory(&D3DViewport7,sizeof(D3DVIEWPORT7));

	D3DViewport7.dwX		= 0;
	D3DViewport7.dwY		= 0; 
	D3DViewport7.dwWidth	= m_xoffset;
	D3DViewport7.dwHeight	= m_yoffset; 
	D3DViewport7.dvMinZ		= 0.0f;
	D3DViewport7.dvMaxZ		= 1.0f;
	m_lpDirect3DDevice7->SetViewport(&D3DViewport7);
	
    sor_rect.top    = 0;
    sor_rect.left   = 0;
    sor_rect.right  = m_xoffset;
    sor_rect.bottom = m_yoffset;

	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_ALPHABLENDENABLE,true);
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_COLORKEYENABLE,  true);
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_ZENABLE,         false);
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_DESTBLEND,D3DBLEND_INVSRCALPHA);
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCALPHA);

	// Schwarze Ränder werden nicht gezeichnet
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_DESTBLEND,D3DBLEND_ONE);
	m_lpDirect3DDevice7->SetRenderState( D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);

	m_bInitialized= true;

	this->Restore();

	// kein Fehler
	return 0;
}

int
CGidxGraphic::Graphic(int x,int y,int bpp,int mode)
{
	ShutDown();

	m_windowed    = mode;
	m_xoffset     = x;
	m_yoffset     = y;
	
	if(m_windowed==0)
	{
		// Vorbereiten für Vollbildschirm
		Flip = &CGidxGraphic::FlipFullscreen;

		switch(bpp)
		{
		case 16:
			m_bpp     = 16;
			SetPixel  = &CGidxGraphic::SetPixel16;
			GetPixel  = &CGidxGraphic::GetPixel16;
			Collision = &CGidxGraphic::Collision16;
			LockBuffer= &CGidxGraphic::LockBuffer16;
			break;
		case 32:
			m_bpp     = 32;
			GetPixel  = &CGidxGraphic::GetPixel32;
			SetPixel  = &CGidxGraphic::SetPixel32;
			Collision = &CGidxGraphic::Collision32;
			LockBuffer= &CGidxGraphic::LockBuffer32;
			break;
		default:
			m_bpp     = 16;
			SetPixel  = &CGidxGraphic::SetPixel16;
			GetPixel  = &CGidxGraphic::GetPixel16;
			Collision = &CGidxGraphic::Collision16;
			LockBuffer= &CGidxGraphic::LockBuffer16;
			break;
		}
		return this->InitFullscreen();
	}

	if(m_windowed!=0)
	{	
		// Vorbereiten für eine Fensteranwendung
		Flip = &CGidxGraphic::FlipWindowed;

		switch(m_bpp)//m_bpp wird von giDXWin ermittelt.
		{
		case 16:
			SetPixel  = &CGidxGraphic::SetPixel16;
			GetPixel  = &CGidxGraphic::GetPixel16;
			Collision = &CGidxGraphic::Collision16;
			LockBuffer= &CGidxGraphic::LockBuffer16;
			break;
		case 32:
			SetPixel  = &CGidxGraphic::SetPixel32;
			GetPixel  = &CGidxGraphic::GetPixel32;
			Collision = &CGidxGraphic::Collision32;
			LockBuffer= &CGidxGraphic::LockBuffer32;
			break;
		default:
			MessageBox(NULL,L"  Das System hat eine Auflösung ungleich 16/32 Bit  ",L"ERROR!",MB_OK);
			SendMessage(m_hwnd,WM_DESTROY,0,0);
			break;
		}
		return this->InitWindowed();
	}
	return 0;
}

int
CGidxGraphic::Cls()
{
	HRESULT	ddrval;
	DDBLTFX	ddbltfx;	
	RECT	rect;

	ZeroMemory(&ddbltfx, sizeof(DDBLTFX));
	ddbltfx.dwSize     = sizeof(DDBLTFX);
	ddbltfx.dwFillColor= m_color;  

	SetRect(&rect,0,0,m_xoffset,m_yoffset);

	ddrval=m_lpDDSBack->Blt(&rect,NULL,&rect,DDBLT_COLORFILL,&ddbltfx);
	if(ddrval != DD_OK)
		return 1;

	return 0;
}

unsigned long
CGidxGraphic::Color(unsigned int r, unsigned int g, unsigned int b)
{	 
	m_color=(((unsigned long)(m_MaskRed  *(r/2.55/100))) &m_MaskRed)  +
		    (((unsigned long)(m_MaskGreen*(g/2.55/100))) &m_MaskGreen)+
		    (((unsigned long)(m_MaskBlue *(b/2.55/100))) &m_MaskBlue);

	m_r = r;
	m_g = g;
	m_b = b;

	return m_color;
}

int 
CGidxGraphic::FlipFullscreen(unsigned long flag)
{
	HRESULT ddrval;

	ddrval=m_lpDDSPrimary->Flip(NULL,flag);
	if(ddrval != DD_OK)
	{
		ddrval = m_lpDDSPrimary->Restore();
		if(ddrval != DD_OK)
			return 1;
		ddrval = m_lpDDSPrimary->Flip(NULL,DDFLIP_WAIT);
		if(ddrval!=DD_OK)
			return 2;
		return 0;
	}
	return 0;
}

int 
CGidxGraphic::FlipWindowed(unsigned long flag)
{
	HRESULT ddrval;

	RECT des_rect;
	GetClientRect(m_hwnd,&des_rect);

	MapWindowPoints(m_hwnd, NULL, (LPPOINT)&des_rect, 2);

	ddrval=m_lpDDSPrimary->Blt(&des_rect,m_lpDDSBack, &sor_rect, DDBLT_WAIT, NULL);
	if (ddrval!=DD_OK)
	{
		ddrval = m_lpDDSPrimary->Restore();
		if(ddrval != DD_OK)
			return 1;
		ddrval=m_lpDDSPrimary->Blt(&des_rect,m_lpDDSBack, &sor_rect, DDBLT_WAIT, NULL);
		if(ddrval!=DD_OK)
			return 2;
	}
	return 0;
}

int
CGidxGraphic::CreateImage(LPIMAGE lpimg, int x, int y, int memtype)
{
	lpimg->img    = NULL;

	HRESULT			ddrval;
	DDSURFACEDESC2	ddsd;

	ZeroMemory(&ddsd,sizeof(ddsd)); 
    ddsd.dwSize   = sizeof(DDSURFACEDESC2); 
    ddsd.dwFlags  = DDSD_CAPS | DDSD_WIDTH | DDSD_HEIGHT | DDSD_TEXTURESTAGE ;
    ddsd.dwWidth  = x; 
    ddsd.dwHeight = y; 

	// Orginalgröße speichern. 
	SetRect(&lpimg->org,0,0,x,y);
	// Änder sich später wenn Textur angepasst wird
	SetRect(&lpimg->rect,0,0,x,y);

	if(memtype==0)
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY; 
	else if(memtype==1)
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
 	// Als Textur verwednen.
	else if(memtype==2)
	{
		//Device beschreibung
		D3DDEVICEDESC7 DD3DDeviceDesc7;
		ZeroMemory(&DD3DDeviceDesc7,sizeof(D3DDEVICEDESC7));

		m_lpDirect3DDevice7->GetCaps(&DD3DDeviceDesc7);

		if ((DD3DDeviceDesc7.deviceGUID==IID_IDirect3DHALDevice)||
			(DD3DDeviceDesc7.deviceGUID==IID_IDirect3DTnLHalDevice))
		{
			ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
		else
		{ 
			ddsd.ddsCaps.dwCaps	|= DDSCAPS_SYSTEMMEMORY;
		}

		if (DD3DDeviceDesc7.dpcTriCaps.dwTextureCaps&D3DPTEXTURECAPS_POW2)
		{
			for (ddsd.dwWidth=1;(unsigned long)lpimg->rect.right > ddsd.dwWidth; ddsd.dwWidth<<=1);
				for (ddsd.dwHeight=1;(unsigned long)lpimg->rect.bottom  > ddsd.dwHeight;ddsd.dwHeight<<=1);
	
			// Neue Texturgröße im Speicher
			SetRect(&lpimg->rect,0,0,ddsd.dwWidth,ddsd.dwHeight);
		}
	 
		if(DD3DDeviceDesc7.dpcTriCaps.dwTextureCaps&D3DPTEXTURECAPS_SQUAREONLY)
		{
			if(ddsd.dwWidth > ddsd.dwHeight)
				ddsd.dwHeight = ddsd.dwWidth;
			else
				ddsd.dwWidth  = ddsd.dwHeight;

			// Neue Texturgröße im Speicher
			SetRect(&lpimg->rect,0,0,ddsd.dwWidth,ddsd.dwHeight);
		}

		lpimg->ftu = (float) (lpimg->org.right)  / (float)(lpimg->rect.right) ;
		lpimg->ftv = (float) (lpimg->org.bottom) / (float)(lpimg->rect.bottom);

		lpimg->istextur = true;
	}

	ddrval=m_lpDD->CreateSurface(&ddsd, &lpimg->img,NULL); 
	if (ddrval!=DD_OK) 
		return 1;

    SetRect(&lpimg->rect,0,0,x,y);
	
	lpimg->type = IMG_FROMUSER;

	// Kollisonabfrage
	lpimg->bitfield   = new bool[lpimg->width * lpimg->height];
	lpimg->pixelgenau = false;

	return 0;
}

int
CGidxGraphic::LoadImg(const wchar_t* filename, LPIMAGE lpimg,int memtype)
{
	HRESULT					ddrval    = NULL;
	DDSURFACEDESC2			ddsd;       
	HDC						hdc       = NULL;
	HDC						bit_dc    = NULL;
	HBITMAP					bit       = NULL;
	BITMAP					bitmap;
	BOOL                    from_file = true;

	bit=(HBITMAP) LoadImage(NULL,filename,IMAGE_BITMAP,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
	if (!bit) 
		return 1;

    GetObject( bit, sizeof(BITMAP), &bitmap );
	
	ZeroMemory(&ddsd,sizeof(ddsd));
	ddsd.dwSize         = sizeof(DDSURFACEDESC2);
	ddsd.dwFlags        = DDSD_CAPS| DDSD_WIDTH | DDSD_HEIGHT ;
	ddsd.ddsCaps.dwCaps	= DDSCAPS_TEXTURE;
	ddsd.dwWidth        = bitmap.bmWidth;
	ddsd.dwHeight       = bitmap.bmHeight;

	// Orginalgröße speichern. 
	SetRect(&lpimg->org,0,0,bitmap.bmWidth,bitmap.bmHeight);
	// Änder sich später wenn Textur angepasst wird
	SetRect(&lpimg->rect,0,0,bitmap.bmWidth,bitmap.bmHeight);

	if(memtype==0)
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_SYSTEMMEMORY; 
	else if(memtype==1)
		ddsd.ddsCaps.dwCaps = DDSCAPS_OFFSCREENPLAIN|DDSCAPS_VIDEOMEMORY;
 	// Als Textur verwednen.
	else if(memtype==2)
	{
		//Device beschreibung
		D3DDEVICEDESC7 DD3DDeviceDesc7;
		ZeroMemory(&DD3DDeviceDesc7,sizeof(D3DDEVICEDESC7));

		m_lpDirect3DDevice7->GetCaps(&DD3DDeviceDesc7);

		if ((DD3DDeviceDesc7.deviceGUID==IID_IDirect3DHALDevice)||
			(DD3DDeviceDesc7.deviceGUID==IID_IDirect3DTnLHalDevice))
		{
			ddsd.ddsCaps.dwCaps2 = DDSCAPS2_TEXTUREMANAGE;
		}
		else
		{ 
			ddsd.ddsCaps.dwCaps	|= DDSCAPS_SYSTEMMEMORY;
		}

		if (DD3DDeviceDesc7.dpcTriCaps.dwTextureCaps&D3DPTEXTURECAPS_POW2)
		{
			for (ddsd.dwWidth=1;(unsigned long)lpimg->rect.right > ddsd.dwWidth; ddsd.dwWidth<<=1);
				for (ddsd.dwHeight=1;(unsigned long)lpimg->rect.bottom  > ddsd.dwHeight;ddsd.dwHeight<<=1);
	
			// Neue Texturgröße im Speicher
			SetRect(&lpimg->rect,0,0,ddsd.dwWidth,ddsd.dwHeight);
		}
	 
		if(DD3DDeviceDesc7.dpcTriCaps.dwTextureCaps&D3DPTEXTURECAPS_SQUAREONLY)
		{
			if(ddsd.dwWidth > ddsd.dwHeight)
				ddsd.dwHeight = ddsd.dwWidth;
			else
				ddsd.dwWidth  = ddsd.dwHeight;

			// Neue Texturgröße im Speicher
			SetRect(&lpimg->rect,0,0,ddsd.dwWidth,ddsd.dwHeight);
		}

		lpimg->ftu = (float) (lpimg->org.right)  / (float)(lpimg->rect.right) ;
		lpimg->ftv = (float) (lpimg->org.bottom) / (float)(lpimg->rect.bottom);

		lpimg->istextur = true;
	}

	ddrval=m_lpDD->CreateSurface(&ddsd,&lpimg->img,NULL);
	
	if (ddrval!=DD_OK)
		return 2;

	lpimg->img->GetDC(&hdc);
	bit_dc=CreateCompatibleDC(hdc);
	SelectObject(bit_dc,bit);
	BitBlt(hdc,0,0,bitmap.bmWidth,bitmap.bmHeight,bit_dc,0,0,SRCCOPY);
	lpimg->img->ReleaseDC(hdc);
	DeleteDC(bit_dc);

	DeleteObject(bit);	
	
	if(from_file)
	{
		int i=0;
		while(filename[i]!='\0')
		{
			lpimg->filename[i]=filename[i];
			i++;
		}
		lpimg->filename[i]= '\0';
		lpimg->type       = IMG_FROMFILE;
	}
	else
	{
		// 
	}

	lpimg->bitfield  = new bool[lpimg->org.right * lpimg->org.bottom];
	lpimg->pixelgenau= false;
	
	return 0;
}

int 
CGidxGraphic::DrawImage(int x, int y, LPIMAGE lpimg, int flag, unsigned char alpha)
{
	HRESULT	ddrval = NULL;

	if(lpimg==NULL)
		return 0;

	RECT temprect;

	temprect.bottom = lpimg->rect.bottom;
	temprect.left   = lpimg->rect.left;
	temprect.right  = lpimg->rect.right;
	temprect.top    = lpimg->rect.top;	

	if(lpimg->istextur)
	{
			float fX1	= (float) x;
			float fY1	= (float) y;
			float fX2	= (float) x + lpimg->org.right;
			float fY2	= (float) y + lpimg->org.bottom;
			
			lpimg->square[0] = D3DTLVERTEX(D3DVECTOR(fX1,fY1,0.0f),1.0f,RGBA_MAKE(255,255,255,alpha),0,0.0f      ,0.0f);
			lpimg->square[1] = D3DTLVERTEX(D3DVECTOR(fX2,fY1,0.0f),1.0f,RGBA_MAKE(255,255,255,alpha),0,lpimg->ftu,0.0f);
			lpimg->square[2] = D3DTLVERTEX(D3DVECTOR(fX1,fY2,0.0f),1.0f,RGBA_MAKE(255,255,255,alpha),0,0.0f      ,lpimg->ftv);
			lpimg->square[3] = D3DTLVERTEX(D3DVECTOR(fX2,fY2,0.0f),1.0f,RGBA_MAKE(255,255,255,alpha),0,lpimg->ftu,lpimg->ftv);

			m_lpDirect3DDevice7->SetTexture(0,lpimg->img);

			m_lpDirect3DDevice7->SetTextureStageState(0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE);

			m_lpDirect3DDevice7->BeginScene();
			
			m_lpDirect3DDevice7->DrawPrimitive(D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX,lpimg->square,4,0);

			m_lpDirect3DDevice7->EndScene();

			return 0;
	}
	else
	{
		if(flag&1) // Clipping
		{
			if(x<0)
			{
				temprect.left-=x;
				x = 0;
			}
			if(x+lpimg->rect.right>m_xoffset)
				temprect.right=m_xoffset-x;
			
			if(y<0)
			{
				temprect.top-=y;
				y = 0;
			}
			if(y+lpimg->rect.bottom>m_yoffset)
				temprect.bottom=m_yoffset-y;
		}
		
		if(flag&2) // Colorkey
		{
			if(lpimg->img!=NULL)
			{
				ddrval=m_lpDDSBack->BltFast(x, y,lpimg->img,&temprect,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);	
			}


			if (ddrval!=DD_OK) 
				return 1;
			else
				return 0;
		}

		if(lpimg->img!=NULL)
		{
			// Weder Colorkey noch Clipping
			ddrval=m_lpDDSBack->BltFast(x, y,lpimg->img,&temprect,DDBLTFAST_WAIT);
		}

		//if (ddrval!=DD_OK) 
		//	return 1;
		//else
			return 0;

	}

	return 0;
}

int 
CGidxGraphic::DrawRect(int x, int y, LPIMAGE lpimg, 
					   int sx,int sy, int width, int height, 
					   int flag, unsigned char alpha)
{
	HRESULT	ddrval;

	if(lpimg->istextur)
	{       
		    // Größe des Mesh
			float fX1	= (float) x; 
			float fY1	= (float) y;
			float fX2	= (float) x + width *1.0f;
			float fY2	= (float) y + height*1.0f;

			// Texturkoordinaten
			float tu1    = (float)  sx          / (float)lpimg->rect.right;
			float tv1    = (float)  sy          / (float)lpimg->rect.bottom;
			float tu2    = (float) (sx + width) / (float)lpimg->rect.right;
			float tv2    = (float) (sy + height)/ (float)lpimg->rect.bottom;

			lpimg->square[0] = D3DTLVERTEX(D3DVECTOR(fX1,fY1,0.0f), 1.0f, RGBA_MAKE(255,255,255,alpha), 0, tu1, tv1);
			lpimg->square[1] = D3DTLVERTEX(D3DVECTOR(fX2,fY1,0.0f), 1.0f, RGBA_MAKE(255,255,255,alpha), 0, tu2, tv1);
			lpimg->square[2] = D3DTLVERTEX(D3DVECTOR(fX1,fY2,0.0f), 1.0f, RGBA_MAKE(255,255,255,alpha), 0, tu1, tv2);
			lpimg->square[3] = D3DTLVERTEX(D3DVECTOR(fX2,fY2,0.0f), 1.0f, RGBA_MAKE(255,255,255,alpha), 0, tu2, tv2);

			m_lpDirect3DDevice7->SetTexture(0,lpimg->img);

			m_lpDirect3DDevice7->BeginScene();
			
			m_lpDirect3DDevice7->DrawPrimitive(D3DPT_TRIANGLESTRIP,D3DFVF_TLVERTEX,lpimg->square,4,0);

			m_lpDirect3DDevice7->EndScene();

			return 0;
	}
	else
	{
		RECT temprect;

		temprect.left  = sx; 
		temprect.top   = sy;
		temprect.right = width + sx;
		temprect.bottom= height+ sy;

		if(flag&1)
		{
			if(x<0)
			{
				temprect.left-=x;
				x = 0;
			}
			if(x+temprect.right-sx>m_xoffset)
				temprect.right=m_xoffset-x+sx;
			
			if(y<0)
			{
				temprect.top-=y;
				y = 0;
			}
			if(y+temprect.bottom-sy>m_yoffset)
				temprect.bottom=m_yoffset-y+sy;
		}

		if(flag&2)
		{
			ddrval=m_lpDDSBack->BltFast(x, y,lpimg->img,&temprect,DDBLTFAST_WAIT|DDBLTFAST_SRCCOLORKEY);	
			if (ddrval!=DD_OK) 
				return 1;
			return 0;
		}

		ddrval=m_lpDDSBack->BltFast(x, y,lpimg->img,&temprect,DDBLTFAST_WAIT);
		if (ddrval!=DD_OK) 
			return 1;
	}

	return 0;
}

void 
CGidxGraphic::ColorKey(LPIMAGE bild, unsigned int r, unsigned int g, unsigned int b)
{
	DDCOLORKEY      key;
	unsigned long   color;
	
	color = (((unsigned long)(m_MaskRed  *(r/2.55/100))) &m_MaskRed)  +
		    (((unsigned long)(m_MaskGreen*(g/2.55/100))) &m_MaskGreen)+
		    (((unsigned long)(m_MaskBlue *(b/2.55/100))) &m_MaskBlue);

	key.dwColorSpaceLowValue  = color;
	key.dwColorSpaceHighValue = color;
	
	bild->colorkey   = color;
	bild->r          = r;
	bild->g          = g;
	bild->b          = b;
	bild->usecolorkey= true;

	if (bild->img != NULL)
	{
		bild->img->SetColorKey(DDCKEY_SRCBLT, &key);

		this->LockBuffer32(bild);
		for (unsigned int y = 0; y < bild->org.bottom; y++)
		{
			for (unsigned int x = 0; x < bild->org.right; x++)
			{
				unsigned long transparent = this->GetPixel32(x, y);
				if (transparent == this->Color(r, g, b))
					bild->bitfield[x * bild->org.bottom + y] = false;
				else
					bild->bitfield[x * bild->org.bottom + y] = true;
			}
		}
		this->UnlockBuffer(bild);
	}

	bild->pixelgenau = true;
}

int 
CGidxGraphic::CopyRect(int x, int y, 
					   int img_width, int img_height, 
					   int x2, int y2, 
					   LPIMAGE lpimg1, LPIMAGE lpimg2)
{	
	HRESULT ddrval;
	RECT	temprect;

	int z=y+img_height;
	int w=x+img_width;

	SetRect(&temprect,x,y,w,z);

	lpimg2->x=x;
	lpimg2->y=y;
	lpimg2->type=IMG_FROMIMG;
	lpimg2->rect.left=0;
	lpimg2->rect.top =0;
	lpimg2->rect.bottom=img_height;
	lpimg2->rect.right=img_width;

	ddrval=lpimg2->img->BltFast(x2, y2,lpimg1->img,&temprect,NULL);
	if(ddrval!=DD_OK)
		return 1;
	return 0;
}

bool
CGidxGraphic::CollisionRect(int x1, int y1,LPIMAGE img1, int x2, int y2,LPIMAGE img2)
{
	if( (x1 + img1->kollision.right) <= x2)
        return false;

	if( (y1 + img1->kollision.bottom) <= y2)
        return false;

	if( (x2 + img2->kollision.right) <= x1)
        return false;

	if( (y2 + img2->kollision.bottom) <= y1)
        return false;

    return true;
}

bool 
CGidxGraphic::Collision32(int x1,int y1,LPIMAGE img1,unsigned int frame1,
						  int x2,int y2,LPIMAGE img2,unsigned int frame2)
{	
	int xg1, yg1, xg2, yg2	=0;
	int xOffset1			=0;
	int yOffset1			=0;
	int xOffset2			=0;
	int yOffset2			=0;
	int breite,hoehe		=0;
	int xof1				=0;
	int yof1				=0;
	int xof2				=0;
	int yof2				=0;

	// Restliche Berechnung
	if( (x1 + img1->kollision.right) <= x2)
        return false;

	if( (y1 + img1->kollision.bottom) <= y2)
        return false;

	if( (x2 + img2->kollision.right) <= x1)
        return false;

	if( (y2 + img2->kollision.bottom) <= y1)
        return false;

	if( !img1->pixelgenau && !img2->pixelgenau)
		return true;

	// Teil für animierte Sprites
	if(frame1<1)
		frame1=1;
	if(frame1>img1->frame)
		frame1=img1->frame;
	if(frame2<1)
		frame2=1;
	if(frame2>img2->frame)
		frame2=img2->frame;

	xof1 = (((int)(frame1-1)%(img1->reihe))*img1->width);
	yof1 = (((int)(frame1-1)/img1->reihe)*img1->height);

	xof2 = (((int)(frame2-1)%(img2->reihe))*img2->width);
	yof2 = (((int)(frame2-1)/img2->reihe)*img2->height);

	// Restliche Berechnungen
	xg1 = img1->kollision.right  - img1->kollision.left;
    xg2 = img2->kollision.right  - img2->kollision.left;
    yg1 = img1->kollision.bottom - img1->kollision.top;
    yg2 = img2->kollision.bottom - img2->kollision.top;

	if(x1<x2)                  
        xOffset1 = xOffset1+x2-x1;
    else                                        
    if(x1>x2)                    
        xOffset2 = xOffset2+x1-x2;

    if(y1<y2)                    
        yOffset1 = yOffset1+y2-y1;
    else                                        
    if(y1>y2)                   
        yOffset2 = yOffset2+y1-y2;

	if(xg1<xg2)
        breite = xg1;
    else
        breite = xg2;

    if(yg1<yg2)
        hoehe  = yg1;
    else
        hoehe  = yg2;

	for(int i=0; i<breite; i=i+1)
    {
        for(int j=0; j<hoehe; j=j+1)
        {
            if((xOffset2+i < xg2) && (yOffset2+j < yg2)    && (xOffset1+i < xg1) && (yOffset1+j < yg1))
            {
                if((img1->bitfield[(i+xOffset1+xof1) * (img1->org.bottom) + j+yOffset1+yof1]==true)
				&& (img2->bitfield[(i+xOffset2+xof2) * (img2->org.bottom) + j+yOffset2+yof2]==true))
				{
					return true;
			
				}
            }
        }
    }

	return false;
}

bool
CGidxGraphic::Collision16(int x1,int y1,LPIMAGE img1,unsigned int frame1,
						  int x2,int y2,LPIMAGE img2,unsigned int frame2)
{	
	// Überbleibsel aus älteren Versionen
	bool result = false;

	int xg1, yg1, xg2, yg2	=0;
	int xOffset1			=0;
	int yOffset1			=0;
	int xOffset2			=0;
	int yOffset2			=0;
	int breite,hoehe		=0;
	int xof1				=0;
	int yof1				=0;
	int xof2				=0;
	int yof2				=0;

	// Teil für animierte Sprites
	if(frame1<1)
		frame1=1;
	if(frame1>img1->frame)
		frame1=img1->frame;
	if(frame2<1)
		frame2=1;
	if(frame2>img2->frame)
		frame2=img2->frame;

	xof1 = (((int)(frame1-1)%(img1->reihe))*img1->width);
	yof1 = (((int)(frame1-1)/img1->reihe)*img1->height);

	xof2 = (((int)(frame2-1)%(img2->reihe))*img2->width);
	yof2 = (((int)(frame2-1)/img2->reihe)*img2->height);

	// Restliche Berechnung
	if( (x1 + img1->kollision.right) <= x2)
        return result;

	if( (y1 + img1->kollision.bottom) <= y2)
        return result;

	if( (x2 + img2->kollision.right) <= x1)
        return result;

	if( (y2 + img2->kollision.bottom) <= y1)
        return result;

	xg1 = img1->kollision.right  - img1->kollision.left;
    xg2 = img2->kollision.right  - img2->kollision.left;
    yg1 = img1->kollision.bottom - img1->kollision.top;
    yg2 = img2->kollision.bottom - img2->kollision.top;

	if(x1<x2)                  
        xOffset1 = xOffset1+x2-x1;
    else                                        
    if(x1>x2)                    
        xOffset2 = xOffset2+x1-x2;

    if(y1<y2)                    
        yOffset1 = yOffset1+y2-y1;
    else                                        
    if(y1>y2)                   
        yOffset2 = yOffset2+y1-y2;

	if(xg1<xg2)
        breite = xg1;
    else
        breite = xg2;

    if(yg1<yg2)
        hoehe  = yg1;
    else
        hoehe  = yg2;

	for(int i=0; i<breite; i=i+1)
    {
        for(int j=0; j<hoehe; j=j+1)
        {
            if((xOffset2+i < xg2) && (yOffset2+j < yg2)    && (xOffset1+i < xg1) && (yOffset1+j < yg1))
            {
                if((img1->bitfield[(i+xOffset1+xof1) * (img1->org.bottom) + j+yOffset1+yof1]==true)
				&& (img2->bitfield[(i+xOffset2+xof2) * (img2->org.bottom) + j+yOffset2+yof2]==true))
				{
					result = true;
			
				}
            }
        }
    }

	return result ;
}

void
CGidxGraphic::FontColor(int r, int g, int b)
{
	m_fontcolor=RGB(r,g,b);
}

HFONT
CGidxGraphic::CreateFont(int height, int width, int fnWeight, wchar_t* face)
{
	return ::CreateFont(height,width,0,0,fnWeight,0,0,0,0,0,0,0,FIXED_PITCH, L"ARIAL");
}

void
CGidxGraphic::SetFont(HFONT font)
{
	m_font=font;
}

void
CGidxGraphic::Print(int x, int y, const wchar_t* text)
{    
   	HDC hdc=NULL;
	m_lpDDSBack->GetDC(&hdc);
	
	HGDIOBJ oldObj= SelectObject(hdc,m_font);  
    SetBkMode(hdc,TRANSPARENT);   
    SetTextColor(hdc,m_fontcolor);

	TextOut(hdc,x,y,text,lstrlen(text));

	m_lpDDSBack->ReleaseDC(hdc);
}

HWND
CGidxGraphic::GetHWND()
{
	return m_hwnd;
}

void
CGidxGraphic::GetBackBuffer(LPLPIMAGE lplpimg)
{
	(*lplpimg)->img=m_lpDDSBack;
	SetRect(&(*lplpimg)->rect,0,0,m_xoffset,m_yoffset);
}

int
CGidxGraphic::LockBuffer16(LPIMAGE image)
{
	HRESULT			ddrval;
	DDSURFACEDESC2	ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddrval=image->img->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	if (ddrval!=DD_OK) 
		return 1;
	m_iPitch = (USHORT)ddsd.lPitch>>1;
    m_pVram16= (USHORT*)ddsd.lpSurface;

	m_rect = image->rect;

	return 0;
}

int
CGidxGraphic::LockBuffer32(LPIMAGE image)
{
	HRESULT			ddrval;
	DDSURFACEDESC2	ddsd;

    ZeroMemory(&ddsd, sizeof(DDSURFACEDESC2));
    ddsd.dwSize = sizeof(DDSURFACEDESC2);
    ddrval=image->img->Lock(NULL, &ddsd, DDLOCK_SURFACEMEMORYPTR | DDLOCK_WAIT, NULL);
	if (ddrval!=DD_OK) 
		return 1;
	m_iPitch = (ULONG)ddsd.lPitch>>2;
    m_pVram32= (ULONG*)ddsd.lpSurface;

	m_rect = image->rect;

	return 0;
}

int
CGidxGraphic::UnlockBuffer(LPIMAGE image)
{
	HRESULT	ddrval;
	ddrval=image->img->Unlock(NULL);
	if(ddrval!=DD_OK)
		return 1;
	return 0;
}

void
CGidxGraphic::SetPixel16(int x, int y)
{
	if(x>=0 && x<=m_rect.right && y>=0 && y<=m_rect.bottom)
		m_pVram16[x + y*m_iPitch] = (USHORT)m_color;
}

void
CGidxGraphic::SetPixel32(int x, int y)
{
	if(x>=0 && x<=m_rect.right && y>=0 && y<=m_rect.bottom)
		m_pVram32[x + y*m_iPitch] = m_color;
}

int
CGidxGraphic::GetX()
{
	return m_xoffset;
}

int 
CGidxGraphic::GetY()
{
	return m_yoffset;
}

int 
CGidxGraphic::GetBPP()
{
	return m_bpp;
}

int
CGidxGraphic::GetModus()
{
	return m_windowed; 
}

ULONG
CGidxGraphic::GetPixel16(int x, int y)
{
	if(x>=0 && x<=m_rect.right && y>=0 && y<=m_rect.bottom)
		return m_pVram16[x + y*m_iPitch];
	return 0;
}

ULONG
CGidxGraphic::GetPixel32(int x, int y)
{
	if(x>=0 && x<=m_rect.right && y>=0 && y<=m_rect.bottom)
		return m_pVram32[x + y*m_iPitch];
	return 0;
}

int
CGidxGraphic::GetRed(ULONG rgb)
{
	return ((rgb & m_MaskRed)>>(m_bbit+m_gbit))<<(8-m_rbit);
}

int
CGidxGraphic::GetGreen(ULONG rgb)
{
	return ((rgb & m_MaskGreen)>>m_bbit)<<(8-m_gbit);
}

int
CGidxGraphic::GetBlue(ULONG rgb)
{
	return ((rgb & m_MaskBlue))<<(8-m_bbit);
}

int
CGidxGraphic::ImageWidth(LPIMAGE img)
{
	return img->org.right;
}

int
CGidxGraphic::ImageHeight(LPIMAGE img)
{
	return img->org.bottom;
}

void
CGidxGraphic::GetMemory(LPLPIMAGE lplpimg)
{
	IMG	*temp = m_img;

	if(m_img==NULL)
	{
		(*lplpimg)       = new IMG;

		(*lplpimg)->img  = NULL;
		(*lplpimg)->prev = NULL;
		(*lplpimg)->next = NULL;
		(*lplpimg)->type = 0;
		(*lplpimg)->source =NULL;

		m_img = (*lplpimg);
	}
	else
	{
		while(temp->next!=NULL)
			temp=temp->next;

		temp->next       = new IMG;
		temp->next->img  = NULL;
		temp->next->next = NULL;
		temp->next->prev = temp;
		temp->next->type = 0;
		temp->next->source =NULL;

		(*lplpimg) = temp->next;
	}

}

void
CGidxGraphic::FreeImage(LPIMAGE lpimage)
{
	if(lpimage->prev==NULL)
	{
		m_img = lpimage->next;
				
		if(lpimage->next!=NULL)
			lpimage->next->prev=NULL;

		lpimage->next=NULL;
		
		if(lpimage!=NULL)
		{
			if(lpimage->img && lpimage->img != m_lpDDSBack)
			{
				lpimage->img->Release();
				lpimage->img=NULL;
			}
			delete lpimage;
		}
		
		lpimage = NULL;
	}
	else if(lpimage->next==NULL)
	{
		lpimage->prev->next=NULL;
		
		if(lpimage!=NULL)
		{
			if(lpimage->img && lpimage->img != m_lpDDSBack)
			{
				lpimage->img->Release();
				lpimage->img=NULL;
			}
			delete lpimage;
		}
		
		lpimage = NULL;
	}
	else
	{
		lpimage->next->prev = lpimage->prev;
		lpimage->prev->next = lpimage->next;

		if(lpimage!=NULL)
		{
			if(lpimage->img && lpimage->img != m_lpDDSBack)
			{
				lpimage->img->Release();
				lpimage->img=NULL;
			}
			delete lpimage;
		}
		
		lpimage = NULL;
	}
}

