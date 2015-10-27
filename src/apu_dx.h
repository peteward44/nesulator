///////////////////////////////////////////////////////////////////////////////////
//                                                                               //
//   nemulator (an NES emulator)                                                 //
//                                                                               //
//   Copyright (C) 2003-2005 James Slepicka <james@nemulator.com>                //
//                                                                               //
//   This program is free software; you can redistribute it and/or modify        //
//   it under the terms of the GNU General Public License as published by        //
//   the Free Software Foundation; either version 2 of the License, or           //
//   (at your option) any later version.                                         //
//                                                                               //
//   This program is distributed in the hope that it will be useful,             //
//   but WITHOUT ANY WARRANTY; without even the implied warranty of              //
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the               //
//   GNU General Public License for more details.                                //
//                                                                               //
//   You should have received a copy of the GNU General Public License           //
//   along with this program; if not, write to the Free Software                 //
//   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA   //
//                                                                               //
///////////////////////////////////////////////////////////////////////////////////

#ifndef APU_DX_H
#define APU_DX_H

#ifdef USE_DIRECTSOUND

#pragma comment( lib, "dsound.lib" )
#pragma comment( lib, "winmm.lib" )
#pragma comment( lib, "dxguid.lib" )
 
#include "APU/nes_apu/nes_apu.h"
//#include "APU/nes_apu/sound_writer.hpp"
#include "windows.h"
#include "mmsystem.h"
#include "dsound.h"

#define SAMPLE_BUFFER_SIZE 65536
#define BUFFER_MSEC 100
#define BASERATE 1789773
#define MAX_LO_ADJUST 3
#define MAX_HI_ADJUST 3
#define ADJUST_DELAY 15

class APU_DX
{
public:
	APU_DX(void);
	~APU_DX(void);

	Byte_t ReadFromRegister( UInt16_t offset );
	void WriteToRegister( UInt16_t offset, Byte_t data );

	int EndFrame();
	double BufferedFrames(void);
	
	void SetVolume(long volume);
	DWORD GetBufferSize(void);

private:
	static int DMCRead( void* tag, cpu_addr_t addr );

	double samplesPerFrame;
	WAVEFORMATEX wf;
	blip_time_t time;
	bool active;
	Nes_Apu apu;
	Blip_Buffer buf;
//	Sound_Writer *aiff;
	blip_time_t end_time;
	LPDIRECTSOUND lpDS;
	LPDIRECTSOUNDBUFFER buffer;
	DSBUFFERDESC bufferdesc;
	void Play(void);
	blip_sample_t out_buf[SAMPLE_BUFFER_SIZE];
	DWORD bufferOffset;
	size_t samples_read;
	int CopyBuffer(LPBYTE destBuffer, DWORD destBufferSize);
	short *pOutBuffer;
	int GetMaxWrite(void);
	int loThreshold, hiThreshold;
	
	int writeBufferSize;
};


#endif


#endif

