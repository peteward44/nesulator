
#include "stdafx.h"
#include "mappers.h"
#include "mainboard.h"
#include "mapper4.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Mapper0 : public IMapper
{
protected:
public:
	Mapper0( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );

	virtual std::string GetName() const { return "NROM"; }

};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Nintendo MMC1
class Mapper1 : public IMapper
{
private:
	Byte_t val;
	int count;

	Byte_t registers[4];
	bool chrSwitch4k, lowPrgSwitch, prgSwitch16k;
	bool wRamEnabled;

	void SyncChrMirrors();
	void SyncPrgMirrors();

public:
	Mapper1( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual void OnSaveState( std::ostream& ostr );
	virtual void OnLoadState( std::istream& istr );

	virtual void Write8SRam( UInt16_t offset, Byte_t data );
	virtual Byte_t Read8SRam( UInt16_t offset );
	
	virtual void OnReset( bool cold );

	virtual std::string GetName() const { return "Nintendo MMC1"; }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// UNROM
class Mapper2 : public IMapper
{
private:

public:
	Mapper2( PPU_MIRRORING_METHOD mirroringMethod );
	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "UNROM"; }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// CNROM
class Mapper3 : public IMapper
{
private:

public:
	Mapper3( PPU_MIRRORING_METHOD mirroringMethod );
	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );
	virtual void Write8ChrRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "CNROM"; }
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



// MMC3
//
//const int mmc3IrqPixel = 257;
//
//class Mapper4 : public IMapper
//{
//private:
//	Byte_t bankSwapByte;
//	bool prgRamDisableWrite, chipEnable;
//
//	int irqCounter, irqLatch;
//	bool interruptsEnabled;
//	Byte_t banks[8];
//
//	int masterTickCount, lastA12Raise;
//
//	void Sync();
//
//	FORCE_INLINE void DecrementIrqCounter();
//	FORCE_INLINE void DoThisEveryScanline( int syncMTC, int pixel );
//
//public:
//	Mapper4( PPU_MIRRORING_METHOD mirroringMethod );
//
//	virtual void OnReset( bool cold );
//	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );
//
//	virtual void Write8SRam( UInt16_t offset, Byte_t data );
//	virtual Byte_t Read8SRam( UInt16_t offset );
//
//	virtual void Synchronise( int syncMTC );
//	virtual int GetNextEvent( int cpuMTC );
//	virtual void OnEndFrame();
//
//	virtual void OnSaveState( std::ostream& ostr );
//	virtual void OnLoadState( std::istream& istr );
//
//	virtual std::string GetName() const { return "Nintendo MMC3"; }
//
//	virtual void PPUA12Latch();
//
//	virtual bool HasKnownProblems() const { return true; } // not enough testing, IRQ timing problems, apparent on many games (see todo.txt)
//
//};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// iNes #6: FFE F4xxx - PRG/16K, VROM/8K, NT, IRQ

class Mapper6 : public IMapper
{
protected:

public:
	Mapper6( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "FFE F4xxx"; }

