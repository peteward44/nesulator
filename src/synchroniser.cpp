

#include "synchroniser.h"
#include "main.h"
#include "mainboard.h"
#include <cassert>
#include <algorithm>
#include <boost/bind/bind.hpp>

using namespace boost::placeholders;

Synchroniser::Synchroniser()
{
	cpuMtc = currentNextEvent = 0;
	mIsSynchronising = false;
	mBufferedSync = false;
	memset( objects, 0, sizeof( Synchronisable* ) * SYNC_COUNT );

	resetConnection = g_nesMainboard->ResetEvent.connect( boost::bind( &Synchroniser::OnReset, this, _1 ) );
}


Synchroniser::~Synchroniser()
{
	resetConnection.disconnect();
}


void Synchroniser::OnReset( bool cold )
{
	cpuMtc = currentNextEvent = 0;
	mIsSynchronising = false;
}


void Synchroniser::RegisterListener( int id, Synchronisable* syncObject )
{
	assert( id >= 0 && id < SYNC_COUNT );
	objects[ id ] = syncObject;
}


void Synchroniser::Synchronise( int ticks )
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

	assert( runto >= 0 );

	int frameEnd = g_nesMainboard->GetModeConstants()->FrameScanlines() * MASTER_CYCLES_PER_PPU * PPU_TICKS_PER_SCANLINE;

	// run all other components to the cpu mtc
	for ( int i=0; i<SYNC_COUNT; ++i )
	{
		if ( objects[i] != NULL )
			objects[i]->Synchronise( runto );
 	}

	// do end frame stuff if that time has come
	if ( runto >= frameEnd )
	{
		for ( int i=0; i<SYNC_COUNT; ++i )
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


int Synchroniser::GetNextEvent( int frameTicks )
{
	int smallest = frameTicks;
	int cpuMtc = this->GetCpuMTC();

	for ( int i=0; i<SYNC_COUNT; ++i )
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


void Synchroniser::NewNextEvent( int nextEvent )
{
	const bool validCurrentNextEvent = currentNextEvent > 0;
	const bool newEventSoonerThanCurrentEvent = nextEvent < currentNextEvent;
	const bool newEventNotTooLate = nextEvent >= this->GetCpuMTC();

	bool nextEventChanged = false;

	if ( !validCurrentNextEvent )
	{
		currentNextEvent = nextEvent;
		nextEventChanged = true;
	}
	else if ( newEventSoonerThanCurrentEvent )
	{
		if ( newEventNotTooLate )
		{
			currentNextEvent = nextEvent;
			nextEventChanged = true;
		}
		else
		{
			const int nextEventNextFrame = nextEvent + g_nesMainboard->GetModeConstants()->FrameScanlines() * MASTER_CYCLES_PER_PPU * PPU_TICKS_PER_SCANLINE;
			if ( nextEventNextFrame < currentNextEvent )
			{
				currentNextEvent = nextEventNextFrame;
				nextEventChanged = true;
			}
		}
	}

	if ( nextEventChanged )
	{
		//#ifdef LOG_PROCESSOR_INSTRUCTIONS
		//if ( Log::IsTypeEnabled( LOG_MISC ) )
		//	Log::Write( LOG_MISC, ( boost::format( "Synchroniser next event changed to '%1%' current '%2%'" ) % currentNextEvent % g_nesMainboard->GetSynchroniser()->GetCpuMTC() ).str().c_str() );
		//#endif
	}
}


void Synchroniser::RunCycle()
{
	int frameEnd = g_nesMainboard->GetModeConstants()->FrameScanlines() * MASTER_CYCLES_PER_PPU * PPU_TICKS_PER_SCANLINE;
	currentNextEvent = GetNextEvent( frameEnd );

	// run cpu
	while ( cpuMtc < currentNextEvent )
	{
		int cpuTicks = g_nesMainboard->GetProcessor()->HandlePendingInterrupts( g_nesMainboard->GetCPUMemory() );
		if ( cpuTicks == 0 )
			cpuTicks = g_nesMainboard->GetProcessor()->ExecuteInstructionFast( g_nesMainboard->GetCPUMemory() );
		g_nesMainboard->GetPPU()->HandleSpriteTransfer();

		cpuMtc += cpuTicks * g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU();
	}

	// run all other components to the cpu mtc
	Synchronise( -1 );

	if ( mBufferedSync )
		Synchronise( -1 );
	mBufferedSync = false;
}


void Synchroniser::AdvanceCpuMTC( int advance )
{
	cpuMtc += advance;
}


int Synchroniser::GetCpuMTC() const
{
	return cpuMtc + g_nesMainboard->GetProcessor()->GetSubCycle() * g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU();
}


void Synchroniser::SaveState( std::ostream& ostr )
{
	WriteType< boost::uint32_t >( ostr, cpuMtc );
	WriteType< boost::uint32_t >( ostr, currentNextEvent );
}


void Synchroniser::LoadState( std::istream& istr )
{
	cpuMtc = ReadType< boost::uint32_t >( istr );
	currentNextEvent = ReadType< boost::uint32_t >( istr );
}
