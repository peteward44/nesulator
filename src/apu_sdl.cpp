

#include "main.h"
#include "apu_sdl.h"
#include "mainboard.h"
#include "APU/nes_apu/apu_snapshot.h"

using namespace boost::placeholders;

// static

int APU_SDL::DMCRead( void* tag, cpu_addr_t addr )
{
	return g_nesMainboard->GetCPUMemory()->Read8( addr );
}


void APU_SDL::IRQChanged( void* tag )
{
	// time for the irq has changed
	APU_SDL* apu = (APU_SDL*)tag;
	apu->CalculateWhenIrqDue();

	//if ( Log::IsTypeEnabled( LOG_MISC ) )
	//	Log::Write( LOG_MISC, ( boost::format( "IRQ time changed on APU: %1%" ) % apu->nextIrq ).str() );
}


///


APU_SDL::APU_SDL()
{
	nextIrq = -1;
	mLastCalculatedNextIrqTime = -1;
	masterTickCount = 0;

	soundQueue = new Sound_Queue();
	if ( !soundQueue )
		throw std::runtime_error( "Error creating Sound_Queue object" );
	
	if ( soundQueue->init( SOUND_RATE ) )
		throw std::runtime_error( ( boost::format( "Error setting sound rate of %1% KHz" ) % SOUND_RATE ).str() );

	buf.clock_rate( APUBaseRate );
	apu.output( &buf );

	blargg_err_t error = buf.sample_rate( SOUND_RATE );
	if ( error )
		throw std::runtime_error( ( boost::format( "Error setting buffer sound rate of %1% KHz" ) % SOUND_RATE ).str() );

	apu.dmc_reader( DMCRead, this );
	apu.irq_notifier( IRQChanged, this );

	g_nesMainboard->GetSynchroniser()->RegisterListener( SYNC_APU, this );
	resetConnection = g_nesMainboard->ResetEvent.connect( boost::bind( &APU_SDL::OnReset, this, _1 ) );
}


void APU_SDL::OnReset( bool cold )
{
	masterTickCount = 0;
	nextIrq = -1;
	apu.reset( !g_nesMainboard->GetModeConstants()->IsNTSC() );
	//if ( !cold )
	//{
	//	WriteToRegister( 0x4015, 0 );
	//}
	//else
	//{
	//	WriteToRegister( 0x4015, 0 );
	//	WriteToRegister( 0x4017, 0 );

	//	for ( int i=0; i<0x10; ++i )
	//		WriteToRegister( 0x4000 + i, 0 );
	//}
}


APU_SDL::~APU_SDL()
{
	resetConnection.disconnect();

	delete soundQueue;
	soundQueue = NULL;
}


void APU_SDL::WriteToRegister( UInt16_t offset, Byte_t data )
{
	if ( offset >= apu.start_addr && offset <= apu.end_addr )
	{
		g_nesMainboard->GetSynchroniser()->Synchronise();
		apu.write_register( g_nesMainboard->GetSynchroniser()->GetCpuMTC() / g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU(), offset, data );
	}
}


Byte_t APU_SDL::ReadFromRegister( UInt16_t offset )
{
	if ( offset == apu.status_addr )
	{
		g_nesMainboard->GetSynchroniser()->Synchronise();
		return apu.read_status( g_nesMainboard->GetSynchroniser()->GetCpuMTC() / g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU() );
	}
	else
	{
		Log::Write( LOG_ERROR, ( boost::wformat( L"Read from unknown APU register 0x%1$X - Emulation may be inaccurate and problematic" ) % offset ).str() );
		return 0;
	}
}


void APU_SDL::OnEndFrame()
{
	int cputicks = g_nesMainboard->GetSynchroniser()->GetCpuMTC() / g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU();

	apu.end_frame( cputicks );
	buf.end_frame( cputicks );

	// Read some samples out of Blip_Buffer if there are enough to
	// fill our output buffer
	if ( buf.samples_avail() >= APUOutBufferSize )
	{
		size_t count = buf.read_samples( outBuffer, APUOutBufferSize );

		if ( g_options->SoundEnabled && g_options->ApplicationSpeed == 0 ) // dont play sound if disabled or not running at normal speed
			soundQueue->write( outBuffer, (int)count );
	}

	masterTickCount = 0;
	CalculateWhenIrqDue();
}


