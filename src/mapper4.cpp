
#include "stdafx.h"
#include "mapper4.h"



Mapper4::Mapper4( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;

	bankSwapByte = 0;
	prgRamDisableWrite = false;
	chipEnable = interruptsEnabled = true;
	irqCounter = irqLatch = 0;
	mReloadFlag = false;

	masterTickCount = lastA12Raise = 0;

	mSpriteAddress = mScreenAddress = false;
	mRenderingEnabled = false;

	banks[6] = 0;
	banks[7] = 1;

	banks[0] = 0;
	banks[1] = 2;
	banks[2] = 4;
	banks[3] = 5;
	banks[4] = 6;
	banks[5] = 7;
}


void Mapper4::OnReset( bool cold )
{
	if ( Get1kChrBankCount() == 0 )
		UseVRAM();

	irqCounter = irqLatch = 0;
	mReloadFlag = false;
	masterTickCount = lastA12Raise = 0;

	SyncBanks();

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper4::DecrementIrqCounter( int tickCount, bool fromRegisterWrite )
{
	if ( fromRegisterWrite && lastA12Raise > 0 && tickCount - lastA12Raise <= 16 * MASTER_CYCLES_PER_PPU )
	{
		return;
	}

	lastA12Raise = tickCount;

	bool forceIrq = false;

	if ( mReloadFlag || irqCounter == 0 )
	{
		forceIrq = mReloadFlag && irqLatch == 0;// MMC3 revA behaviour
		irqCounter = irqLatch;
		mReloadFlag = false;

		if ( forceIrq && interruptsEnabled )
			g_nesMainboard->GetProcessor()->HoldIRQLineLow( true );
	}
	else
	{
		#ifdef LOG_MAPPER_EVENTS
			int x = 0, y = 0;
			g_nesMainboard->GetPPU()->TicksToScreenCoordinates( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), &x, &y );
			if ( Log::IsTypeEnabled( LOG_MAPPER ) )
				Log::Write( LOG_MAPPER, ( boost::format( "IRQ counter decremented '%3%' %1%x%2% (%4%)" ) % x % y % (int)irqCounter % ( fromRegisterWrite ? "A12" : "SL" ) ).str() );
		#endif

		if ( irqCounter > 0 )
			irqCounter--;

		if ( irqCounter == 0 )
		{
		#ifdef LOG_MAPPER_EVENTS
			int x = 0, y = 0;
			g_nesMainboard->GetPPU()->TicksToScreenCoordinates( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), &x, &y );
			if ( Log::IsTypeEnabled( LOG_MAPPER ) )
				Log::Write( LOG_MAPPER, ( boost::format( "IRQ triggered coords %1%x%2% (%3%)" ) % x % y % ( fromRegisterWrite ? "A12" : "SL" ) ).str() );
		#endif

			if ( interruptsEnabled )
				g_nesMainboard->GetProcessor()->HoldIRQLineLow( true );
		}
	}
}


void Mapper4::PPUA12Latch()
{
	g_nesMainboard->GetSynchroniser()->Synchronise();

	DecrementIrqCounter( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), true );
}


int Mapper4::CalculateNextA12Raise( int cpuMTC, int& upperLimit ) const
{
	const int currentScanline = ( cpuMTC / ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU ) ) - g_nesMainboard->GetModeConstants()->VBlankScanlines();

	int pixelEvent = -1;
	int firstScanline = 0;
	if ( mRenderingEnabled )
	{
		if ( mSpriteAddress && !mScreenAddress )
		{
			pixelEvent = 260;// 265;
			firstScanline = 1;
		}
		else if ( mScreenAddress && !mSpriteAddress )
		{
			pixelEvent = 324; // 9;
			firstScanline = 0;
		}
	}
	
	if ( pixelEvent < 0 )
		return -1;

	const int lowerLimit = ( g_nesMainboard->GetModeConstants()->VBlankScanlines() + firstScanline ) * ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU );
	upperLimit = ( g_nesMainboard->GetModeConstants()->FrameScanlines() - 1 + firstScanline ) * ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU );

	if ( cpuMTC >= upperLimit )
		return -1;

	const int modmtc = cpuMTC % ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU ); // ticks from the start of the current scanline
	const int scanlineEvent = MASTER_CYCLES_PER_PPU * pixelEvent; // ticks from the start of the scanline when IRQ is decremented

	int startMtc = cpuMTC - modmtc + scanlineEvent; // ticks till next irq decrement event

	if ( startMtc <= cpuMTC )
		startMtc += PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU; // if we have already passed the irq event, move onto next scanline

	if ( startMtc < lowerLimit )
		startMtc = lowerLimit + scanlineEvent;

	return startMtc;
}


