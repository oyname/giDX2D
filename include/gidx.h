
/*---------------------------------------------------------
						  _ _______   __ ___  
					 	 (_)  _  \ \ / /(__ \
					 __ _ _| | | |\ V /  / _/
					/ _` | | | | |/   \ (____)
				   | (_| | | |/ // /^\ \
					\__, |_|___// / VERSION 3.25 12 2019
			        	__/ |
				   |___/    for VisualStudio V 16.3.4     
                                 
-----------------------------------------------------------
	gidx.h - Header-Datei
-----------------------------------------------------------
begin			: 3.10.2004

last changed		: 25.12.2019

---------------------------------------------------------*/

#ifndef GIDX_H
#define GIDX_H

#pragma comment(lib,"gidxwin.lib")

#include <stdio.h>
#include <math.h>
#include <windows.h>
#include "gidxgraphic.h"
#include "gidxinput.h"
#include "gidxsound.h"

/*
	Mathematische Sachen
*/
#define GX_PI    	   0.017453292f

/*
	Definiert für die Funktion flip().
	Die Werte wurden aus dem DX SDK übernommen
*/
#define GXFLIP_WAIT    0x00000001L
#define GXFLIP_NOVSYNC 0x00000008L

/*
	giDX Zeug. Wird verwendet beim initialisieren, 
	laden von Bitmaps, zeichnen der Bitmaps etc...
*/
#define GXFULLSCREEN   0
#define GXDEBUG        1
#define GXSYSTEMMEMORY 0
#define GXVIDEOMEMORY  1
#define GXTEXTURE      2
#define GXCLIPPING     1
#define GXCOLORKEY     2

/*
	Wird in giDXGraphic verwendet
	und definiert einige Farbformate
*/
#define GXRGB555       0
#define GXRGB565       1
#define GXRGB16        2
#define GXRGB24        3
#define GXRGB32        4

/*
	Wird verwendet von createfont()

	Es können auch die Windows Konstanten
	verwendet werden.
*/
#define GXDONTCARE	   FW_DONTCARE     
#define	GXTHIN		   FW_THIN   
#define	GXEXTRALIGHT   FW_EXTRALIGHT  
#define	GXULTRALIGHT   FW_ULTRALIGHT   
#define	GXLIGHT		   FW_LIGHT  
#define	GXNORMAL	   FW_NORMAL  
#define	GXREGULAR	   FW_REGULAR  
#define	GXMEDIUM	   FW_MEDIUM  
#define	GXSEMIBOLD	   FW_SEMIBOLD  
#define	GXDEMIBOLD	   FW_DEMIBOLD  
#define	GXBOLD		   FW_BOLD  
#define	GXEXTRABOLD	   FW_EXTRABOLD  
#define	GXULTRABOLD	   FW_ULTRABOLD  
#define	GXHEAVY		   FW_HEAVY 
#define	GXBLACK		   FW_BLACK  

/*
	Schliesst das Fenster beim verlassen der main() Funktion 
*/
#define GXRETURN       PostMessage(gethwnd(),WM_QUIT,0,0)

//
//	Speicherreservierung und Verwaltung
//
inline 
void 
freeimage(LPIMAGE lpimg){
	pGraphic->FreeImage(lpimg);
}

//
// Windows Zeug
//
inline 
int
apptitle(wchar_t* text){
	return SetWindowText(pGraphic->GetHWND(),text);
}

inline 
HWND
gethwnd(){
	return pGraphic->GetHWND();
}

inline
bool
iswindow(){
	return !pGraphic->IsWindow();
}

//
//	G I D X G R A P H I C 
//
inline 
int 
graphic(int x,int y, int bpp, int mode = 1){
	
	/*	    RÜCKGABEWERTE

		0	OK
		1	DirectDrawCreateEx
		2	SetCooperativeLevel
		3	SetDisplayMode
		4	CreateSurface
		5   GetAttachedSurface
		6	GetDisplayMode
		7	CreateClipper
		8	SetHWnd
		9	SetClipper
		10	QueryInterface
		11	CreateDevice
		110 GetHWND

	*/

	if(IsWindow(pGraphic->GetHWND()))
	{
		int res=0;

		pInput->SetScreenXY(x,y);
		res = pInput->InitInput(mode);
		if(res!=0) return res;

		res = pGraphic->Graphic(x,y,bpp,mode);
		if(res!=0) return res; 
	}
	else
		return 110;

	return 0;
}

