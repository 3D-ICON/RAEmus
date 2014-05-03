/******************************************************************************
Ootake
�EDirect3D�ł̕`��ɂ��Ή������BVista�Ή��B
�EWindow�\���ɂ�DirectDraw���g���悤�ɂ����B
�E���񂵋@�\��t�����B

Copyright(C)2006-2009 Kitao Nakamura.
	�����ŁE��p�ł����J�Ȃ���Ƃ��͕K���\�[�X�R�[�h��Y�t���Ă��������B
	���̍ۂɎ���ł��܂��܂���̂ŁA�ЂƂ��Ƃ��m�点����������ƍK���ł��B
	���I�ȗ��p�͋ւ��܂��B
	���Ƃ́uGNU General Public License(��ʌ��O���p�����_��)�v�ɏ����܂��B

*******************************************************************************
	[Screen.c]

	Implement ScreenInterface.

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/
#include <stdio.h>
#include <math.h> //pow()�֐��ŕK�v
#include "Screen.h"
#include "ScreenDD.h"
#include "ScreenD3D.h"
//#include "GDIScreen.h"
#include "TIMER.h"
#include "VDC.h"
#include "CDROM.h"
#include "APU.h"
#include "App.h"
#include "MainBoard.h"
#include "WinMain.h"
#include "Printf.h"

Uint32 _Gamma[8]; //Kitao�ǉ��B�K���}���v�Z�������l�����Ă����Bv1.14�������BUint32�ɂ����ق����������������B
Uint32 _GammaS80[8]; //Kitao�ǉ��B�X�L�������C��80%�p
Uint32 _GammaS90[8]; //Kitao�ǉ��B�X�L�������C��90%�p
Uint32 _MonoTableR[256]; //���m�N���ϊ��p�e�[�u���B�������̂��ߕK�v�Bv2.28
Uint32 _MonoTableG[256]; //
Uint32 _MonoTableB[256]; //

static Sint32	_Width;
static Sint32	_Height;
static Sint32	_Magnification;	//������(Screen.cpp)�ł�_Magnification�̓X�N���[���V���b�g�p�\�������̂Ƃ��́A����ƈ�v���Ȃ����Ƃ�����̂Œ��ӁBv2.28�L
static Sint32	_BitsPerPixel;  //������(Screen.cpp)�ł�_BitsPerPixel�́A"DirectDraw�t���X�N���[���J���[�̐ݒ�"�̒l�ł����āA���ݕ\������BitsPerPixel�ƈ�v���Ă���Ƃ͌���Ȃ��̂Œ��ӁBv2.28�L
static Uint32	_Flags;

//Kitao�ǉ��B���A�v���̃E�B���h�E�̏�Ԃ�ۑ����Ă������߂̕ϐ��Bv2.24
static HWND				_OtherAppWindowHWnd[512];
static WINDOWPLACEMENT	_OtherAppWindowPlacement[512];
static Sint32			_OtherAppWindowN;


//Kitao�ǉ��B�E�B���h�E�ʒu�ۑ��̂��߂̃R�[���o�b�N�Bv2.24
static BOOL
CALLBACK EnumWindowsSaveProc(HWND hWnd, LPARAM lParam)
{
	if ((IsWindowVisible(hWnd))&&(hWnd != WINMAIN_GetHwnd()))
	{
		if (_OtherAppWindowN < 512)
		{
			_OtherAppWindowHWnd[_OtherAppWindowN] = hWnd;
			_OtherAppWindowPlacement[_OtherAppWindowN].length = sizeof(WINDOWPLACEMENT);
			GetWindowPlacement(hWnd, &_OtherAppWindowPlacement[_OtherAppWindowN]);
			_OtherAppWindowN++;
		}
	}
	return TRUE;
}

//Kitao�ǉ��B�E�B���h�E�ʒu�ۑ��Bv2.24
void
SCREEN_SaveWindowPosition()
{
	_OtherAppWindowN = 0;
	EnumWindows(EnumWindowsSaveProc, NULL);
	//PRINTF("WindowN %d",_OtherAppWindowN); //test
}

//Kitao�ǉ��B�E�B���h�E�ʒu��߂����߂̃R�[���o�b�N�Bv2.24
static BOOL
CALLBACK EnumWindowsLoadProc(HWND hWnd, LPARAM lParam)
{
	int		i;	

	for (i=0; i<_OtherAppWindowN; i++)
		if (_OtherAppWindowHWnd[i] == hWnd)
		{
			SetWindowPlacement(hWnd, &_OtherAppWindowPlacement[i]);
			break;
		}
	return TRUE;
}

//Kitao�ǉ��B�E�B���h�E�ʒu��߂��Bv2.24
void
SCREEN_LoadWindowPosition()
{
	EnumWindows(EnumWindowsLoadProc, NULL);
}


/*-----------------------------------------------------------------------------
	[Init]
		�X�N���[�����[�h��������(�ύX)���܂��B Kitao�X�V�Bv2.28
-----------------------------------------------------------------------------*/
BOOL
SCREEN_Init(
	Sint32		width,
	Sint32		height,
	Sint32		magnification, //Kitao�ǉ�
	Uint32		bitsPerPixel,
	Uint32		flags)
{
	BOOL	ret;
	Uint32	i;

	_Width = width;
	_Height = height;
	_Magnification = magnification; //Kitao�ǉ�
	_BitsPerPixel = bitsPerPixel;
	_Flags = flags;

	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_Deinit();
			ret = SCREEND3D_Init(_Width, _Height, _Magnification, _Flags);
			break;
		case 2: //DirectDraw
			SCREENDD_Deinit();
			ret = SCREENDD_Init(_Width, _Height, _Magnification, _BitsPerPixel, _Flags);
			break;
		default:
			ret = FALSE;
			break;
	}

	//���m�N���ϊ��p�e�[�u�����쐬�Bv2.28�ǉ�
	if ((APP_GetDrawMethod() == 2)&&(SCREENDD_GetBitsPerPixel() == 16))
	{	//16bit�J���[�iDirectDraw�̂݁j
		for (i=0; i<32; i++)
		{
			//R,G,B�̋P�x�𕽋ω����ă��m�N����
			_MonoTableR[i] = (Uint32)((pow((i * 0.298912) / 32.0, 1.0/1.076900) * 32.0)); //����ʂ̈Â���}���邽�߃K���}���グ��B
			_MonoTableG[i] = (Uint32)((pow((i * 0.586611) / 32.0, 1.0/1.076900) * 32.0)); //��RGB�������Ƃ��ɒl�I�[�o�[���Ȃ����߂ɏ����_�ȉ��͐؂�̂āB
			_MonoTableB[i] = (Uint32)((pow((i * 0.114478) / 32.0, 1.0/1.076900) * 32.0)); //��16bit�͐؂�̂Ă�ꂽ�Ԃ�̈Â����傫���̂ł������l�����ăK���}������B
		}
	}
	else
	{	//32bit�J���[
		for (i=0; i<256; i++)
		{
			//R,G,B�̋P�x�𕽋ω����ă��m�N����
			_MonoTableR[i] = (Uint32)((pow((i * 0.298912) / 256.0, 1.0/1.0752080) * 256.0)); //����ʂ̈Â���}���邽�߃K���}���グ��B
			_MonoTableG[i] = (Uint32)((pow((i * 0.586611) / 256.0, 1.0/1.0752080) * 256.0)); //��RGB�������Ƃ��ɒl�I�[�o�[���Ȃ����߂ɏ����_�ȉ��͐؂�̂āB
			_MonoTableB[i] = (Uint32)((pow((i * 0.114478) / 256.0, 1.0/1.0752080) * 256.0)); //
		}
	}

	return ret;
}