	virtual bool HasKnownProblems() const { return true; } // not tested enough
};



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// iNes #7: AOROM
// used by RARE games, battletoads, wizard & warriors

class Mapper7 : public IMapper
{
protected:

public:
	Mapper7( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "AOROM"; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// FFE F3xxx

class Mapper8 : public IMapper
{
protected:

public:
	Mapper8( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "FFE F3xxx"; }

	virtual bool HasKnownProblems() const { return true; } // not tested enough
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Color Dreams

class Mapper11 : public IMapper
{
protected:

public:
	Mapper11( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "Color Dreams"; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// CPROM: Used by Videomation (bitmap drawing program)

class Mapper13 : public IMapper
{
protected:

public:
	Mapper13( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "CPROM"; }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 100-in-1

class Mapper15 : public IMapper
{
public:
	Mapper15( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "X-in-1"; }

	virtual bool HasKnownProblems() const { return true; } // Pirate rom mapper
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// FFE F8xxx

class Mapper17 : public IMapper
{
public:
	Mapper17( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8EXRam( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "FFE F8xxx"; }

	virtual bool HasKnownProblems() const { return true; } // not tested enough
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Jaleco SS8806

class Mapper18 : public IMapper
{
private:
	Byte_t prgBits[ 3 ];
	Byte_t chrBits[ 8 ];

public:
	Mapper18( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "Jaleco SS8806"; }

	virtual bool HasKnownProblems() const { return true; } // not tested enough
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// iNes #34: Nina-1

class Mapper34 : public IMapper
{
protected:

public:
	Mapper34( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );

	virtual void Write8SRam( UInt16_t offset, Byte_t data );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "NINA-1"; }

	virtual bool HasKnownProblems() const { return true; } // not tested enough
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// GNROM

class Mapper66 : public IMapper
{
protected:

public:
	Mapper66( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "GNROM"; }

	virtual bool HasKnownProblems() const { return true; } // not tested enough
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Sunsoft FME-7

class Mapper69 : public IMapper
{
protected:
	int commandNum;

public:
	Mapper69( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "Sunsoft FME-7"; }

	virtual bool HasKnownProblems() const { return true; } // not tested enough / features missing
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Irem 74HC161/32

class Mapper78 : public IMapper
{
protected:

public:
	Mapper78( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual std::string GetName() const { return "Irem 74HC161/32"; }

	virtual bool HasKnownProblems() const { return true; } // not tested enough
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper0::Mapper0( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper0::OnReset( bool cold )
{
	if ( Get32kPrgBankCount() >= 1 )
	{
		Switch32kPrgBank( 0 );
	}
	else if ( Get16kPrgBankCount() == 1 )
	{
		Switch16kPrgBank( 0, true );
		Switch16kPrgBank( 0, false );
	}

	if ( Get1kChrBankCount() == 0 )
	{
		UseVRAM();
	}
	else
	{
		this->Switch8kChrBank( 0 );
	}

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper1::Mapper1( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
	memset( registers, 0, sizeof( Byte_t ) * 4 );
	registers[0] = 0x0C;
	chrSwitch4k = false;
	val = 0;
	count = 0;
	lowPrgSwitch = prgSwitch16k = true;
	wRamEnabled = true;
}


void Mapper1::OnReset( bool cold )
{
	Switch16kPrgBank( 0, true );
	Switch16kPrgBank( this->Get16kPrgBankCount() - 1, false );

	if ( Get8kChrBankCount() == 0 )
	{
		UseVRAM();
	}
	else
	{
		Switch8kChrBank( 0 );
	}

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper1::SyncChrMirrors()
{
	// re-sync mirrors
//	if ( this->Get4kChrBankCount() > 0 )
	{
		if ( chrSwitch4k )
		{
			Switch4kChrBank( registers[1] & 0x1F, true );
			Switch4kChrBank( registers[2] & 0x1F, false );
		}
		else
		{
			Switch8kChrBank( ( (registers[1] & 0x1F) >> 1 ) );
		}
	}
}


void Mapper1::SyncPrgMirrors()
{
	if ( prgSwitch16k )
	{
		if ( lowPrgSwitch )
		{
			Switch16kPrgBank( (( registers[3] & 0xF ) ), true );
		}
		else
		{
			Switch16kPrgBank( (( registers[3] & 0xF ) ), false );
		}
	}
	else
	{
		Switch32kPrgBank( ((registers[3] & 0xF) >> 1) );
	}
}


void Mapper1::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	if ( ( data & 0x80 ) > 0 )
	{
		val = 0;
		count = 0;
		lowPrgSwitch = prgSwitch16k = true;
		SyncPrgMirrors();
		return;
	}

	val |= (data & 0x01) << count++;

	if ( count >= 5 )
	{
		g_nesMainboard->GetSynchroniser()->Synchronise();

		if ( IS_INT_BETWEEN( offset, 0x8000, 0xA000 ) )
		{
			registers[0] = val;

			// bit 0 - Horizontal / vertical mirror switch
			switch ( val & 0x3 )
			{
			case 0:
				mirroringMethod = PPU_MIRROR_SINGLESCREEN_NT0;
				break;
			case 1:
				mirroringMethod = PPU_MIRROR_SINGLESCREEN_NT1;
				break;
			case 2:
				mirroringMethod = PPU_MIRROR_VERTICAL;
				break;
			case 3:
				mirroringMethod = PPU_MIRROR_HORIZONTAL;
				break;
			}

			// bit 2 - high or low prg switching
			lowPrgSwitch = ( val & 0x4 ) > 0;

			// bit 3 - 32kb / 16kb prg switching
			prgSwitch16k = ( val & 0x8 ) > 0;

			// bit 4 - switch 4kb/8kb chrrom switching mode
			chrSwitch4k = ( ( val & 0x10 ) > 0 );

			SyncChrMirrors();
			SyncPrgMirrors();

			g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
		}
		else if ( IS_INT_BETWEEN( offset, 0xA000, 0xC000 ) )
		{
			registers[1] = val;

			SyncChrMirrors();
		}
		else if ( IS_INT_BETWEEN( offset, 0xC000, 0xE000 ) )
		{
			registers[2] = val;

			SyncChrMirrors();
		}
		else if ( IS_INT_BETWEEN( offset, 0xE000, 0x10000 ) )
		{
			registers[3] = val;
			
			wRamEnabled = ( val & 0x10 ) == 0;
			SyncPrgMirrors();
		}

		count = 0;
		val = 0;
	}
}


void Mapper1::Write8SRam( UInt16_t offset, Byte_t data )
{
	if ( wRamEnabled )
	{
		IMapper::Write8SRam( offset, data );
	}
}


Byte_t Mapper1::Read8SRam( UInt16_t offset )
{
	if ( wRamEnabled )
		return IMapper::Read8SRam( offset );
	else
		return 0; // TODO: supposed to be open bus
}


void Mapper1::OnSaveState( std::ostream& ostr )
{
	WriteType< Byte_t >( ostr, val );
	WriteType< boost::uint32_t >( ostr, count );

	for ( int i=0; i<4; ++i )
		WriteType< Byte_t >( ostr, registers[i] );

	WriteType< boost::uint8_t >( ostr, chrSwitch4k ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, lowPrgSwitch ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, prgSwitch16k ? 1 : 0 );
}


void Mapper1::OnLoadState( std::istream& istr )
{
	val = ReadType< Byte_t >( istr );
	count = ReadType< boost::uint32_t >( istr );

	for ( int i=0; i<4; ++i )
		registers[i] = ReadType< Byte_t >( istr );

	chrSwitch4k = ReadType< boost::uint8_t >( istr ) > 0;
	lowPrgSwitch = ReadType< boost::uint8_t >( istr ) > 0;
	prgSwitch16k = ReadType< boost::uint8_t >( istr ) > 0;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper2::Mapper2( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper2::OnReset( bool cold )
{
	Switch16kPrgBank( 0, true );
	Switch16kPrgBank( this->Get16kPrgBankCount() - 1, false );

	UseVRAM();

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper2::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();

	Switch16kPrgBank( data, true );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper3::Mapper3( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper3::OnReset( bool cold )
{
	if ( Get16kPrgBankCount() == 1 )
	{
		Switch16kPrgBank( 0, true );
		Switch16kPrgBank( 0, false );
	}
	else //if ( Get32kPrgBankCount() == 1 )
	{
		Switch32kPrgBank( 0 );
	}

	Switch8kChrBank( 0 );

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper3::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();
	Switch8kChrBank( data & 0x3 );
}


void Mapper3::Write8ChrRom( UInt16_t offset, Byte_t data )
{
	// do nothing - CHR rom not writable
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
//Mapper4::Mapper4( PPU_MIRRORING_METHOD mm )
//{
//	this->mirroringMethod = mm;
//
//	bankSwapByte = 0;
//	prgRamDisableWrite = false;
//	chipEnable = interruptsEnabled = true;
//	irqCounter = irqLatch = 0;
//
//	masterTickCount = lastA12Raise = 0;
//
//	banks[6] = 0;
//	banks[7] = 1;
//
//	banks[0] = 0;
//	banks[1] = 2;
//	banks[2] = 4;
//	banks[3] = 5;
//	banks[4] = 6;
//	banks[5] = 7;
//}
//
//
//void Mapper4::OnReset( bool cold )
//{
//	if ( Get1kChrBankCount() == 0 )
//		UseVRAM();
//
//	irqCounter = irqLatch = 0;
//	masterTickCount = lastA12Raise = 0;
//
//	Sync();
//
//	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
//}
//
//
//void Mapper4::DecrementIrqCounter()
//{
//	if ( --irqCounter < 0 )
//	{
//		irqCounter = irqLatch;
//
//	#ifdef LOG_MAPPER_EVENTS
//		if ( Log::IsTypeEnabled( LOG_MAPPER ) )
//			Log::Write( LOG_MAPPER, ( boost::format( "IRQ counter reset and IRQ started: Latch: %1%" ) % irqLatch ).str() );
//	#endif
//
//		if ( interruptsEnabled )
//			g_nesMainboard->GetProcessor()->HoldIRQLineLow( true );
//	}
//
//	#ifdef LOG_MAPPER_EVENTS
//		if ( Log::IsTypeEnabled( LOG_MAPPER ) )
//			Log::Write( LOG_MAPPER, ( boost::format( "IRQ counter decemented: Counter: %1% Latch: %2%" ) % irqCounter % irqLatch ).str() );
//	#endif
//}
//
//
//void Mapper4::PPUA12Latch()
//{
//	//int ticks = g_nesMainboard->GetSynchroniser()->GetCpuMTC();
//	//if ( ticks - lastA12Raise > 8 * MASTER_CYCLES_PER_PPU )
//	//{
//	//	DecrementIrqCounter();
//	//}
//
//	//lastA12Raise = ticks;
//}
//
//
//void Mapper4::DoThisEveryScanline( int syncMTC, int pixel )
//{
//	// tickLimit is the start of the rendering frame - only started being clocked when rendering
//	int tickLimit = ( 1 + g_nesMainboard->GetModeConstants()->VBlankScanlines() ) * ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU );
//
//	int modmtc = masterTickCount % ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU ); // ticks from the start of the current scanline
//	int scanlineEvent = MASTER_CYCLES_PER_PPU * pixel; // ticks from the start of the scanline when IRQ is decremented
//
//	int startMtc = masterTickCount - modmtc + scanlineEvent; // ticks till next irq decrement event
//
//	if ( startMtc < masterTickCount )
//		startMtc += PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU; // if we have already passed the irq event, move onto next scanline
//
//	if ( startMtc < tickLimit )
//		startMtc = tickLimit + scanlineEvent;
//
//	for ( int mtc = startMtc; mtc < syncMTC; mtc += PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU )
//	{
//		DecrementIrqCounter();
//	}
//}
//
//
//void Mapper4::Synchronise( int syncMTC )
//{
//	/*
//The heart of the MMC3. The PPU will cause A12 to rise when it fetches CHR from the right pattern table ($1xxx).
//In "normal" conditions (BG uses $0xxx, all sprites use $1xxx), this will occur 8 times per scanline (once for each sprite).
//However the BG could also be the culprit (if BG uses $1xxx and all sprites use $0xxx -- ?as seen in Armadillo?), in which case A12 will rise 34 times.
//These 42 times per scanline are key times which I call "rise points":
//
//BG rise points: 4, 12, 20, ... , 244, 252
//Sp rise points: 260, 268, ..., 308, 316
//BG rise points: 324, 332 
//	*/
//
//	//if ( g_nesMainboard->GetPPU()->IsRenderingEnabled() )
//	//{
//	//	// tickLimit is the start of the rendering frame - only started being clocked when rendering
//	//	int tickLimit = ( g_mainboard->GetModeConstants()->VBlankScanlines() ) * ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU );
//
//	////	int modmtc = masterTickCount % ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU ); // ticks from the start of the current scanline
//
//	//	for ( int mtc = tickLimit; mtc < syncMTC; mtc += PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU )
//	//	{
//	//		// BG uses $1xxx
//	//		if ( g_mainboard->GetPPU()->ppuControl1.screenPatternTableAddress )
//	//		{
//	//			for ( int ticks = mtc + 4 * MASTER_CYCLES_PER_PPU; ticks <= mtc + 252 * MASTER_CYCLES_PER_PPU; ticks += 8 * MASTER_CYCLES_PER_PPU )
//	//			{
//	//				if ( ticks > masterTickCount && ticks <= syncMTC )
//	//				{
//	//			#ifdef LOG_MAPPER_EVENTS
//	//				if ( Log::IsTypeEnabled( LOG_MAPPER ) )
//	//					Log::Write( LOG_MAPPER, ( boost::format( "Background rise point: %1%" ) % ticks ).str() );
//	//			#endif
//
//	//					if ( ticks - lastA12Raise > 8 * MASTER_CYCLES_PER_PPU )
//	//					{
//	//						DecrementIrqCounter();
//	//					}
//
//	//					lastA12Raise = ticks;
//	//				}
//	//			}
//	//		}
//
//	//		// SP uses $0xxx
//	//		if ( g_mainboard->GetPPU()->ppuControl1.spritePatternTableAddress )
//	//		{
//	//			for ( int ticks = mtc + 260 * MASTER_CYCLES_PER_PPU; ticks <= mtc + 316 * MASTER_CYCLES_PER_PPU; ticks += 8 * MASTER_CYCLES_PER_PPU )
//	//			{
//	//				if ( ticks > masterTickCount && ticks <= syncMTC )
//	//				{
//	//			#ifdef LOG_MAPPER_EVENTS
//	//				if ( Log::IsTypeEnabled( LOG_MAPPER ) )
//	//					Log::Write( LOG_MAPPER, ( boost::format( "Sprite rise point: %1%" ) % ticks ).str() );
//	//			#endif
//
//	//					if ( ticks - lastA12Raise > 8 * MASTER_CYCLES_PER_PPU )
//	//					{
//	//						DecrementIrqCounter();
//	//					}
//
//	//					lastA12Raise = ticks;
//	//				}
//	//			}
//	//		}
//
//	//		// BG uses $1xxx (last 2 tile prefetches)
//	//		if ( g_mainboard->GetPPU()->ppuControl1.screenPatternTableAddress )
//	//		{
//	//			for ( int ticks = mtc + 324 * MASTER_CYCLES_PER_PPU; ticks <= mtc + 332 * MASTER_CYCLES_PER_PPU; ticks += 8 * MASTER_CYCLES_PER_PPU )
//	//			{
//	//				if ( ticks > masterTickCount && ticks <= syncMTC )
//	//				{
//
//	//			#ifdef LOG_MAPPER_EVENTS
//	//				if ( Log::IsTypeEnabled( LOG_MAPPER ) )
//	//					Log::Write( LOG_MAPPER, ( boost::format( "Background rise point (late): %1%" ) % ticks ).str() );
//	//			#endif
//
//	//					if ( ticks - lastA12Raise > 8 * MASTER_CYCLES_PER_PPU )
//	//					{
//	//						DecrementIrqCounter();
//	//					}
//
//	//					lastA12Raise = ticks;
//	//				}
//	//			}
//	//		}
//	//	}
//	//}
//
//	//masterTickCount = syncMTC;
//
///*
//If sprites are set to $1000-1FFF and the background is set to $0000-0FFF, then A12 will change from 0 to 1 at cycle 260 of each scanline, then change from 1 to 0 at cycle 320 of each scanline.
//
//If sprites are set to $0000-0FFF and the background is set to $1000-1FFF, then A12 will change from 1 to 0 at cycle 256 of each scanline, then change from 0 to 1 at cycle 324 of each scanline. 
//*/
//
//	if ( g_nesMainboard->GetPPU()->IsRenderingEnabled() )
//	{
//		if ( g_nesMainboard->GetPPU()->ppuControl1.spritePatternTableAddress && !g_nesMainboard->GetPPU()->ppuControl1.screenPatternTableAddress )
//		{
//			DoThisEveryScanline( syncMTC, 260 );
//		}
//		else if ( !g_nesMainboard->GetPPU()->ppuControl1.spritePatternTableAddress && g_nesMainboard->GetPPU()->ppuControl1.screenPatternTableAddress )
//		{
//			DoThisEveryScanline( syncMTC, 324 );
//		}
//	}
//
//	masterTickCount = syncMTC;
//}
//
//
//void Mapper4::OnEndFrame()
//{
//	masterTickCount = lastA12Raise = 0;
//}
//
//
//int Mapper4::GetNextEvent( int cpuMTC )
//{
//	// Attempt to predict the next time an IRQ will execute, given the current PPU status.
//	// This method will be called again if the PPU status changes (big sprites on, rendering disabled or write to 2006 for example)
//
//	// tickLimit is the start of the rendering frame - only started being clocked when rendering
//	int tickLimit = ( 1 + g_nesMainboard->GetModeConstants()->VBlankScanlines() ) * ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU );
//
//	int modmtc = cpuMTC % ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU ); // ticks from the start of the current scanline
//	int scanlineEvent = MASTER_CYCLES_PER_PPU * mmc3IrqPixel; // ticks from the start of the scanline when IRQ is decremented
//
//	int startMtc = cpuMTC - modmtc + scanlineEvent; // ticks till next irq decrement event
//
//	while ( startMtc <= cpuMTC )
//		startMtc += PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU; // if we have already passed the irq event, move onto next scanline
//
//	if ( startMtc < tickLimit )
//		startMtc = tickLimit + scanlineEvent;
//
//	return startMtc;
//
//	//return cpuMTC + 15;
//}
//
//
//void Mapper4::Sync()
//{
//	g_nesMainboard->GetSynchroniser()->Synchronise();
//
//	Switch8kPrgBank( banks[7], 1 );
//	Switch8kPrgBank( Get8kPrgBankCount() - 1, 3 );
//
//	if ( IS_BIT_SET( bankSwapByte, 6 ) )
//	{
//		Switch8kPrgBank( Get8kPrgBankCount() - 2, 0 );
//		Switch8kPrgBank( banks[6], 2 );
//	}
//	else
//	{
//		Switch8kPrgBank( banks[6], 0 );
//		Switch8kPrgBank( Get8kPrgBankCount() - 2, 2 );
//	}
///*
//           0: Select 2 KB CHR bank at PPU $0000-$07FF (or $1000-$17FF);
//           1: Select 2 KB CHR bank at PPU $0800-$0FFF (or $1800-$1FFF);
//           2: Select 1 KB CHR bank at PPU $1000-$13FF (or $0000-$03FF);
//           3: Select 1 KB CHR bank at PPU $1400-$17FF (or $0400-$07FF);
//           4: Select 1 KB CHR bank at PPU $1800-$1BFF (or $0800-$0BFF);
//           5: Select 1 KB CHR bank at PPU $1C00-$1FFF (or $0C00-$0FFF);
//*/
//	if ( IS_BIT_SET( bankSwapByte, 7 ) )
//	{
//		Switch1kChrBank( banks[2], 0 );
//		Switch1kChrBank( banks[3], 1 );
//		Switch1kChrBank( banks[4], 2 );
//		Switch1kChrBank( banks[5], 3 );
//		
//	//	Switch2kChrBank( banks[0], 2 );
//	//	Switch2kChrBank( banks[1], 3 );
//		Switch1kChrBank( banks[0], 4 );
//		Switch1kChrBank( banks[0] + 1, 5 );
//		Switch1kChrBank( banks[1], 6 );
//		Switch1kChrBank( banks[1] + 1, 7 );
//	}
//	else
//	{
//		Switch1kChrBank( banks[0], 0 );
//		Switch1kChrBank( banks[0] + 1, 1 );
//		Switch1kChrBank( banks[1], 2 );
//		Switch1kChrBank( banks[1] + 1, 3 );
//
//	//	Switch2kChrBank( banks[0], 0 );
//	//	Switch2kChrBank( banks[1], 1 );
//
//		Switch1kChrBank( banks[2], 4 );
//		Switch1kChrBank( banks[3], 5 );
//		Switch1kChrBank( banks[4], 6 );
//		Switch1kChrBank( banks[5], 7 );
//	}
//}
//
//
//void Mapper4::Write8PrgRom( UInt16_t offset, Byte_t data )
//{
//	if ( IS_INT_BETWEEN( offset, 0x8000, 0xA000 ) )
//	{
//		if ( !IS_BIT_SET( offset, 0 ) )
//		{ // even
//			bankSwapByte = data;
//		//	Sync();
//		}
//		else
//		{ // odd
//			banks[ bankSwapByte & 0x7 ] = data;
//			Sync();
//
//			//switch ( bankSwapByte & 0x7 )
//			//{
//			//case 0:
//			//	g_mainboard->GetPPU()->RunTo( g_mainboard->GetProcessor()->GetSubCycleMasterTickCount() );
//			//	Switch2kChrBank( data/2, IS_BIT_SET( bankSwapByte, 7 ) ? 2 : 0 );
//			//	break;
//			//case 1:
//			//	g_mainboard->GetPPU()->RunTo( g_mainboard->GetProcessor()->GetSubCycleMasterTickCount() );
//			//	Switch2kChrBank( data/2, IS_BIT_SET( bankSwapByte, 7 ) ? 3 : 1 );
//			//	break;
//			//case 2:
//			//	g_mainboard->GetPPU()->RunTo( g_mainboard->GetProcessor()->GetSubCycleMasterTickCount() );
//			//	Switch1kChrBank( data, IS_BIT_SET( bankSwapByte, 7 ) ? 0 : 4 );
//			//	break;
//			//case 3:
//			//	g_mainboard->GetPPU()->RunTo( g_mainboard->GetProcessor()->GetSubCycleMasterTickCount() );
//			//	Switch1kChrBank( data, IS_BIT_SET( bankSwapByte, 7 ) ? 1 : 5 );
//			//	break;
//			//case 4:
//			//	g_mainboard->GetPPU()->RunTo( g_mainboard->GetProcessor()->GetSubCycleMasterTickCount() );
//			//	Switch1kChrBank( data, IS_BIT_SET( bankSwapByte, 7 ) ? 2 : 6 );
//			//	break;
//			//case 5:
//			//	g_mainboard->GetPPU()->RunTo( g_mainboard->GetProcessor()->GetSubCycleMasterTickCount() );
//			//	Switch1kChrBank( data, IS_BIT_SET( bankSwapByte, 7 ) ? 3 : 7 );
//			//	break;
//			//case 6:
//			//	Switch8kPrgBank( data, IS_BIT_SET( bankSwapByte, 6 ) ? 2 : 0 );
//			//	break;
//			//case 7:
//			//	Switch8kPrgBank( data, 1 );
//			//	break;
//			//}
//		}
//	}
//	else if ( IS_INT_BETWEEN( offset, 0xA000, 0xC000 ) )
//	{
//		if ( !IS_BIT_SET( offset, 0 ) )
//		{ // even
//			g_nesMainboard->GetSynchroniser()->Synchronise();
//			g_nesMainboard->GetPPU()->ChangeMirroringMethod( IS_BIT_SET( data, 0 ) ? PPU_MIRROR_HORIZONTAL : PPU_MIRROR_VERTICAL );
//		}
//		else
//		{ // odd
//			prgRamDisableWrite = IS_BIT_SET( data, 6 );
//			chipEnable = IS_BIT_SET( data, 7 );
//		}
//	}
//	else if ( IS_INT_BETWEEN( offset, 0xC000, 0xE000 ) )
//	{
//		if ( !IS_BIT_SET( offset, 0 ) )
//		{ // even
//			irqCounter = data;
//
//			#ifdef LOG_MAPPER_EVENTS
//				if ( Log::IsTypeEnabled( LOG_MAPPER ) )
//					Log::Write( LOG_MAPPER, ( boost::format( "IRQ counter manually set: %1%" ) % irqCounter ).str() );
//			#endif
//		}
//		else
//		{ // odd
//			irqLatch = data;
//
//			#ifdef LOG_MAPPER_EVENTS
//				if ( Log::IsTypeEnabled( LOG_MAPPER ) )
//					Log::Write( LOG_MAPPER, ( boost::format( "IRQ latch manually set: Counter: %1% Latch: %2%" ) % irqCounter % irqLatch ).str() );
//			#endif
//		}
//	}
//	else if ( offset >= 0xE000 )
//	{
//		if ( !IS_BIT_SET( offset, 0 ) )
//		{ // even - TODO: "Writing any value to this register will disable MMC3 interrupts AND acknowledge any pending interrupts."
//			interruptsEnabled = false;
//			irqCounter = irqLatch;
//
//			#ifdef LOG_MAPPER_EVENTS
//				if ( Log::IsTypeEnabled( LOG_MAPPER ) )
//					Log::Write( LOG_MAPPER, ( boost::format( "Interrupts disabled on mapper" ) ).str() );
//			#endif
//		}
//		else
//		{ // odd
//			interruptsEnabled = true;
//
//			#ifdef LOG_MAPPER_EVENTS
//				if ( Log::IsTypeEnabled( LOG_MAPPER ) )
//					Log::Write( LOG_MAPPER, ( boost::format( "Interrupts enabled on mapper" ) ).str() );
//			#endif
//		}
//	}
//}
//
//
//void Mapper4::Write8SRam( UInt16_t offset, Byte_t data )
//{
//	if ( chipEnable && !prgRamDisableWrite )
//		IMapper::Write8SRam( offset, data );
//}
//
//
//Byte_t Mapper4::Read8SRam( UInt16_t offset )
//{
//	return chipEnable ? IMapper::Read8SRam( offset ) : 0;
//}
//
//
//void Mapper4::OnSaveState( std::ostream& ostr )
//{
//	WriteType< Byte_t >( ostr, bankSwapByte );
//	WriteType< boost::uint8_t >( ostr, prgRamDisableWrite ? 1 : 0 );
//	WriteType< boost::uint8_t >( ostr, chipEnable ? 1 : 0 );
//	WriteType< boost::int32_t >( ostr, irqCounter );
//	WriteType< boost::int32_t >( ostr, irqLatch );
//	WriteType< boost::uint8_t >( ostr, interruptsEnabled ? 1 : 0 );
//
//	for ( int i=0; i<8; ++i )
//		WriteType< Byte_t >( ostr, banks[i] );
//
//	WriteType< boost::int32_t >( ostr, masterTickCount );
//}
//
//
//void Mapper4::OnLoadState( std::istream& istr )
//{
//	bankSwapByte = ReadType< Byte_t >( istr );
//	prgRamDisableWrite = ReadType< boost::uint8_t >( istr ) > 0;
//	chipEnable = ReadType< boost::uint8_t >( istr ) > 0;
//	irqCounter = ReadType< boost::int32_t >( istr );
//	irqLatch = ReadType< boost::int32_t >( istr );
//	interruptsEnabled = ReadType< boost::uint8_t >( istr ) > 0;
//
//	for ( int i=0; i<8; ++i )
//		banks[i] = ReadType< Byte_t >( istr );
//
//	masterTickCount = ReadType< boost::int32_t >( istr );
//}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper7::Mapper7( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = PPU_MIRROR_SINGLESCREEN_NT0;
}


void Mapper7::OnReset( bool cold )
{
	Switch32kPrgBank( 0 );

//	if ( Get8kChrBankCount() == 0 )
		UseVRAM();
//	else
//		Switch8kChrBank( 0 );

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( PPU_MIRROR_SINGLESCREEN_NT0 );
}


void Mapper7::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();

	Switch32kPrgBank( data & 0xF );
	g_nesMainboard->GetPPU()->ChangeMirroringMethod( ( data & 0x10 ) > 0 ? PPU_MIRROR_SINGLESCREEN_NT1 : PPU_MIRROR_SINGLESCREEN_NT0 );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper6::Mapper6( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper6::OnReset( bool cold )
{
	Switch16kPrgBank( 0, true );
	Switch16kPrgBank( 7, false );
	Switch8kChrBank( 0 );

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper6::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();

	Switch8kChrBank( data & 0x3 );
	Switch16kPrgBank( ( data & 0x3C ) >> 2, true );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper8::Mapper8( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper8::OnReset( bool cold )
{
	Switch16kPrgBank( 0, true );
	Switch16kPrgBank( 1, false );
	Switch8kChrBank( 0 );

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper8::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();

	Switch8kChrBank( data & 0x7 );
	Switch16kPrgBank( ( data & ~0x7 ) >> 3, true );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper11::Mapper11( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper11::OnReset( bool cold )
{
	Switch32kPrgBank( 0 );
	Switch8kChrBank( 0 );

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper11::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();

	Switch32kPrgBank( data & 0xF );
	Switch8kChrBank( ( ( data & 0xF0 ) >> 4 ) );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper13::Mapper13( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper13::OnReset( bool cold )
{
	Switch32kPrgBank( 0 );
	UseVRAM( 16 );

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper13::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();

	Switch4kChrBank( data, false );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper15::Mapper15( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper15::OnReset( bool cold )
{
	Switch32kPrgBank( 0 );
	UseVRAM();

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper15::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();
	g_nesMainboard->GetPPU()->ChangeMirroringMethod( ( data & 0x40 ) > 0 ? PPU_MIRROR_HORIZONTAL : PPU_MIRROR_VERTICAL );

	bool upper8k = ( data & 0x80 ) > 0;
	int prgBank = data & 0x3F;

	switch ( offset & 0x3 )
	{
	case 2: // 8k mode ( 1 8k bank mirrored )
		for ( int i=0; i<4; ++i )
			Switch8kPrgBank( prgBank * 2 + ( upper8k ? 1 : 0 ), i );
		break;
	case 3: // 16k mode ( 1 16k bank mirrored )
		Switch16kPrgBank( prgBank, true );
		Switch16kPrgBank( prgBank, false );
		break;
	case 0: // 32k mode ( 1 32k bank, or if odd number, same as 16k mode )
		Switch16kPrgBank( prgBank, true );
		Switch16kPrgBank( prgBank | 1, false );
		break;
	case 1: // 128k mode ( 1 16k bank, last 16k bank fixed at 0xC00 )
		Switch16kPrgBank( prgBank, true );
		Switch16kPrgBank( Get16kPrgBankCount() - 1, false );
		break;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper17::Mapper17( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper17::OnReset( bool cold )
{
	Switch32kPrgBank( 0 );
	Switch8kChrBank( 0 );

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper17::Write8EXRam( UInt16_t offset, Byte_t data )
{
	switch ( offset )
	{
	case 0x4504:
		Switch8kPrgBank( data, 0 );
		break;
	case 0x4505:
		Switch8kPrgBank( data, 1 );
		break;
	case 0x4506:
		Switch8kPrgBank( data, 2 );
		break;
	case 0x4507:
		Switch8kPrgBank( data, 3 );
		break;
	case 0x4510:
		g_nesMainboard->GetSynchroniser()->Synchronise();
		Switch1kChrBank( data, 0 );
		break;
	case 0x4511:
		g_nesMainboard->GetSynchroniser()->Synchronise();
		Switch1kChrBank( data, 1 );
		break;
	case 0x4512:
		g_nesMainboard->GetSynchroniser()->Synchronise();
		Switch1kChrBank( data, 2 );
		break;
	case 0x4513:
		g_nesMainboard->GetSynchroniser()->Synchronise();
		Switch1kChrBank( data, 3 );
		break;
	case 0x4514:
		g_nesMainboard->GetSynchroniser()->Synchronise();
		Switch1kChrBank( data, 4 );
		break;
	case 0x4515:
		g_nesMainboard->GetSynchroniser()->Synchronise();
		Switch1kChrBank( data, 5 );
		break;
	case 0x4516:
		g_nesMainboard->GetSynchroniser()->Synchronise();
		Switch1kChrBank( data, 6 );
		break;
	case 0x4517:
		g_nesMainboard->GetSynchroniser()->Synchronise();
		Switch1kChrBank( data, 7 );
		break;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//Mapper18::Mapper18( PPU_MIRRORING_METHOD mm )
//{
//	memset( prgBits, 0, 3 );
//	memset( chrBits, 0, 8 );
//	this->mirroringMethod = mm;
//}
//
//
//void Mapper18::OnTurnOn()
//{
//	Switch16kPrgBank( 0, true );
//	Switch16kPrgBank( this->Get16kPrgBankCount() - 1, false );
//}
//
//
//void Mapper18::OnWrite8( UInt16_t offset, Byte_t data )
//{
//	g_nesMainboard->GetPPU()->RunTo( g_mainboard->GetProcessor()->GetSubCycleMasterTickCount() );
//
//	switch ( offset )
//	{
//	case 0x8000:
//		prgBits[0] |= data & 0xF;
//		break;
//	case 0x8001:
//		prgBits[0] |= ( data & 0xF ) << 4;
//		break;
//	case 0x8002:
//		prgBits[1] |= data & 0xF;
//		break;
//	case 0x8003:
//		prgBits[1] |= ( data & 0xF ) << 4;
//		break;
//	case 0x9000:
//		prgBits[2] |= data & 0xF;
//		break;
//	case 0x9001:
//		prgBits[2] |= ( data & 0xF ) << 4;
//		break;
//	case 0x9002:
//		prgBits[3] |= data & 0xF;
//		break;
//	case 0x9003:
//		prgBits[3] |= ( data & 0xF ) << 4;
//		break;
//	//case 0xA000:
//	//	chrBits[0] |= data & 0xF;
//	//	break;
//	//case 0xA001:
//	//	chrBits[0] |= ( data & 0xF ) << 4;
//	//	break;
//	//case 0xA000:
//	//	chrBits[1] |= data & 0xF;
//	//	break;
//	//case 0xA001:
//	//	chrBits[1] |= ( data & 0xF ) << 4;
//	//	break;
//	//case 0xA000:
//	//	chrBits[2] |= data & 0xF;
//	//	break;
//	//case 0xA001:
//	//	chrBits[2] |= ( data & 0xF ) << 4;
//	//	break;
//	//case 0xA000:
//	//	chrBits[3] |= data & 0xF;
//	//	break;
//	//case 0xA001:
//	//	chrBits[3] |= ( data & 0xF ) << 4;
//	//	break;
//	//case 0xA000:
//	//	chrBits[4] |= data & 0xF;
//	//	break;
//	//case 0xA001:
//	//	chrBits[4] |= ( data & 0xF ) << 4;
//	//	break;
//	//case 0xA000:
//	//	chrBits[5] |= data & 0xF;
//	//	break;
//	//case 0xA001:
//	//	chrBits[5] |= ( data & 0xF ) << 4;
//	//	break;
//	//case 0xA000:
//	//	chrBits[6] |= data & 0xF;
//	//	break;
//	//case 0xA001:
//	//	chrBits[6] |= ( data & 0xF ) << 4;
//	//	break;
//	//case 0xA000:
//	//	chrBits[7] |= data & 0xF;
//	//	break;
//	//case 0xA001:
//	//	chrBits[7] |= ( data & 0xF ) << 4;
//	//	break;
//	}
//}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper34::Mapper34( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper34::OnReset( bool cold )
{
	Switch32kPrgBank( 0 );
	if ( Get8kChrBankCount() == 0 )
	{
		UseVRAM();
	}
	else
	{
		Switch8kChrBank( 0 );
	}

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper34::Write8SRam( UInt16_t offset, Byte_t data )
{
	if ( offset == 0x7FFE )
	{
		g_nesMainboard->GetSynchroniser()->Synchronise();
		Switch4kChrBank( data, true );
	}
	else if ( offset == 0x7FFF )
	{
		g_nesMainboard->GetSynchroniser()->Synchronise();
		Switch4kChrBank( data, false );
	}
	else if ( offset == 0x7FFD )
	{
		Switch32kPrgBank( data );
	}
}


void Mapper34::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	Switch32kPrgBank( data );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper66::Mapper66( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper66::OnReset( bool cold )
{
	Switch32kPrgBank( 0 );
	Switch8kChrBank( 0 );

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper66::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();

	Switch8kChrBank( data & 0x3 );
	Switch32kPrgBank( ( data & 0x30 ) >> 4 );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper69::Mapper69( PPU_MIRRORING_METHOD mm )
{
	commandNum = 0;
	this->mirroringMethod = mm;
}


void Mapper69::OnReset( bool cold )
{
	Switch8kPrgBank( 0, 0 );
	Switch8kPrgBank( 1, 1 );
	Switch8kPrgBank( 2, 2 );
	Switch8kPrgBank( Get8kPrgBankCount() - 1, 3 );

	Switch8kChrBank( 0 );

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper69::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();
	switch ( offset )
	{
	case 0x8000:
		commandNum = data & 0xF;
		break;
	case 0xA000:
		if ( commandNum < 8 )
		{
			Switch1kChrBank( data, commandNum );
		}
		else if ( commandNum == 8 )
		{ // 0x6000 - 0x7FFF switch
			if ( !IS_BIT_SET( data, 6 ) )
			{ // 8k ROM (from prg, i assume?)
		//		Switch8kPrgBank( data & 0x1F, 0 );
			}
			else
			{
				if ( IS_BIT_SET( data, 7 ) )
				{ // 8k SRAM
				}
				else
				{ // 8k "psuedo-random numbers" ????
				}
			}
		}
		else if ( IS_INT_BETWEEN( commandNum, 9, 12 ) )
		{
			Switch8kPrgBank( data, commandNum - 9 );
		}
		else if ( commandNum == 12 )
		{
			switch ( data & 0x3 )
			{
			case 0:
				g_nesMainboard->GetPPU()->ChangeMirroringMethod( PPU_MIRROR_VERTICAL );
				break;
			case 1:
				g_nesMainboard->GetPPU()->ChangeMirroringMethod( PPU_MIRROR_HORIZONTAL );
				break;
			case 2:
				g_nesMainboard->GetPPU()->ChangeMirroringMethod( PPU_MIRROR_SINGLESCREEN_NT0 );
				break;
			case 3:
				g_nesMainboard->GetPPU()->ChangeMirroringMethod( PPU_MIRROR_SINGLESCREEN_NT1 );
				break;
			}
		}
		else if ( commandNum == 13 )
		{ // IRQ control
		}
		else if ( commandNum == 14 )
		{ // Low byte of scanline counter
		}
		else if ( commandNum == 15 )
		{ // High byte of scanline counter
		}

		break;
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


Mapper78::Mapper78( PPU_MIRRORING_METHOD mm )
{
	this->mirroringMethod = mm;
}


void Mapper78::OnReset( bool cold )
{
	Switch16kPrgBank( 0, true );
	Switch16kPrgBank( this->Get16kPrgBankCount() - 1, false );

	//     "   - The first 8K VROM bank may or may not be swapped into $0000 when
    //       the cart is reset. I have no ROM images to test."
	Switch8kChrBank( 0 );

	g_nesMainboard->GetPPU()->ChangeMirroringMethod( mirroringMethod );
}


void Mapper78::Write8PrgRom( UInt16_t offset, Byte_t data )
{
	g_nesMainboard->GetSynchroniser()->Synchronise();

	Switch16kPrgBank( data & 0xF, true );
	Switch8kChrBank( ( data & 0xF0 ) >> 4 );
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// factory

/* Estimated number of games with mapper (other mappers had <10 games)
Mapper 004: 569
Mapper 001: 481
Mapper 000: 260
Mapper 002: 200
Mapper 003: 145
Mapper 007: 56
Mapper 011: 35
Mapper 019: 32
Mapper 016: 26
Mapper 099: 25
Mapper 005: 24
Mapper 018: 16
Mapper 066: 16
Mapper 033: 15
Mapper 079: 15
Mapper 045: 14
Mapper 071: 14
Mapper 113: 12
Mapper 245: 11
Mapper 023: 11
Mapper 069: 11 
*/

MemoryMapperPtr_t CreateMemoryMapper( int mapperid, PPU_MIRRORING_METHOD mirroringMethod )
{
	switch ( mapperid )
	{
	case 0: // no mapper
		return MemoryMapperPtr_t( new Mapper0( mirroringMethod ) );
	case 1: // Nintendo MMC1
		return MemoryMapperPtr_t( new Mapper1( mirroringMethod ) );
	case 2:
		return MemoryMapperPtr_t( new Mapper2( mirroringMethod ) );
	case 3:
		return MemoryMapperPtr_t( new Mapper3( mirroringMethod ) );
	case 4:
		return MemoryMapperPtr_t( new Mapper4( mirroringMethod ) );
	case 6:
		return MemoryMapperPtr_t( new Mapper6( mirroringMethod ) );
	case 7:
		return MemoryMapperPtr_t( new Mapper7( mirroringMethod ) );
//	case 8:
//		return MemoryMapperPtr_t( new Mapper8( mirroringMethod ) );
	case 11:
		return MemoryMapperPtr_t( new Mapper11( mirroringMethod ) );
	case 13:
		return MemoryMapperPtr_t( new Mapper13( mirroringMethod ) );
	case 15:
		return MemoryMapperPtr_t( new Mapper15( mirroringMethod ) );
//	case 17:
//		return MemoryMapperPtr_t( new Mapper17( mirroringMethod ) );
	//case 18:
	//	return MemoryMapperPtr_t( new Mapper18( mirroringMethod ) );
	case 34:
		return MemoryMapperPtr_t( new Mapper34( mirroringMethod ) );
	case 66:
		return MemoryMapperPtr_t( new Mapper66( mirroringMethod ) );
//	case 69:
//		return MemoryMapperPtr_t( new Mapper69( mirroringMethod ) );
	case 78:
		return MemoryMapperPtr_t( new Mapper78( mirroringMethod ) );
	default:
		throw std::runtime_error( ( boost::format( "Unsupported memory mapper type: %1%" ) % mapperid ).str().c_str() );
	}

	return MemoryMapperPtr_t( );
}


IMapper::IMapper()
{
	mirroringMethod = PPU_MIRROR_HORIZONTAL;
	memset( prgPagesIds, 0, 4 * sizeof( int ) );
	memset( patternTablesIds, 0, 8 * sizeof( int ) );

	resetConnection = g_nesMainboard->ResetEvent.connect( boost::bind( &IMapper::OnReset, this, _1 ) );
	g_nesMainboard->GetSynchroniser()->RegisterListener( SYNC_MAPPER, this );
}


IMapper::~IMapper()
{
	resetConnection.disconnect();
}


void IMapper::SaveState( std::ostream& ostr )
{
	WriteType< boost::uint32_t >( ostr, static_cast< boost::uint32_t >( patternpages.size() ) );
	for ( unsigned int i=0; i<patternpages.size(); ++i )
		patternpages[i]->SaveState( ostr );

	for ( int i=0; i<4; ++i )
		WriteType< boost::int32_t >( ostr, prgPagesIds[ i ] );

	for ( int i=0; i<8; ++i )
		WriteType< boost::int32_t >( ostr, patternTablesIds[ i ] );

	sram.SaveState( ostr );
	expansionRam.SaveState( ostr );
	WriteType< boost::uint32_t >( ostr, mirroringMethod );

	OnSaveState( ostr );
}


void IMapper::LoadState( std::istream& istr )
{
	boost::uint32_t patternTableCount = ReadType< boost::uint32_t >( istr );
	patternpages.reserve( patternTableCount );
	for ( unsigned int i=0; i<patternTableCount; ++i )
	{
		PatternTablePtr_t patterntable = PatternTablePtr_t( new PatternTable_t() );
		patterntable->LoadState( istr );
		patternpages.push_back( patterntable );
	}

	for ( int i=0; i<4; ++i )
		this->SetPrgPage( ReadType< boost::int32_t >( istr ), i );

	for ( int i=0; i<8; ++i )
		this->SetChrPage( ReadType< boost::int32_t >( istr ), i );

	sram.LoadState( istr );
	expansionRam.LoadState( istr );
	mirroringMethod = (PPU_MIRRORING_METHOD)ReadType< boost::uint32_t >( istr );

	OnLoadState( istr );
}


void IMapper::Switch8kPrgBank( int id, int pos )
{
	id %= this->prgpages.size();

#ifdef LOG_MAPPER_EVENTS
	if ( Log::IsTypeEnabled( LOG_MAPPER ) )
		Log::Write( LOG_MAPPER, ( boost::format( "PRG 8K Switch %1% %2%" ) % pos % id ).str() );
#endif

	this->SetPrgPage( id, pos );
}


void IMapper::Switch16kPrgBank( int id, bool low )
{
	if ( Get16kPrgBankCount() > 0 )
	{
		id = ( id * 2 ) % static_cast<int>( prgpages.size() );

#ifdef LOG_MAPPER_EVENTS
	if ( Log::IsTypeEnabled( LOG_MAPPER ) )
		Log::Write( LOG_MAPPER, ( boost::format( "PRG 16K Switch %1% %2%" ) % (low ? 0 : 2) % id ).str() );
#endif

		for ( int i=0; i<2; ++i )
			this->SetPrgPage( id + i, i + (low ? 0 : 2) );
	}
}


void IMapper::Switch32kPrgBank( int id )
{
	if ( Get32kPrgBankCount() > 0 )
	{
		id = ( id * 4 ) % static_cast<int>( prgpages.size() );

#ifdef LOG_MAPPER_EVENTS
	if ( Log::IsTypeEnabled( LOG_MAPPER ) )
		Log::Write( LOG_MAPPER, ( boost::format( "PRG 32K Switch %1%" ) % id ).str() );
#endif

		for ( int i=0; i<4; ++i )
			this->SetPrgPage( id + i, i );
	}
}


void IMapper::Switch1kChrBank( int id, int pos )
{
	id %= patternpages.size();

#ifdef LOG_MAPPER_EVENTS
	if ( Log::IsTypeEnabled( LOG_MAPPER ) )
		Log::Write( LOG_MAPPER, ( boost::format( "CHR 1K Switch %1% %2%" ) % pos % id ).str() );
#endif

	this->SetChrPage( id, pos );
}


void IMapper::Switch2kChrBank( int id, int pos )
{
	if ( Get2kChrBankCount() > 0 )
	{
		id = ( id * 2 ) % static_cast<int>( patternpages.size() );

#ifdef LOG_MAPPER_EVENTS
	if ( Log::IsTypeEnabled( LOG_MAPPER ) )
		Log::Write( LOG_MAPPER, ( boost::format( "CHR 2K Switch %1% %2%" ) % pos % id ).str() );
#endif

		for ( int i=0; i<2; ++i )
			this->SetChrPage( id + i, ( pos * 2 ) + i );
	}
}


void IMapper::Switch4kChrBank( int id, bool low )
{
	if ( Get4kChrBankCount() > 0 )
	{
		id = ( id * 4 ) % static_cast<int>( patternpages.size() );

#ifdef LOG_MAPPER_EVENTS
	if ( Log::IsTypeEnabled( LOG_MAPPER ) )
		Log::Write( LOG_MAPPER, ( boost::format( "CHR 4K Switch %1% %2%" ) % (low ? 0 : 4) % id ).str() );
#endif

		for ( int i=0; i<4; ++i )
			this->SetChrPage( id + i, i + (low ? 0 : 4) );
	}
}


void IMapper::Switch8kChrBank( int id )
{
	if ( Get8kChrBankCount() > 0 )
	{
		id = ( id * 8 ) % static_cast<int>( patternpages.size() );

#ifdef LOG_MAPPER_EVENTS
	if ( Log::IsTypeEnabled( LOG_MAPPER ) )
		Log::Write( LOG_MAPPER, ( boost::format( "CHR 8K Switch %1%" ) % id ).str() );
#endif

		for ( int i=0; i<8; ++i )
			this->SetChrPage( id + i, i );
	}
}


void IMapper::UseVRAM( int numbanks )
{
	patternpages.clear();

#ifdef LOG_MAPPER_EVENTS
	if ( Log::IsTypeEnabled( LOG_MAPPER ) )
		Log::Write( LOG_MAPPER, ( boost::format( "This cartridge has its own VRAM consisting of %1%KB of memory" ) % numbanks ).str() );
#endif

	for ( int i=0; i<numbanks; ++i )
	{
		PatternTablePtr_t pt = PatternTablePtr_t( new PatternTable_t() );
		patternpages.push_back( pt );

		if ( i < 8 )
			this->SetChrPage( i, i );
	}
}


// IMapper overridable methods

void IMapper::Write8EXRam( UInt16_t offset, Byte_t data )
{
	expansionRam.Data[ offset - 0x4018 ] = data;
}


Byte_t IMapper::Read8EXRam( UInt16_t offset )
{
	return expansionRam.Data[ offset - 0x4018 ];
}

// read and writes to 0x6000 -> 0x7FFF
void IMapper::Write8SRam( UInt16_t offset, Byte_t data )
{
	sram.Data[ offset - 0x6000 ] = data;
}

Byte_t IMapper::Read8SRam( UInt16_t offset )
{
	return sram.Data[ offset - 0x6000 ];
}

// 0x8000 -> 0xFFFF
void IMapper::Write8PrgRom( UInt16_t offset, Byte_t data )
{
}

Byte_t IMapper::Read8PrgRom( UInt16_t offset )
{
	int pagenum = ( offset - 0x8000 ) / 0x2000;
	int aid = offset % 0x2000;
	return prgPagePtrs[ pagenum ]->Data[ aid ];
}

// VRAM 0x0000 -> 0x2000
void IMapper::Write8ChrRom( UInt16_t offset, Byte_t data )
{
//	Log::Write( LOG_MISC, (boost::format( "Write to CHR Rom: 0x%1$X Data: %2$X" ) % offset % (int)data ).str() );

	int id = offset / 0x400;
	assert( patternTables[ id ].get() != 0 );
  patternTables[ id ]->Data[ offset % 0x400 ] = data;
}

Byte_t IMapper::Read8ChrRom( UInt16_t offset, bool dataOnly )
{
	int id = offset / 0x400;
	assert( patternTables[ id ].get() != 0 );
	return patternTables[ id ]->Data[ offset % 0x400 ];
}


void IMapper::Synchronise( int syncMTC )
{

}


int IMapper::GetNextEvent( int cpuMTC )
{
	return -1;
}


void IMapper::OnEndFrame( )
{

}
