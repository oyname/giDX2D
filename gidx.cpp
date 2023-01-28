#include <gidxgraphic.h>
#include <gidxinput.h>
#include <gidxsound.h>
#include <gidx.h>

/*
	Die einzelnen Objekte der giDX² Engine
*/
gdx::LPGIDXGRAPHIC  pGraphic=NULL;
gdx::LPGIDXINPUT	pInput  =NULL;
gdx::LPGIDXSOUND	pSound  =NULL;

/*
	Diese Funktionen benötigt giDX² für die Verwaltung der Objekte.
	Sie werden von giDXWin aufgerufen.
*/
int
CreateGidx(int hinst,int hwnd,int bpp){

	pGraphic = new gdx::CGidxGraphic;
	if(pGraphic==NULL)
		return 1;

	pInput = new gdx::CGidxInput;
	if(pInput==NULL)
		return 2;

	pSound = new gdx::CGidxSound;
	if(pSound==NULL)
		return 3;
	
	pGraphic->Init((HWND)hwnd,bpp);
	pInput->Init((HINSTANCE)hinst,(HWND)hwnd);
	pSound->Init((HWND)hwnd);

	int res;

	res=pSound->InitSound();
	if(res!=0)
		return res;


	return 0;	
}

void
Release(){
	if(pGraphic!=NULL)
		delete pGraphic;

	if(pInput!=NULL)
		delete pInput;

	if(pSound!=NULL)
		delete pSound;	
}

void
ShutDown(void){
	if(pGraphic!=NULL)
		pGraphic->CloseWindow();
}