/*-----------------------------------------------------------------------------
	[Deinit]
		�X�N���[���̏I���������s�Ȃ��܂��B
-----------------------------------------------------------------------------*/
void
SCREEN_Deinit()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_Deinit();
			break;
		case 2: //DirectDraw
			SCREENDD_Deinit();
			break;
	}
}


/*-----------------------------------------------------------------------------
	[ToggleFullScreen]
		�X�N���[�����E�C���h�E�^�t���X�N���[���ɐ؂�ւ��܂��D
-----------------------------------------------------------------------------*/
BOOL
SCREEN_ToggleFullScreen()
{
	if (_Flags & SCREEN_FFULLSCREEN)
		_Flags &= ~SCREEN_FFULLSCREEN;
	else
		_Flags |= SCREEN_FFULLSCREEN;
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_Deinit();
			return SCREEND3D_Init(_Width, _Height, _Magnification, _Flags);
		case 2: //DirectDraw
			SCREENDD_Deinit();
			return SCREENDD_Init(_Width, _Height, _Magnification, _BitsPerPixel, _Flags);
		default:
			return FALSE;
	}
}


/*-----------------------------------------------------------------------------
	[WaitVBlank]
		�����A�����Ԃ�҂��܂��B 
-----------------------------------------------------------------------------*/
//Kitao�X�V
BOOL
SCREEN_WaitVBlank(
	BOOL	bDraw) //bDraw��TRUE�ɂ��ČĂԂƕ`����s���BFALSE�̏ꍇVSync�҂��̂݁BDirect3D���p���p�BKitao�ǉ��B
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			return SCREEND3D_WaitVBlank(bDraw);
		case 2: //DirectDraw
			return SCREENDD_WaitVBlank(FALSE); //DirectDraw�̂Ƃ��͏�ɕ`��͍s���Ȃ��B
		default:
			return FALSE;
	}
}


