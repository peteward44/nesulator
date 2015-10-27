
#include "stdafx.h"
#include "main.h"
#include "SnesSynchroniser.h"
#include "SnesMainboard.h"



//void SnesSynchroniser::RunCycle()SnesSynchroniser
//{
//	int cpuTicks = g_snesMainboard->GetProcessor()->HandlePendingInterrupts( g_snesMainboard->GetCPUMemory() );
//	if ( cpuTicks == 0 )
//		cpuTicks = g_snesMainboard->GetProcessor()->ExecuteInstructionFast( g_snesMainboard->GetCPUMemory() );
//}
//




SnesSynchroniser::SnesSynchroniser()
{
	cpuMtc = currentNextEvent = 0;
	mIsSynchronising = false;
	mBufferedSync = false;
	memset( objects, 0, sizeof( SnesSynchronisable* ) * SNES_SYNC_COUNT );

	resetConnection = g_snesMainboard->ResetEvent.connect( boost::bind( &SnesSynchroniser::OnReset, this, _1 ) );
}


SnesSynchroniser::~SnesSynchroniser()
{
	resetConnection.disconnect();
}


void SnesSynchroniser::OnReset( bool cold )
{
	cpuMtc = currentNextEvent = 0;
	mIsSynchronising = false;
}


void SnesSynchroniser::RegisterListener( int id, SnesSynchronisable* syncObject )
{
	assert( id >= 0 && id < SNES_SYNC_COUNT );
	objects[ id ] = syncObject;
}


void SnesSynchroniser::Synchronise( int ticks )
{
	if ( mIsSynchronising )
	{
		// don't sync now, but 'buffer' a sync operation to do it again after this sync
		mBufferedSync = true;
		return;
	}

	mIsSynchronising = true;
	int realCpuMtc = GetCpuMTC();

	int runto = ticks > 0 ? ticks : realCpuMtc;
	if ( runto > realCpuMtc )
		runto = realCpuMtc;

	int frameEnd = 0; //g_snesMainboard->GetModeConstants()->FrameScanlines() * MASTER_CYCLES_PER_PPU * PPU_TICKS_PER_SCANLINE;

	// run all other components to the cpu mtc
	for ( int i=0; i<SNES_SYNC_COUNT; ++i )
	{
		if ( objects[i] != NULL )
			objects[i]->Synchronise( runto );
	}

	// do end frame stuff if that time has come
	if ( runto >= frameEnd )
	{
		for ( int i=0; i<SNES_SYNC_COUNT; ++i )
		{
			if ( objects[i] != NULL )
			{
				objects[i]->OnEndFrame();
			}
		}

		cpuMtc -= frameEnd;
		currentNextEvent -= frameEnd;

		g_nesMainboard->StartNewFrame( );
	}

	mIsSynchronising = false;
}


int SnesSynchroniser::GetNextEvent( int frameTicks )
{
	int smallest = frameTicks;
	int cpuMtc = this->GetCpuMTC();

	for ( int i=0; i<SNES_SYNC_COUNT; ++i )
	{
		if ( objects[i] != NULL )
		{
			int val = objects[i]->GetNextEvent( cpuMtc );
			if ( val > 0 && ( smallest < 0 || val < smallest ) )
				smallest = val;
		}
	}

	return smallest;
}


void SnesSynchroniser::NewNextEvent( int nextEvent )
{
	if ( nextEvent < currentNextEvent )
		currentNextEvent = nextEvent;
}


void SnesSynchroniser::RunCycle()
{
	int frameEnd = g_nesMainboard->GetModeConstants()->FrameScanlines() * MASTER_CYCLES_PER_PPU * PPU_TICKS_PER_SCANLINE;
	currentNextEvent = GetNextEvent( frameEnd );

	// run cpu
	while ( cpuMtc < currentNextEvent )
	{
		int cpuTicks = g_nesMainboard->GetProcessor()->HandlePendingInterrupts( g_nesMainboard->GetCPUMemory() );
		if ( cpuTicks == 0 )
			cpuTicks = g_nesMainboard->GetProcessor()->ExecuteInstructionFast( g_nesMainboard->GetCPUMemory() );

		cpuMtc += cpuTicks * g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU();
	}

	// run all other components to the cpu mtc
	Synchronise( -1 );

	if ( mBufferedSync )
		Synchronise( -1 );
	mBufferedSync = false;
}


void SnesSynchroniser::AdvanceCpuMTC( int advance )
{
	cpuMtc += advance;
}


int SnesSynchroniser::GetCpuMTC() const
{
	return cpuMtc;// + g_snesMainboard->GetProcessor()->GetSubCycle() * g_snesMainboard->GetModeConstants()->MasterCyclesPerCPU();
}


void SnesSynchroniser::SaveState( std::ostream& ostr )
{
	WriteType< boost::uint32_t >( ostr, cpuMtc );
	WriteType< boost::uint32_t >( ostr, currentNextEvent );
}


void SnesSynchroniser::LoadState( std::istream& istr )
{
	cpuMtc = ReadType< boost::uint32_t >( istr );
	currentNextEvent = ReadType< boost::uint32_t >( istr );
}
