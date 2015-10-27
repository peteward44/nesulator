

#ifndef APU_SDL_H
#define APU_SDL_H

#include "APU/nes_apu/Nes_Apu.h"
#include "APU/nes_apu/Blip_Buffer.h"

#include "APU/Sound_Queue.h"
#include "synchroniser.h"


const size_t APUOutBufferSize = 4096;
const int APUBaseRate = 1789773;

// Audio Processing Unit, using SDL_Sound


class APU_SDL : public Synchronisable, public HasState
{
private:
	boost::signals::connection resetConnection;

	Nes_Apu apu;
	Blip_Buffer buf;
	Sound_Queue* soundQueue;

	int masterTickCount, nextIrq;
	int mLastCalculatedNextIrqTime;

	blip_sample_t outBuffer[ APUOutBufferSize ];

	static int DMCRead( void* tag, cpu_addr_t addr );
	static void IRQChanged( void* tag );

	void CalculateWhenIrqDue();
	void TriggerIrqIfNecessary( int cpuMTC );

public:
	APU_SDL();
	~APU_SDL();

	void OnReset( bool cold );

	void WriteToRegister( UInt16_t offset, Byte_t data );
	Byte_t ReadFromRegister( UInt16_t offset );

	void SetClockRate( float fps );

	virtual void Synchronise( int syncMTC );
	virtual int GetNextEvent( int cpuMTC );
	virtual void OnEndFrame();

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );
};


#endif