void*
SCREEN_GetBuffer()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			return SCREEND3D_GetBuffer();
		case 2: //DirectDraw
			return SCREENDD_GetBuffer();
		default:
			return NULL;
	}
}


const Sint32
SCREEN_GetBufferPitch()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			return SCREEND3D_GetBufferPitch();
		case 2: //DirectDraw
			return SCREENDD_GetBufferPitch();
		default:
			return 0;
	}
}


/*-----------------------------------------------------------------------------
	[FillRect]
		�o�b�N�o�b�t�@�Ɏw��̐F�̋�`��`���܂��B
	�ĂԑO�� SCREEN_Lock() ���܂��傤�B
-----------------------------------------------------------------------------*/
void
SCREEN_FillRect(
	Sint32		x,
	Sint32		y,
	Sint32		width,
	Sint32		height,
	Uint32		color)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_FillRect(x, y, width, height, color);
			break;
		case 2: //DirectDraw
			SCREENDD_FillRect(x, y, width, height, color);
			break;
	}
}


//Kitao�ǉ��B�X�N���[���S�̂��N���A����Bv1.43
void
SCREEN_Clear()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_Clear();
			break;
		case 2: //DirectDraw
			SCREENDD_Clear();
			break;
	}
}


/*-----------------------------------------------------------------------------
	[Blt]
		pSrc ����o�b�N�o�b�t�@�։摜���������݂܂��B�g��^�k���A
	�ĂԑO�� SCREEN_Lock() ���܂��傤�B
-----------------------------------------------------------------------------*/
//Kitao�X�V�B���C�����Ƃɉ𑜓x��ς��Ă���Q�[��(���Ղ̌��C������120%�Ȃ�)�ɑΉ��B
void
SCREEN_Blt(
	Uint32*		pSrc,
	Sint32		srcX,
	Sint32		srcY,
	Uint16*		pSrcW,	//Kitao�X�V�B�]�����̉��s�N�Z�����B��srcH���C���̐��Ԃ�
	Sint32		srcH,	//Kitao�X�V�BdstW��dstH �̓J�b�g�����B(�����ŌŒ肹���A�l�X�ȑ傫���ł̃y�[�X�g�ɑΉ����邽��)
	Sint32		executeCode)  //Kitao�ǉ��B���s�R�[�h�B0�c�G���R�[�h�����s���B1�c�v���C�}����ʂ֓]�����s���B
							  //					   3�c���E�ɍ���(�I�[�o�[�X�L������)��z�u���Ă̓]��(���Ƃ�1�Ɠ���)
							  //					   5�c���E�̃I�[�o�[�X�L���������J�b�g���Ă̓]��(���Ƃ�1�Ɠ���)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_Blt(pSrc, srcX, srcY, pSrcW, srcH, executeCode);
			break;
		case 2: //DirectDraw
			SCREENDD_Blt(pSrc, srcX, srcY, pSrcW, srcH, executeCode);
			break;
	}
}


//Kitao�ǉ��BVSync(�����A���҂�)���s�����ǂ�����ݒ�B���݂̃f�B�X�v���C�\������VSync���s���邩�ǂ����̃`�F�b�N���s���B
void
SCREEN_SetSyncTo60HzScreen(
	BOOL	bSyncTo60HzScreen)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_SetSyncTo60HzScreen(bSyncTo60HzScreen);
			break;
		case 2: //DirectDraw
			SCREENDD_SetSyncTo60HzScreen(bSyncTo60HzScreen);
			break;
	}
}

//Kitao�ǉ��BVSync(�����A���҂�)���s���Ă��邩�ǂ����𓾂�i���݂̃f�B�X�v���C�\������VSync���s���邩�ǂ����̃`�F�b�N�𔽉f�����l�j�B
BOOL
SCREEN_GetSyncTo60HzScreen()
{
	BOOL	bSyncTo60HzScreen;

	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			bSyncTo60HzScreen = SCREEND3D_GetSyncTo60HzScreen();
			break;
		case 2: //DirectDraw
			bSyncTo60HzScreen = SCREENDD_GetSyncTo60HzScreen();
			break;
	}

	return bSyncTo60HzScreen;
}


