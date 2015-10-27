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

#include "stdafx.h"
#include "apu_dx.h"
#include "mainboard.h"


#ifdef USE_DIRECTSOUND


int APU_DX::DMCRead( void* tag, cpu_addr_t addr )
{
//	return ((APU*)tag)->ReadFromRegister( addr );
	return g_mainboard->GetCPUMemory()->Read8( addr );
}


APU_DX::APU_DX(void)
{
	buffer = NULL;
	active = false;
	buf.sample_rate(48000);
	buf.clock_rate(BASERATE);

	apu.output(&buf);
	time = 0;

	if (DS_OK != DirectSoundCreate(NULL, &lpDS, NULL))
		MessageBox(NULL, "DirectSoundCreate failed", "", MB_OK);
//	if (DS_OK != IDirectSound_SetCooperativeLevel(lpDS, NULL /*hWnd*/, DSSCL_PRIORITY))
//		MessageBox(NULL, "IDirectSound_SetCooperativeLevel failed", "", MB_OK);


	ZeroMemory(&wf, sizeof(WAVEFORMATEX));
	wf.wFormatTag = WAVE_FORMAT_PCM;
	wf.nChannels = 1;
	wf.nSamplesPerSec = 48000;
	wf.wBitsPerSample = 16;
	wf.nBlockAlign = wf.wBitsPerSample / 8 * wf.nChannels;
	wf.nAvgBytesPerSec = wf.nSamplesPerSec * wf.nBlockAlign;



	bufferdesc.dwSize = sizeof(DSBUFFERDESC); 
	bufferdesc.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_CTRLVOLUME; 
	bufferdesc.dwBufferBytes = (wf.nAvgBytesPerSec * BUFFER_MSEC)/1000;

	bufferdesc.dwReserved = 0; 
	bufferdesc.lpwfxFormat = &wf; 

	if (DS_OK != lpDS->CreateSoundBuffer(&bufferdesc, &buffer, NULL))
	{
		//MessageBox(NULL, "CreateSoundBuffer failed", "", MB_OK);
		return;
	}
	active = true;

	loThreshold = (int)((wf.nAvgBytesPerSec / 60) * 1.5);
	hiThreshold = (int)(wf.nAvgBytesPerSec / 60);

	writeBufferSize = loThreshold;
	bufferOffset = bufferdesc.dwBufferBytes - writeBufferSize;
	samplesPerFrame = wf.nSamplesPerSec / 60.0;
	buffer->SetCurrentPosition(0);
	buffer->Play(0, 0, DSBPLAY_LOOPING);

	apu.dmc_reader( DMCRead, this );
}

DWORD APU_DX::GetBufferSize(void)
{
	return bufferdesc.dwBufferBytes;
}

void APU_DX::SetVolume(long volume)
{
	if (buffer)
		buffer->SetVolume(volume);

}

APU_DX::~APU_DX(void)
{
	buffer->Stop();
	if (lpDS)
		IDirectSound_Release(lpDS);
}


Byte_t APU_DX::ReadFromRegister( UInt16_t offset )
{
	//TODO: don't know what time should be set to
	if ( offset == apu.status_addr )
		return apu.read_status( time );
	else
		return 0;
}

void APU_DX::WriteToRegister( UInt16_t offset, Byte_t data )
{
	if (active && offset >= apu.start_addr && offset <= apu.end_addr)
		apu.write_register( time++, offset, data );
}


int APU_DX::GetMaxWrite(void)
{
	DWORD playCursor;

	buffer->GetCurrentPosition(&playCursor, NULL);

	if (bufferOffset <= playCursor)
		return playCursor - bufferOffset;
	else
		return bufferdesc.dwBufferBytes - bufferOffset + playCursor;
}

int APU_DX::EndFrame()
{
	static int lo = 0;
	static int hi = 0;
	static int mid = 0;

	static int sampleAdjust = 0;
	static int adjustFactor = 0;

	if (!active)
		return 0;
	
	time = 0;
	apu.end_frame(29781);
	buf.end_frame(29781);
	
	int bytesAvailable;
	while (buf.samples_avail())
	{
		bytesAvailable = GetMaxWrite() / 2;
		if (bytesAvailable > SAMPLE_BUFFER_SIZE)
			bytesAvailable = SAMPLE_BUFFER_SIZE;
		bytesAvailable &= 0xfffffffe;

		if (!bytesAvailable)
			continue;
		if (samples_read = buf.read_samples(out_buf, bytesAvailable))
			Play();
		else
			break;
	}
	
	bytesAvailable = GetMaxWrite();
	if (bytesAvailable > loThreshold)
	{
		hi = 0;
		if (lo++ % ADJUST_DELAY == 0)
			sampleAdjust++;
		if (sampleAdjust > MAX_LO_ADJUST)
			sampleAdjust = MAX_LO_ADJUST;
	}
	else if (bytesAvailable < hiThreshold)
	{
		lo = 0;
		if (hi++ % ADJUST_DELAY == 0)
			sampleAdjust--;
		if (sampleAdjust < -MAX_HI_ADJUST)
			sampleAdjust = -MAX_HI_ADJUST;
	}
	else
	{
		if (sampleAdjust != 0)
		{
			if (sampleAdjust > 0)
				sampleAdjust--;
			else
				sampleAdjust++;
			hi = lo = 0;
		}
	}

	double clockAdjustment = BASERATE * samplesPerFrame / (samplesPerFrame + sampleAdjust);
	buf.clock_rate((long)clockAdjustment);

	return sampleAdjust;
}

double APU_DX::BufferedFrames(void)
{
	return (bufferdesc.dwBufferBytes - GetMaxWrite()) / (wf.nAvgBytesPerSec / 60.0);
}

int APU_DX::CopyBuffer(LPBYTE destBuffer, DWORD destBufferSize)
{
	unsigned int i = 0;
	short *p = (short*)destBuffer;

	while (i < destBufferSize && (samples_read--) > 0)
	{
		*p++ = *pOutBuffer++;
		i+=2;
	}
	return i;
}

void APU_DX::Play(void)
{
	HRESULT hr;
	LPBYTE lpbuf1 = NULL;
	LPBYTE lpbuf2 = NULL;
	DWORD dwsize1 = 0;
	DWORD dwsize2 = 0;
	DWORD dwbyteswritten1 = 0;
	DWORD dwbyteswritten2 = 0;

	// Lock the sound buffer
	hr = buffer->Lock (bufferOffset, (DWORD)(samples_read*2), (LPVOID *)&lpbuf1, &dwsize1, (LPVOID *)&lpbuf2, &dwsize2, 0);
	if (hr == DS_OK)
	{
		pOutBuffer = out_buf;
		if ((dwbyteswritten1 = CopyBuffer(lpbuf1, dwsize1)) == dwsize1)
			if (lpbuf2)
				dwbyteswritten2 = CopyBuffer(lpbuf2, dwsize2);


		// Update our buffer offset and unlock sound buffer
		bufferOffset = (bufferOffset + dwbyteswritten1 + dwbyteswritten2) % bufferdesc.dwBufferBytes;

		buffer->Unlock (lpbuf1, dwbyteswritten1, lpbuf2, dwbyteswritten2);

	}
}


#endif

