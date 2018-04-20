/******************************************************************************
Ootake
�E"�d<N"�,�,�,�,�-�'�,�<N,�,��?,�,�,�,�,ŁA"�d<N"�,�-hZ~,�,�,�,�,�,�,��B
�ECOM,̏?S�?�,�SJ.�,�,�,�,�,�,�,�,�,�,�,��BVista,�ftf@fCf<f_fCfAf�fOZg-pZz,�
  �?S�?�,�,�,�,�,�,�,�.s^�'�,�,�,��Bv1.05
�Ef}f<f`f�fffBfAf^fCf},̐�"x,�,�,�,ŏ�,�,�,�,�,�,�,�,�,��Bv1.55

Copyright(C)2006-2010 Kitao Nakamura.
    Attach the source code when you open the remodeling version and the
    succession version to the public. and, please contact me by E-mail.
    Business use is prohibited.
	Additionally, it applies to "GNU General Public License". 
	?�'�"ŁEO�Op"�,�O�SJ,�,�,�,�,�,�.K,�f\�[fXfR�[fh,�"Y.t,�,�,�,�,�,��B
	,�,̍�,�Z-O�,�,�,�,�,�,�,�,�,ŁA,�,�,�,�,�'m,�,�,�,�,�,�,�,ƍK,�,�,��B
	��"I,�-~-p,�<�,�,�,��B
	,�,�,́uGNU General Public License(^�"�O��O-~-p<-'�O_-�')�v,ɏ?,�,�,��B

*******************************************************************************
	[main]
		-{fvf�fWfFfNfg,�f�fCf"S֐",�,��D

		The main function of the project.

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

#define _WIN32_DCOM //v2.17�X�V

#include <objbase.h>
#include "App.h"

// gcc ,� main ,�'�<`,�,�,�,�,�,�
// WinMain ,�,�,���,� main ,�O�,�,�,�,�,�,�,�,��D�D�D
// __main__ ,�,�,�,�,�,�,� workaround.
int
__main__(
	int			argc,
	char**		argv)
{
	HANDLE		hMutex;
	TIMECAPS	tc;

	//Kitao'�?��B"�d<N"�,�-hZ~
	hMutex = CreateMutex(NULL, TRUE, "Ootake Emulator"); //f~f.�[fefbfNfX,̍쐬
	if (GetLastError() == ERROR_ALREADY_EXISTS) //,�,�,�Ootake,�<N"�,�,�,�,�,�
		return 0; //<N"�,�,�,ɏI-�

	//CoInitializeEx(NULL, COINIT_MULTITHREADED); //Kitao'�?��Bv2.17�X�V�BZQ�l�FfAfp�[fgf�f"fg(COINIT_APARTMENTTHREADED),�,�?�,���,��d,�S�,�,�,�,�(,�,�,�,�'�,�MTA,�,�STA,�,�,�,��^-�S�Su,�'�,�)�Bv2.19<L
	//CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); //Kitao'�?��Bv2.17�X�V�BZQ�l�FfAfp�[fgf�f"fg(COINIT_APARTMENTTHREADED),�,�?�,���,��d,�S�,�,�,�,�(,�,�,�,�'�,�MTA,�,�STA,�,�,�,��^-�S�Su,�'�,�)�Bv2.19<L
	timeGetDevCaps(&tc, sizeof(tc));
	timeBeginPeriod(tc.wPeriodMin); //Kitao'�?��Bf^fCf}��"x,�,�,�,ŏ�,�,�,�,�,�,�,�,�,��B

	if (!APP_Init(argc, argv))
		return -1;

	while (APP_ProcessEvents() != APP_QUIT);

	APP_Deinit();

	timeEndPeriod(tc.wPeriodMin); //Kitao'�?�
	//CoUninitialize(); //Kitao'�?�

	return 0;
}