//Kitao�ǉ��B�e�L�X�g���b�Z�[�W��ݒ�
void
SCREEN_SetMessageText(
	char*	pText)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_SetMessageText(pText);
			break;
		case 2: //DirectDraw
			SCREENDD_SetMessageText(pText);
			break;
	}
}


//Kitao�ǉ��B�K���}�i���邳�����j���v�Z�ς݂̃e�[�u����p�ӁBv2.28�X�V�BDirect3D��DirectDraw�ŋ��p�ɂ����B
void
SCREEN_SetGamma(
	Sint32	scanLineType,
	Sint32	scanLineDensity, //�X�L�������C���̔Z�x(%)
	BOOL	bTvMode)
{
	Sint32	magnification;
	Sint32	bitsPerPixel;
	int 	a,i;
	double	d = APP_GetGammaValue(); //�c���X�L�������C�����̊�{�K���}�l
	Sint32	b = APP_GetBrightValue(); //�u���C�g�l�X

	//��{�ݒ�ł͂Ȃ��A���ݎ��ۂɕ\������Ă���Magnification��BitsPerPixel���擾����Bv2.28
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			magnification = SCREEND3D_GetMagnification();
			bitsPerPixel = 32; //32bit�Œ�
			break;
		case 2: //DirectDraw
			magnification = SCREENDD_GetMagnification();
			bitsPerPixel = SCREENDD_GetBitsPerPixel();
			break;
		default:
			magnification = 2;
			bitsPerPixel = 32;
			break;
	}

	if (bTvMode)
	{
		if ((scanLineType != 0)&&(magnification >= 2)) //TV Mode ��x2,x3,x4�̂Ƃ�
			scanLineType = 4;
	}
	else
	{
		if (scanLineType == 4) //�X�^�[�gTV Mode ���ݒ肵�Ă����āATvMode���ꎞ�I��FALSE�̏�ԂȂ�
			scanLineType = 1; //�X�y�V����(�c��)�X�L�������C���ŕ`��
	}

	if ((scanLineType != 0)&&(magnification >= 2)) //�X�L�������C���̏ꍇ�A�X�L�������C���ňÂ��Ȃ�Ԃ�A�K���}�𖾂�߂ɁB
	{
		switch (scanLineType)
		{
			case 1: //�c���X�L�������C��
				break;
			case 2: //�������X�L�������C��
			case 3: //�c�����X�L�������C��(������)
			case 4: //TV Mode
				d = (1-15/800)*d; // (1-15/800)*1.305 �c�����Ɣ�ׂĖ��邢�Ԃ������
				if (APP_GetOptimizeGamma())
					d = d * (1+(80-(double)scanLineDensity)*0.005); // �_�E���������邳�����グ��Bv2.35�X�V
				break;
			default:
				d = (1-95/800)*d;
				break;
		}
	}
	else //�m���X�L�������C����������x1�̏ꍇ
		d = (1-95/800)*d; // (1-95/800)*1.305 �c�����Ɣ�ׂĖ��邢�Ԃ������

	if ((scanLineType>=2)&&(scanLineType<=4)) //���X�L�������C���̏ꍇ�Bv2.35�X�V
	{
		if (bitsPerPixel == 16)
		{
			for (i=0; i<=7 ; i++)
			{
				if (i == 0)
					a = 0; //���͐^������
				else
					a = (i << 2) + b; //+1�B��������������͈͂ŏ����߂ɂ����ق����ڂɂ����Ȃ��B
				_Gamma[i] = (Uint32)((pow((double)a / 32.0, 1.0/d) * 32.0) +0.5);
				_GammaS80[i] = (Uint32)((pow((double)a*((double)scanLineDensity / 100) / 32.0, 1.0/d) * 32.0) +0.5); //�X�L�������C���{���p
				_GammaS90[i] = (Uint32)((pow((double)a*((double)(scanLineDensity+(100-scanLineDensity)/2) / 100) / 32.0, 1.0/d) * 32.0) +0.5); //�X�L�������C���ƃh�b�g�̋��E�p
			}
		}
		else //32�r�b�g�J���[�̏ꍇ
		{
			for (i=0; i<=7 ; i++)
			{
				if (i == 0)
					a = 0; //���͐^������
				else
					a = (i << 5) + b; //+1�B��������������͈͂ŏ����߂ɂ����ق����ڂɂ����Ȃ��B
				_Gamma[i] = (Uint32)((pow((double)a / 256.0, 1.0/d) * 256.0) +0.5);
				_GammaS80[i] = (Uint32)((pow((double)a*((double)scanLineDensity / 100) / 256.0, 1.0/d) * 256.0) +0.5); //�X�L�������C���{���p
				_GammaS90[i] = (Uint32)((pow((double)a*((double)(scanLineDensity+(100-scanLineDensity)/2) / 100) / 256.0, 1.0/d) * 256.0) +0.5); //�X�L�������C���ƃh�b�g�̋��E�p
			}
		}
	}
	else //���X�L�������C���ȊO�̏ꍇ�B���X�L�������C��80%����Ƃ���Bv2.35�X�V
	{
		if (bitsPerPixel == 16)
		{
			for (i=0; i<=7 ; i++)
			{
				a = i*4 + (Sint32)((double)b/9 * ((double)i/7) + 0.5); //���͐^�����ɁB���͔���������͈͂ŏ����߂ɂ����ق����ڂɂ����Ȃ��Bv2.35�X�V
				_Gamma[i] = (Uint32)((pow((double)a / 32.0, 1.0/d) * 32.0) +0.5);
				_GammaS80[i] = (Uint32)((pow((double)a*0.80 / 32.0, 1.0/d) * 32.0) +0.5); //�X�L�������C��80%�p�i�X�L�������C���{���j
				_GammaS90[i] = (Uint32)((pow((double)a*0.90 / 32.0, 1.0/d) * 32.0) +0.5); //�X�L�������C��90%�p�i�X�L�������C���ƃh�b�g�̋��E�p�j
			}
		}
		else //32�r�b�g�J���[�̏ꍇ
		{
			for (i=0; i<=7 ; i++)
			{
				a = i*32 + (Sint32)((double)b * ((double)i/7) + 0.5); //���͐^�����ɁB���͔���������͈͂ŏ����߂ɂ����ق����ڂɂ����Ȃ��Bv2.35�X�V
				_Gamma[i] = (Uint32)((pow((double)a / 256.0, 1.0/d) * 256.0) +0.5);
				_GammaS80[i] = (Uint32)((pow((double)a*0.80 / 256.0, 1.0/d) * 256.0) +0.5); //�X�L�������C��80%�p�i�X�L�������C���{���j
				_GammaS90[i] = (Uint32)((pow((double)a*0.90 / 256.0, 1.0/d) * 256.0) +0.5); //�X�L�������C��90%�p�i�X�L�������C���ƃh�b�g�̋��E�p�j
			}
		}
	}
}