void Mapper4::SpriteScreenEnabledUpdate( bool spriteAddress, bool screenAddress )
{
	mSpriteAddress = spriteAddress;
	mScreenAddress = screenAddress;
	// report new next event
	const int newEvent = GetNextEvent( g_nesMainboard->GetSynchroniser()->GetCpuMTC() );
	if ( newEvent >= 0 )
		g_nesMainboard->GetSynchroniser()->NewNextEvent( newEvent );
}


void Mapper4::RenderingEnabledChanged( bool enabled )
{
	mRenderingEnabled = enabled;
	// report new next event
	const int newEvent = GetNextEvent( g_nesMainboard->GetSynchroniser()->GetCpuMTC() );
	if ( newEvent >= 0 )
		g_nesMainboard->GetSynchroniser()->NewNextEvent( newEvent );
}


void Mapper4::Synchronise( int syncMTC )
{
	/*
The heart of the MMC3. The PPU will cause A12 to rise when it fetches CHR from the right pattern table ($1xxx).
In "normal" conditions (BG uses $0xxx, all sprites use $1xxx), this will occur 8 times per scanline (once for each sprite).
However the BG could also be the culprit (if BG uses $1xxx and all sprites use $0xxx -- ?as seen in Armadillo?), in which case A12 will rise 34 times.
These 42 times per scanline are key times which I call "rise points":

BG rise points: 4, 12, 20, ... , 244, 252
Sp rise points: 260, 268, ..., 308, 316
BG rise points: 324, 332 
	
If sprites are set to $1000-1FFF and the background is set to $0000-0FFF, then A12 will change from 0 to 1 at cycle 260 of each scanline, then change from 1 to 0 at cycle 320 of each scanline.

If sprites are set to $0000-0FFF and the background is set to $1000-1FFF, then A12 will change from 1 to 0 at cycle 256 of each scanline, then change from 0 to 1 at cycle 324 of each scanline. 
*/
	// tickLimit is the start of the rendering frame - only started being clocked when rendering
	int upperLimit = 0;
	const int startMtc = CalculateNextA12Raise( masterTickCount, upperLimit );
	if ( startMtc >= 0 && upperLimit >= 0 )
	{
		for ( int mtc = startMtc; mtc <= std::min( upperLimit, syncMTC ); mtc += PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU )
		{
			DecrementIrqCounter( mtc, false );
		}
	}
	masterTickCount = syncMTC;
}


void Mapper4::OnEndFrame()
{
	masterTickCount = lastA12Raise = 0;
}


int Mapper4::GetNextEvent( int cpuMTC )
{
	int upperLimit = 0;
	return CalculateNextA12Raise( cpuMTC, upperLimit );
}


void Mapper4::SyncBanks()
{
	g_nesMainboard->GetSynchroniser()->Synchronise();

	Switch8kPrgBank( banks[7], 1 );
	Switch8kPrgBank( Get8kPrgBankCount() - 1, 3 );

	if ( IS_BIT_SET( bankSwapByte, 6 ) )
	{
		Switch8kPrgBank( Get8kPrgBankCount() - 2, 0 );
		Switch8kPrgBank( banks[6], 2 );
	}
	else
	{
		Switch8kPrgBank( banks[6], 0 );
		Switch8kPrgBank( Get8kPrgBankCount() - 2, 2 );
	}
/*
           0: Select 2 KB CHR bank at PPU $0000-$07FF (or $1000-$17FF);
           1: Select 2 KB CHR bank at PPU $0800-$0FFF (or $1800-$1FFF);
           2: Select 1 KB CHR bank at PPU $1000-$13FF (or $0000-$03FF);
           3: Select 1 KB CHR bank at PPU $1400-$17FF (or $0400-$07FF);
           4: Select 1 KB CHR bank at PPU $1800-$1BFF (or $0800-$0BFF);
           5: Select 1 KB CHR bank at PPU $1C00-$1FFF (or $0C00-$0FFF);
*/
	if ( IS_BIT_SET( bankSwapByte, 7 ) )
	{
		Switch1kChrBank( banks[2], 0 );
		Switch1kChrBank( banks[3], 1 );
		Switch1kChrBank( banks[4], 2 );
		Switch1kChrBank( banks[5], 3 );
		
		Switch1kChrBank( banks[0], 4 );
		Switch1kChrBank( banks[0] + 1, 5 );
		Switch1kChrBank( banks[1], 6 );
		Switch1kChrBank( banks[1] + 1, 7 );
	}
	else
	{
		Switch1kChrBank( banks[0], 0 );
		Switch1kChrBank( banks[0] + 1, 1 );
		Switch1kChrBank( banks[1], 2 );
		Switch1kChrBank( banks[1] + 1, 3 );

		Switch1kChrBank( banks[2], 4 );
		Switch1kChrBank( banks[3], 5 );
		Switch1kChrBank( banks[4], 6 );
		Switch1kChrBank( banks[5], 7 );
	}
}