void APU_SDL::CalculateWhenIrqDue()
{
//	if ( !g_nesMainboard->GetProcessor()->GetStatusRegister().Interrupt )
	{
		const cpu_time_t earliestIrq = apu.earliest_irq();
		if ( earliestIrq != Nes_Apu::no_irq )
		{
			const int ticksTillNextIrq = earliestIrq * g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU();
			nextIrq = /*masterTickCount +*/ ticksTillNextIrq;

			if ( nextIrq >= 0 )
			{
//				if ( Log::IsTypeEnabled( LOG_MISC ) )
//					Log::Write( LOG_MISC, ( boost::format( "Recalculating IRQ time, current '%1%' next '%2%'" ) % masterTickCount % nextIrq ).str() );

				g_nesMainboard->GetSynchroniser()->NewNextEvent( nextIrq );
				if ( mLastCalculatedNextIrqTime != nextIrq )
				{
					mLastCalculatedNextIrqTime = nextIrq;
					//if ( nextIrq == 0 )
					//{
					//	int g = 0;
					//}
				}
			}
		}
		else
		{
			nextIrq = -1;
	//		g_nesMainboard->GetProcessor()->HoldIRQLineLow( false );
		}
	}
//	else
//		nextIrq = -1;

	//if ( nextIrq > g_mainboard->GetModeConstants()->FrameScanlines() * MASTER_CYCLES_PER_PPU * PPU_TICKS_PER_SCANLINE )
	//	nextIrq -= g_mainboard->GetModeConstants()->FrameScanlines() * MASTER_CYCLES_PER_PPU * PPU_TICKS_PER_SCANLINE;
}


void APU_SDL::SetClockRate( float fps )
{
//	buf.clock_rate( ( APUBaseRate * NTSC_REFRESHRATE ) / fps );
}


void APU_SDL::Synchronise( int syncMTC )
{
	const int cpuClocks = ( syncMTC / g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU() ) - 1;
	apu.run_until( cpuClocks >= 0 ? cpuClocks : 0 );
//	CalculateWhenIrqDue();
	TriggerIrqIfNecessary( syncMTC );
	masterTickCount = syncMTC;
}


void APU_SDL::TriggerIrqIfNecessary( int cpuMTC )
{
	if ( nextIrq < 0 )
	{
//		g_nesMainboard->GetProcessor()->HoldIRQLineLow( false );
	}
	else if ( nextIrq >= 0 && cpuMTC >= nextIrq && ( masterTickCount < nextIrq || nextIrq == 0 ) )
	{
//		if ( Log::IsTypeEnabled( LOG_MISC ) )
//			Log::Write( LOG_MISC, ( boost::format( "IRQ enabled on APU: %1% %2%" ) % cpuMTC % nextIrq ).str() );
//		g_nesMainboard->GetProcessor()->HoldIRQLineLow( true );
	}
}


int APU_SDL::GetNextEvent( int cpuMTC )
{
//	if ( !g_nesMainboard->GetProcessor()->GetStatusRegister().Interrupt )
	{

		//if ( Log::IsTypeEnabled( LOG_MISC ) )
		//	Log::Write( LOG_MISC, ( boost::format( "IRQ time currently: %1%" ) % nextIrq ).str() ); 

		return cpuMTC < nextIrq ? nextIrq : -1;
	}
//	else
//		return -1;
}


void APU_SDL::SaveState( std::ostream& ostr )
{
	apu_snapshot_t snapshot;
	apu.save_snapshot( &snapshot );
	ostr.write( reinterpret_cast< const char* >( &snapshot ), sizeof( snapshot ) );
}


void APU_SDL::LoadState( std::istream& istr )
{
	apu_snapshot_t snapshot;
	istr.read( reinterpret_cast< char* >( &snapshot ), sizeof( snapshot ) );
	apu.load_snapshot( snapshot );
}