//Kitao�ǉ��B�O���VBlank�҂����I�����������Ԃ��B
DWORD
SCREEN_GetLastTimeSyncTime()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			return SCREEND3D_GetLastTimeSyncTime();
		case 2: //DirectDraw
			return SCREENDD_GetLastTimeSyncTime();
		default:
			return 0;
	}
}


//Kitao�ǉ��B�X�N���[���V���b�g��Bitmap���������ށBv2.12
void
SCREEN_WriteScreenshot(
	FILE*	fp)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_WriteScreenshot(fp);
			break;
		case 2: //DirectDraw
			SCREENDD_WriteScreenshot(fp);
			break;
	}
}


//Kitao�ǉ��B�`��{����ݒ肷��Bv2.36
void
SCREEN_SetMagnification(
	Sint32	magnification)
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			SCREEND3D_SetMagnification(magnification);
			break;
		case 2: //DirectDraw
			SCREENDD_SetMagnification(magnification);
			break;
	}
}


//Kitao�ǉ��B���ۂɁu�`�揈���Ŏg�p���Ă���v�`��{���𓾂�Bv2.36
Sint32
SCREEN_GetMagnification()
{
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			return SCREEND3D_GetMagnification();
		case 2: //DirectDraw
			return SCREENDD_GetMagnification();
	}
	return 0;
}