void Mapper4::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	if ( IS_INT_BETWEEN( offset, 0x8000, 0xA000 ) )
	{
		if ( !IS_BIT_SET( offset, 0 ) )
		{ // even
			bankSwapByte = data;
			SyncBanks();
		}
		else
		{ // odd
			banks[ bankSwapByte & 0x7 ] = data;
			SyncBanks();
		}
	}
	else if ( IS_INT_BETWEEN( offset, 0xA000, 0xC000 ) )
	{
		if ( !IS_BIT_SET( offset, 0 ) )
		{ // even
			g_nesMainboard->GetSynchroniser()->Synchronise();
			g_nesMainboard->GetPPU()->ChangeMirroringMethod( IS_BIT_SET( data, 0 ) ? PPU_MIRROR_HORIZONTAL : PPU_MIRROR_VERTICAL );
		}
		else
		{ // odd
			prgRamDisableWrite = IS_BIT_SET( data, 6 );
			chipEnable = IS_BIT_SET( data, 7 );
		}
	}
	else if ( IS_INT_BETWEEN( offset, 0xC000, 0xE000 ) )
	{
		if ( !IS_BIT_SET( offset, 0 ) )
		{ // even
			irqLatch = data;
		}
		else
		{ // odd
			mReloadFlag = true;
		}
	}
	else if ( offset >= 0xE000 )
	{
		if ( !IS_BIT_SET( offset, 0 ) )
		{ // even - TODO: "Writing any value to this register will disable MMC3 interrupts AND acknowledge any pending interrupts."
			interruptsEnabled = false;
			g_nesMainboard->GetProcessor()->HoldIRQLineLow( false );

			#ifdef LOG_MAPPER_EVENTS
				if ( Log::IsTypeEnabled( LOG_MAPPER ) )
					Log::Write( LOG_MAPPER, ( boost::format( "Interrupts disabled on mapper" ) ).str() );
			#endif
		}
		else
		{ // odd
			interruptsEnabled = true;

			#ifdef LOG_MAPPER_EVENTS
				if ( Log::IsTypeEnabled( LOG_MAPPER ) )
					Log::Write( LOG_MAPPER, ( boost::format( "Interrupts enabled on mapper" ) ).str() );
			#endif
		}
	}
}


void Mapper4::Write8SRam( UInt16_t offset, Byte_t data )
{
	if ( chipEnable && !prgRamDisableWrite )
		IMapper::Write8SRam( offset, data );
}


Byte_t Mapper4::Read8SRam( UInt16_t offset )
{
	return chipEnable ? IMapper::Read8SRam( offset ) : 0;
}


void Mapper4::OnSaveState( std::ostream& ostr )
{
	WriteType< Byte_t >( ostr, bankSwapByte );
	WriteType< boost::uint8_t >( ostr, prgRamDisableWrite ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, chipEnable ? 1 : 0 );
	WriteType< boost::int32_t >( ostr, irqCounter );
	WriteType< boost::int32_t >( ostr, irqLatch );
	WriteType< boost::uint8_t >( ostr, interruptsEnabled ? 1 : 0 );

	for ( int i=0; i<8; ++i )
		WriteType< Byte_t >( ostr, banks[i] );

	WriteType< boost::int32_t >( ostr, masterTickCount );
}


void Mapper4::OnLoadState( std::istream& istr )
{
	bankSwapByte = ReadType< Byte_t >( istr );
	prgRamDisableWrite = ReadType< boost::uint8_t >( istr ) > 0;
	chipEnable = ReadType< boost::uint8_t >( istr ) > 0;
	irqCounter = ReadType< boost::int32_t >( istr );
	irqLatch = ReadType< boost::int32_t >( istr );
	interruptsEnabled = ReadType< boost::uint8_t >( istr ) > 0;

	for ( int i=0; i<8; ++i )
		banks[i] = ReadType< Byte_t >( istr );

	masterTickCount = ReadType< boost::int32_t >( istr );
}