inline 
LPIMAGE
copyimage(LPIMAGE source,int mode = GXVIDEOMEMORY )
{
	LPIMAGE temp;

	pGraphic->GetMemory(&temp);

	temp->type   = IMG_FROMIMG;
	temp->source = source;
	temp->memtype= mode;

	pGraphic->CreateImage(temp, source->rect.right, source->rect.bottom,mode);
	pGraphic->CopyRect(0,0,source->rect.right, source->rect.bottom,0,0,source,temp);

	return temp;
}

inline 
int 
cls(){
	return pGraphic->Cls();
}

inline 
unsigned long 
color(int r, int g, int b){
	return pGraphic->Color(r,g,b);
}

inline 
int 
flip(int flag){
	if(!IsIconic(gethwnd()))
	{
		Sleep(1);
		return (pGraphic->*(pGraphic->Flip))(flag);
	}
	else
	{
		bool shutdown=true;
		while(IsIconic(gethwnd()))
		{   // Solange Fenster minimiert ist...
			if(shutdown)
			{   
				//...Fahre DirectX runter...
				pGraphic->ShutDown();
				shutdown=false;
			}
			//...und warte
			//Sleep(1);
		}
		// Das Warten hat ein Ende. Initialisere DirectX und 
		// lade alle Bilder wieder in den Speicher.
		graphic(pGraphic->GetX(),pGraphic->GetY(),pGraphic->GetBPP(),pGraphic->GetModus());
	}
	return 0;
}

inline 
int
createimage(LPLPIMAGE lplpimg, int x, int y, int mem = GXVIDEOMEMORY){
	pGraphic->GetMemory(lplpimg);

	(*lplpimg)->usecolorkey =false;
	(*lplpimg)->width       = 0;
	(*lplpimg)->height      = 0;
	(*lplpimg)->frame       = 0;
	(*lplpimg)->source      = NULL;
	(*lplpimg)->istextur    = false;
	(*lplpimg)->memtype     = mem;

	return pGraphic->CreateImage(*lplpimg,x,y,mem);
}

inline 
int 
loadimage(const wchar_t* file_name, LPLPIMAGE lplpimg, int operate = GXVIDEOMEMORY ){
	int ret;

	pGraphic->GetMemory(lplpimg);
	
	(*lplpimg)->usecolorkey = false;
	(*lplpimg)->frame       = 1;
	(*lplpimg)->source      = NULL;
	(*lplpimg)->istextur    = false;
	(*lplpimg)->memtype     = operate;

	ret = pGraphic->LoadImg( file_name, *lplpimg, operate);

	(*lplpimg)->width				  = (*lplpimg)->rect.right;
	(*lplpimg)->height				  = (*lplpimg)->rect.bottom;
	(*lplpimg)->kollision.right       = (*lplpimg)->rect.right;
	(*lplpimg)->kollision.bottom      = (*lplpimg)->rect.bottom;
	(*lplpimg)->kollision.left		  = 0; 
	(*lplpimg)->kollision.top		  = 0; 
	(*lplpimg)->reihe       =  1;

	return ret;
}

inline
int
loadanimimage(const wchar_t* filename, LPLPIMAGE lplpimg, int width, int height, int frame, int operate = GXVIDEOMEMORY){
	int ret;

	pGraphic->GetMemory(lplpimg);

	(*lplpimg)->usecolorkey = false;
	(*lplpimg)->frame       = frame;
	(*lplpimg)->source      = NULL;
	(*lplpimg)->istextur    = false;
	(*lplpimg)->memtype     = operate;

	ret = pGraphic->LoadImg( filename, *lplpimg, operate);

	(*lplpimg)->width				= width;
	(*lplpimg)->height				= height;
	(*lplpimg)->kollision.right     = width;
	(*lplpimg)->kollision.bottom    = height;
	(*lplpimg)->kollision.left		= 0; 
	(*lplpimg)->kollision.top		= 0;
	(*lplpimg)->reihe               = pGraphic->ImageWidth((*lplpimg))/width;

	return ret;
}

inline 
int 
drawimage(int x, int y, LPIMAGE img, int flag, unsigned char alpha=255){
	return pGraphic->DrawImage(x,y,img,flag,alpha);
}

inline
int
drawanimimage(int x, int y, LPIMAGE img, unsigned int frame, int flag = GXCOLORKEY|GXCLIPPING, unsigned char alpha=255){
	if(frame<1)
		frame=1;
	if(frame>img->frame)
		frame=img->frame;

	pGraphic->DrawRect(x,y,img,
		              ((int)(frame-1)%(img->reihe))*img->width,
					  ((int)(frame-1)/img->reihe)*img->height,
					  img->width,img->height,flag,alpha);

	return ((int)(frame-1)%(img->reihe))*img->width;
}