//Kitao�ǉ��B�`�掞�̃h�b�g�g�嗦��ݒ肷��B
void
SCREEN_SetPixelMagnification(
	Sint32*		wMag,
	Sint32*		hMag)
{
	Sint32	magnification;

	//��{�ݒ�ł͂Ȃ��A���ݎ��ۂɕ\������Ă���Magnification���擾����Bv2.28
	switch (APP_GetDrawMethod())
	{
		case 1: //Direct3D
			magnification = SCREEND3D_GetMagnification();
			break;
		case 2: //DirectDraw
			magnification = SCREENDD_GetMagnification();
			break;
		default:
			magnification = 2;
			break;
	}

	if (magnification >= 2) //x2�ȏ�̏ꍇ
	{
		if (APP_GetScanLineType() != 0)
		{
			if (APP_GetTvMode())
				*wMag = 1;
			else
				*wMag = 2; //x2�ȏ�����͂Q�{�h�b�g�Œ�B�i���x�A�b�v�{�g�傳�ꂽ�Ƃ��Ƀo�C���j�A�t�B���^�����������ɂ�����j
			*hMag = magnification; //�c�͔{���Ԃ�̃\�[�X��p�ӂ��ē]���B
		}
		else
		{
			*wMag = 2;
			if (magnification == 2)
				*hMag = magnification; //�c�͔{���Ԃ�̃\�[�X��p�ӂ��ē]���B
			else
				*hMag = magnification-1; //3x,4x�̂Ƃ��́A���ꂼ��2x,3x�Ɋg��B�i�W���M�[�y�������x�A�b�v�j
		}
	}
	else //x1�̏ꍇ
		*wMag = *hMag = 1;
}


//Kitao�ǉ��B�f�B�X�v���C�̐����������g���𑪒肵�ĕԂ��Bv2.43
Sint32
SCREEN_GetVerticalScanFrequency()
{
	Sint32	vsf = 0;
	Sint32	a;
	DWORD	t, t2;
	BOOL	bHideMessage;

	if (APP_GetFullScreen())
	{
		bHideMessage = APP_GetFullHideMessage(); //�ޔ�
		APP_SetFullHideMessage(FALSE); //���b�Z�[�W��K����ʓ��ɕ\���B
	}
	else
	{
		bHideMessage = APP_GetHideMessage(); //�ޔ�
		APP_SetHideMessage(FALSE); //�\�����x���Ԃɍ����悤�ɁA���b�Z�[�W��K����ʓ��ɕ\���B
	}
	PRINTF("Checking Now... Please wait for 60 seconds.");
	MAINBOARD_ScreenUpdateFast();

	SetCursor(LoadCursor(NULL, IDC_WAIT)); //�J�[�\���������v�ɁB

	SCREEN_WaitVBlank(FALSE); //"�O��VBlank���I���������"���X�V���邽�߂ɕK�v�B
	t = timeGetTime();
	t2 = t + 60000;
	while (t2 < t) //�I���\�莞���̃^�C�}�[�J�E���^���I�[�o�[�t���[���Ă����ꍇ�A�J�n�\�莞���̃^�C�}�[�J�E���^��0�ɖ߂�܂ő҂B
	{
		SCREEN_WaitVBlank(FALSE); //"�O��VBlank���I���������"���X�V���邽�߂ɕK�v�B
		t = timeGetTime();
		t2 = t + 60000;
	}
	
	while (timeGetTime() <= t2)
	{
		vsf++;
		Sleep(12); //12�B�A��V-Sync�����ɂ��v���s�ǂ�h�����߃E�F�C�g���K�v(�Ƃ���V-Sync�ݒ�I�t���ɕK�v)�B11���ƌv���s��(�����茻��)���������B���܂�傫��Sleep������ƒx���}�V���ŋt�ɏ��������ɂ��v���s�ǁB
		SCREEN_WaitVBlank(FALSE);
	}

	vsf -= 6; //�덷������̂��A-6�ł��傤�ǎ��ۂ̎��g���\�L(�f�B�X�v���C��info�{�^��)�Ɠ����ɂȂ����B�S�Ă̊��ł��ꂪ���Ă͂܂�Ȃ��ƈӖ����Ȃ��̂ŗv�m�F�B
	vsf = vsf * 10 / 6;
	//�l�̌ܓ�
	a = vsf % 10; //��̈ʂ�ޔ�
	vsf /= 10; //��̈ʂ��J�b�g
	if (a < 2)
		vsf *= 10;
	else if (a < 7)
		vsf = vsf * 10 + 5;
	else
		vsf = vsf * 10 + 10;

	//���b�Z�[�W�\���ݒ�����ɖ߂��B
	if (APP_GetFullScreen())
		APP_SetFullHideMessage(bHideMessage);
	else
		APP_SetHideMessage(bHideMessage);

	SetCursor(LoadCursor(NULL, IDC_ARROW)); //�J�[�\�������ɖ߂�

	return vsf;
}