inline 
int
drawrect(int x,int y,LPIMAGE img,int sx,int sy, int width, int height, int flag = GXCOLORKEY|GXCLIPPING , unsigned char alpha=255){
	return pGraphic->DrawRect(x,y,img,sx,sy,width,height,flag,alpha);
}

inline 
bool
collision(int x1, int y1, LPIMAGE img1,int frame1,
		  int x2, int y2, LPIMAGE img2,int frame2){

	return (pGraphic->*(pGraphic->Collision))(x1,y1,img1,frame1,
											  x2,y2,img2,frame2);
}

inline
bool
collisionrect(int x1, int y1, LPIMAGE img1, int x2, int y2, LPIMAGE img2){
	return pGraphic->CollisionRect(x1,y1,img1,x2,y2,img2);
}

inline
void
colorkey(LPIMAGE img, unsigned int r, unsigned int g,unsigned int b){
	if(img==NULL) return;
	pGraphic->ColorKey(img,r,g,b);
}

inline 
int
copyrect(int x,int y,int width,int height,int x2,int y2,LPIMAGE source, LPIMAGE dest){
	return pGraphic->CopyRect(x,y,width,height,x2,y2,source,dest);
}

inline 
void
print(int x, int y, const wchar_t* text){
	pGraphic->Print(x,y,text);
}

inline 
HFONT
createfont(int x, int y, int fnWeight, wchar_t* face){
	return pGraphic->CreateFont(x,y,fnWeight,face);
}

inline
void
setfont(HFONT font){
	pGraphic->SetFont(font);
}

inline 
void
fontcolor(int r, int g, int b){
	pGraphic->FontColor(r,g,b);
}

inline 
void
getbackbuffer(LPLPIMAGE lplpimg){
	pGraphic->GetMemory(lplpimg);	
	pGraphic->GetBackBuffer(lplpimg);
}

inline 
int
lockbuffer(LPIMAGE img){
	return (pGraphic->*(pGraphic->LockBuffer))(img);
}

inline 
int
unlockbuffer(LPIMAGE img){
	return pGraphic->UnlockBuffer(img);
}

inline 
void
setpixel(int x, int y){
	(pGraphic->*(pGraphic->SetPixel))(x,y);
}

inline 
unsigned long
getpixel(int x, int y){
	return (pGraphic->*(pGraphic->GetPixel))(x,y);
}

inline
int
getred(unsigned long rgb){
	return pGraphic->GetRed(rgb);
}

inline 
int
getgreen(unsigned long rgb){
	return pGraphic->GetGreen(rgb);
}

inline 
int
getblue(unsigned long rgb){
	return pGraphic->GetBlue(rgb);
}

inline 
int
getmode(LPIMAGE lpimg){
	return pGraphic->GetMode(lpimg->img);
}

inline
unsigned int
imagewidht(LPIMAGE img){
	return pGraphic->ImageWidth(img);
}

inline
int
imageheight(LPIMAGE img){
	return pGraphic->ImageHeight(img);
}
 
//
// G I D X I N P U T
//
inline
bool waitkey(){
	while(pInput->WaitKey());
	return true;
}

inline
bool keydown(int key){
	return pInput->KeyDown(key);
}

inline
bool keyhit(int key){
	return pInput->KeyHit(key);
}

inline
bool statkey(void){
	 return pInput->Statkey();
}

inline 
bool statmouse(void){
	 return (pInput->*(pInput->Statmouse))();
}

inline
int getmousex(void){
	return  (pInput->*(pInput->GetMouseX))();
}

inline
int getmousey(void){
	return  (pInput->*(pInput->GetMouseY))();
}

inline
bool mousedown(int button){
	if(button==0)
		return pInput->MouseDown0();
	if(button==1)
		return pInput->MouseDown1();
	return false;
}

//
// G I D X S O U N D 
//
inline
int loadsound(const wchar_t* filename,LPSND* lpsnd){
	return pSound->LoadSnd(filename,lpsnd);
}

inline
void play(LPSND lpsnd, long pan, long vol,long loop){
	pSound->PlaySnd(lpsnd->snd,pan,vol,loop);
}

inline
void stop(LPSND lpsnd,bool reset){
	pSound->StopSnd(lpsnd->snd,reset);
}

inline
void balance(LPSND lpsnd, long pan){
	pSound->Balance(lpsnd->snd,pan);
}

inline
void frequency(LPSND lpsnd, long freq){
	pSound->Frequency(lpsnd->snd,freq);
}

inline
void freesound(LPSND lpsnd){
	pSound->FreeSound(lpsnd);
}

#endif//GIDX_H

// REVISION 2
